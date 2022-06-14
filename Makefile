###############################################
##	Simple Makefile for Endlessh Report Gen	 ##
##				©️ Simon Cahill 2022		  ##
###############################################

.PHONY: endlessh-report all clean install docs
.SILENT: debug clean docs

endlessh-report: main.cpp
	${CXX} -s -O2 -o $@ $^

endlessh-report-debug: main.cpp
	${CXX} -O0 -ggdb -o $@ $^

debug: endlessh-report-debug
	ls -lah $^

release: endlessh-report
	ls -lah $^

all: endlessh-report endlessh-report-debug docs

clean:
	rm -Rfv *.o endlessh-report endlessh-report-debug

docs:
	doxygen Doxyfile

install: endlessh-report
	cp -v $^ /usr/local/bin/
