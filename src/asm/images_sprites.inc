; Generated by make_images.py

sprites_num_images: equ 89

; buffer_ids:
BUF_0TILE_EMPTY: equ 256
BUF_1TILE_CROSS: equ 257
BUF_2TILE_HORIZ: equ 258
BUF_3TILE_VERT: equ 259
BUF_4TILE_SQUARE: equ 260
BUF_5TILE_CIRCLE: equ 261
BUF_6TILE_PAD: equ 262
BUF_7TILE_TURRET: equ 263
BUF_CIRCLE: equ 264
BUF_CRATER: equ 265
BUF_EXPLOSION_A: equ 266
BUF_EXPLOSION_B: equ 267
BUF_EXPLOSION_C: equ 268
BUF_EXPLOSION_D: equ 269
BUF_EXPLOSION_E: equ 270
BUF_FIREBALL_A: equ 271
BUF_FIREBALL_B: equ 272
BUF_LASER_A: equ 273
BUF_LASER_B: equ 274
BUF_PAD: equ 275
BUF_SEEKER_000: equ 276
BUF_SEEKER_008: equ 277
BUF_SEEKER_016: equ 278
BUF_SEEKER_024: equ 279
BUF_SEEKER_032: equ 280
BUF_SEEKER_040: equ 281
BUF_SEEKER_048: equ 282
BUF_SEEKER_056: equ 283
BUF_SEEKER_064: equ 284
BUF_SEEKER_072: equ 285
BUF_SEEKER_080: equ 286
BUF_SEEKER_088: equ 287
BUF_SEEKER_096: equ 288
BUF_SEEKER_104: equ 289
BUF_SEEKER_112: equ 290
BUF_SEEKER_120: equ 291
BUF_SEEKER_128: equ 292
BUF_SEEKER_136: equ 293
BUF_SEEKER_144: equ 294
BUF_SEEKER_152: equ 295
BUF_SEEKER_160: equ 296
BUF_SEEKER_168: equ 297
BUF_SEEKER_176: equ 298
BUF_SEEKER_184: equ 299
BUF_SEEKER_192: equ 300
BUF_SEEKER_200: equ 301
BUF_SEEKER_208: equ 302
BUF_SEEKER_216: equ 303
BUF_SEEKER_224: equ 304
BUF_SEEKER_232: equ 305
BUF_SEEKER_240: equ 306
BUF_SEEKER_248: equ 307
BUF_SHIP_0L: equ 308
BUF_SHIP_1C: equ 309
BUF_SHIP_2R: equ 310
BUF_SHIP_SMALL: equ 311
BUF_STAR: equ 312
BUF_TURRET_000: equ 313
BUF_TURRET_008: equ 314
BUF_TURRET_016: equ 315
BUF_TURRET_024: equ 316
BUF_TURRET_032: equ 317
BUF_TURRET_040: equ 318
BUF_TURRET_048: equ 319
BUF_TURRET_056: equ 320
BUF_TURRET_064: equ 321
BUF_TURRET_072: equ 322
BUF_TURRET_080: equ 323
BUF_TURRET_088: equ 324
BUF_TURRET_096: equ 325
BUF_TURRET_104: equ 326
BUF_TURRET_112: equ 327
BUF_TURRET_120: equ 328
BUF_TURRET_128: equ 329
BUF_TURRET_136: equ 330
BUF_TURRET_144: equ 331
BUF_TURRET_152: equ 332
BUF_TURRET_160: equ 333
BUF_TURRET_168: equ 334
BUF_TURRET_176: equ 335
BUF_TURRET_184: equ 336
BUF_TURRET_192: equ 337
BUF_TURRET_200: equ 338
BUF_TURRET_208: equ 339
BUF_TURRET_216: equ 340
BUF_TURRET_224: equ 341
BUF_TURRET_232: equ 342
BUF_TURRET_240: equ 343
BUF_TURRET_248: equ 344

