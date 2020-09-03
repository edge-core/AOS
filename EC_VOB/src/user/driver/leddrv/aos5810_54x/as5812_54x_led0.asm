;
; led port remap: led port 0-63 == physical port 65-128
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

; user port 17-36
	ld	a,0x3f
	ld	(portnum),a
xg_portloop_1:
	port	a
	call	xg_port
	dec	(portnum)
	ld	a,(portnum)
	cmp	a,0x2b
	jnz	xg_portloop_1

; user port 5-16
	ld	a,0x1f
	ld	(portnum),a
xg_portloop_2:
	port	a
	call	xg_port
	dec	(portnum)
	ld	a,(portnum)
	cmp	a,0x13
	jnz	xg_portloop_2

; user port 1-4
	ld	a,0x0f
	ld	(portnum),a
xg_portloop_3:
	port	a
	call	xg_port
	dec	(portnum)
	ld	a,(portnum)
	cmp	a,0x0b
	jnz	xg_portloop_3

	send	0x48

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
	pack
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
