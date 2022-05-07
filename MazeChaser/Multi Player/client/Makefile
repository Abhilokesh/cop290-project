#OBJS specifies which files to compile as part of the project
OBJS = 30_scrolling.cpp

#CC specifies which compiler we're using
CC = g++

#COMPILER_FLAGS specifies the additional compilation options we're using
# -w suppresses all warnings
COMPILER_FLAGS = -g -w -std=c++17 -O2 -Wall -Wextra -pedantic -Wformat=2 -Wstrict-aliasing=2 -MMD

#LINKER_FLAGS specifies the libraries we're linking against
LINKER_FLAGS = $(shell sdl2-config --cflags --libs) -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lenet

#OBJ_NAME specifies the name of our exectuable
OBJ_NAME = client
ifeq ($(shell uname -s),Darwin)
	LINKER_FLAGS += -I/usr/local/include
endif

#This is the target that compiles our executable
all : $(OBJS)
	$(CC) $(OBJS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)
