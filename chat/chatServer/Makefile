CXX = g++
CXXFLAGS = -Wall -std=c++11 -pthread
TARGET = chatServer
SRCS = main.cpp chatServer.cpp
OBJS = $(SRCS:.cpp=.o)

$(TARGET):$(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o:%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -rf $(TARGET) $(OBJS)