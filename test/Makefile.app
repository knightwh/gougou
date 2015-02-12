APP=IndexBuilderBase
SRC=code/$(APP).cpp
OBJ=bin/$(APP)

INC = ../include
LIB = ../lib

CXX = g++
CXXFLAGS = -g -I$(INC) -Wall
CPPLDFLAGS = -lz -lpthread -lm

all:
	$(CXX) $(CXXFLAGS) $(LIB)/* $(SRC) -o $(OBJ)



clean:
	rm -f $(OBJ)
