CC     := gcc

LDFLAGS:= -shared
LIBS   := -lglib-2.0
RM     := rm -f

TARGET_LIB = libccof.so

MAKE    := make
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
	@echo "$(CC) $(CFLAGS)"
	@echo "$(OBJS)"


.PHONY: tlib
tlib: $(TARGET_LIB)

$(TARGET_LIB):$(OBJS)
	$(CC) $(LIBS) $(LDFLAGS) $(OBJS) -o $(TARGET_LIB) 

.PHONY: all
all:
	$(MAKE) objects
	$(MAKE) tlib


.PHONY: clean
clean: 
	$(RM) $(TARGET_LIB) $(OBJS)
	$(RM) -r $(OBJDIR)