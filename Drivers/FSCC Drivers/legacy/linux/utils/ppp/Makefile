CC=gcc
LDFLAGS=-lutil
NAME=pty

${NAME} : ${NAME}.o
	${CC} ${LDFLAGS} -o $@ $<
${NAME}.o : ${NAME}.c
	${CC} -c $<
	
all: ${NAME}

install: 
	cp ${NAME} ../bin

clean:
	${RM} -f *.o ${NAME}
