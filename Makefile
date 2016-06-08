TARGET=rlimit
SRC=rlimit.cpp

OBJ=$(SRC:.cpp=.o)

VERSION=0.0.2
CFLAGS+=-g -Wall -Werror
LDFLAGS+=-static

PACKAGE=$(TARGET)-$(VERSION)

DIST=COPYING Makefile README rlimit.cpp rlimit
DIST_FILES=$(patsubst %,$(PACKAGE)/%,$(DIST))

all: $(TARGET)

%.o: %.cpp
	$(CXX) -c $< $(CFLAGS)

$(TARGET): $(OBJ)
	$(CXX) -o $@ $(OBJ) $(LDFLAGS)

dist: tidy $(TARGET)
	ln -fs . $(PACKAGE)
	tar -hcjvf $(PACKAGE).tar.bz2 $(DIST_FILES)
	rm -f $(PACKAGE)

tidy:
	rm -f *~ \#*

clean: tidy
	rm -f $(TARGET) *.o $(PACKAGE) $(PACKAGE).tar.bz2
