CC     = mpicc
CFLAGS = -g -Wall
LDLIBS = -lm


OBJS =  cfg.o\
    lib.o \
    timer.o \
    elib.o \
	pclust.o\
    dtype.o \
    producer.o\
    buffer.o \
    master.o\
    queue.o\
    consumer.o \
    forest.o \
    pairs.o \
    sort.o \
    dynamic.o \
    union.o \
    loadseq.o \
    spmaster.o \
    csque.o
    

all: pclust

pclust: $(OBJS)
	$(CC) $(OBJS) -o pclust $(LDLIBS)

install:
	cp ./pclust  ~/bin/pclust

clean:
	rm -f *.o pclust
