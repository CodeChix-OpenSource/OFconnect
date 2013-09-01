CC     := gcc

LDFLAGS := -shared
LIBS   := $(shell pkg-config --libs glib-2.0)
RM     := rm -f
MAJOR_VERSION := 0
MINOR_VERSION := 0
NAME     := ccof
LIBNAME  := lib$(NAME).so
SONAME   := $(LIBNAME).$(MAJOR_VERSION)
REALNAME := $(SONAME).$(MINOR_VERSION)
COMLIB   := /usr/local/lib
COMINCL  := /usr/local/include

#set environment variable $MAKE to the correct
# binary. Useful when there are multiple versions
# installed. 
ifndef MAKE
	MAKE := make
endif
CCDIR   := $(CURDIR)
SRCDIR  := $(CCDIR)/src
INCLDIR := $(CCDIR)/include
TESTDIR := $(CCDIR)/tests
OBJDIR  := $(CCDIR)/obj
DOCDIR  := $(CCDIR)/doc

INCLUDES := -I$(CCDIR)/include
SRCS     := $(wildcard $(SRCDIR)/*.c)
OBJS     := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))


# Mandatory requirement: GLib-2.0, pkg-config 0.26
# Need to add a check for this later. Best place to do 
# this check will be a configure.ac once we have 
# autoconf setup.
CFLAGS := $(shell pkg-config --cflags glib-2.0) \
	$(INCLUDES) -Wall -Wextra -g -fPIC \
	-Wl,-export-dynamic
LDFLAGS:= -shared -Wl,-soname,$(SONAME)

TCFLAGS := $(shell pkg-config --cflags glib-2.0) \
	$(INCLUDES) -Wall -Wextra -g 

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
all: $(REALNAME)

$(REALNAME):$(OBJS)
	@echo "$(CC) $(LIBS) $(LDFLAGS) $(OBJS) -o $(REALNAME) -lc"
	$(CC) $(LIBS) $(LDFLAGS) $(OBJS) -o $(REALNAME) -lc


.PHONY: clean
clean: 
	$(RM) $(REALNAME) $(OBJS)
	$(RM) -r $(OBJDIR)

.PHONY: install
#install needs sudo permissions
#cp libccof.so.0.0 /usr/local/lib/
#ldconfig -n -v /usr/local/lib/
#ln -sf /usr/local/lib/libccof.so.0 /usr/local/lib/libccof.so
#cp include/cc_of_lib.h /usr/local/include/
install:
	cp $(REALNAME) $(COMLIB)/
	ldconfig -n -v $(COMLIB)
	ln -sf $(COMLIB)/$(SONAME) $(COMLIB)/$(LIBNAME)
	cp $(INCLDIR)/cc_of_lib.h $(COMINCL)/

.PHONY: test
test: $(OBJS)
	$(CC) -c tests/pollthr_test.c -o tests/pollthr_test.o $(TCFLAGS)
	$(CC) $(OBJS) tests/pollthr_test.o -o tests/pollthr_test $(LIBS)