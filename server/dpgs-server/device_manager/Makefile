CXX = g++
CXXFLAGS = -std=c++17 -Wall
TARGET = device_manager

SRCS = main.cpp device_manager.cpp sender/sender.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
