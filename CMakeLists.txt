cmake_minimum_required(VERSION 3.10)

project(msdscript)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

add_executable(msdscript ./MSDscriptCommandLine/MSDscriptCommandLine/catch.h ./MSDscriptCommandLine/MSDscriptCommandLine/pointermgmt.h ./MSDscriptCommandLine/MSDscriptCommandLine/parse.hpp ./MSDscriptCommandLine/MSDscriptCommandLine/expr.hpp ./MSDscriptCommandLine/MSDscriptCommandLine/val.hpp ./MSDscriptCommandLine/MSDscriptCommandLine/env.hpp ./MSDscriptCommandLine/MSDscriptCommandLine/step.hpp  ./MSDscriptCommandLine/MSDscriptCommandLine/continue.hpp ./MSDscriptCommandLine/MSDscriptCommandLine/main.cpp ./MSDscriptCommandLine/MSDscriptCommandLine/test.cpp ./MSDscriptCommandLine/MSDscriptCommandLine/parse.cpp ./MSDscriptCommandLine/MSDscriptCommandLine/expr.cpp ./MSDscriptCommandLine/MSDscriptCommandLine/val.cpp ./MSDscriptCommandLine/MSDscriptCommandLine/env.cpp ./MSDscriptCommandLine/MSDscriptCommandLine/step.cpp ./MSDscriptCommandLine/MSDscriptCommandLine/continue.cpp)

add_library(msd ./MSDscriptCommandLine/MSDscriptCommandLine/parse.cpp ./MSDscriptCommandLine/MSDscriptCommandLine/expr.cpp ./MSDscriptCommandLine/MSDscriptCommandLine/val.cpp ./MSDscriptCommandLine/MSDscriptCommandLine/env.cpp ./MSDscriptCommandLine/MSDscriptCommandLine/step.cpp ./MSDscriptCommandLine/MSDscriptCommandLine/continue.cpp)