	.comm	_ti92plus,1

| XXX see h/globals.h
| G = 0x5c00

| void cpuidle(int mask);
.global cpuidle
cpuidle:
	move.w	%sr,%d1
	move.w	#0x2700,%sr
	move.w	#0x280,0x600018 | XXX: why do we do this?
	move	4(%sp),%d2	| interrupt mask
.ifdef TI89
	move.b	%d2,0x600005 | shut off the cpu until an interrupt is triggered
	nop
.else
	move	#(disable_flash_end-disable_flash)/2-1,%d0
	lea	disable_flash,%a0
	jbsr	exec_in_ram
.endif
	move.w	#0x2000,%sr  | let interrupt handlers run
	move.w	%d1,%sr
	rts

| this must be run from RAM
disable_flash:
	move.w	0x185e00,%d0 | shut down flash rom
	move.b	%d2,0x600005 | shut off the cpu until an interrupt is triggered
	nop
	move.w	%d0,0x185e00
	nop
	nop
	rts
disable_flash_end:

exec_ram = 0x5c00+4+8 | XXX see globals.h

| input:
|  %a0 = code to execute in ram
|  %d0 = one less than the size of code, in 16-bit words, to execute in ram
.global exec_in_ram
exec_in_ram:
	lea     exec_ram,%a1
0:	move.w  (%a0)+,(%a1)+
	dbra    %d0,0b

	jbra     exec_ram        | Execute code in RAM

.global _WaitKeyboard
_WaitKeyboard:
	moveq	#0x58,%d0
	dbf	%d0,.
	rts
