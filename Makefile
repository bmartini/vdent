# The default configuration is "release" if invoked with just "make":
ifeq ($(MODE),)
    MODE=release
endif


# The directories containing the source files, separated by ':'
VPATH=.:src/


# The source files: regardless of where they reside in the source tree,
# VPATH will locate them...
SOURCE = \
	Indenter.cpp \
	StreamHandler.cpp \
	verilog-indent.cpp


# Build a Dependency list and an Object list, by replacing the .cpp
# extension to .d for dependency files, and .o for object files.
DEPENDENCY = $(patsubst %.cpp, $(MODE)/deps/%.d, $(filter %.cpp, $(SOURCE)))
OBJECTS = $(patsubst %.cpp, $(MODE)/%.o, $(filter %.cpp, $(SOURCE)))


# Name of final binary
TARGET=vdent


# What compiler to use for generating dependencies: it will be invoked with -MM -MP
CXXDEP = g++
CXX = g++


# What include flags to pass to the compiler
ifeq ($(MODE),debug)
    INCLUDEFLAGS=
else
    INCLUDEFLAGS=
endif


# Separate compile options per configuration
ifeq ($(MODE),debug)
    CXXFLAGS += -O0 -Wall -DDEBUG -g ${INCLUDEFLAGS}
else
    CXXFLAGS += -O2 -Wall ${INCLUDEFLAGS}
endif


# A common link flag for all configurations
ifeq ($(MODE),debug)
    LDFLAGS = 
else
    LDFLAGS = 
endif


.PHONY: all 
all:	inform bin/${TARGET}


inform:
ifneq ($(MODE),release)
ifneq ($(MODE),debug)
	@echo "Invalid configuration "$(MODE)" specified."
	@echo "To specify a configuration mode when running make, the syntax to use is:"
	@echo "make MODE=debug"
	@echo  
	@echo  "Possible choices for configuration are 'release' and 'debug'"
	@exit 1
endif
endif
	@echo "Configuration "$(MODE)
	@echo "------------------------"


bin/${TARGET}: ${OBJECTS} | inform
	@mkdir -p bin 
	$(CXX) -g -o $@ $^ ${LDFLAGS}


$(MODE)/%.o: %.cpp
	@mkdir -p $(MODE)
	$(CXX) -c $(CXXFLAGS) -o $@ $<


$(MODE)/deps/%.d: %.cpp
	@mkdir -p $(MODE)/deps
	@echo Generating dependencies for $<
	@set -e ; $(CXXDEP) -MM -MP $(INCLUDEFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,$(MODE)\/\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$


.PHONY: clean
clean:
	@rm -rf debug/* release/* bin/$(TARGET)


# Unless "make clean" is called, include the dependency files
# which are auto-generated. Don't fail if they are missing
# (-include), since they will be missing in the first invocation!
ifneq ($(MAKECMDGOALS),clean)
    -include ${DEPENDENCY}
endif
