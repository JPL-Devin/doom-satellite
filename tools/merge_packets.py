#!/usr/bin/env python3
"""Stand-in merged telemetry packet list for DoomFlight.

DoomFlight's TlmPacketizer downlinks telemetry produced by both DoomFlight and DoomCoprocessor (received over the
GenericHub). FPP packet sets can only name channels of the deployment that defines them, so this tool:

1. merges the DoomFlight and DoomCoprocessor dictionaries with fprime-merge-dictionary,
2. combines both deployments' telemetry packet sets into a single packet set,
3. writes a merged GDS dictionary carrying that packet set, and
4. writes the C++ packet list (Svc::TlmPacketizerPacketList) DoomFlight hands to its TlmPacketizer.

Channel sizes are computed from the type definitions in the dictionary of the deployment producing the channel. The
output is deterministic: packets are ordered by id and channels keep their packet-set order.
"""
import argparse
import json
import re
import subprocess
import sys
import tempfile
from pathlib import Path

PROJECT_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_FLIGHT = (
    PROJECT_ROOT / "build-artifacts/zephyr/fprime-zephyr-deployment/dict/DoomFlightTopologyDictionary.json"
)
DEFAULT_COPROCESSOR = (
    PROJECT_ROOT
    / "build-artifacts/aarch64-linux/DoomSatellite_DoomCoprocessor/dict/DoomCoprocessorTopologyDictionary.json"
)
DEFAULT_DICTIONARY_OUT = PROJECT_ROOT / "build-artifacts/merged/DoomSatelliteTopologyDictionary.json"
DEFAULT_CPP_OUT = PROJECT_ROOT / "DoomSatellite/DoomFlight/Top/DoomSatelliteMergedPackets"
MERGED_PACKET_SET = "DoomSatellitePackets"
NAMESPACE = "DoomSatelliteMergedPackets"

PRIMITIVE_SIZES = {"bool": 1}


class TypeSizer:
    """Computes serialized sizes of dictionary types"""

    def __init__(self, dictionary):
        self.definitions = {t["qualifiedName"]: t for t in dictionary["typeDefinitions"]}
        self.size_store = self.size_of(self.definitions["FwSizeStoreType"]["underlyingType"])

    def size_of(self, type_ref):
        kind = type_ref["kind"]
        if kind in ("integer", "float"):
            return type_ref["size"] // 8
        if kind == "bool":
            return PRIMITIVE_SIZES["bool"]
        if kind == "string":
            return type_ref["size"] + self.size_store
        if kind == "qualifiedIdentifier":
            return self.size_of_definition(self.definitions[type_ref["name"]])
        raise ValueError(f"Unsupported type kind '{kind}' in {type_ref}")

    def size_of_definition(self, definition):
        kind = definition["kind"]
        if kind == "alias":
            return self.size_of(definition["underlyingType"])
        if kind == "enum":
            return self.size_of(definition["representationType"])
        if kind == "array":
            return definition["size"] * self.size_of(definition["elementType"])
        if kind == "struct":
            return sum(
                member.get("size", 1) * self.size_of(member["type"]) for member in definition["members"].values()
            )
        raise ValueError(f"Unsupported type definition kind '{kind}' for {definition['qualifiedName']}")


def channel_table(dictionary):
    """Map channel name to (id, serialized size) using the producing deployment's own types"""
    sizer = TypeSizer(dictionary)
    return {
        channel["name"]: (channel["id"], sizer.size_of(channel["type"])) for channel in dictionary["telemetryChannels"]
    }


def merge_dictionaries(flight_path, coprocessor_path, merge_tool):
    """Merge the two dictionaries with fprime-merge-dictionary and return the merged JSON"""
    with tempfile.TemporaryDirectory() as temporary:
        output = Path(temporary) / "merged.json"
        command = [
            merge_tool,
            "--name",
            "DoomSatellite",
            "--permissive",
            "--no-namespace",
            "--output",
            str(output),
            str(flight_path),
            str(coprocessor_path),
        ]
        result = subprocess.run(command, capture_output=True, text=True)
        if result.returncode != 0 or not output.exists():
            sys.stderr.write(result.stdout + result.stderr)
            raise SystemExit(f"fprime-merge-dictionary failed ({result.returncode})")
        return json.loads(output.read_text())


