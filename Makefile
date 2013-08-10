CC     := gcc
CFLAGS := -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -Wall -Wextra -g # -fpic vs -fPIC ??
LDFLAGS:= -shared
LIBS   := -lglib-2.0
RM     := rm -f

TARGET_LIB = libccof.so

ROOTDIR := $(CURDIR)
#ROOTDIR := /Users/edeedhu/hub/forked/CC-ONF-driver
SRCDIR  := $(ROOTDIR)/src
INCLDIR := $(ROOTDIR)/include
TESTDIR := $(ROOTDIR)/tests
OBJDIR  := $(ROOTDIR)/obj
DOCDIR  := $(ROOTDIR)/doc


SRCS     := $(wildcard $(SRCDIR)/*.c)
OBJS     := $(SRCS:%.c=%.o)
INCLUDES := -I

.PHONY: all
all: $(TARGET_LIB)

#------------------------------
#$(OBJS):$(SRCS)
#	$(CC) $(CFLAGS) $< $@

#$(OBJS): | $(OBJDIR)

#$(OBJDIR):
#	mkdir $(OBJDIR)
#------------------------------

#$(TARGET_LIB):$(OBJS)
#	$(CC) $(LDFLAGS) -o $@ $^

#$(SRCS:.c=.d):%.d:%.c
#	$(CC) $(CFLAGS) -MM $@ $<
#include $(SRCS:.c=.o)

#$(OBJS):$(SRCS)
#	$(CC) $(INCLUDES) $(CFLAGS) -o $(OBJS) -c $(SRCS)

$(SRCS .c.o):
	$(CC) $(INCLUDES) $(CFLAGS) -c $< -o $@
	
$(TARGET_LIB):$(OBJS)
	$(CC) $(LIBS) $(LDFLAGS) $(OBJS) -o $(TARGET_LIB) 

#$(OBJS): $(SRCS)
#	$(CC) $(CFLAGS) -o $@ -c $<

.PHONY: clean
clean: $(RM) $(TARGET_LIB) $(OBJS) $(SRCS:.c=.d) # $(OBJDIR)