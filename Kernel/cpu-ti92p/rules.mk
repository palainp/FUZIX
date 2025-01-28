export TIGCC=/home/user/gcc4ti/
export CALC=TI92P
export CROSS_LD = $(TIGCC)/bin/tigcc -nostdlib --outputbin --flash-os
export CROSS_CC = $(TIGCC)/bin/tigcc
export CROSS_CCOPTS=-c -g -Os -fno-strict-aliasing -fomit-frame-pointer -fno-stack-protector -fno-PIC -fno-builtin -mno-bss  -Wall -I$(ROOT_DIR)/cpu-ti92p -I$(ROOT_DIR)/platform-$(TARGET) -I$(ROOT_DIR)/include -I$(ROOT_DIR) -D$(CALC)=
# export CROSS_AS=$(CROSS_CC) $(CROSS_CCOPTS)
export CROSS_AS=$(TIGCC)/bin/m68k-coff-tigcc-gcc -c -Wa,-m68881,--warn,--defsym,$(CALC)=
export CROSS_CC_SEG1=
export CROSS_CC_SEG2=
export CROSS_CC_SEG3=
# Fixme: we should split discard off
export CROSS_CC_SEGDISC=
export CROSS_CC_VIDEO=
export CROSS_CC_FONT=
export CROSS_CC_NETWORK=
export ASOPTS=
export ASMEXT = .S
export BINEXT = .o
export BITS = 32
export EXECFORMAT=32
