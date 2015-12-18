export AR=ar
export AR_NAME=libglcnc.a
export CC=gcc
export NETCDF_INCLUDE_PATH=/usr/local/include/
export NETCDF_LIB_PATH=/usr/local/lib64/
PROG_NAME_FVCOM=slr-fvm
LIB_DIR=lib
export LIB_FLAG=-lglcnc -lnetcdf -lgdal
export CPPFLAGS:=-I${NETCDF_INCLUDE_PATH} -I./${LIB_DIR}
export PLATFORM=-m64
export CFLAGS:=${PLATFORM} -Wall -O3 #-g
export LDFLAGS:=-L${NETCDF_LIB_PATH} -L${LIB_DIR} ${LIB_FLAG}
objs:=$(patsubst %.c,%.o,$(wildcard *.c))

.PHONY:all clean ${PROG_NAME_FVCOM} ${LIB_DIR}

all:${LIB_DIR} ${PROG_NAME_FVCOM}

${PROG_NAME_FVCOM}:${LIB_DIR} ${objs}
	${CC} -o $@ ${CFLAGS} ${PROG_NAME_FVCOM}.o ${LDFLAGS}
${LIB_DIR}:
	${MAKE} --directory=$@
%.o:%.c
	${CC} -c ${CPPFLAGS} ${CFLAGS} $< -o $@
clean:
	${MAKE} clean --directory=${LIB_DIR}
	@echo "remove all object files and main target"
	rm ./*.o ./${PROG_NAME_FVCOM}
