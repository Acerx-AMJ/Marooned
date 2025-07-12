# Compiler and flags
CC = g++
CFLAGS = -std=c++17 -Wall -Wextra -O2

# Source and objects
SRC = $(wildcard src/*.cpp)
OBJ = $(SRC:.cpp=.o)

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

clean:
	del /Q src\*.o $(OUT)



# CC = g++
# CFLAGS = -O2 -Wall -std=c++17
# INCLUDE = -IC:\raylib\raylib\src
# LIBS = -LC:\raylib\raylib\src -lraylib -lopengl32 -lgdi32 -lwinmm

# SRC =  $(wildcard src/*.c src/*.cpp)
# OUT = Marooned.exe

# all:
# 	$(CC) $(CFLAGS) $(INCLUDE) $(SRC) -o $(OUT) $(LIBS)

