# Compiler and flags
CC = g++
CFLAGS = -std=c++17 -Wall -Wextra -O2 -MMD

# Source and objects
SRC = $(wildcard src/*.cpp)
OBJ = $(SRC:.cpp=.o)
DEP = $(OBJ:.o=.d)

# Output
OUT = Marooned.exe

# Include paths (default system include)
INCLUDE_PATHS =

# Linker flags for Windows + Raylib 5
LDFLAGS = -lraylib -lopengl32 -lgdi32 -lwinmm

# Build
all: $(OUT)

$(OUT): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDE_PATHS) -c $< -o $@

-include $(DEP)

clean:
	rm -f src/*.o src/*.d $(OUT)
