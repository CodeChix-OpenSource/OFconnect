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
TESTSRCS := $(wildcard $(TESTDIR)/*.c)
TESTOBJS := $(patsubst $(TESTDIR)/%.c,$(TESTDIR)/%.o,$(TESTSRCS))
TESTPROGS:= $(patsubst $(TESTDIR)/%.c,$(TESTDIR)/%.exe,$(TESTSRCS))
TESTXML  := $(patsubst $(TESTDIR)/%.c,$(TESTDIR)/log-%.xml,$(TESTSRCS))
TESTHTML := $(patsubst $(TESTDIR)/%.c,$(TESTDIR)/log-%.html,$(TESTSRCS))

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


all: $(REALNAME)

$(REALNAME):$(OBJS)
	@echo "$(CC) $(LIBS) $(LDFLAGS) $(OBJS) -o $(REALNAME) -lc"
	$(CC) $(LIBS) $(LDFLAGS) $(OBJS) -o $(REALNAME) -lc


.PHONY: help
help:
	@echo "make objects  : build library objects"
	@echo "make all      : compile and link library"
	@echo "make install  : install library"
	@echo "                Needs root permissions"
	@echo "make clean    : cleans all object files of library"
	@echo "make test     : compile all tests"
	@echo "make runtest  : execute all tests"
	@echo "                Creates xml and html log files"
	@echo "make cleantest: cleans all temporary test files"
	@echo "                including log files"

# make objects

objects: $(OBJS)

$(OBJS): | $(OBJDIR)

$(OBJDIR): 
	mkdir $(OBJDIR)

$(OBJDIR)/%.o : $(SRCDIR)/%.c 
	$(CC) $(CFLAGS) -c $< -o $@


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



$(TESTDIR)/%.o : $(TESTDIR)/%.c
	$(CC) -c $< -o $@ $(TCFLAGS)


$(TESTDIR)/%.exe : $(TESTDIR)/%.o
	$(CC) $(OBJS) $< -o $@ $(LIBS)


# make test
test: $(TESTPROGS) 

$(TESTPROGS) : $(TESTOBJS)

$(TESTOBJS) : $(OBJS)

$(TESTDIR)/log-%.xml : $(TESTDIR)/%.exe
	@echo  "gtester"
	gtester --verbose -o=$@ -k $<

$(TESTDIR)/log-%.html : $(TESTDIR)/log-%.xml
	@echo  "gtester"
	#use sed to update the xml file
	sed "\|</gtester>| i \
	  <info>\
	    <package>TEST PACKAGE</package>\
	    <version>0</version>\
	    <revision>1</revision>\
	  </info>\
	" --in-place='' $<
	gtester-report $< > $@

.PHONY : runtest
runtest: $(TESTHTML)

$(TESTHTML) : $(TESTXML)

.PHONY : cleantest
cleantest:
	$(RM) $(TESTOBJS) $(TESTPROGS) $(TESTXML) $(TESTHTML)


.PHONY: clean
clean: 
	$(RM) $(REALNAME) $(OBJS)
	$(RM) -r $(OBJDIR)
	$(RM) $(TESTOBJS) $(TESTPROGS)

