telemetry packets DoomFlightPackets {

  packet Health id 1 group 1 {
    CdhCore.cmdDisp.CommandsDispatched
    CdhCore.cmdDisp.CommandsDropped
    ComCcsds.comQueue.comQueueDepth
    ComCcsds.commsBufferManager.HiBuffs
    hubBufferManager.HiBuffs
    rateGroup10Hz.RgMaxTime
    rateGroup1Hz.RgMaxTime
  }

  packet HealthWarnings id 2 group 1 {
    CdhCore.$health.PingLateWarnings
    CdhCore.events.EventsDropped
    ComCcsds.commsBufferManager.NoBuffs
    ComCcsds.commsBufferManager.EmptyBuffs
    hubBufferManager.NoBuffs
    hubBufferManager.EmptyBuffs
    rateGroup10Hz.RgCycleSlips
    rateGroup1Hz.RgCycleSlips
  }


  packet HealthAuxillary id 3 group 2 {
    ComCcsds.commsBufferManager.TotalBuffs
    ComCcsds.commsBufferManager.CurrBuffs
    ComCcsds.comQueue.buffQueueDepth
    hubBufferManager.TotalBuffs
    hubBufferManager.CurrBuffs
    CdhCore.tlmSend.GroupConfigs
    CdhCore.tlmSend.SectionEnabled
  }

  packet Version id 4 group 3 {
    CdhCore.version.FrameworkVersion
    CdhCore.version.ProjectVersion
    CdhCore.version.LibraryVersion01
    CdhCore.version.LibraryVersion02
  }


  # DOOM frame telemetry from frameTlmProcessor, written for Doom.DOWNSAMPLE_FACTOR = 8:
  # FrameRow000..049 are packetized and the higher rows are omitted. Update it with the factor.
  packet FlightFrameAdapter id 0x10 group 1 {
    frameAdapter.PackedRejected
    frameAdapter.FramesUnpacked
    frameAdapter.FramesRejected
    frameAdapter.PalettesUnpacked
    frameAdapter.PalettesRejected
  }

  packet DoomPalette id 0x101 group 1 {
    frameTlmProcessor.PaletteOut
  }

  packet DoomFrameRow000 id 0x200 group 1 {
    frameTlmProcessor.FrameRow000
  }
  packet DoomFrameRow001 id 0x201 group 1 {
    frameTlmProcessor.FrameRow001
  }
  packet DoomFrameRow002 id 0x202 group 1 {
    frameTlmProcessor.FrameRow002
  }
  packet DoomFrameRow003 id 0x203 group 1 {
    frameTlmProcessor.FrameRow003
  }
  packet DoomFrameRow004 id 0x204 group 1 {
    frameTlmProcessor.FrameRow004
  }
  packet DoomFrameRow005 id 0x205 group 1 {
    frameTlmProcessor.FrameRow005
  }
  packet DoomFrameRow006 id 0x206 group 1 {
    frameTlmProcessor.FrameRow006
  }
  packet DoomFrameRow007 id 0x207 group 1 {
    frameTlmProcessor.FrameRow007
  }
  packet DoomFrameRow008 id 0x208 group 1 {
    frameTlmProcessor.FrameRow008
  }
  packet DoomFrameRow009 id 0x209 group 1 {
    frameTlmProcessor.FrameRow009
  }
  packet DoomFrameRow010 id 0x20a group 1 {
    frameTlmProcessor.FrameRow010
  }
  packet DoomFrameRow011 id 0x20b group 1 {
    frameTlmProcessor.FrameRow011
  }
  packet DoomFrameRow012 id 0x20c group 1 {
    frameTlmProcessor.FrameRow012
  }
  packet DoomFrameRow013 id 0x20d group 1 {
    frameTlmProcessor.FrameRow013
  }
  packet DoomFrameRow014 id 0x20e group 1 {
    frameTlmProcessor.FrameRow014
  }
  packet DoomFrameRow015 id 0x20f group 1 {
    frameTlmProcessor.FrameRow015
  }
  packet DoomFrameRow016 id 0x210 group 1 {
    frameTlmProcessor.FrameRow016
  }
  packet DoomFrameRow017 id 0x211 group 1 {
    frameTlmProcessor.FrameRow017
  }
  packet DoomFrameRow018 id 0x212 group 1 {
    frameTlmProcessor.FrameRow018
  }
  packet DoomFrameRow019 id 0x213 group 1 {
    frameTlmProcessor.FrameRow019
  }
  packet DoomFrameRow020 id 0x214 group 1 {
    frameTlmProcessor.FrameRow020
  }
  packet DoomFrameRow021 id 0x215 group 1 {
    frameTlmProcessor.FrameRow021
  }
  packet DoomFrameRow022 id 0x216 group 1 {
    frameTlmProcessor.FrameRow022
  }
  packet DoomFrameRow023 id 0x217 group 1 {
    frameTlmProcessor.FrameRow023
  }
  packet DoomFrameRow024 id 0x218 group 1 {
    frameTlmProcessor.FrameRow024
  }
  packet DoomFrameRow025 id 0x219 group 1 {
    frameTlmProcessor.FrameRow025
  }
  packet DoomFrameRow026 id 0x21a group 1 {
    frameTlmProcessor.FrameRow026
  }
  packet DoomFrameRow027 id 0x21b group 1 {
    frameTlmProcessor.FrameRow027
  }
  packet DoomFrameRow028 id 0x21c group 1 {
    frameTlmProcessor.FrameRow028
  }
  packet DoomFrameRow029 id 0x21d group 1 {
    frameTlmProcessor.FrameRow029
  }
  packet DoomFrameRow030 id 0x21e group 1 {
    frameTlmProcessor.FrameRow030
  }
  packet DoomFrameRow031 id 0x21f group 1 {
    frameTlmProcessor.FrameRow031
  }
  packet DoomFrameRow032 id 0x220 group 1 {
    frameTlmProcessor.FrameRow032
  }
  packet DoomFrameRow033 id 0x221 group 1 {
    frameTlmProcessor.FrameRow033
  }
  packet DoomFrameRow034 id 0x222 group 1 {
    frameTlmProcessor.FrameRow034
  }
  packet DoomFrameRow035 id 0x223 group 1 {
    frameTlmProcessor.FrameRow035
  }
  packet DoomFrameRow036 id 0x224 group 1 {
    frameTlmProcessor.FrameRow036
  }
  packet DoomFrameRow037 id 0x225 group 1 {
    frameTlmProcessor.FrameRow037
  }
  packet DoomFrameRow038 id 0x226 group 1 {
    frameTlmProcessor.FrameRow038
  }
  packet DoomFrameRow039 id 0x227 group 1 {
    frameTlmProcessor.FrameRow039
  }
  packet DoomFrameRow040 id 0x228 group 1 {
    frameTlmProcessor.FrameRow040
  }
  packet DoomFrameRow041 id 0x229 group 1 {
    frameTlmProcessor.FrameRow041
  }
  packet DoomFrameRow042 id 0x22a group 1 {
    frameTlmProcessor.FrameRow042
  }
  packet DoomFrameRow043 id 0x22b group 1 {
    frameTlmProcessor.FrameRow043
  }
  packet DoomFrameRow044 id 0x22c group 1 {
    frameTlmProcessor.FrameRow044
  }
  packet DoomFrameRow045 id 0x22d group 1 {
    frameTlmProcessor.FrameRow045
  }
  packet DoomFrameRow046 id 0x22e group 1 {
    frameTlmProcessor.FrameRow046
  }
  packet DoomFrameRow047 id 0x22f group 1 {
    frameTlmProcessor.FrameRow047
  }
  packet DoomFrameRow048 id 0x230 group 1 {
    frameTlmProcessor.FrameRow048
  }
  packet DoomFrameRow049 id 0x231 group 1 {
    frameTlmProcessor.FrameRow049
  }

} omit {
  CdhCore.cmdDisp.CommandErrors
  # Only has one library, no custom versions
  CdhCore.version.LibraryVersion03
  CdhCore.version.LibraryVersion04
  CdhCore.version.LibraryVersion05
  CdhCore.version.LibraryVersion06
  CdhCore.version.LibraryVersion07
  CdhCore.version.LibraryVersion08
  CdhCore.version.LibraryVersion09
  CdhCore.version.LibraryVersion10
  CdhCore.version.CustomVersion01
  CdhCore.version.CustomVersion02
  CdhCore.version.CustomVersion03
  CdhCore.version.CustomVersion04
  CdhCore.version.CustomVersion05
  CdhCore.version.CustomVersion06
  CdhCore.version.CustomVersion07
  CdhCore.version.CustomVersion08
  CdhCore.version.CustomVersion09
  CdhCore.version.CustomVersion10
  # Unused on DoomFlight: frameAdapter only unpacks
  frameAdapter.FramesPacked
  frameAdapter.PalettesPacked
  # Rows beyond Doom.DOWNSAMPLED_HEIGHT are never written
  frameTlmProcessor.FrameRow050
  frameTlmProcessor.FrameRow051
  frameTlmProcessor.FrameRow052
  frameTlmProcessor.FrameRow053
  frameTlmProcessor.FrameRow054
  frameTlmProcessor.FrameRow055
  frameTlmProcessor.FrameRow056
  frameTlmProcessor.FrameRow057
  frameTlmProcessor.FrameRow058
  frameTlmProcessor.FrameRow059
  frameTlmProcessor.FrameRow060
  frameTlmProcessor.FrameRow061
  frameTlmProcessor.FrameRow062
  frameTlmProcessor.FrameRow063
  frameTlmProcessor.FrameRow064
  frameTlmProcessor.FrameRow065
  frameTlmProcessor.FrameRow066
  frameTlmProcessor.FrameRow067
  frameTlmProcessor.FrameRow068
  frameTlmProcessor.FrameRow069
  frameTlmProcessor.FrameRow070
  frameTlmProcessor.FrameRow071
  frameTlmProcessor.FrameRow072
  frameTlmProcessor.FrameRow073
  frameTlmProcessor.FrameRow074
  frameTlmProcessor.FrameRow075
  frameTlmProcessor.FrameRow076
  frameTlmProcessor.FrameRow077
  frameTlmProcessor.FrameRow078
  frameTlmProcessor.FrameRow079
  frameTlmProcessor.FrameRow080
  frameTlmProcessor.FrameRow081
  frameTlmProcessor.FrameRow082
  frameTlmProcessor.FrameRow083
  frameTlmProcessor.FrameRow084
  frameTlmProcessor.FrameRow085
  frameTlmProcessor.FrameRow086
  frameTlmProcessor.FrameRow087
  frameTlmProcessor.FrameRow088
  frameTlmProcessor.FrameRow089
  frameTlmProcessor.FrameRow090
  frameTlmProcessor.FrameRow091
  frameTlmProcessor.FrameRow092
  frameTlmProcessor.FrameRow093
  frameTlmProcessor.FrameRow094
  frameTlmProcessor.FrameRow095
  frameTlmProcessor.FrameRow096
  frameTlmProcessor.FrameRow097
  frameTlmProcessor.FrameRow098
  frameTlmProcessor.FrameRow099
  frameTlmProcessor.FrameRow100
  frameTlmProcessor.FrameRow101
  frameTlmProcessor.FrameRow102
  frameTlmProcessor.FrameRow103
  frameTlmProcessor.FrameRow104
  frameTlmProcessor.FrameRow105
  frameTlmProcessor.FrameRow106
  frameTlmProcessor.FrameRow107
  frameTlmProcessor.FrameRow108
  frameTlmProcessor.FrameRow109
  frameTlmProcessor.FrameRow110
  frameTlmProcessor.FrameRow111
  frameTlmProcessor.FrameRow112
  frameTlmProcessor.FrameRow113
  frameTlmProcessor.FrameRow114
  frameTlmProcessor.FrameRow115
  frameTlmProcessor.FrameRow116
  frameTlmProcessor.FrameRow117
  frameTlmProcessor.FrameRow118
  frameTlmProcessor.FrameRow119
  frameTlmProcessor.FrameRow120
  frameTlmProcessor.FrameRow121
  frameTlmProcessor.FrameRow122
  frameTlmProcessor.FrameRow123
  frameTlmProcessor.FrameRow124
  frameTlmProcessor.FrameRow125
  frameTlmProcessor.FrameRow126
  frameTlmProcessor.FrameRow127
  frameTlmProcessor.FrameRow128
  frameTlmProcessor.FrameRow129
  frameTlmProcessor.FrameRow130
  frameTlmProcessor.FrameRow131
  frameTlmProcessor.FrameRow132
  frameTlmProcessor.FrameRow133
  frameTlmProcessor.FrameRow134
  frameTlmProcessor.FrameRow135
  frameTlmProcessor.FrameRow136
  frameTlmProcessor.FrameRow137
  frameTlmProcessor.FrameRow138
  frameTlmProcessor.FrameRow139
  frameTlmProcessor.FrameRow140
  frameTlmProcessor.FrameRow141
  frameTlmProcessor.FrameRow142
  frameTlmProcessor.FrameRow143
  frameTlmProcessor.FrameRow144
  frameTlmProcessor.FrameRow145
  frameTlmProcessor.FrameRow146
  frameTlmProcessor.FrameRow147
  frameTlmProcessor.FrameRow148
  frameTlmProcessor.FrameRow149
  frameTlmProcessor.FrameRow150
  frameTlmProcessor.FrameRow151
  frameTlmProcessor.FrameRow152
  frameTlmProcessor.FrameRow153
  frameTlmProcessor.FrameRow154
  frameTlmProcessor.FrameRow155
  frameTlmProcessor.FrameRow156
  frameTlmProcessor.FrameRow157
  frameTlmProcessor.FrameRow158
  frameTlmProcessor.FrameRow159
  frameTlmProcessor.FrameRow160
  frameTlmProcessor.FrameRow161
  frameTlmProcessor.FrameRow162
  frameTlmProcessor.FrameRow163
  frameTlmProcessor.FrameRow164
  frameTlmProcessor.FrameRow165
  frameTlmProcessor.FrameRow166
  frameTlmProcessor.FrameRow167
  frameTlmProcessor.FrameRow168
  frameTlmProcessor.FrameRow169
  frameTlmProcessor.FrameRow170
  frameTlmProcessor.FrameRow171
  frameTlmProcessor.FrameRow172
  frameTlmProcessor.FrameRow173
  frameTlmProcessor.FrameRow174
  frameTlmProcessor.FrameRow175
  frameTlmProcessor.FrameRow176
  frameTlmProcessor.FrameRow177
  frameTlmProcessor.FrameRow178
  frameTlmProcessor.FrameRow179
  frameTlmProcessor.FrameRow180
  frameTlmProcessor.FrameRow181
  frameTlmProcessor.FrameRow182
  frameTlmProcessor.FrameRow183
  frameTlmProcessor.FrameRow184
  frameTlmProcessor.FrameRow185
  frameTlmProcessor.FrameRow186
  frameTlmProcessor.FrameRow187
  frameTlmProcessor.FrameRow188
  frameTlmProcessor.FrameRow189
  frameTlmProcessor.FrameRow190
  frameTlmProcessor.FrameRow191
  frameTlmProcessor.FrameRow192
  frameTlmProcessor.FrameRow193
  frameTlmProcessor.FrameRow194
  frameTlmProcessor.FrameRow195
  frameTlmProcessor.FrameRow196
  frameTlmProcessor.FrameRow197
  frameTlmProcessor.FrameRow198
  frameTlmProcessor.FrameRow199
  frameTlmProcessor.FrameRow200
  frameTlmProcessor.FrameRow201
  frameTlmProcessor.FrameRow202
  frameTlmProcessor.FrameRow203
  frameTlmProcessor.FrameRow204
  frameTlmProcessor.FrameRow205
  frameTlmProcessor.FrameRow206
  frameTlmProcessor.FrameRow207
  frameTlmProcessor.FrameRow208
  frameTlmProcessor.FrameRow209
  frameTlmProcessor.FrameRow210
  frameTlmProcessor.FrameRow211
  frameTlmProcessor.FrameRow212
  frameTlmProcessor.FrameRow213
  frameTlmProcessor.FrameRow214
  frameTlmProcessor.FrameRow215
  frameTlmProcessor.FrameRow216
  frameTlmProcessor.FrameRow217
  frameTlmProcessor.FrameRow218
  frameTlmProcessor.FrameRow219
  frameTlmProcessor.FrameRow220
  frameTlmProcessor.FrameRow221
  frameTlmProcessor.FrameRow222
  frameTlmProcessor.FrameRow223
  frameTlmProcessor.FrameRow224
  frameTlmProcessor.FrameRow225
  frameTlmProcessor.FrameRow226
  frameTlmProcessor.FrameRow227
  frameTlmProcessor.FrameRow228
  frameTlmProcessor.FrameRow229
  frameTlmProcessor.FrameRow230
  frameTlmProcessor.FrameRow231
  frameTlmProcessor.FrameRow232
  frameTlmProcessor.FrameRow233
  frameTlmProcessor.FrameRow234
  frameTlmProcessor.FrameRow235
  frameTlmProcessor.FrameRow236
  frameTlmProcessor.FrameRow237
  frameTlmProcessor.FrameRow238
  frameTlmProcessor.FrameRow239
  frameTlmProcessor.FrameRow240
  frameTlmProcessor.FrameRow241
  frameTlmProcessor.FrameRow242
  frameTlmProcessor.FrameRow243
  frameTlmProcessor.FrameRow244
  frameTlmProcessor.FrameRow245
  frameTlmProcessor.FrameRow246
  frameTlmProcessor.FrameRow247
  frameTlmProcessor.FrameRow248
  frameTlmProcessor.FrameRow249
  frameTlmProcessor.FrameRow250
  frameTlmProcessor.FrameRow251
  frameTlmProcessor.FrameRow252
  frameTlmProcessor.FrameRow253
  frameTlmProcessor.FrameRow254
  frameTlmProcessor.FrameRow255
  frameTlmProcessor.FrameRow256
  frameTlmProcessor.FrameRow257
  frameTlmProcessor.FrameRow258
  frameTlmProcessor.FrameRow259
  frameTlmProcessor.FrameRow260
  frameTlmProcessor.FrameRow261
  frameTlmProcessor.FrameRow262
  frameTlmProcessor.FrameRow263
  frameTlmProcessor.FrameRow264
  frameTlmProcessor.FrameRow265
  frameTlmProcessor.FrameRow266
  frameTlmProcessor.FrameRow267
  frameTlmProcessor.FrameRow268
  frameTlmProcessor.FrameRow269
  frameTlmProcessor.FrameRow270
  frameTlmProcessor.FrameRow271
  frameTlmProcessor.FrameRow272
  frameTlmProcessor.FrameRow273
  frameTlmProcessor.FrameRow274
  frameTlmProcessor.FrameRow275
  frameTlmProcessor.FrameRow276
  frameTlmProcessor.FrameRow277
  frameTlmProcessor.FrameRow278
  frameTlmProcessor.FrameRow279
  frameTlmProcessor.FrameRow280
  frameTlmProcessor.FrameRow281
  frameTlmProcessor.FrameRow282
  frameTlmProcessor.FrameRow283
  frameTlmProcessor.FrameRow284
  frameTlmProcessor.FrameRow285
  frameTlmProcessor.FrameRow286
  frameTlmProcessor.FrameRow287
  frameTlmProcessor.FrameRow288
  frameTlmProcessor.FrameRow289
  frameTlmProcessor.FrameRow290
  frameTlmProcessor.FrameRow291
  frameTlmProcessor.FrameRow292
  frameTlmProcessor.FrameRow293
  frameTlmProcessor.FrameRow294
  frameTlmProcessor.FrameRow295
  frameTlmProcessor.FrameRow296
  frameTlmProcessor.FrameRow297
  frameTlmProcessor.FrameRow298
  frameTlmProcessor.FrameRow299
  frameTlmProcessor.FrameRow300
  frameTlmProcessor.FrameRow301
  frameTlmProcessor.FrameRow302
  frameTlmProcessor.FrameRow303
  frameTlmProcessor.FrameRow304
  frameTlmProcessor.FrameRow305
  frameTlmProcessor.FrameRow306
  frameTlmProcessor.FrameRow307
  frameTlmProcessor.FrameRow308
  frameTlmProcessor.FrameRow309
  frameTlmProcessor.FrameRow310
  frameTlmProcessor.FrameRow311
  frameTlmProcessor.FrameRow312
  frameTlmProcessor.FrameRow313
  frameTlmProcessor.FrameRow314
  frameTlmProcessor.FrameRow315
  frameTlmProcessor.FrameRow316
  frameTlmProcessor.FrameRow317
  frameTlmProcessor.FrameRow318
  frameTlmProcessor.FrameRow319
  frameTlmProcessor.FrameRow320
  frameTlmProcessor.FrameRow321
  frameTlmProcessor.FrameRow322
  frameTlmProcessor.FrameRow323
  frameTlmProcessor.FrameRow324
  frameTlmProcessor.FrameRow325
  frameTlmProcessor.FrameRow326
  frameTlmProcessor.FrameRow327
  frameTlmProcessor.FrameRow328
  frameTlmProcessor.FrameRow329
  frameTlmProcessor.FrameRow330
  frameTlmProcessor.FrameRow331
  frameTlmProcessor.FrameRow332
  frameTlmProcessor.FrameRow333
  frameTlmProcessor.FrameRow334
  frameTlmProcessor.FrameRow335
  frameTlmProcessor.FrameRow336
  frameTlmProcessor.FrameRow337
  frameTlmProcessor.FrameRow338
  frameTlmProcessor.FrameRow339
  frameTlmProcessor.FrameRow340
  frameTlmProcessor.FrameRow341
  frameTlmProcessor.FrameRow342
  frameTlmProcessor.FrameRow343
  frameTlmProcessor.FrameRow344
  frameTlmProcessor.FrameRow345
  frameTlmProcessor.FrameRow346
  frameTlmProcessor.FrameRow347
  frameTlmProcessor.FrameRow348
  frameTlmProcessor.FrameRow349
  frameTlmProcessor.FrameRow350
  frameTlmProcessor.FrameRow351
  frameTlmProcessor.FrameRow352
  frameTlmProcessor.FrameRow353
  frameTlmProcessor.FrameRow354
  frameTlmProcessor.FrameRow355
  frameTlmProcessor.FrameRow356
  frameTlmProcessor.FrameRow357
  frameTlmProcessor.FrameRow358
  frameTlmProcessor.FrameRow359
  frameTlmProcessor.FrameRow360
  frameTlmProcessor.FrameRow361
  frameTlmProcessor.FrameRow362
  frameTlmProcessor.FrameRow363
  frameTlmProcessor.FrameRow364
  frameTlmProcessor.FrameRow365
  frameTlmProcessor.FrameRow366
  frameTlmProcessor.FrameRow367
  frameTlmProcessor.FrameRow368
  frameTlmProcessor.FrameRow369
  frameTlmProcessor.FrameRow370
  frameTlmProcessor.FrameRow371
  frameTlmProcessor.FrameRow372
  frameTlmProcessor.FrameRow373
  frameTlmProcessor.FrameRow374
  frameTlmProcessor.FrameRow375
  frameTlmProcessor.FrameRow376
  frameTlmProcessor.FrameRow377
  frameTlmProcessor.FrameRow378
  frameTlmProcessor.FrameRow379
  frameTlmProcessor.FrameRow380
  frameTlmProcessor.FrameRow381
  frameTlmProcessor.FrameRow382
  frameTlmProcessor.FrameRow383
  frameTlmProcessor.FrameRow384
  frameTlmProcessor.FrameRow385
  frameTlmProcessor.FrameRow386
  frameTlmProcessor.FrameRow387
  frameTlmProcessor.FrameRow388
  frameTlmProcessor.FrameRow389
  frameTlmProcessor.FrameRow390
  frameTlmProcessor.FrameRow391
  frameTlmProcessor.FrameRow392
  frameTlmProcessor.FrameRow393
  frameTlmProcessor.FrameRow394
  frameTlmProcessor.FrameRow395
  frameTlmProcessor.FrameRow396
  frameTlmProcessor.FrameRow397
  frameTlmProcessor.FrameRow398
  frameTlmProcessor.FrameRow399
}
