LIB := lib/
TESTS := tests/

DIRS = $(LIB) $(TESTS)
CLEANDIRS = $(DIRS:%=clean-%)

all: $(DIRS)

$(DIRS):
	$(MAKE) -C $@

$(TESTS): $(LIB)

clean: $(CLEANDIRS)
$(CLEANDIRS):
	$(MAKE) -C $(@:clean-%=%) clean

.PHONY: subdirs $(DIRS)
.PHONY: subdirs $(CLEANDIRS)
.PHONY: all clean
