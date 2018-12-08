#CROSS_COMPILE=mips-linux-uclibc-gnu-
CC=$(CROSS_COMPILE)gcc
CPP=$(CROSS_COMPILE)g++
AS=$(CROSS_COMPILE)as
LD=$(CROSS_COMPILE)ld
STRIP=$(CROSS_COMPILE)strip

ROOTDIR := .
SRCDIR := $(ROOTDIR)/src
INCLUDE := -I $(ROOTDIR)/src
INCLUDE += -I $(ROOTDIR)/src/verify
INCLUDE += -I $(ROOTDIR)/src/log

OBJS := $(SRCDIR)/cstd_enc_key.o
OBJS += $(SRCDIR)/verify/verify.o

LIBS :=-Wl,-Bstatic -lcrypto -lssl -Wl,-Bdynamic -lpthread -ldl

CFLAGS := $(INCLUDE) $(LIBDIR) $(LIBS) -Wall -O2 -ffunction-sections -fdata-sections -Wl,-Map=object.map,--cref,--gc-section -fvisibility=hidden 
# CFLAGS := $(INCLUDE) $(LIBDIR) $(LIBS) -Wall -g
TARGETS := cstd_enc_key

$(TARGETS) : ${OBJS}
	$(CC)  -o $@ ${OBJS} $(LIBS) 
	$(STRIP) $(TARGETS) 

clean:
	@rm -f $(OBJS)
	@rm -rf $(TARGETS)

