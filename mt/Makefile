CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -pthread
# Runtime: jedan parametar N (7–15), tabla N x N. Veći niz pri kompilaciji:
#   CXXFLAGS += -DBOARD_ROWS_MAX=24 -DBOARD_COLS_MAX=24
TARGET = mt_runner

SRC = main.cpp mcts.cpp negamax.cpp board.cpp
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