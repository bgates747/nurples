; image_type: equ 0
; image_width: equ image_type+3
; image_height: equ image_width+3
; image_filesize: equ image_height+3
; image_filename: equ image_filesize+3
; image_bufferId: equ image_filename+3
; image_record_size: equ image_bufferId+3

sprites_num_images: equ 1

BUF_SHIP_1C: equ 309

sprites_image_list: ; type; width; height; filename; bufferId:
	dl 1, 16, 16, 256, fn_ship_1c, 309

fn_ship_1c: db "sprites/ship_1c.rgba2",0 
