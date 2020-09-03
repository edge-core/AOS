;
; led port remap: led port 0-63 == physical port 65-128
;
; user port 49-54
; - support 4x10g mode
; - support qsfp 10g mode led
;

	ld	a,(phase)
	inc	a
	cmp	a,0x19
	jc	chkblink
	sub	a,a
chkblink:
	ld	(phase),a
	sub	b,b
	cmp	a,0x0f
	jnc	noblink
	inc	b
noblink:
	ld	(blink),b

; user port 49-54 qsfp 10g mode led
; sequence (physical port): 97 101 81 105 109 77
	ld	a,0x20
	call	qsfp_lanes
	ld	a,0x24
	call	qsfp_lanes
	ld	a,0x10
	call	qsfp_lanes
	ld	a,0x28
	call	qsfp_lanes
	ld	a,0x2c
	call	qsfp_lanes
	ld	a,0x0c
	call	qsfp_lanes

; user port 49-54 qsfp 40g mode led
; sequence (physical port): 109 105 101 97 81 77
	ld	a,0x2c
	call	qsfp_cage
	ld	a,0x28
	call	qsfp_cage
	ld	a,0x24
	call	qsfp_cage
	ld	a,0x20
	call	qsfp_cage
	ld	a,0x10
	call	qsfp_cage
	ld	a,0x0c
	call	qsfp_cage

; user port 37-48
	ld	a,0x0b
	ld	(portnum),a
xg_portloop:
	port	a
	call	xg_port
	pack
	pack
	dec	(portnum)
	ld	a,(portnum)
	cmp	a,0xff
	jnz	xg_portloop

	send	0x36

xg_port:
	pushst	LINKUP
	pushst	LINKEN
	tand
	pop
	jnc	xg_led_black
	pushst	RX
	pushst	TX
	tor
	push	(blink)
	tand
	pop
	jc	xg_led_black
	pushst	SPEED_C
	pushst	SPEED_M
	tand
	pop
	jc	xg_led_green
xg_led_amber:
	pushst	ZERO
	pushst	ZERO
	jmp	xg_link_end
xg_led_green:
	pushst	ZERO
	pushst	ONE
	jmp	xg_link_end
xg_led_black:
	pushst	ONE
	pushst	ONE
xg_link_end:
	ret

qsfp_cage:
	ld b,PORTDATA
	add b,a
	ld b,(b)
	tst b,PORTDATA_40G
	jnc qsfp_cage_off
	port	a
	call	xg_port
	pop
	pack
	ret
qsfp_cage_off:
	pushst	ONE
	pack
	ret

qsfp_lanes:
	ld b,PORTDATA
	add b,a
	ld b,(b)
	tst b,PORTDATA_40G
	jc qsfp_lanes_off
	port	a
	call	xg_port
	pop
	pack
	inc	a
	port	a
	call	xg_port
	pop
	pack
	inc	a
	port	a
	call	xg_port
	pop
	pack
	inc	a
	port	a
	call	xg_port
	pop
	pack
	ret
qsfp_lanes_off:
	pushst	ONE
	pack
	pushst	ONE
	pack
	pushst	ONE
	pack
	pushst	ONE
	pack
	ret

;========================================================
; data storage
portnum equ 0xFF ;temp to hold which port we're working on
blink   equ 0xFD ;1 if blink on, 0 if blink off
phase   equ 0xFE ;phase within 30 Hz

;========================================================
; symbolic labels
; this gives symbolic names to the various bits of the port status fields
RX      equ 0x0 ; received packet
TX      equ 0x1 ; transmitted packet
COLL    equ 0x2 ; collision indicator
SPEED_C equ 0x3 ; 100 Mbps
SPEED_M equ 0x4 ; 1000 Mbps
DUPLEX  equ 0x5 ; half/full duplex
FLOW    equ 0x6 ; flow control capable
LINKUP  equ 0x7 ; link down/up status
LINKEN  equ 0x8 ; link disabled/enabled status
ZERO    equ 0xE ; always 0
ONE     equ 0xF ; always 1

; port status updated by ledproc_linkscan_cb()
PORTDATA        equ     0xa0
PORTDATA_LINK   equ     0x0
PORTDATA_40G    equ     0x1
