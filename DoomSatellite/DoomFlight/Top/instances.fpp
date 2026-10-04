module DoomFlight {

  # ----------------------------------------------------------------------
  # Base ID Convention
  # ----------------------------------------------------------------------
  #
  # All Base IDs follow the 8-digit hex format: 0xDSSCCxxx
  #
  # Where:
  #   D   = Deployment digit (1 for this deployment)
  #   SS  = Subtopology digits (00 for main topology, 01-05 for subtopologies)
  #   CC  = Component digits (00, 01, 02, etc.)
  #   xxx = Reserved for internal component items (events, commands, telemetry)
  #

  # ----------------------------------------------------------------------
  # Defaults
  # ----------------------------------------------------------------------

  module Default {
    constant QUEUE_SIZE = 10
    constant STACK_SIZE = 8 * 1024 # Must match prj.conf thread stack size
  }

  # ----------------------------------------------------------------------
  # Active component instances
  # ----------------------------------------------------------------------

  instance rateGroup10Hz: Svc.ActiveRateGroup base id 0x10001000 \
    queue size Default.QUEUE_SIZE \
    stack size Default.STACK_SIZE \
    priority 3

  instance rateGroup1Hz: Svc.ActiveRateGroup base id 0x10002000 \
    queue size Default.QUEUE_SIZE \
    stack size Default.STACK_SIZE \
    priority 4


  # ----------------------------------------------------------------------
  # Queued component instances
  # ----------------------------------------------------------------------


  # ----------------------------------------------------------------------
  # Passive component instances
  # ----------------------------------------------------------------------

  instance chronoTime: Zephyr.ZephyrTime base id 0x10010000

  instance rateGroupDriver: Svc.RateGroupDriver base id 0x10011000

  instance timer: Zephyr.ZephyrRateDriver base id 0x10013000

  instance comDriver: Zephyr.ZephyrUartDriver base id 0x10014000

  instance nullPrmDb: Components.NullPrmDb base id 0x10015000

  instance touchReset: Zephyr.ZephyrTouchReset base id 0x10016000

  instance hub: Svc.GenericHub base id 0x10017000

  instance hubComDriver: Drv.Udp base id 0x10018000

  instance hubByteStreamAdapter: Drv.ByteStreamBufferAdapter base id 0x10019000

  instance hubBufferManager: Svc.BufferManager base id 0x1001A000

  instance cmdSplitter: Svc.CmdSplitter base id 0x1001B000

  @ Repeats each packed frame and palette from the hub: one copy is echoed to DoomCoprocessor, one goes to frameAdapter
  instance frameRepeater: Svc.BufferRepeater base id 0x1001C000

  @ Unpacks frames and palettes received from DoomCoprocessor for frameTlmProcessor
  instance frameAdapter: Components.FrameBufferAdapter base id 0x1001D000

  @ Converts frames and palettes from DoomCoprocessor into row and palette telemetry. Keeps the DoomSubtopology base
  @ id, so channel ids match a DoomSubtopology deployment.
  instance frameTlmProcessor: Doom.FrameTlmProcessor base id DoomSubtopologyConfig.BASE_ID + 0x03000

}
