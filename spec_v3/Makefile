CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
TARGET = spec_v3_runner

SRC = main.cpp board.cpp draw_board.cpp \
      negamax.cpp mcts.cpp greedy.cpp input_player.cpp
OBJ = $(SRC:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $(TARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f $(OBJ) $(TARGET)

run: all
	./$(TARGET)
