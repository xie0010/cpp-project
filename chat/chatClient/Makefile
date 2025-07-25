CXX = g++
CXXFLAGS = -Wall -std=c++11 -pthread
TARGET = chatClient
SRCS = main.cpp chatClient.cpp
OBJS = $(SRCS:.cpp=.o)

$(TARGET):$(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY:clean
clean:
	rm -rf $(TARGET) $(OBJS)