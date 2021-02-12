msdscript: main.cpp use_arguments.cpp use_arguments.hpp expr.cpp expr.hpp catch.h
	c++ --std=c++2a -O2 -Wall -fsanitize=undefined -fsanitize=address -o msdscript main.cpp use_arguments.cpp expr.cpp

.PHONY: test
test: msdscript
	./msdscript --test

