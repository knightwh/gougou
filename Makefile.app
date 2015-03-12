APP=BuildIndex
SRC=code/$(APP).cpp
OBJ=bin/$(APP)

INC = ./include
LIB = ./lib

CXX = g++
CXXFLAGS = -O3 -I$(INC)
CPPLDFLAGS = -lz -lpthread -lm

all:
	$(CXX) $(CXXFLAGS) $(LIB)/* $(SRC) -o $(OBJ)



clean:
	rm -f $(OBJ)
