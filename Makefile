COMPILER = c++

FLAGS = --std=c++2a -O2 -Wall -fsanitize=undefined -fsanitize=address -o



FILEPATH = ./MSDscriptCommandLine/MSDscriptCommandLine/

HEADERFILES =$(FILEPATH)expr.hpp $(FILEPATH)catch.h $(FILEPATH)val.hpp $(FILEPATH)pointermgmt.h $(FILEPATH)parse.hpp $(FILEPATH)env.hpp

SOURCEFILES = $(FILEPATH)main.cpp $(FILEPATH)expr.cpp $(FILEPATH)val.cpp $(FILEPATH)parse.cpp $(FILEPATH)env.cpp

EXENAME = msdscript

$(EXENAME): $(SOURCEFILES) $(HEADERFILES)
	$(COMPILER) $(FLAGS) $(EXENAME) $(SOURCEFILES)


TESTFILEPATH = ./RandomizedTestGenerator/RandomizedTestGenerator/

TESTHEADERFILES = $(TESTFILEPATH)exec.h

TESTSOURCEFILES = $(TESTFILEPATH)main.cpp $(TESTFILEPATH)exec.cpp

TESTEXENAME = test_msdscript

$(TESTEXENAME): $(TESTSOURCEFILES) $(TESTHEADERFILES)
	$(COMPILER) $(FLAGS) $(TESTEXENAME) $(TESTSOURCEFILES)


.PHONY: test
test: msdscript
	./msdscript --test

