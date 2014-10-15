CFLAGS = -O2
CC = gcc
RM = rm
LDFLAGS = -lm

NAME=poll

all: ${NAME}

${NAME}: ${NAME}.o
	${CC} ${LDFLAGS} ${CFLAGS} -o $@ $<

${NAME}.o: ${NAME}.c
	${CC} -c $<

.PHONY: install
install:
	cp ${NAME} ../bin
	@echo ${NAME}" copied to fsccp/util/bin directory"

.PHONY: clean
clean:
	@-$(RM) -fv *.o $(NAME) ../bin/$(NAME)
