TARGET := fat32-actor

CFLAGS  := -Wall -Werror
LDFLAGS :=

OBJS := main.o								\
	dispatch.o							\
	fat32/data_reader.o						\
	fat32/fat_reader.o						\
	fat32/global.o							\
	fat32/img.o							\
	fat32/location.o						\
	interact/input.o						\
	interact/loglevel.o						\
	job/debug/fat.o							\
	job/dummy.o							\
	job/help.o							\
	job/info.o							\
	job/load.o							\
	job/quit.o							\
	utils/error.o							\
	utils/exit.o							\
	utils/file.o							\
	utils/string.o							\
	utils/memory.o

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
	-rm **/*.o
