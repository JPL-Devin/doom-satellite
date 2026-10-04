# Components::FrameBufferAdapter

## 1. Introduction

`Components::FrameBufferAdapter` is a passive component that converts the DOOM frame pipeline's typed ports
(`Doom.RawFrame`, `Doom.PaletteSend`) to and from `Fw::Buffer`, so that downsampled frames and palettes can cross one
GenericHub buffer port. Fan-out to several consumers happens on the typed ports, before packing or after unpacking,
through `Components::FrameRepeater`; the adapter itself only converts at the hub boundary.

In DoomSatellite:

- **DoomCoprocessor** `frameAdapter` packs each downsampled frame and palette repeated by `frameRepeater` onto
  `hub.bufferIn`, and unpacks (counts) the copies DoomFlight echoes back on `hub.bufferOut`.
- **DoomFlight** `frameAdapter` unpacks each packed buffer from `hub.bufferOut` onto `frameRepeater`, which feeds
  `frameTlmProcessor` (row and palette telemetry downlinked over CDC ACM) and a second instance, `frameEchoAdapter`,
  that re-packs the frame onto `hub.bufferIn` as the echo to DoomCoprocessor.

## 2. Requirements

| Requirement | Description | Verification |
| --- | --- | --- |
| FRAME-BUFFER-ADAPTER-001 | The component shall pack each valid frame received on `frameIn` into a buffer holding a `FRAME` kind byte, the frame number, width, height and `width * height` pixels, and send it on `packedOut`. | Unit test |
| FRAME-BUFFER-ADAPTER-002 | The component shall pack each palette received on `paletteIn` into a buffer holding a `PALETTE` kind byte and the serialized `Doom::Palette`, and send it on `packedOut`. | Unit test |
| FRAME-BUFFER-ADAPTER-003 | The component shall reject a frame on `frameIn` whose width or height is zero or exceeds the downsampled frame size, whose pixel buffer is null, or whose pixel buffer is smaller than `width * height`, emitting `FrameRejected` with the failed check. | Unit test |
| FRAME-BUFFER-ADAPTER-004 | The component shall pack into fixed member storage with no dynamic allocation, and shall reject a frame or palette with `BUFFER_IN_USE` while the previous packed buffer of that kind has not been returned on `packedOutReturn`. | Unit test |
| FRAME-BUFFER-ADAPTER-005 | The component shall unpack each buffer received on `packedIn` by its kind byte: a valid frame to `frameOut`, a valid palette to `paletteOut`. | Unit test, integration test |
| FRAME-BUFFER-ADAPTER-006 | The component shall reject a buffer on `packedIn` that is null, shorter than a kind byte, or carries an unknown kind (`PackedRejected`); a frame whose header is short, whose geometry is invalid or whose size does not match its geometry (`FrameRejected`); and a palette whose size is not the packed palette size (`PaletteRejected`). | Unit test |
| FRAME-BUFFER-ADAPTER-007 | The component shall return every buffer received on `packedIn` on `packedInReturn` exactly once, whether or not it was unpacked. | Unit test |
| FRAME-BUFFER-ADAPTER-009 | The component shall report packed, unpacked and rejected frame and palette counts, and rejected packed buffers, as telemetry. | Unit test, integration test |

## 3. Design

### 3.1 Ports

| Kind | Name | Type | Description |
| --- | --- | --- | --- |
| `guarded input` | `frameIn` | `Doom.RawFrame` | Frame to pack |
| `guarded input` | `paletteIn` | `Doom.PaletteSend` | Palette to pack |
| `output` | `packedOut` | `Fw.BufferSend` | Packed frame or palette in component storage; must be returned on `packedOutReturn` |
| `sync input` | `packedOutReturn` | `Fw.BufferSend` | Return of buffers sent on `packedOut` |
| `guarded input` | `packedIn` | `Fw.BufferSend` | Packed frame or palette to unpack |
| `output` | `packedInReturn` | `Fw.BufferSend` | Return of every buffer received on `packedIn` |
| `output` | `frameOut` | `Doom.RawFrame` | Unpacked frame; the pixels reference the received buffer and are valid only during the call |
| `output` | `paletteOut` | `Doom.PaletteSend` | Unpacked palette |

