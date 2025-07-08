CXX = g++
CXXFLAGS = -std=c++17 -Wall -O2
INCLUDES = -Isender

SRCS = device_manager.cpp sender/sender.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = device_manager

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f *.o sender/*.o $(TARGET)