def merge_packets(dictionaries):
    """Combine the packet sets of all dictionaries into one list of packets with channel ids and sizes"""
    packets = []
    for dictionary in dictionaries:
        channels = channel_table(dictionary)
        for packet_set in dictionary.get("telemetryPacketSets", []):
            for packet in packet_set["members"]:
                entries = []
                for name in packet["members"]:
                    if name not in channels:
                        raise SystemExit(f"Packet {packet['name']} names unknown channel {name}")
                    entries.append((name, *channels[name]))
                packets.append({"name": packet["name"], "id": packet["id"], "group": packet["group"], "channels": entries})
    packets.sort(key=lambda packet: packet["id"])

    for previous, current in zip(packets, packets[1:]):
        if previous["id"] == current["id"]:
            raise SystemExit(f"Packet id {current['id']:#x} used by both {previous['name']} and {current['name']}")
    names = [packet["name"] for packet in packets]
    duplicates = sorted({name for name in names if names.count(name) > 1})
    if duplicates:
        raise SystemExit(f"Duplicate packet names: {', '.join(duplicates)}")

    sizes = {}
    for packet in packets:
        for name, channel_id, size in packet["channels"]:
            if sizes.setdefault(channel_id, (name, size)) != (name, size):
                raise SystemExit(f"Channel id {channel_id:#x} has conflicting definitions: {sizes[channel_id]} {name}")
    return packets, len(sizes)


def merged_packet_set(dictionaries, packets):
    omitted = sorted({name for d in dictionaries for s in d.get("telemetryPacketSets", []) for name in s["omitted"]})
    return {
        "name": MERGED_PACKET_SET,
        "members": [
            {
                "name": packet["name"],
                "id": packet["id"],
                "group": packet["group"],
                "members": [name for name, _, _ in packet["channels"]],
            }
            for packet in packets
        ],
        "omitted": omitted,
    }


def identifier(name):
    return re.sub(r"[^A-Za-z0-9_]", "_", name)


HEADER = "// Generated by tools/merge_packets.py from the DoomFlight and DoomCoprocessor dictionaries. Do not edit.\n"


def render_hpp(packets, num_channels):
    return (
        HEADER
        + f"""
#ifndef DOOMSATELLITE_DOOMSATELLITEMERGEDPACKETS_HPP
#define DOOMSATELLITE_DOOMSATELLITEMERGEDPACKETS_HPP

#include "Svc/TlmPacketizer/TlmPacketizerTypes.hpp"

namespace DoomFlight {{
namespace {NAMESPACE} {{

//! Number of packets in the merged packet list
constexpr FwChanIdType NUM_PACKETS = {len(packets)};

//! Number of distinct channels in the merged packet list
constexpr FwChanIdType NUM_CHANNELS = {num_channels};

//! Merged DoomFlight and DoomCoprocessor packet list
extern const Svc::TlmPacketizerPacketList packetList;

}}  // namespace {NAMESPACE}
}}  // namespace DoomFlight

#endif
"""
    )