### 3.2 Packed format

All fields are big-endian (F Prime serialization).

| Kind | Layout | Size |
| --- | --- | --- |
| `FRAME` (0) | `U8 kind`, `U32 frameNumber`, `U16 width`, `U16 height`, `width * height` pixels | 9 + `width * height` bytes (4,009 at 80x50) |
| `PALETTE` (1) | `U8 kind`, serialized `Doom::Palette` | 1 + `Doom::Palette::SERIALIZED_SIZE` bytes |

### 3.3 Buffer ownership

- **Pack:** `packedOut` lends `m_frameStorage` or `m_paletteStorage`. Each is lent at most once at a time; the
  `packedOutReturn` handler asserts the returned pointer is one of the two and that it was lent. GenericHub copies the
  buffer into its own allocation and returns it synchronously.
- **Unpack:** `packedIn` never keeps the buffer: it is returned on `packedInReturn` before the handler returns.
  `frameOut` passes a view of the received pixels that is valid only during the call.
- **Echo (DoomFlight):** GenericHub requires `bufferInReturn[i]` to reach the component that called `bufferIn[i]`,
  so the echo is produced by a dedicated packing instance (`frameEchoAdapter`) fed from the unpacked frame; no
  received buffer is held beyond its `packedIn` call. The unpack and re-pack instances are distinct because the
  guarded `packedIn` -> `frameOut` -> `frameIn` chain would re-enter a single instance's mutex.

### 3.4 Concurrency

`frameIn`, `paletteIn` and `packedIn` are guarded: on DoomCoprocessor, frames are packed on the DOOM rate group thread
while echoed buffers are unpacked on the hub's UDP receive thread, and both update the rejection counters.
`packedOutReturn` is sync because GenericHub returns buffers within the `packedOut` call, from the calling thread; a
guarded return port would deadlock on the held mutex.

When `packedOut` is not connected, `frameIn` and `paletteIn` drop their input without counting it.

### 3.5 Events

| Name | Severity | Arguments | Description |
| --- | --- | --- | --- |
| `PackedRejected` | WARNING_LO, throttled to 5 | `reason: FrameBufferStatus`, `size` | Kind of a `packedIn` buffer could not be read |
| `FrameRejected` | WARNING_LO, throttled to 5 | `reason`, `width`, `height`, `size` | Frame not packed or unpacked |
| `PaletteRejected` | WARNING_LO, throttled to 5 | `reason`, `size` | Palette not packed or unpacked |

Throttles are cleared by the next successful frame or palette.

### 3.6 Telemetry

| Name | Type | Description |
| --- | --- | --- |
| `PackedRejected` | `U32` | `packedIn` buffers that were neither a frame nor a palette |
| `FramesPacked` | `U32` | Frames packed onto `packedOut` |
| `FramesUnpacked` | `U32` | Frames unpacked onto `frameOut` |
| `FramesRejected` | `U32` | Frames rejected in either direction |
| `PalettesPacked` | `U32` | Palettes packed onto `packedOut` |
| `PalettesUnpacked` | `U32` | Palettes unpacked onto `paletteOut` |
| `PalettesRejected` | `U32` | Palettes rejected in either direction |

## 4. Unit tests

| Test | Requirements |
| --- | --- |
| `Pack.Frame` | 001, 009 |
| `Pack.Palette` | 002, 009 |
| `Pack.RejectsInvalidFrame` | 003 |
| `Pack.RejectsWhileLent` | 004 |
| `Unpack.RoundTripsFrame`, `Unpack.RoundTripsPalette` | 005, 007 |
| `Unpack.RejectsInvalidPacked`, `Unpack.RejectsInvalidFrame`, `Unpack.RejectsInvalidPalette` | 006, 007 |

Integration tests in `DoomSatellite/DoomFlight/test/int/frame_test.py` exercise 005 and 009 across the physical hub
with DOOM running on DoomCoprocessor.