sprites_image_list: ; type; width; height; filename; bufferId:
	dl 1, 16, 16, 256, fn_0tile_empty, 256
	dl 1, 16, 16, 256, fn_1tile_cross, 257
	dl 1, 16, 16, 256, fn_2tile_horiz, 258
	dl 1, 16, 16, 256, fn_3tile_vert, 259
	dl 1, 16, 16, 256, fn_4tile_square, 260
	dl 1, 16, 16, 256, fn_5tile_circle, 261
	dl 1, 16, 16, 256, fn_6tile_pad, 262
	dl 1, 16, 16, 256, fn_7tile_turret, 263
	dl 1, 16, 16, 256, fn_circle, 264
	dl 1, 16, 16, 256, fn_crater, 265
	dl 1, 16, 16, 256, fn_explosion_a, 266
	dl 1, 16, 16, 256, fn_explosion_b, 267
	dl 1, 16, 16, 256, fn_explosion_c, 268
	dl 1, 16, 16, 256, fn_explosion_d, 269
	dl 1, 16, 16, 256, fn_explosion_e, 270
	dl 1, 7, 7, 49, fn_fireball_a, 271
	dl 1, 7, 7, 49, fn_fireball_b, 272
	dl 1, 5, 13, 65, fn_laser_a, 273
	dl 1, 5, 13, 65, fn_laser_b, 274
	dl 1, 16, 16, 256, fn_pad, 275
	dl 1, 16, 16, 256, fn_seeker_000, 276
	dl 1, 16, 16, 256, fn_seeker_008, 277
	dl 1, 16, 16, 256, fn_seeker_016, 278
	dl 1, 16, 16, 256, fn_seeker_024, 279
	dl 1, 16, 16, 256, fn_seeker_032, 280
	dl 1, 16, 16, 256, fn_seeker_040, 281
	dl 1, 16, 16, 256, fn_seeker_048, 282
	dl 1, 16, 16, 256, fn_seeker_056, 283
	dl 1, 16, 16, 256, fn_seeker_064, 284
	dl 1, 16, 16, 256, fn_seeker_072, 285
	dl 1, 16, 16, 256, fn_seeker_080, 286
	dl 1, 16, 16, 256, fn_seeker_088, 287
	dl 1, 16, 16, 256, fn_seeker_096, 288
	dl 1, 16, 16, 256, fn_seeker_104, 289
	dl 1, 16, 16, 256, fn_seeker_112, 290
	dl 1, 16, 16, 256, fn_seeker_120, 291
	dl 1, 16, 16, 256, fn_seeker_128, 292
	dl 1, 16, 16, 256, fn_seeker_136, 293
	dl 1, 16, 16, 256, fn_seeker_144, 294
	dl 1, 16, 16, 256, fn_seeker_152, 295
	dl 1, 16, 16, 256, fn_seeker_160, 296
	dl 1, 16, 16, 256, fn_seeker_168, 297
	dl 1, 16, 16, 256, fn_seeker_176, 298
	dl 1, 16, 16, 256, fn_seeker_184, 299
	dl 1, 16, 16, 256, fn_seeker_192, 300
	dl 1, 16, 16, 256, fn_seeker_200, 301
	dl 1, 16, 16, 256, fn_seeker_208, 302
	dl 1, 16, 16, 256, fn_seeker_216, 303
	dl 1, 16, 16, 256, fn_seeker_224, 304
	dl 1, 16, 16, 256, fn_seeker_232, 305
	dl 1, 16, 16, 256, fn_seeker_240, 306
	dl 1, 16, 16, 256, fn_seeker_248, 307
	dl 1, 16, 16, 256, fn_ship_0l, 308
	dl 1, 16, 16, 256, fn_ship_1c, 309
	dl 1, 16, 16, 256, fn_ship_2r, 310
	dl 1, 8, 8, 64, fn_ship_small, 311
	dl 1, 5, 5, 25, fn_star, 312
	dl 1, 16, 16, 256, fn_turret_000, 313
	dl 1, 16, 16, 256, fn_turret_008, 314
	dl 1, 16, 16, 256, fn_turret_016, 315
	dl 1, 16, 16, 256, fn_turret_024, 316
	dl 1, 16, 16, 256, fn_turret_032, 317
	dl 1, 16, 16, 256, fn_turret_040, 318
	dl 1, 16, 16, 256, fn_turret_048, 319
	dl 1, 16, 16, 256, fn_turret_056, 320
	dl 1, 16, 16, 256, fn_turret_064, 321
	dl 1, 16, 16, 256, fn_turret_072, 322
	dl 1, 16, 16, 256, fn_turret_080, 323
	dl 1, 16, 16, 256, fn_turret_088, 324
	dl 1, 16, 16, 256, fn_turret_096, 325
	dl 1, 16, 16, 256, fn_turret_104, 326
	dl 1, 16, 16, 256, fn_turret_112, 327
	dl 1, 16, 16, 256, fn_turret_120, 328
	dl 1, 16, 16, 256, fn_turret_128, 329
	dl 1, 16, 16, 256, fn_turret_136, 330
	dl 1, 16, 16, 256, fn_turret_144, 331
	dl 1, 16, 16, 256, fn_turret_152, 332
	dl 1, 16, 16, 256, fn_turret_160, 333
	dl 1, 16, 16, 256, fn_turret_168, 334
	dl 1, 16, 16, 256, fn_turret_176, 335
	dl 1, 16, 16, 256, fn_turret_184, 336
	dl 1, 16, 16, 256, fn_turret_192, 337
	dl 1, 16, 16, 256, fn_turret_200, 338
	dl 1, 16, 16, 256, fn_turret_208, 339
	dl 1, 16, 16, 256, fn_turret_216, 340
	dl 1, 16, 16, 256, fn_turret_224, 341
	dl 1, 16, 16, 256, fn_turret_232, 342
	dl 1, 16, 16, 256, fn_turret_240, 343
	dl 1, 16, 16, 256, fn_turret_248, 344

