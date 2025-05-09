TARGET := fat32-actor

CFLAGS  := -Wall -Werror
LDFLAGS :=

OBJS := main.o								\
	dispatch.o							\
	interact/input.o						\
	interact/loglevel.o						\
	job/dummy.o							\
	job/quit.o							\
	utils/exit.o

.all: build
.PHONY: clean tidy


build: ${OBJS}
	${CC} ${CFLAGS} ${LDFLAGS} -o ${TARGET} ${OBJS}

%.o: %.c
	${CC} ${CFLAGS} -c $< -o $@

clean: tidy
	-rm ${TARGET}

tidy:
	-rm *.o
