CC=gcc
LDFLAGS=-lm
NAME=setclock

${NAME} : ${NAME}.o ics30703.o
	${CC} ${LDFLAGS} -o $@ $< ics30703.o
${NAME}.o : ${NAME}.c
	${CC} -c $<

ics30703.o : ics30703.c
	${CC} -c $<
	
all: ${NAME}

install: 
	cp ${NAME} ../bin

clean:
	${RM} -f *.o ${NAME}
