
EXE = nbt_dump

TEST_EXE = test_nbt

STATIC_LIB = libnbtpp.a

SRCS = \
       src/nbt_dump.cpp \
       src/nbt.cpp \

OBJS := $(SRCS:%.cpp=%.o)

INCDIRS = -Iinclude

TEST_SRCS = \
	    test/test_main.cpp \
	    test/test_nbt.cpp \
	    test/test_swaps.cpp \

CXXFLAGS += -std=gnu++14 -Wall -MD -g $(INCDIRS)

LDFLAGS += -g

TEST_OBJS := $(TEST_SRCS:%.cpp=%.o)

TEST_INCDIRS = -Ilib/Catch2/single_include

TEST_CXXFLAGS := $(CXXFLAGS) $(TEST_INCDIRS)

DEPS := $(SRCS:%.cpp=%.d) $(TEST_SRCS:%.cpp=%.d)

all: $(EXE) $(STATIC_LIB)

.PHONY: test
test: $(TEST_EXE) test_data

$(EXE): src/nbt_dump.o $(STATIC_LIB)
	$(CXX) $(LDFLAGS) -o $@ $^

$(STATIC_LIB): src/nbt.o
	ar rcs $@ $^

$(TEST_OBJS): CXXFLAGS += $(TEST_CXXFLAGS)

$(TEST_EXE): $(TEST_OBJS) $(STATIC_LIB)
	$(CXX) $(LDFLAGS) -o $@ $^

.PHONY: test_data
test_data:
	make -C test/data

.PHONY: check
check: test
	./$(TEST_EXE)


.PHONY: clean
clean:
	rm -f $(EXE) $(STATIC_LIB) $(OBJS) $(TEST_EXE) $(TEST_OBJS) $(DEPS)
	make -C test/data clean

.PHONY: tags
tags:
	ctags -R src include test

-include $(DEPS)
