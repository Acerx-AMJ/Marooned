CC := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2 -MMD -MP -Iinclude
SRC := $(wildcard src/*.cpp) $(wildcard src/**/*.cpp)
OBJ := $(patsubst src/%.cpp, build/%.o, $(SRC))
DEP := $(OBJ:.o=.d)

UNAME_S := $(shell uname -s)
OUT := build/marooned

ifeq ($(OS),Windows_NT)
	OUT := $(OUT).exe
	LDLIBS := -lraylib -lopengl32 -lgdi32 -lwinmm
else
	ifeq ($(shell pkg-config --exists raylib && echo yes), yes)
		CXXFLAGS += $(shell pkg-config --cflags raylib)
		LDLIBS := $(shell pkg-config --libs raylib)
	else
	# Depending on your distro you might also need -lXrandr -lXi -lXcursor -lXinerama
		LDLIBS := -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
	endif
endif

all: create_build_dir $(OUT)

create_build_dir:
	mkdir -p build

$(OUT): $(OBJ)
	$(CC) $(CXXFLAGS) -o $@ $^ $(LDLIBS)

build/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CXXFLAGS) -c $< -o $@
build/%/%.o: src/%/%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CXXFLAGS) -c $< -o $@

-include $(DEP)

.PHONY: clean
clean:
	rm -rf build/*
