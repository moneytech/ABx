include makefile.common

# This may change
DEFINES += -D_CONSOLE
INCLUDES += -I../abscommon/abscommon
TARGETDIR = ../Bin
TARGET = $(TARGETDIR)/abfile
SOURDEDIR = ../abfile/abfile
OBJDIR = obj/x64/Release/abfile
LIBS += -lpthread -luuid -llua5.3 -labscommon -lssl -lcrypto -labcrypto -lstdc++fs -lpugixml
CXXFLAGS += -fexceptions
PCH = $(SOURDEDIR)/stdafx.h
CXXFLAGS += -Werror -Wimplicit-fallthrough=0
# End changes

SRC_FILES = $(filter-out $(SOURDEDIR)/stdafx.cpp, $(wildcard $(SOURDEDIR)/*.cpp))

CXXFLAGS += $(DEFINES) $(INCLUDES)

OBJ_FILES := $(patsubst $(SOURDEDIR)/%.cpp, $(OBJDIR)/%.o, $(SRC_FILES))
#$(info $(OBJ_FILES))

GCH = $(PCH).gch

all: $(TARGET)

$(TARGET): $(GCH) $(OBJ_FILES)
	@$(MKDIR_P) $(@D)
	$(LINKCMD_EXE) $(OBJ_FILES) $(LIBS)

$(OBJDIR)/%.o: $(SOURDEDIR)/%.cpp
	@$(MKDIR_P) $(@D)
	$(CXX) $(CXXFLAGS) -MMD -c $< -o $@

# PCH
$(GCH): $(PCH)
	$(CXX) -x c++-header $(CXXFLAGS) -c $< -o $@

-include $(OBJ_FILES:.o=.d)

.PHONY: clean
clean:
	rm -f $(GCH) $(OBJ_FILES) $(TARGET)