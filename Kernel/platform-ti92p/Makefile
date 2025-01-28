
CSRCS = \
	libc.c \
	main.c \
	tbd.c \


ASRCS = \
	../lowlevel-ti92p.S \
	calc.S

DSRCS = \


DOBJS = $(patsubst ../dev/%.c,%.o, $(DSRCS))

LSRCS = \


LOBJS = $(patsubst ../lib/%.c,%.o, $(LSRCS))

COBJS = $(CSRCS:.c=.o)
AOBJS = $(ASRCS:.S=.o)
OBJS  = $(COBJS) $(AOBJS) $(DOBJS) $(LOBJS)

CROSS_CCOPTS += -I../dev/

all:	$(OBJS)

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
	rm -f *.o fuzix.elf loader.elf loader.map loader.bin core *~ 

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
