CXX = g++
CXXFLAGS = -std=c++17 -O3 -Wall -Wextra -pthread
LDFLAGS = -pthread

TARGET = password_cracker_v4
SOURCES = main.cpp ../common/password_cracker.cpp
OBJECTS = main.o ../common/password_cracker.o

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c main.cpp -o main.o

../common/password_cracker.o: ../common/password_cracker.cpp
	$(CXX) $(CXXFLAGS) -c ../common/password_cracker.cpp -o ../common/password_cracker.o

clean:
	rm -f $(OBJECTS) $(TARGET)

smart: clean
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET) $(LDFLAGS)
	@echo "🚀 SMART OPTIMIZATIONS COMPILED! 🚀" 