#CROSS_COMPILE=mips-linux-uclibc-gnu-
CC=$(CROSS_COMPILE)gcc
CPP=$(CROSS_COMPILE)g++
AS=$(CROSS_COMPILE)as
LD=$(CROSS_COMPILE)ld
STRIP=$(CROSS_COMPILE)strip

ROOTDIR := .
SRCDIR := $(ROOTDIR)/src
INCLUDE := -I $(ROOTDIR)/src
# INCLUDE += -I $(SRCDIR)mempool

OBJS += $(SRCDIR)/cstd_test.o

# LIBDIR := -L ./lib

# LIBS := -lm 
LIBS := -lcstd_safety

# CFLAGS := $(INCLUDE) $(LIBDIR) -Wall -O2 -ffunction-sections -fdata-sections -Wl,-Map=object.map,--cref,--gc-section -fvisibility=hidden
# CFLAGS := $(INCLUDE) $(LIBDIR) $(LIBS) -Wall -O2 -ffunction-sections -fdata-sections -Wl,-Map=object.map,--cref,--gc-section -fvisibility=hidden 
CFLAGS := $(INCLUDE) $(LIBDIR) $(LIBS) -Wall -g
TARGETS := cstd_test

$(TARGETS) : ${OBJS}
	$(CC) $(CFLAGS)  -o $@ ${OBJS} $(LIBS) 
	# $(STRIP) $(TARGETS) 

clean:
	@find . -name "*.o"  | xargs rm -f
	@rm -rf $(TARGETS)

