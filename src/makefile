CC = g++
CFLAGS = -g -Wall -std=c++11
LIB = -lm
INCLUDE = rclcpp/rclcpp.hpp
LDFLAGS = -L.
PROG = panTiltExecutable
SRCS = panTiltNode.cpp
OBJ = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)

all : $(PROG)

$(PROG): $(OBJ)
	$(CC) -o $(PROG) $(OBJ) $(LDFLAGS) $(LIB) $(INCLUDE)

.cpp.o:
	$(CC) -c $(CFLAGS) $(INCLUDE) $< -o $@

# cleanup
clean:
	rm -f *.o $(PROG)