def render_cpp(packets):
    lines = [
        HEADER,
        '#include "DoomSatellite/DoomFlight/Top/DoomSatelliteMergedPackets.hpp"',
        "",
        '#include "Fw/Time/Time.hpp"',
        '#include "TlmPacketizerConfig/TlmPacketizerCfg.hpp"',
        "",
        "namespace DoomFlight {",
        f"namespace {NAMESPACE} {{",
        "",
        "static_assert(NUM_PACKETS <= Svc::MAX_PACKETIZER_PACKETS, \"MAX_PACKETIZER_PACKETS too small for merged packets\");",
        "static_assert(NUM_CHANNELS <= Svc::MAX_PACKETIZER_CHANNELS, \"MAX_PACKETIZER_CHANNELS too small for merged packets\");",
        "",
        "namespace {",
        "",
        "//! Packet header: descriptor, packet id, and time tag",
        "constexpr FwSizeType PACKET_HEADER_SIZE =",
        "    sizeof(FwPacketDescriptorType) + sizeof(FwTlmPacketizeIdType) + Fw::Time::SERIALIZED_SIZE;",
        "",
    ]
    for packet in packets:
        packet_id = identifier(packet["name"])
        data_size = sum(size for _, _, size in packet["channels"])
        lines.append(f"// Packet {packet['name']}: id {packet['id']:#x}, group {packet['group']}, {data_size} data bytes")
        lines.append(f"constexpr Svc::TlmPacketizerChannelEntry {packet_id}Channels[] = {{")
        for name, channel_id, size in packet["channels"]:
            lines.append(f"    {{{channel_id:#010x}, {size}}},  // {name}")
        lines.append("};")
        lines.append(
            f"static_assert(PACKET_HEADER_SIZE + {data_size} <= FW_COM_BUFFER_MAX_SIZE, "
            f"\"Packet {packet['name']} exceeds FW_COM_BUFFER_MAX_SIZE\");"
        )
        lines.append(
            f"constexpr Svc::TlmPacketizerPacket {packet_id} = {{{packet_id}Channels, {packet['id']:#x}, "
            f"{packet['group']}, FW_NUM_ARRAY_ELEMENTS({packet_id}Channels)}};"
        )
        lines.append("")
    lines.append("}  // namespace")
    lines.append("")
    lines.append("const Svc::TlmPacketizerPacketList packetList = {")
    lines.append("    {")
    for packet in packets:
        lines.append(f"        &{identifier(packet['name'])},")
    lines.append("    },")
    lines.append("    NUM_PACKETS};")
    lines.append("")
    lines.append(f"}}  // namespace {NAMESPACE}")
    lines.append("}  // namespace DoomFlight")
    return "\n".join(lines) + "\n"


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--flight", type=Path, default=DEFAULT_FLIGHT, help="DoomFlight dictionary")
    parser.add_argument("--coprocessor", type=Path, default=DEFAULT_COPROCESSOR, help="DoomCoprocessor dictionary")
    parser.add_argument("--dictionary-out", type=Path, default=DEFAULT_DICTIONARY_OUT, help="Merged GDS dictionary")
    parser.add_argument(
        "--cpp-out", type=Path, default=DEFAULT_CPP_OUT, help="Output path for the packet list, without extension"
    )
    parser.add_argument("--merge-tool", default="fprime-merge-dictionary", help="fprime-merge-dictionary executable")
    parser.add_argument("--check", action="store_true", help="Fail if the generated packet list is out of date")
    arguments = parser.parse_args()

    dictionaries = [json.loads(path.read_text()) for path in (arguments.flight, arguments.coprocessor)]
    packets, num_channels = merge_packets(dictionaries)

    outputs = {
        arguments.cpp_out.with_suffix(".hpp"): render_hpp(packets, num_channels),
        arguments.cpp_out.with_suffix(".cpp"): render_cpp(packets),
    }
    if arguments.check:
        stale = [str(path) for path, text in outputs.items() if not path.exists() or path.read_text() != text]
        if stale:
            raise SystemExit(f"Out of date (run tools/merge_packets.py): {', '.join(stale)}")
    else:
        for path, text in outputs.items():
            path.write_text(text)

    merged = merge_dictionaries(arguments.flight, arguments.coprocessor, arguments.merge_tool)
    merged["telemetryPacketSets"] = [merged_packet_set(dictionaries, packets)]
    arguments.dictionary_out.parent.mkdir(parents=True, exist_ok=True)
    arguments.dictionary_out.write_text(json.dumps(merged, indent=2) + "\n")

    largest = max(packets, key=lambda packet: sum(size for _, _, size in packet["channels"]))
    print(
        f"{len(packets)} packets, {num_channels} channels; largest packet {largest['name']} "
        f"({sum(size for _, _, size in largest['channels'])} data bytes)"
    )
    print(f"Merged dictionary: {arguments.dictionary_out}")


if __name__ == "__main__":
    main()
