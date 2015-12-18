objs:=$(patsubst %.c,%.o,$(wildcard *.c))

.PHONY:clean all
all:${objs}

%.o:%.c
	${CC} -c ${CPPFLAGS} ${CFLAGS} $< -o $@
	${AR} ru ${AR_NAME} $*.o
clean:
	@echo "remove all object files and lib"
	rm ./*.o ./${AR_NAME}
