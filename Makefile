CC = g++
CFLAGS = -O2 -Wall -std=c++17
INCLUDE = -IC:\raylib\raylib\src
LIBS = -LC:\raylib\raylib\src -lraylib -lopengl32 -lgdi32 -lwinmm

SRC =  $(wildcard src/*.cpp)
OUT = Marooned.exe

all:
	$(CC) $(CFLAGS) $(INCLUDE) $(SRC) -o $(OUT) $(LIBS)
