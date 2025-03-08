LDFLAGS = -lm

RM = rm -rf

SOURCES = bytecode.o error.o lexer.o main.o parser.o stacks.o utils.o vm.o

all: h

clean:
	$(RM) h

h: $(SOURCES)
