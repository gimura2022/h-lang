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
LDFLAGS += -fPIC
CFLAGS += $(LDFLAGS)

.PHONY: all
all: h libh.so libh.a

.PHONY: clean
clean:
	$(RM) $(OBJS) h libh.so libh.a

libh.so: $(OBJS)
	$(CC) -shared -o $@ $(OBJS) $(CFLAGS) $(LDFLAGS) $(LDLIBS)

libh.a: $(OBJS)
	$(AR) rcs $@ $(OBJS)

h: h.c install_lib
	$(CC) h.c -o h -lh $(LDLIBS) $(CFLAGS)

.PHONY: install
install: all
	install -d $(DESTDIR)/bin/ $(DESTDIR)/share/man/man1/
	install -m755 h $(DESTDIR)/bin/
	install -m644 h.1 $(DESTDIR)/share/man/man1/

.PHONY: uninstall
uninstall:
	$(RM) $(DESTDIR)/lib/libh.a
	$(RM) $(DESTDIR)/lib/libh.so
	$(RM) $(DESTDIR)/include/h.h
	$(RM) $(DESTDIR)/bin/h
	$(RM) $(DESTDIR)/share/man/man1/h.1

.PHONY: install_lib
install_lib: libh.so libh.a
	install -d $(DESTDIR)/lib $(DESTDIR)/include
	install -m644 libh.a $(DESTDIR)/lib
	install -m755 libh.so $(DESTDIR)/lib
	install -m644 h.h $(DESTDIR)/include

README: README.7
	mandoc $< | col -b > $@
