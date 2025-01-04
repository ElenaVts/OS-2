CC = gcc
CFLAGS = -O0 -Wall
LDFLAGS = -lm

DELAY=750

all: work1 work2 work3 work4 work5 work6 work7


work1: work.c
	$(CC) $(CFLAGS) -DLOAD=1 -DDELAY=$(DELAY) -o work1 work.c

work2: work.c
	$(CC) $(CFLAGS) -DLOAD=2 -DDELAY=$(DELAY) -o work2 work.c

work3: work.c
	$(CC) $(CFLAGS) -DLOAD=3 -DDELAY=$(DELAY) -o work3 work.c

work4: work.c
	$(CC) $(CFLAGS) -DLOAD=4 -DDELAY=$(DELAY) -o work4 work.c

work5: work.c
	$(CC) $(CFLAGS) -DLOAD=5 -DDELAY=$(DELAY) -o work5 work.c

work6: work.c
	$(CC) $(CFLAGS) -DLOAD=6 -DDELAY=$(DELAY) -o work6 work.c

work7: work.c
	$(CC) $(CFLAGS) -DLOAD=7 -DDELAY=$(DELAY) -o work7 work.c

clean:
	rm -f work[1-7]
