CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra
LDFLAGS = -pthread

TARGET = password_cracker_v1
SOURCES = main.cpp ../common/password_cracker.cpp
OBJECTS = $(SOURCES:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run 