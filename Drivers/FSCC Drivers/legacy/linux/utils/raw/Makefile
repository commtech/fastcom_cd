NAME=raw	

all: ${NAME}

${NAME}:
	gcc -o disp_bin disp_bin.c
	gcc -o read read.c
	gcc -o ts_comp ts_comp.c
	gcc -o write_file write_file.c
	gcc -o write_random write_random.c

install:

clean:
	${RM} -f *.o ${NAME}
	${RM} -f disp_bin
	${RM} -f read
	${RM} -f ts_comp
	${RM} -f write_file
	${RM} -f write_random

