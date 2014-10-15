CC=gcc
LDFLAGS=-lm
NAME=setfscc

${NAME} : ${NAME}.o
	${CC} ${LDFLAGS} -o $@ $<
${NAME}.o : ${NAME}.c
	${CC} -c $<
	
all: ${NAME}
install: 
	cp ${NAME} ../bin
	cp hdlcset ../bin
	cp hdlcset_232 ../bin
clean:
	${RM} -f *.o ${NAME}
