"""Integration tests for the raw DOOM frame path across the GenericHub

DoomCoprocessor repeats each downsampled frame and palette to its Python frameReader and to frameAdapter, which packs
it into a buffer and sends it over the hub. DoomFlight unpacks it and repeats it on the native types: one copy goes to
frameTlmProcessor and is downlinked as frame row and palette telemetry, the other is re-packed by frameEchoAdapter and
echoed back to DoomCoprocessor. DOOM must be running on DoomCoprocessor (started with -S).

Run against a GDS loaded with the merged DoomFlight + DoomCoprocessor dictionary:
    pytest --dictionary DoomSatelliteDictionary.json frame_test.py
"""
from fprime_gds.common.testing_fw import predicates

TIMEOUT = 10
REJECTION_CHANNELS = [
    "DoomFlight.frameAdapter.PackedRejected",
    "DoomFlight.frameAdapter.FramesRejected",
    "DoomFlight.frameAdapter.PalettesRejected",
    "DoomFlight.frameEchoAdapter.FramesRejected",
    "DoomFlight.frameEchoAdapter.PalettesRejected",
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


def test_frames_repeated_on_coprocessor(fprime_test_api):
    """DoomCoprocessor repeats each downsampled frame to the packer and the Python reader"""
    assert_counting(fprime_test_api, "DoomCoprocessor.frameRepeater.FramesRepeated")


def test_frames_read_in_python(fprime_test_api):
    """The Python frameReader reads the repeated frames"""
    assert_counting(fprime_test_api, "DoomCoprocessor.frameReader.FramesRead")
    fprime_test_api.assert_telemetry(
        fprime_test_api.get_telemetry_pred("DoomCoprocessor.frameReader.LastFrame", predicates.greater_than(0)),
        start="NOW",
        timeout=TIMEOUT,
    )


def test_frames_unpacked_on_flight(fprime_test_api):
    """DoomFlight receives and unpacks each frame, then repeats it on the native type"""
    assert_counting(fprime_test_api, "DoomFlight.frameAdapter.FramesUnpacked")
    assert_counting(fprime_test_api, "DoomFlight.frameRepeater.FramesRepeated")


def test_frames_echoed_to_coprocessor(fprime_test_api):
    """The repeated frame is re-packed, returns across the hub and DoomCoprocessor unpacks it"""
    assert_counting(fprime_test_api, "DoomFlight.frameEchoAdapter.FramesPacked")
    assert_counting(fprime_test_api, "DoomCoprocessor.frameAdapter.FramesUnpacked")


def test_palette_reaches_flight_and_echo(fprime_test_api):
    """The palette sent at startup is read in Python, unpacked on DoomFlight and on DoomCoprocessor after the echo"""
    for channel in [
        "DoomCoprocessor.frameReader.PalettesRead",
        "DoomFlight.frameAdapter.PalettesUnpacked",
        "DoomFlight.frameEchoAdapter.PalettesPacked",
        "DoomCoprocessor.frameAdapter.PalettesUnpacked",
    ]:
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
