

bin_PROGRAMS = nbt_dump
nbt_dump_SOURCES = src/nbt_dump.cpp src/nbt.cpp
nbt_dump_CPPFLAGS = $(AM_CPPFLAGS)

lib_LTLIBRARIES = libnbtpp.la
libnbtpp_la_SOURCES = src/nbt.cpp

include_HEADERS = include/nbt.hpp

AM_CPPFLAGS = -Iinclude

AM_CXXFLAGS = -std=c++17
