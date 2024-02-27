CC = gcc
CFLAGS = -g -c -o
TARGET = hw2
TESTCASE = TestCase1.o TestCase2.o TestCase3.o
OBJS = Init.o main.o Scheduler.o SchedCore.o Thread.o ThreadDataStruct.o $(TESTCASE)


$(TARGET): $(OBJS)
	$(CC) -g -o $@ $^ -pthread
	rm -f *.o

Init.o: Init.c
	gcc $(CFLAGS) Init.o Init.c

main.o: main.c
	gcc $(CFLAGS) main.o main.c

Scheduler.o: Scheduler.c
	gcc $(CFLAGS) Scheduler.o Scheduler.c

SchedCore.o: SchedCore.c
	gcc $(CFLAGS) SchedCore.o SchedCore.c

Thread.o: Thread.c
	gcc $(CFLAGS) Thread.o Thread.c

ThreadDataStruct.o: ThreadDataStruct.c
	gcc $(CFLAGS) ThreadDataStruct.o ThreadDataStruct.c

TestCase1.o: TestCase1.c
	gcc $(CFLAGS) TestCase1.o TestCase1.c

TestCase2.o: TestCase2.c
	gcc $(CFLAGS) TestCase2.o TestCase2.c

TestCase3.o: TestCase3.c
	gcc $(CFLAGS) TestCase3.o TestCase3.c

clean:
	rm -f $(TARGET) *.o
