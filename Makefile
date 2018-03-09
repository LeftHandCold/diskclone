# GNU Makefile

build ?= release

OUT := build/$(build)

default: all


# Do not specify CFLAGS or LIBS on the make invocation line - specify
# XCFLAGS or XLIBS instead. Make ignores any lines in the makefile that
# set a variable that was set on the command line.
CFLAGS += $(XCFLAGS) -Iinclude
LIBS += $(XLIBS) -lm

LIBS += $(ZLIB_LIBS)

CFLAGS += $(ZLIB_CFLAGS)

ALL_DIR += $(OUT)/source/hdtd
ALL_DIR += $(OUT)/source/dos
ALL_DIR += $(OUT)/source/gpt
ALL_DIR += $(OUT)/source/ntfs

# --- Commands ---

ifneq "$(verbose)" "yes"
QUIET_AR = @ echo ' ' ' ' AR $@ ;
QUIET_CC = @ echo ' ' ' ' CC $@ ;
QUIET_CXX = @ echo ' ' ' ' CXX $@ ;
QUIET_GEN = @ echo ' ' ' ' GEN $@ ;
QUIET_LINK = @ echo ' ' ' ' LINK $@ ;
QUIET_MKDIR = @ echo ' ' ' ' MKDIR $@ ;
QUIET_RM = @ echo ' ' ' ' RM $@ ;
QUIET_TAGS = @ echo ' ' ' ' TAGS $@ ;
QUIET_WINDRES = @ echo ' ' ' ' WINDRES $@ ;
endif

CC_CMD = $(QUIET_CC) $(CC) $(CFLAGS) -o $@ -c $<
CXX_CMD = $(QUIET_CXX) $(CXX) $(filter-out -Wdeclaration-after-statement,$(CFLAGS)) -o $@ -c $<
AR_CMD = $(QUIET_AR) $(AR) cr $@ $^
LINK_CMD = $(QUIET_LINK) $(CC) $(LDFLAGS) -o $@ $^ $(LIBS)
MKDIR_CMD = $(QUIET_MKDIR) mkdir -p $@
RM_CMD = $(QUIET_RM) rm -f $@
TAGS_CMD = $(QUIET_TAGS) ctags $^
WINDRES_CMD = $(QUIET_WINDRES) $(WINDRES) $< $@

# --- Rules ---

$(ALL_DIR) $(OUT) :
	$(MKDIR_CMD)

$(OUT)/%.a :
	$(AR_CMD)

$(OUT)/%.exe: $(OUT)/%.o | $(ALL_DIR)
	$(LINK_CMD)


$(OUT)/%.o : %.c | $(ALL_DIR)
	$(CC_CMD)

.PRECIOUS : $(OUT)/%.o # Keep intermediates from chained rules

# --- File lists ---

HDTDHDR := include/hdtd.h $(wildcard include/hdtd/*.h)
DOS_HDR := include/dos.h $(wildcard include/dos/*.h)
GPT_HDR := include/gpt.h $(wildcard include/gpt/*.h)
NTFS_HDR := include/ntfs.h $(wildcard include/ntfs/*.h)

HDTDSRC := $(sort $(wildcard source/hdtd/*.c))
DOS_SRC := $(sort $(wildcard source/dos/*.c))
GPT_SRC := $(sort $(wildcard source/gpt/*.c))
NTFS_SRC := $(sort $(wildcard source/ntfs/*.c))

HDTDSRC_HDR := $(wildcard source/hdtd/*.h)
DOS_SRC_HDR := $(wildcard source/dos/*.h)
GPT_SRC_HDR := $(wildcard source/gpt/*.h)
NTFS_SRC_HDR := $(wildcard source/ntfs/*.h)

HDTDOBJ := $(HDTDSRC:%.c=$(OUT)/%.o)
DOS_OBJ := $(DOS_SRC:%.c=$(OUT)/%.o)
GPT_OBJ := $(GPT_SRC:%.c=$(OUT)/%.o)
NTFS_OBJ := $(NTFS_SRC:%.c=$(OUT)/%.o)

$(HDTDOBJ) : $(HDTDHDR) $(HDTDSRC_HDR)
$(DOS_OBJ) : $(HDTDHDR) $(DOS_HDR) $(DOS_SRC_HDR)
$(GPT_OBJ) : $(HDTDHDR) $(GPT_HDR) $(GPT_SRC_HDR)
$(NTFS_OBJ) : $(HDTDHDR) $(NTFS_HDR) $(NTFS_SRC_HDR)
$(DOS_OBJ) : $(HDTDSRC_HDR) 
$(GPT_OBJ) : $(HDTDSRC_HDR) 
$(NTFS_OBJ) : $(HDTDSRC_HDR) 


# --- Library ---

HDDISKCLOEN_LIB = $(OUT)/libdc.a

MHDDISKCLOEN_OBJ := \
	$(HDTDOBJ) \
	$(DOS_OBJ) \
	$(GPT_OBJ) \
	$(NTFS_OBJ) \

$(HDDISKCLOEN_LIB) : $(MHDDISKCLOEN_OBJ)

INSTALL_LIBS := $(HDDISKCLOEN_LIB)

libs: $(HDDISKCLOEN_LIB)

# --- Clean and Default ---


all: libs

clean:
	rm -rf $(OUT)

release:
	$(MAKE) build=release
debug:
	$(MAKE) build=debug

.PHONY: all clean libs
