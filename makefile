TARGET := fat32-actor

CFLAGS  := -Wall -Werror
LDFLAGS :=

OBJS := main.o


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
