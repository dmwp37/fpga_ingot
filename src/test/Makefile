# this turns off the suffix rules built into make
.SUFFIXES:

SHELL         := /bin/sh                                                                                                                                                                              


# automatic rules for collecting and assembling all possible source
# files.

# obj output dir
OBJ_DIR       := obj

INC_DIRS      := -I../../hdr -I../

CFLAGS        := $(INC_DIRS) -g -Wall -Werror -msse2 -D_GNU_SOURCE
LD_FLAGS      := -L.. -lfpga_net -lgdb_print -lpthread -lrt

# target define
TARGET        := fpga_test

# source files
SRC_FILES     := $(wildcard *.c)

C_SRC_FILES   := $(filter %.c, $(SRC_FILES))
CPP_SRC_FILES := $(filter %.cpp, $(SRC_FILES))

# obj files
C_OBJ_FILES   := $(C_SRC_FILES:%.c=$(OBJ_DIR)/%.o)
CPP_OBJ_FILES := $(CPP_SRC_FILES:%.cpp=$(OBJ_DIR)/%.o)

OBJ_FILES     := $(C_OBJ_FILES) $(CPP_OBJ_FILES)

# Dep generated files
C_DEPS        := $(C_SRC_FILES:%.c=$(OBJ_DIR)/%.d)
CPP_DEPS      := $(CPP_SRC_FILES:%.cpp=$(OBJ_DIR)/%.d)
DEPS          := $(C_DEPS) $(CPP_DEPS)


.PHONY: all clean deploy

# This is the default target.  It must be the first declared target.
all: $(TARGET)

#
# rules definition
#

# clean
clean:
	@$(MAKE) -C ../ clean
	@$(RM) $(OBJ_FILES) $(TARGET)
	@$(RM) -r $(OBJ_DIR)

install: $(TARGET)
	$(INSTALL_DIR) $(bindir)
	$(INSTALL_PRG) $(TARGET) $(bindir)

uninstall:
	$(RM) $(bindir)/$(TARGET)

#
# Rules for generating target
#
$(TARGET): $(OBJ_DIR) $(OBJ_FILES) ../libfpga_net.a
	@echo "generating $@ ..."
	@$(CC) -g -o $@ $(OBJ_FILES) $(LD_FLAGS) 

../libfpga_net.a:
	@$(MAKE) -C ../

#
# Rules for compiling cpp code
#

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

$(C_OBJ_FILES): $(OBJ_DIR)/%.o: %.c
	@echo "compiling $< ..."
	@$(CC) $(CFLAGS) -o $@ -c $<

$(CPP_OBJ_FILES): $(OBJ_DIR)/%.o: %.cpp
	@echo "compiling $< ..."
	@$(CC) $(CFLAGS) -o $@ -c $<

#
# Special rules for generating dependencies
#

dep_dir: $(OBJ_DIR)

$(C_DEPS):$(OBJ_DIR)/%.d: %.c
#	@echo Generating dependencies: $@ ...
	@set -e; $(CC) -MM $(CFLAGS) $< | sed -e 's#^\($*\).o:#$(OBJ_DIR)/\1.o $@:#' > $@

$(CPP_DEPS):$(OBJ_DIR)/%.d: %.cpp
#	@echo Generating dependencies: $@ ...
	@set -e; $(CC) -MM $(CFLAGS) $< | sed -e 's#^\($*\).o:#$(OBJ_DIR)/\1.o $@:#' > $@

# Dep include
is_clean := $(filter clean clean-all, $(MAKECMDGOALS))
ifeq ($(is_clean),)
  -include $(DEPS) dep_dir
endif

