ImNCLUDE_DIRS = 
LIB_DIRS = 

CDEFS=
CFLAGS= -O -g $(INCLUDE_DIRS) $(CDEFS) -DLINUX
LIBS=-lpthread -lrt

HFILES=

CFILES2= deadlock.c

SRCS2= ${HFILES} ${CFILES2}

OBJS2= ${CFILES2:.c=.o}

all: deadlock

clean:
	-rm -f *.o *.d *.exe deadlock

# pthread3: pthread3.o
# 	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $(OBJS3) $(LIBS)

# pthread3ok: pthread3ok.o
# 	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $(OBJS1) $(LIBS)

# pthread3amp: pthread3amp.o
# 	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $(OBJS5) $(LIBS)

deadlock: deadlock.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $(OBJS2) $(LIBS)

# deadlock_timeout: deadlock_timeout.o
# 	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $(OBJS4) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) -c $<