; files_list: ; filename:
fn_0tile_empty: db "sprites/0tile_empty.rgba2",0 
fn_1tile_cross: db "sprites/1tile_cross.rgba2",0 
fn_2tile_horiz: db "sprites/2tile_horiz.rgba2",0 
fn_3tile_vert: db "sprites/3tile_vert.rgba2",0 
fn_4tile_square: db "sprites/4tile_square.rgba2",0 
fn_5tile_circle: db "sprites/5tile_circle.rgba2",0 
fn_6tile_pad: db "sprites/6tile_pad.rgba2",0 
fn_7tile_turret: db "sprites/7tile_turret.rgba2",0 
fn_circle: db "sprites/circle.rgba2",0 
fn_crater: db "sprites/crater.rgba2",0 
fn_explosion_a: db "sprites/explosion_a.rgba2",0 
fn_explosion_b: db "sprites/explosion_b.rgba2",0 
fn_explosion_c: db "sprites/explosion_c.rgba2",0 
fn_explosion_d: db "sprites/explosion_d.rgba2",0 
fn_explosion_e: db "sprites/explosion_e.rgba2",0 
fn_fireball_a: db "sprites/fireball_a.rgba2",0 
fn_fireball_b: db "sprites/fireball_b.rgba2",0 
fn_laser_a: db "sprites/laser_a.rgba2",0 
fn_laser_b: db "sprites/laser_b.rgba2",0 
fn_pad: db "sprites/pad.rgba2",0 
fn_seeker_000: db "sprites/seeker_000.rgba2",0 
fn_seeker_008: db "sprites/seeker_008.rgba2",0 
fn_seeker_016: db "sprites/seeker_016.rgba2",0 
fn_seeker_024: db "sprites/seeker_024.rgba2",0 
fn_seeker_032: db "sprites/seeker_032.rgba2",0 
fn_seeker_040: db "sprites/seeker_040.rgba2",0 
fn_seeker_048: db "sprites/seeker_048.rgba2",0 
fn_seeker_056: db "sprites/seeker_056.rgba2",0 
fn_seeker_064: db "sprites/seeker_064.rgba2",0 
fn_seeker_072: db "sprites/seeker_072.rgba2",0 
fn_seeker_080: db "sprites/seeker_080.rgba2",0 
fn_seeker_088: db "sprites/seeker_088.rgba2",0 
fn_seeker_096: db "sprites/seeker_096.rgba2",0 
fn_seeker_104: db "sprites/seeker_104.rgba2",0 
fn_seeker_112: db "sprites/seeker_112.rgba2",0 
fn_seeker_120: db "sprites/seeker_120.rgba2",0 
fn_seeker_128: db "sprites/seeker_128.rgba2",0 
fn_seeker_136: db "sprites/seeker_136.rgba2",0 
fn_seeker_144: db "sprites/seeker_144.rgba2",0 
fn_seeker_152: db "sprites/seeker_152.rgba2",0 
fn_seeker_160: db "sprites/seeker_160.rgba2",0 
fn_seeker_168: db "sprites/seeker_168.rgba2",0 
fn_seeker_176: db "sprites/seeker_176.rgba2",0 
fn_seeker_184: db "sprites/seeker_184.rgba2",0 
fn_seeker_192: db "sprites/seeker_192.rgba2",0 
fn_seeker_200: db "sprites/seeker_200.rgba2",0 
fn_seeker_208: db "sprites/seeker_208.rgba2",0 
fn_seeker_216: db "sprites/seeker_216.rgba2",0 
fn_seeker_224: db "sprites/seeker_224.rgba2",0 
fn_seeker_232: db "sprites/seeker_232.rgba2",0 
fn_seeker_240: db "sprites/seeker_240.rgba2",0 
fn_seeker_248: db "sprites/seeker_248.rgba2",0 
fn_ship_0l: db "sprites/ship_0l.rgba2",0 
fn_ship_1c: db "sprites/ship_1c.rgba2",0 
fn_ship_2r: db "sprites/ship_2r.rgba2",0 
fn_ship_small: db "sprites/ship_small.rgba2",0 
fn_star: db "sprites/star.rgba2",0 
fn_turret_000: db "sprites/turret_000.rgba2",0 
fn_turret_008: db "sprites/turret_008.rgba2",0 
fn_turret_016: db "sprites/turret_016.rgba2",0 
fn_turret_024: db "sprites/turret_024.rgba2",0 
fn_turret_032: db "sprites/turret_032.rgba2",0 
fn_turret_040: db "sprites/turret_040.rgba2",0 
fn_turret_048: db "sprites/turret_048.rgba2",0 
fn_turret_056: db "sprites/turret_056.rgba2",0 
fn_turret_064: db "sprites/turret_064.rgba2",0 
fn_turret_072: db "sprites/turret_072.rgba2",0 
fn_turret_080: db "sprites/turret_080.rgba2",0 
fn_turret_088: db "sprites/turret_088.rgba2",0 
fn_turret_096: db "sprites/turret_096.rgba2",0 
fn_turret_104: db "sprites/turret_104.rgba2",0 
fn_turret_112: db "sprites/turret_112.rgba2",0 
fn_turret_120: db "sprites/turret_120.rgba2",0 
fn_turret_128: db "sprites/turret_128.rgba2",0 
fn_turret_136: db "sprites/turret_136.rgba2",0 
fn_turret_144: db "sprites/turret_144.rgba2",0 
fn_turret_152: db "sprites/turret_152.rgba2",0 
fn_turret_160: db "sprites/turret_160.rgba2",0 
fn_turret_168: db "sprites/turret_168.rgba2",0 
fn_turret_176: db "sprites/turret_176.rgba2",0 
fn_turret_184: db "sprites/turret_184.rgba2",0 
fn_turret_192: db "sprites/turret_192.rgba2",0 
fn_turret_200: db "sprites/turret_200.rgba2",0 
fn_turret_208: db "sprites/turret_208.rgba2",0 
fn_turret_216: db "sprites/turret_216.rgba2",0 
fn_turret_224: db "sprites/turret_224.rgba2",0 
fn_turret_232: db "sprites/turret_232.rgba2",0 
fn_turret_240: db "sprites/turret_240.rgba2",0 
fn_turret_248: db "sprites/turret_248.rgba2",0 
