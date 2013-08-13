CC     := gcc

LDFLAGS:= -shared
LIBS   := -lglib-2.0
RM     := rm -f

TARGET_LIB = libccof.so

#set environment variable $MAKE to the correct
# binary. Useful when there are multiple versions
# installed. 
# Mandatory requirement: 3.82 version of make
ifndef MAKE
	MAKE := make
endif
ROOTDIR := $(CURDIR)
SRCDIR  := $(ROOTDIR)/src
INCLDIR := $(ROOTDIR)/include
TESTDIR := $(ROOTDIR)/tests
OBJDIR  := $(ROOTDIR)/obj
DOCDIR  := $(ROOTDIR)/doc

INCLUDES := -I$(ROOTDIR)/include
SRCS     := $(wildcard $(SRCDIR)/*.c)
OBJS     := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))

CFLAGS := -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include \
	  $(INCLUDES) -Wall -Wextra -g # -fpic vs -fPIC ??


.PHONY: objects
objects: $(OBJS)

$(OBJS): | $(OBJDIR)

$(OBJDIR): 
	mkdir $(OBJDIR)

$(OBJS) : $(SRCS)
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: dryrun
dryrun:
	@echo "$(MAKE)"
	@echo "$(CC) $(CFLAGS)"
	@echo "$(OBJS)"
	@echo "$(SRCS)"


.PHONY: tlib
tlib: $(TARGET_LIB)

$(TARGET_LIB):$(OBJS)
	@echo "	$(CC) $(LIBS) $(LDFLAGS) $(OBJS) -o $(TARGET_LIB)"
	$(CC) $(LIBS) $(LDFLAGS) $(OBJS) -o $(TARGET_LIB)

.PHONY: all
all:
	$(MAKE) objects
	$(MAKE) tlib


.PHONY: clean
clean: 
	$(RM) $(TARGET_LIB) $(OBJS)
	$(RM) -r $(OBJDIR)