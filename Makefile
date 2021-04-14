CXXFLAGS = --std=c++2a -O2 
#-fsanitize=address -fsanitize=undefined -fno-sanitize-recover=undefined -Wall 

FILEPATH = ./MSDscriptCommandLine/MSDscriptCommandLine/

HEADERFILES = $(FILEPATH)catch.h $(FILEPATH)pointermgmt.h $(FILEPATH)parse.hpp $(FILEPATH)expr.hpp $(FILEPATH)val.hpp $(FILEPATH)env.hpp $(FILEPATH)step.hpp  $(FILEPATH)continue.hpp 

OBJECTFILES = test.o parse.o expr.o val.o env.o step.o continue.o 

LIBNAME = libmsd.a

$(LIBNAME): $(OBJECTFILES)
	ar -ruv $(LIBNAME) $(OBJECTFILES)

EXENAME = msdscript

$(EXENAME): $(SOURCEFILES) $(OBJECTFILES)
	$(CXX) $(CXXFLAGS) -o $(EXENAME) $(FILEPATH)main.cpp $(OBJECTFILES)

test.o: $(FILEPATH)test.cpp $(HEADERFILES)
	$(CXX) $(CXXFLAGS) -c $<

parse.o: $(FILEPATH)parse.cpp $(HEADERFILES)
	$(CXX) $(CXXFLAGS) -c $<

expr.o: $(FILEPATH)expr.cpp $(HEADERFILES)
	$(CXX) $(CXXFLAGS) -c $<

val.o: $(FILEPATH)val.cpp  $(HEADERFILES)
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

