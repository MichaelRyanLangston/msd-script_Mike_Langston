CXXFLAGS = --std=c++2a -O2 -fsanitize=undefined -fno-sanitize-recover=undefined
#-fsanitize=address -Wall 

FILEPATH = ./MSDscriptCommandLine/MSDscriptCommandLine/

HEADERFILES =$(FILEPATH)expr.hpp $(FILEPATH)catch.h $(FILEPATH)val.hpp $(FILEPATH)pointermgmt.h $(FILEPATH)parse.hpp $(FILEPATH)env.hpp $(FILEPATH)step.hpp  $(FILEPATH)continue.hpp 

SOURCEFILES =$(FILEPATH)expr.cpp $(FILEPATH)val.cpp $(FILEPATH)parse.cpp $(FILEPATH)env.cpp $(FILEPATH)env.cpp  $(FILEPATH)step.cpp  $(FILEPATH)continue.cpp  

OBJECTFILES = expr.o val.o parse.o env.o step.o continue.o

LIBNAME = libmsd.a

EXENAME = msdscript

$(LIBNAME): $(OBJECTFILES)
	ar -ruv $(LIBNAME) $(OBJECTFILES)

$(EXENAME): $(SOURCEFILES) $(OBJECTFILES)
	$(CXX) $(CXXFLAGS) -o $(EXENAME) $(FILEPATH)main.cpp $(OBJECTFILES)

expr.o: $(FILEPATH)expr.cpp $(HEADERFILES)
	$(CXX) $(CXXFLAGS) -c $<

val.o: $(FILEPATH)val.cpp  $(HEADERFILES)
	$(CXX) $(CXXFLAGS) -c $<

parse.o: $(FILEPATH)parse.cpp $(HEADERFILES)
	$(CXX) $(CXXFLAGS) -c $<

env.o: $(FILEPATH)env.cpp $(HEADERFILES) 
	$(CXX) $(CXXFLAGS) -c $<

step.o: $(FILEPATH)step.cpp $(HEADERFILES) 
	$(CXX) $(CXXFLAGS) -c $<

continue.o: $(FILEPATH)continue.cpp $(HEADERFILES) 
	$(CXX) $(CXXFLAGS) -c $<



TESTFILEPATH = ./RandomizedTestGenerator/RandomizedTestGenerator/

TESTHEADERFILES = $(TESTFILEPATH)exec.h

TESTSOURCEFILES = $(TESTFILEPATH)main.cpp $(TESTFILEPATH)exec.cpp

TESTEXENAME = test_msdscript

$(TESTEXENAME): $(TESTSOURCEFILES) $(TESTHEADERFILES)
	$(CXX) $(CXXFLAGS) -o $(TESTEXENAME) $(TESTSOURCEFILES)


.PHONY: test
test: msdscript
	./msdscript --test

