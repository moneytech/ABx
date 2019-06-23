include makefile.common

# This may change
INCLUDES += -I../abscommon/abscommon -I/usr/include/postgresql
TARGETDIR = ../Lib/x64/Release
TARGET = $(TARGETDIR)/libabdb.a
SOURDEDIR = ../abdb/abdb
OBJDIR = obj/x64/Release/abdb
SRC_FILES = \
	$(SOURDEDIR)/Database.cpp \
	$(SOURDEDIR)/DatabaseMysql.cpp \
	$(SOURDEDIR)/DatabasePgsql.cpp \
	$(SOURDEDIR)/DatabaseSqlite.cpp \
	$(SOURDEDIR)/stdafx.cpp \
# End changes

CXXFLAGS += $(DEFINES) $(INCLUDES)

OBJ_FILES := $(patsubst $(SOURDEDIR)/%.cpp, $(OBJDIR)/%.o, $(SRC_FILES))
#$(info $(OBJ_FILES))

all: $(TARGET)

$(TARGET): $(OBJ_FILES)
	@$(MKDIR_P) $(@D)
	$(LINKCMD_LIB) $(OBJ_FILES)

$(OBJDIR)/%.o: $(SOURDEDIR)/%.cpp
	@$(MKDIR_P) $(@D)
	$(CXX) $(CXXFLAGS) -MMD -c $< -o $@

-include $(OBJ_FILES:.o=.d)

.PHONY: clean
clean:
	rm -f $(OBJ_FILES) $(TARGET)
