#CROSS_COMPILE=mips-linux-uclibc-gnu-
CC=$(CROSS_COMPILE)gcc
CPP=$(CROSS_COMPILE)g++
AS=$(CROSS_COMPILE)as
LD=$(CROSS_COMPILE)ld
STRIP=$(CROSS_COMPILE)strip

ROOTDIR := .
SRCDIR := $(ROOTDIR)/src
INCLUDE := -I $(SRCDIR)
INCLUDE += -I $(SRCDIR)/log
INCLUDE += -I $(SRCDIR)/verify
# INCLUDE += -I $(SRCDIR)mempool
SRCS := $(SRCDIR)/cstd_safety.c

OBJS := $(SRCDIR)/cstd_safety.o
OBJS += $(SRCDIR)/verify/verify.o

LIBS := -Wl,-Bstatic -lcrypto -lssl  -Wl,-Bdynamic -lm -lpthread
# LIBS := -lm -static libcrypto.a -static libssl.a

# CFLAGS := $(INCLUDE) $(LIBDIR) -Wall -O2 -ffunction-sections -fdata-sections -Wl,-Map=object.map,--cref,--gc-section -fvisibility=hidden
CFLAGS := $(INCLUDE) $(LIBDIR) -Wall -O2 -ffunction-sections -fdata-sections -Wl,-Map=object.map,--cref,--gc-section -fPIC

TARGET := libcstd_safety.so 

all:${OBJS}
	$(CC) $(CFLAGS) -shared -fPIC -o $(TARGET) ${OBJS} $(LIBS) 
	$(STRIP) $(TARGET) 
	@cp $(TARGET) ./lib/

clean:
	@find . -name "*.o"  | xargs rm -f
	@rm -rf $(TARGETS)

