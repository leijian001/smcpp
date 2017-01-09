TARGET := test

SRC := $(wildcard *.cpp)
OBJ := $(SRC:.cpp=.o)

CC := g++

CFLAG := -g --std=c++11

$(TARGET): $(OBJ)
	$(CC) -o $(TARGET) $(OBJ)

%.o: %.cpp
	$(CC) $(CFLAG) -c $< -o $@

.PHONY: clean run
clean:
	-rm -rf $(OBJ)

run:
	./$(TARGET)
