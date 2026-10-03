#!/usr/bin/env python3
"""Minimal remote GenericHub endpoint for exercising DoomFlight over UDP.

Receives HUB_TYPE_EVENT/HUB_TYPE_CHANNEL messages from the DoomFlight GenericHub, sends CMD_NO_OP commands through
HUB_TYPE_CMD_DISP, and reports HUB_TYPE_CMD_RESP responses. Type widths are taken from the deployment dictionary.
"""
import argparse
import json
import select
import socket
import struct
import time
from collections import Counter

HUB_TYPE_PORT, HUB_TYPE_BUFFER, HUB_TYPE_EVENT, HUB_TYPE_CHANNEL, HUB_TYPE_CMD_DISP, HUB_TYPE_CMD_RESP = range(6)
FW_PACKET_COMMAND, FW_PACKET_TELEM, FW_PACKET_LOG, FW_PACKET_FILE, FW_PACKET_PACKETIZED_TLM = range(5)
FORMATS = {(8, False): "B", (16, False): "H", (32, False): "I", (64, False): "Q",
           (8, True): "b", (16, True): "h", (32, True): "i", (64, True): "q"}


class Dictionary:
    def __init__(self, path):
        with open(path) as handle:
            data = json.load(handle)
        aliases = {}
        for definition in data["typeDefinitions"]:
            if definition.get("kind") == "alias":
                underlying = definition["underlyingType"]
                aliases[definition["qualifiedName"]] = ">" + FORMATS[(underlying["size"], underlying["signed"])]
        self.size_fmt = aliases["FwSizeStoreType"]
        self.desc_fmt = aliases["FwPacketDescriptorType"]
        self.opcode_fmt = aliases["FwOpcodeType"]
        self.event_id_fmt = aliases["FwEventIdType"]
        self.chan_id_fmt = aliases["FwChanIdType"]
        self.packet_id_fmt = aliases["FwTlmPacketizeIdType"]
        self.events = {event["id"]: event["name"] for event in data["events"]}
        self.channels = {channel["id"]: channel["name"] for channel in data["telemetryChannels"]}
        self.commands = {command["name"]: command["opcode"] for command in data["commands"]}
        self.opcodes = {opcode: name for name, opcode in self.commands.items()}
        self.packets = {}
        for packet_set in data.get("telemetryPacketSets", []):
            for packet in packet_set["members"]:
                self.packets[packet["id"]] = packet["name"]


def take(fmt, data, offset):
    value = struct.unpack_from(fmt, data, offset)[0]
    return value, offset + struct.calcsize(fmt)


def hub_message(dictionary, hub_type, port, payload):
    return struct.pack(">II", hub_type, port) + struct.pack(dictionary.size_fmt, len(payload)) + payload


def command_message(dictionary, opcode, context):
    command = struct.pack(dictionary.desc_fmt, FW_PACKET_COMMAND) + struct.pack(dictionary.opcode_fmt, opcode)
    return hub_message(dictionary, HUB_TYPE_CMD_DISP, 0, command + struct.pack(">I", context))


def decode(dictionary, data, stats, verbose):
    hub_type, offset = take(">I", data, 0)
    port, offset = take(">I", data, offset)
    size, offset = take(dictionary.size_fmt, data, offset)
    if size != len(data) - offset:
        stats["bad_size"] += 1
        return
    payload = data[offset:]
    if hub_type == HUB_TYPE_EVENT:
        event_id, _ = take(dictionary.event_id_fmt, payload, 0)
        name = dictionary.events.get(event_id, hex(event_id))
        stats["events"] += 1
        if verbose:
            print(f"EVENT  {name}")
    elif hub_type == HUB_TYPE_CHANNEL:
        channel_id, _ = take(dictionary.chan_id_fmt, payload, 0)
        name = dictionary.channels.get(channel_id, hex(channel_id))
        stats["channels"] += 1
        stats["chan:" + name] += 1
        if verbose:
            print(f"TLM    {name}")
    elif hub_type == HUB_TYPE_PORT:
        com_size, inner = take(dictionary.size_fmt, payload, 0)
        com = payload[inner:inner + com_size]
        descriptor, cursor = take(dictionary.desc_fmt, com, 0)
        if descriptor == FW_PACKET_LOG:
            event_id, _ = take(dictionary.event_id_fmt, com, cursor)
            name = dictionary.events.get(event_id, hex(event_id))
            stats["events"] += 1
            if verbose:
                print(f"EVENT  port={port} {name}")
        elif descriptor == FW_PACKET_PACKETIZED_TLM:
            packet_id, _ = take(dictionary.packet_id_fmt, com, cursor)
            stats["tlm_packets"] += 1
            stats["tlm:" + dictionary.packets.get(packet_id, str(packet_id))] += 1
            if verbose:
                print(f"TLM    port={port} packet={dictionary.packets.get(packet_id, packet_id)} bytes={len(com)}")
        else:
            stats[f"descriptor_{descriptor}"] += 1
    elif hub_type == HUB_TYPE_CMD_RESP:
        opcode, cursor = take(dictionary.opcode_fmt, payload, 0)
        sequence, cursor = take(">I", payload, cursor)
        response, _ = take(">B", payload, cursor)
        stats["cmd_resp_ok" if response == 0 else "cmd_resp_error"] += 1
        if verbose:
            print(f"CMDRSP {dictionary.opcodes.get(opcode, hex(opcode))} seq={sequence} response={response}")
    else:
        stats[f"hub_type_{hub_type}"] += 1


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--dictionary", required=True)
    parser.add_argument("--board", default="192.168.11.2")
    parser.add_argument("--board-port", type=int, default=50556)
    parser.add_argument("--listen-port", type=int, default=50555)
    parser.add_argument("--duration", type=float, default=10.0)
    parser.add_argument("--command-period", type=float, default=1.0)
    parser.add_argument("--command", default="CdhCore.cmdDisp.CMD_NO_OP")
    parser.add_argument("--verbose", action="store_true")
    args = parser.parse_args()

    dictionary = Dictionary(args.dictionary)
    opcode = dictionary.commands[args.command]
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(("0.0.0.0", args.listen_port))
    stats = Counter()
    start = time.monotonic()
    next_command = start
    while time.monotonic() - start < args.duration:
        now = time.monotonic()
        if args.command_period > 0 and now >= next_command:
            sock.sendto(command_message(dictionary, opcode, stats["cmd_sent"]), (args.board, args.board_port))
            stats["cmd_sent"] += 1
            next_command = now + args.command_period
        ready, _, _ = select.select([sock], [], [], 0.05)
        if ready:
            data, _ = sock.recvfrom(4096)
            stats["datagrams"] += 1
            decode(dictionary, data, stats, args.verbose)
    for key in sorted(stats):
        print(f"{key}: {stats[key]}")


if __name__ == "__main__":
    main()
