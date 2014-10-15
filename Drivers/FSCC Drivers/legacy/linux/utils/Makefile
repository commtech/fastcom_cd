# Makefile for the FSCC Utilities directory. 

FSCC_DIRS := setfscc flushrx flushtx getclock getdsr getfeatures \
		istatus readreg reconly rxready setclock setdtr setfeatures \
		start_timer status stop_timer tronly txactive \
		writereg rdwr ppp readcutoff poll external_transmit commandresponse \
		txtimed_repeat #wrspt #fsccsync

.PHONY : $(FSCC_DIRS) install clean

$(FSCC_DIRS):
	@for dir in $(FSCC_DIRS); do 		\
		$(MAKE) -C $$dir;				\
	done

install:
	rm -rf bin
	mkdir bin
	@for dir in $(FSCC_DIRS); do 		\
		$(MAKE) -C $$dir install;		\
	done
	
clean:
	@for dir in $(FSCC_DIRS); do 		\
		$(MAKE) -C $$dir clean;		\
	done
	rm -rf bin	
