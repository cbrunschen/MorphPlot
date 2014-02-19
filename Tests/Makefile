# Makefile for the tests for the morph library

TEST_OBJECTS=TestBoundaries.o TestCircle.o TestExtentsAndDelta.o \
    TestFilling.o TestLine.o TestReconstruction.o TestSeparation.o \
    TestSingleConnected.o TestDistanceTransform.o

TEST_HEADERS=TestBoundaries.h TestCircle.h TestExtentsAndDelta.h \
    TestFilling.h TestLine.h TestReconstruction.h TestSeparation.h \
    TestSingleConnected.h TestDistanceTransform.h Count.h Tests.h

TEST_SRC=Tests.cpp

%.o : %.cpp $(LIB_HEADERS)
	$(CXX) $(CFLAGS) -c -I. -I.. -I../Catch $(INCDIRS) $<

tests : $(TEST_SRC) $(TEST_OBJECTS)
	$(CXX) $(CFLAGS) $(LDFLAGS) -o $@ -I. -I.. -I../Catch $(INCDIRS) -L. -L.. $(LIBDIRS) $< $(TEST_OBJECTS) -l$(LIB) -lpng

test : tests
	./tests

clean :
	-rm tests $(TEST_OBJECTS)
