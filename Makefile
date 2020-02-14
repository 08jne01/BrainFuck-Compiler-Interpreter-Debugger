ifndef $(PROG)
    PROG = bfc
endif

    CC = g++
    CXXFLAGS = -g -std=c++11 -Wall
    OBJS =
    LDFLAGS =

all: $(OBJS)
	$(CC) $(CXXFLAGS) Main.cpp BFCompiler.cpp BFInterpreter.cpp BFFileHandler.cpp BFParser.cpp -w -lncurses -o2 -o bfc

clean:
	rm -f bfc
