CFLAGS := -fPIC -O3 -g -Wall -Werror -I../include/ -I/usr/include/glib-2.0/ -I/usr/lib/glib2-0/include/ -I/usr/include/glib-2.0/glib/
LFLAGS := -lglib-2.0
CC := gcc
MAJOR := 0
MINOR := 1
NAME := cc_onf_driver
VERSION := $(MAJOR).$(MINOR)
LIB_SONAME := lib$(NAME).so
LIB_REALNAME := $(LIB_SONAME).$(VERSION)

# Add new source files here.
# cc_onf_driver.c is a temp file. Remove this file and this comment when you add
# new files here.
# SRC = a.c \
#       b.c \
#       d.c
SRC = cc_onf_driver.c
OBJS = $(patsubst %.c,obj/%.o,$(SRC))

lib: $(LIB_SONAME)

$(LIB_SONAME): $(LIB_REALNAME)
	ldconfig -v -n .
	ln -s $(LIB_SONAME).$(MAJOR) $(LIB_SONAME)
 
$(LIB_REALNAME): $(OBJS)
	@echo Creating shared library.
	$(CC) -shared -Wl,-soname,$(LIB_SONAME).$(MAJOR) $^ -o $@ $(LFLAGS)

$(OBJS) : | obj

obj:
	@mkdir -p $@

obj/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJS) $(LIB_SONAME) $(LIB_REALNAME) $(LIB_SONAME).$(MAJOR)
