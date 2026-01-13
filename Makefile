CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -I./include
LDFLAGS = -lcurl -ltinyxml2

SRC_DIR = src
INCLUDE_DIR = include
OBJ_DIR = obj
BIN_DIR = bin

SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
TARGET = $(BIN_DIR)/mpc_streaming

all: directories $(TARGET)

directories:
	@mkdir -p $(OBJ_DIR) $(BIN_DIR) download decoded_output

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)
	@echo "Build complete: $(TARGET)"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
	@echo "Cleaned build files"

clean-all: clean
	rm -rf download decoded_output
	@echo "Cleaned all generated files"

run: all
	./$(TARGET)

.PHONY: all directories clean clean-all run