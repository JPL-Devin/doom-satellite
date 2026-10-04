"""Integration tests for the raw DOOM frame path across the GenericHub

DoomCoprocessor packs each downsampled frame and palette into a buffer and sends it over the hub. DoomFlight repeats
it: one copy is echoed back to DoomCoprocessor, the other is unpacked for frameTlmProcessor and downlinked as frame
row and palette telemetry. DOOM must be running on DoomCoprocessor (started with -S).

Run against a GDS loaded with the merged DoomFlight + DoomCoprocessor dictionary:
    pytest --dictionary DoomSatelliteDictionary.json frame_test.py
"""
from fprime_gds.common.testing_fw import predicates

TIMEOUT = 10
REJECTION_CHANNELS = [
    "DoomFlight.frameAdapter.PackedRejected",
    "DoomFlight.frameAdapter.FramesRejected",
    "DoomFlight.frameAdapter.PalettesRejected",
    "DoomCoprocessor.frameAdapter.PackedRejected",
    "DoomCoprocessor.frameAdapter.FramesRejected",
    "DoomCoprocessor.frameAdapter.PalettesRejected",
]


def assert_counting(fprime_test_api, channel):
    """Asserts that a counter channel is positive and then increases"""
    first = fprime_test_api.assert_telemetry(
        fprime_test_api.get_telemetry_pred(channel, predicates.greater_than(0)), start="NOW", timeout=TIMEOUT
    )
    fprime_test_api.assert_telemetry(
        fprime_test_api.get_telemetry_pred(channel, predicates.greater_than(first.get_val())),
        start="NOW",
        timeout=TIMEOUT,
    )


def test_frames_packed_on_coprocessor(fprime_test_api):
    """DoomCoprocessor packs downsampled frames for the hub"""
    assert_counting(fprime_test_api, "DoomCoprocessor.frameAdapter.FramesPacked")


def test_frames_unpacked_on_flight(fprime_test_api):
    """DoomFlight receives and unpacks the repeated frame copy"""
    assert_counting(fprime_test_api, "DoomFlight.frameAdapter.FramesUnpacked")


def test_frames_echoed_to_coprocessor(fprime_test_api):
    """The echoed frame copy returns across the hub and DoomCoprocessor unpacks it"""
    assert_counting(fprime_test_api, "DoomCoprocessor.frameAdapter.FramesUnpacked")


def test_palette_reaches_flight_and_echo(fprime_test_api):
    """The palette sent at startup is unpacked on DoomFlight and on DoomCoprocessor after the echo"""
    for channel in ["DoomFlight.frameAdapter.PalettesUnpacked", "DoomCoprocessor.frameAdapter.PalettesUnpacked"]:
        fprime_test_api.assert_telemetry(
            fprime_test_api.get_telemetry_pred(channel, predicates.greater_than(0)), timeout=TIMEOUT
        )


def test_frame_rows_downlinked(fprime_test_api):
    """frameTlmProcessor on DoomFlight converts frames into row telemetry, first to last row"""
    for channel in ["DoomFlight.frameTlmProcessor.FrameRow000", "DoomFlight.frameTlmProcessor.FrameRow049"]:
        fprime_test_api.assert_telemetry(channel, start="NOW", timeout=TIMEOUT)


def test_palette_downlinked(fprime_test_api):
    """frameTlmProcessor on DoomFlight downlinks the palette"""
    fprime_test_api.assert_telemetry("DoomFlight.frameTlmProcessor.PaletteOut", timeout=TIMEOUT)


def test_no_rejections(fprime_test_api):
    """No frame, palette or packed buffer is rejected at either end"""
    for channel in REJECTION_CHANNELS:
        fprime_test_api.assert_telemetry_count(
            0, fprime_test_api.get_telemetry_pred(channel, predicates.greater_than(0)), timeout=0
        )
