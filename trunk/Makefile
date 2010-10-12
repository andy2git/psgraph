CC = mpicc
LD = mpicc
TARGET	= pclust
CFLAGS  = -Wall -c -g
XFLAGS  = -Wall -c -g
LIBS	= -lm
OBJECTS = elib.o \
    cfg.o \
    lib.o \
    timer.o \
    dtype.o \
    producer.o\
    buffer.o \
    master.o\
    queue.o\
    pairs.o \
    forest.o \
    sort.o \
    dynamic.o \
    loadseq.o \
    spmaster.o \
    cslib.o \
    rpool.o \
    spool.o \
    cpool.o \
    consumer.o \
	pclust.o\

all: $(TARGET)

$(TARGET):  $(OBJECTS)
	$(LD) -o $(TARGET) $(OBJECTS) $(LIBS)

elib.o:	elib.c
	$(CC) $(CFLAGS) elib.c

cfg.o:	cfg.c
	$(CC) $(CFLAGS) cfg.c

lib.o:	lib.c
	$(CC) $(CFLAGS) lib.c

timer.o:	timer.c
	$(CC) $(CFLAGS) timer.c

pclust.o:	pclust.c
	$(CC) $(CFLAGS) pclust.c

dtype.o:	dtype.c
	$(CC) $(CFLAGS) dtype.c

producer.o:	producer.c
	$(CC) $(CFLAGS) producer.c

buffer.o:	buffer.c
	$(CC) $(CFLAGS) buffer.c

master.o:	master.c
	$(CC) $(CFLAGS) master.c

queue.o:	queue.c
	$(CC) $(CFLAGS) queue.c

forest.o:	forest.c
	$(CC) $(XFLAGS) forest.c

pairs.o:	pairs.c
	$(CC) $(CFLAGS) pairs.c

sort.o:	sort.c
	$(CC) $(CFLAGS) sort.c

dynamic.o:	dynamic.c
	$(CC) $(CFLAGS) dynamic.c

union.o:	union.c
	$(CC) $(CFLAGS) union.c

loadseq.o:	loadseq.c
	$(CC) $(CFLAGS) loadseq.c

spmaster.o:	spmaster.c
	$(CC) $(CFLAGS) spmaster.c

cslib.o:	cslib.c
	$(CC) $(CFLAGS) cslib.c

csque.o:	csque.c
	$(CC) $(CFLAGS) csque.c

rpool.o:	rpool.c
	$(CC) $(CFLAGS) rpool.c

spool.o:	spool.c
	$(CC) $(CFLAGS) spool.c

cpool.o:	cpool.c
	$(CC) $(CFLAGS) cpool.c

consumer.o:	consumer.c
	$(CC) $(CFLAGS) consumer.c

install:
	cp pclust ~/bin/
	
clean: 
	rm -f $(TARGET) $(OBJECTS)
