CC=g++
COPT=-shared -O3 -Wall -fPIC -o $@ $^

SRCS = $(shell ls | grep [a-zA-Z0-9_+-]*.cpp$)

all: libQDFUtils.so

clean: 
	rm -f *.so 

libQDFUtils.so: $(SRCS)
	$(CC) $(COPT) 

