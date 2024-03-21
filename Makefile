CXX = clang++ -std=c++20 -Werror -Wunused -Wshadow -Wpedantic -Wall -O0 -g
OBJDIR = obj/
EXENAME = kqueue
VPATH		:= src: obj:
SOURCES := main.cpp kqueue.cpp
OBJECTS := $(SOURCES:.cpp=.o)

all: $(EXENAME)

.PHONY: clean run

clean:
	-rm $(OBJDIR)*.o $(EXENAME)
run:
	./$(EXENAME)

$(EXENAME): $(OBJECTS)
	$(CXX) $^ -o $@

$(OBJDIR)main.o: main.cpp kqueue.hpp
	$(CXX) -c $< -o $@

$(OBJDIR)kqueue.o: kqueue.cpp kqueue.hpp
	$(CXX) -c $< -o $@