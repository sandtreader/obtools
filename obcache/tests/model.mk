#==========================================================================
# Standard Makefile for ObTools ObCache generated code
# Work without the ObTools build environment
# Include from test directory, defining XMI to source XMI file
# 
# Copyright (c) 2008 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

# Set DEV for development within the ObCache world, otherwise
# it uses installed packages
DEV=1

# Build directory
BUILD-DIR=build

# Code generation tools
ifdef DEV

# Spot Mac build and set suffix
OSNAME=$(shell uname -s)
ifeq ($(OSNAME), Darwin)
VARIANT-SUFFIX = -osx
endif

# Development versions using development config
# Note this is relative to build directory
GEN=../../../gen
OT-GENERATE-CORE-CPP=$(GEN)/core/build-debug$(VARIANT-SUFFIX)/ot-generate-core-cpp $(GEN)/core/ot-generate-core-cpp.cfg.xml
OT-GENERATE-SQL-SCHEMA=$(GEN)/sql/build-debug$(VARIANT-SUFFIX)/ot-generate-sql-schema $(GEN)/sql/ot-generate-sql-schema.cfg.xml
OT-GENERATE-SQL-CPP=$(GEN)/sql/build-debug$(VARIANT-SUFFIX)/ot-generate-sql-cpp $(GEN)/sql/ot-generate-sql-cpp.cfg.xml

else

# Installed versions, using global config
OT-GENERATE-CORE-CPP=ot-generate-core-cpp
OT-GENERATE-SQL-SCHEMA=ot-generate-sql-schema
OT-GENERATE-SQL-CPP=ot-generate-sql-cpp

endif

# Main target
all: build-dir generate sub-makefile sub-build

# Create build directory
build-dir:
	-@mkdir build

# Generate code inside build directory
generate:
	cd $(BUILD-DIR); $(OT-GENERATE-CORE-CPP) < ../$(XMI)    
	cd $(BUILD-DIR); $(OT-GENERATE-SQL-SCHEMA) < ../$(XMI)
	cd $(BUILD-DIR); $(OT-GENERATE-SQL-CPP) < ../$(XMI)        

# Create Makefile for build directory
sub-makefile:
	cp ../build.mk $(BUILD-DIR)/Makefile

# Call to constructed Makefile
sub-build:
	$(MAKE) -C $(BUILD-DIR)

# Clean up
clean:
	-@rm -rf *~ $(BUILD-DIR)






