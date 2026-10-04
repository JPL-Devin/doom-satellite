# Telemetry packets for the DOOM stream DoomCoprocessor sends across the hub.
#
# DoomCoprocessor has no TlmPacketizer: tools/merge_packets.py merges this packet set with DoomFlight's into the
# packet list DoomFlight's TlmPacketizer uses. Ids start at 0x100 to stay clear of DoomFlight's packet ids.
telemetry packets DoomCoprocessorPackets {

  packet DoomEngine id 0x100 group 1 {
    DoomSubtopology.doom.State
    DoomSubtopology.doom.FrameCount
    DoomSubtopology.doom.KeyQueueDepth
    DoomSubtopology.doom.KeyEventsDropped
    DoomSubtopology.doom.FramesDropped
    DoomSubtopology.doom.FrameRateHz
    DoomSubtopology.doom.FrameDataRateBps
    DoomSubtopology.doom.InputCommandRateHz
    DoomSubtopology.doom.InputDataRateBps
  }

  packet CoprocessorFrameAdapter id 0x102 group 1 {
    frameAdapter.PackedRejected
    frameAdapter.FramesPacked
    frameAdapter.FramesUnpacked
    frameAdapter.FramesRejected
    frameAdapter.PalettesPacked
    frameAdapter.PalettesUnpacked
    frameAdapter.PalettesRejected
  }

  packet CoprocessorFrameReader id 0x103 group 1 {
    frameRepeater.FramesRepeated
    frameRepeater.PalettesRepeated
    frameReader.FramesRead
    frameReader.PalettesRead
    frameReader.LastFrame
    frameReader.MeanPixel
    frameReader.MeanBrightness
  }

} omit {
  # Not sent across the hub
  # Local housekeeping: not routed across the hub
  cmdDisp.CommandsDispatched
  cmdDisp.CommandErrors
  cmdDisp.CommandsDropped
  rateGroupDoom.RgMaxTime
  rateGroupDoom.RgCycleSlips
  rateGroup1Hz.RgMaxTime
  rateGroup1Hz.RgCycleSlips
  hubBufferManager.TotalBuffs
  hubBufferManager.CurrBuffs
  hubBufferManager.HiBuffs
  hubBufferManager.NoBuffs
  hubBufferManager.EmptyBuffs
  DoomSubtopology.doomBufferManager.TotalBuffs
  DoomSubtopology.doomBufferManager.CurrBuffs
  DoomSubtopology.doomBufferManager.HiBuffs
  DoomSubtopology.doomBufferManager.NoBuffs
  DoomSubtopology.doomBufferManager.EmptyBuffs
}
