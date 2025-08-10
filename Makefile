# Compiler and common flags
CC      := g++
CXXFLAGS:= -std=c++17 -Wall -Wextra -O2 -MMD -MP

# Sources / objects
SRC := $(wildcard src/*.cpp)
OBJ := $(SRC:.cpp=.o)
DEP := $(OBJ:.o=.d)

# ===== Platform detection =====
UNAME_S := $(shell uname -s)

# Default output name (overridden below)
OUT := Marooned

ifeq ($(OS),Windows_NT)                       # Windows (CMD/PowerShell/MSYS)
    EXT     := .exe
    OUT     := $(OUT)$(EXT)
    LDLIBS  := -lraylib -lopengl32 -lgdi32 -lwinmm
else                                          # Unixy (Linux/macOS/MSYS)
    # Prefer pkg-config if raylib is installed
    ifeq ($(shell pkg-config --exists raylib && echo yes),yes)
        CXXFLAGS += $(shell pkg-config --cflags raylib)
        LDLIBS   := $(shell pkg-config --libs raylib)
    else
        # Fallback: common Linux libs for raylib
        LDLIBS := -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
        # (Depending on distro you might also need: -lXrandr -lXi -lXcursor -lXinerama)
    endif
endif

# ===== Build rules =====
all: $(OUT)

$(OUT): $(OBJ)
	$(CC) $(CXXFLAGS) -o $@ $^ $(LDLIBS)

src/%.o: src/%.cpp
	$(CC) $(CXXFLAGS) -c $< -o $@

-include $(DEP)

.PHONY: clean
clean:
	rm -f src/*.o src/*.d $(OUT)
