
GLYPHS = \
	glyphsets/small-upper.inc \
	glyphsets/uk.inc \
	glyphsets/small-us.inc \
	glyphsets/sg.inc \
	glyphsets/tcs.inc \

STATUS = \
	glyphsets/status-none.inc \
	glyphsets/status-2nd.inc \
	glyphsets/status-diamond.inc \
	glyphsets/status-shift.inc \
	glyphsets/status-hand.inc \
	glyphsets/status-handlock.inc \
	glyphsets/status-capslock.inc \
	glyphsets/status-compose1.inc \
	glyphsets/status-compose2.inc \
	glyphsets/status-bell.inc \
	glyphsets/status-scrolllock.inc \
	glyphsets/status-batt0.inc \
	glyphsets/status-batt1.inc \
	glyphsets/status-batt2.inc \
	glyphsets/status-batt3.inc \
	glyphsets/status-batt4.inc \
	glyphsets/status-busy.inc \
	glyphsets/status-link0.inc \
	glyphsets/status-link1.inc \
	glyphsets/status-link2.inc \
	glyphsets/status-link3.inc \

CSRCS = \
	libc.c \
	lcd.c \
	main.c \
	heap.c \
	tbd.c \
	kbd.c \
	signal.c \

#	tty.c \
#	../process.c \
#	../devio.c \
#	dev_vt.c \
#	../printf.c \

ASRCS = \
	../lowlevel-ti92p.S \
	calc.S \
	memmove.S \
	memcpy.S \
	memset.S \
	drawglyph.S

DSRCS = \


DOBJS = $(patsubst ../dev/%.c,%.o, $(DSRCS))

LSRCS = \


LOBJS = $(patsubst ../lib/%.c,%.o, $(LSRCS))

COBJS = $(CSRCS:.c=.o)
AOBJS = $(ASRCS:.S=.o)
PBM = $(GLYPHS) $(STATUS)
INC = $(PBM:.pbm=.inc)
OBJS  = $(COBJS) $(AOBJS) $(DOBJS) $(LOBJS)

CROSS_CCOPTS += -I../dev/

all:	$(INC) $(OBJS)

GLYPHS2C = (echo '{' && tail -c1152 | xxd -i && echo '},')
STATUS2C = (echo '{' && tail -c7 | xxd -i && echo '},')

$(GLYPHS): %.inc: %.pbm
	$(GLYPHS2C) <$< >$@

$(STATUS): %.inc: %.pbm
	$(STATUS2C) <$< >$@

$(COBJS): %.o: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(AOBJS): %.o: %.S
	$(CROSS_AS) $(ASOPTS) $< -o $*.o

$(DOBJS): %.o: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(NOBJS): %.o: ../dev/net/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(ROBJS): %.o: ../dev/rc2014/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(LOBJS): %.o: ../lib/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

clean:
	rm -f *.o *~ glyphsets/*.inc

image-9x.tib: $(OBJS)
	# Our LD is weired and append -9x.tib to our created file
	# (because we have the definition of a 92plus calculator in calc.S)
	$(CROSS_LD) $(LDFLAGS) -o image $^

image-9x.9xu: image-9x.tib
	tib2xxu $< $@
	rabbitsign -r $@ -k 0104.key -o $@

image: image-9x.9xu
	echo "image is ready"
	
IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage: image.9xu
	cp $< $(IMAGES)/$<
