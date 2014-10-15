setclock 0 1000000
setclock 1 1000000
fsccset 0 hdlcset.txt
fsccset 1 hdlcset.txt
start /wait cmd /k loop 0 0 4094
start /wait cmd /k loop 1 1 4094
