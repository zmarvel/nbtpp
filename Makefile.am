

bin_PROGRAMS = nbt_dump
nbt_dump_SOURCES = src/nbt_dump.cpp src/nbt.cpp
nbt_dump_CPPFLAGS = $(AM_CPPFLAGS)

lib_LTLIBRARIES = libnbtpp.la
libnbtpp_la_SOURCES = src/nbt.cpp

include_HEADERS = include/nbt.hpp

TESTS = test_nbt
check_PROGRAMS = test_nbt
test_nbt_SOURCES = test/test_main.cpp test/test_nbt.cpp test/test_swaps.cpp
test_nbt_LDADD = libnbtpp.la
test_nbt_CPPFLAGS = -Ilib/Catch2/single_include -Ilib/Catch2/include $(AM_CPPFLAGS)
test_nbt_CXXFLAGS = $(AM_CXXFLAGS)

SUBDIRS=test/data

AM_CPPFLAGS = -Iinclude

AM_CXXFLAGS = -std=gnu++14 -Wall
