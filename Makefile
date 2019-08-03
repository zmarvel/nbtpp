
EXE = nbt_dump

TEST_EXE = test_nbt

STATIC_LIB = libnbtpp.a

LIB_SRCS = src/nbt.cpp \

LIB_OBJS := $(LIB_SRCS:%.cpp=%.o)

EXE_SRCS = \
       src/nbt_dump.cpp \

EXE_OBJS := $(EXE_SRCS:%.cpp=%.o)

INCDIRS = -Iinclude

TEST_SRCS = \
	    test/test_main.cpp \
	    test/test_nbt.cpp \
	    test/test_swaps.cpp \

CXXFLAGS += -std=gnu++14 -Wall -MD -g $(INCDIRS)

LDFLAGS += -g

TEST_OBJS := $(TEST_SRCS:%.cpp=%.o)

TEST_INCDIRS = -Ilib/Catch2/single_include

DEPS := $(LIB_SRCS:%.cpp=%.d) $(EXE_SRCS:%.cpp=%.d) $(TEST_SRCS:%.cpp=%.d)

all: $(EXE) $(STATIC_LIB)

.PHONY: test
test: $(TEST_EXE) test_data

$(EXE): $(EXE_OBJS) $(STATIC_LIB)
	$(CXX) $(LDFLAGS) -o $@ $^

$(EXE_OBJS): $(STATIC_LIB)

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $<

$(STATIC_LIB): $(LIB_OBJS)
	ar rcs $@ $^

$(TEST_OBJS): CXXFLAGS += $(TEST_INCDIRS)

$(TEST_EXE): $(TEST_OBJS) $(STATIC_LIB) test_data
	$(CXX) $(LDFLAGS) -o $@ $(TEST_OBJS) $(STATIC_LIB)


.PHONY: test_data
test_data:
	$(MAKE) -C test/data

.PHONY: check
check: $(TEST_EXE)
	./$(TEST_EXE)


.PHONY: clean
clean:
	rm -f $(EXE) $(STATIC_LIB) $(LIB_OBJS) $(EXE_OBJS) $(TEST_EXE) $(TEST_OBJS) $(DEPS)
	make -C test/data clean

.PHONY: tags ctags
tags ctags:
	ctags -R src include test

-include $(DEPS)
