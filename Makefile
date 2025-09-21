CXX = g++
CXXFLAGS = -std=c++17 -O2 -Iinclude
PKG_CFLAGS := $(shell pkg-config --cflags glfw3)
PKG_LIBS := $(shell pkg-config --libs glfw3)
LIBS = $(PKG_LIBS) -lGL -ldl -pthread

SRC_DIR = src
EXES = red_triangle blue_square shapes creative

.PHONY: all clean

all: $(EXES)

%: $(SRC_DIR)/%.cpp $(SRC_DIR)/glad.c
	$(CXX) $(CXXFLAGS) $(PKG_CFLAGS) $^ -o $@ $(LIBS)

clean:
	rm -f $(EXES)

