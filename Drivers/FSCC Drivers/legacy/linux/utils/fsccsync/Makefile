CC=gcc
LDFLAGS=-lm
NAME=fsccsync

${NAME} : ${NAME}.o
	${CC} ${LDFLAGS} -o $@ $<
${NAME}.o : ${NAME}.c
	${CC} -c $<
	
all: ${NAME}

install: 
	cp ${NAME} ../bin

clean:
	${RM} -f *.o ${NAME}
