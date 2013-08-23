CC     := gcc

LDFLAGS:= -shared
LIBS   := $(shell pkg-config --libs glib-2.0)
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


# Mandatory requirement: GLib-2.0, pkg-config 0.26
# Need to add a check for this later. Best place to do 
# this check will be a configure.ac once we have 
# autoconf setup.
CFLAGS := $(shell pkg-config --cflags glib-2.0) \
	$(INCLUDES) -Wall -Wextra -g -fPIC

.PHONY: objects
objects: $(OBJS)

$(OBJS): | $(OBJDIR)

$(OBJDIR): 
	mkdir $(OBJDIR)

$(OBJDIR)/%.o : $(SRCDIR)/%.c 
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: dryrun
dryrun:
	@echo "$(MAKE)"
	@echo "$(CC) $(CFLAGS)"
	@echo "$(OBJS)"
	@echo "$(SRCS)"


.PHONY: all
all: $(TARGET_LIB)

$(TARGET_LIB):$(OBJS)
	@echo "	$(CC) $(LIBS) $(LDFLAGS) $(OBJS) -o $(TARGET_LIB)"
	$(CC) $(LIBS) $(LDFLAGS) $(OBJS) -o $(TARGET_LIB)


.PHONY: clean
clean: 
	$(RM) $(TARGET_LIB) $(OBJS)
	$(RM) -r $(OBJDIR)
