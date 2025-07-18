CXX = g++
CXXFLAGS = -Wall -std=c++11
TARGET = tftp_server
SRCS = main.cpp tftp_server.cpp
OBJS = $(SRCS:.cpp=.o)

$(TARGET):$(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -rf $(TARGET) $(OBJS)