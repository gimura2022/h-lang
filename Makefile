DESTDIR ?= /usr
RM ?= rm -rf

OBJS += bytecode.o
OBJS += error.o
OBJS += lexer.o
OBJS += parser.o
OBJS += stacks.o
OBJS += utils.o
OBJS += vm.o

LDLIBS += -lm

.PHONY: all
all: h libh.so libh.a

.PHONY: clean
clean:
	$(RM) $(OBJS) h libh.so libh.a

libh.so: $(OBJS)
	$(CC) -shared -o $@ $(LDFLAGS) $(LDLIBS) $(OBJS)

libh.a: $(OBJS)
	$(AR) rcs $@ $(OBJS)

h: h.c install_lib
	$(CC) h.c -o h -lh $(LDLIBS)

.PHONY: install_lib
install_lib: libh.so libh.a
	install -d $(DESTDIR)/lib $(DESTDIR)/include
	install -m644 libh.a $(DESTDIR)/lib
	install -m755 libh.so $(DESTDIR)/lib
	install -m644 h.h $(DESTDIR)/include
