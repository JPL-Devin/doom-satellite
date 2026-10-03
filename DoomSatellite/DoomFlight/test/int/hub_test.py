"""Integration tests for the DoomFlight control node and the DoomCoprocessor across the GenericHub

Run against a GDS loaded with the merged DoomFlight + DoomCoprocessor dictionary:
    pytest --dictionary DoomSatelliteDictionary.json hub_test.py
"""
import pytest


def test_local_no_op(fprime_test_api):
    """CdhCore.cmdDisp handles a local NO_OP routed by the CmdSplitter"""
    fprime_test_api.send_and_assert_event(
        "CdhCore.cmdDisp.CMD_NO_OP",
        events=["CdhCore.cmdDisp.OpCodeDispatched", "CdhCore.cmdDisp.NoOpReceived", "CdhCore.cmdDisp.OpCodeCompleted"],
        timeout=10,
    )


def test_remote_no_op(fprime_test_api):
    """DoomCoprocessor.cmdDisp handles a NO_OP routed across the GenericHub, and its events return over the hub"""
    fprime_test_api.send_and_assert_event(
        "DoomCoprocessor.cmdDisp.CMD_NO_OP",
        events=[
            "DoomCoprocessor.cmdDisp.OpCodeDispatched",
            "DoomCoprocessor.cmdDisp.NoOpReceived",
            "DoomCoprocessor.cmdDisp.OpCodeCompleted",
        ],
        timeout=10,
    )


def test_remote_no_op_string(fprime_test_api):
    """String arguments survive the round trip across the GenericHub"""
    fprime_test_api.send_and_assert_event(
        "DoomCoprocessor.cmdDisp.CMD_NO_OP_STRING",
        ["hello-doom"],
        events=[fprime_test_api.get_event_pred("DoomCoprocessor.cmdDisp.NoOpStringReceived", ["hello-doom"])],
        timeout=10,
    )


@pytest.mark.skip(reason="DoomCoprocessor telemetry is not sent across the hub")
def test_remote_telemetry(fprime_test_api):
    """DoomCoprocessor telemetry is injected into DoomFlight's downlink by the hub"""
    fprime_test_api.send_command("DoomCoprocessor.cmdDisp.CMD_NO_OP")
    fprime_test_api.assert_telemetry("DoomCoprocessor.cmdDisp.CommandsDispatched", timeout=10)
