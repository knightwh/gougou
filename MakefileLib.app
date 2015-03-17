APP=TermMeta
SRC=src/$(APP).cpp
OBJ=lib/$(APP).o

INC = ./include

CXX = g++
CXXFLAGS = -g -I$(INC) -c -Wall
CPPLDFLAGS = -lz -lpthread -lm

all:
	$(CXX) $(CXXFLAGS) $(SRC) -o $(OBJ)

clean:
	rm -f $(OBJ)
