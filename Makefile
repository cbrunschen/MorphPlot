# Makefile for the 'plot' program and its supporting 'morph' library

LIB_OBJECTS=Bitmap.o Primitives.o Circle.o K3M.o PathExtractor.o \
    ColorImage.o Workers.o

LIB_HEADERS=FnvHash.h Hatcher.h Line.h \
    Chain.h Bitmap.h ColorImage.h GreyImage.h Primitives.h \
    Image.h Output.h FloodFill.h Circle.h K3M.h Progress.h \
    PathExtractor.h PlotterPathExtractor.h MillPathExtractor.h \
    Workers.h

LIB=morph
LIB_ARCHIVE=lib$(LIB).a

PLOT_SRC=plot_main.cpp
PLOT_OBJ=plot_main.o
PLOT_BIN=plot

MILL_SRC=mill_main.cpp
MILL_OBJ=mill_main.o
MILL_BIN=mill

STIPPLE_SRC=stipple_main.cpp
STIPPLE_OBJ=stipple_main.o
STIPPLE_BIN=stipple

TEST_MEIJER_SRC=test_meijer.cpp
TEST_MEIJER_OBJ=test_meijer.o
TEST_MEIJER_BIN=test_meijer

# where to look for the libpng header & library files
INCDIRS=-I/usr/local/include
LIBDIRS=-L/usr/local/lib

#COMPILER = gcc
COMPILER = clang
#COMPILER = clang-3.2

ifeq ($(COMPILER),gcc)
  CXX=gcc
  CXXFLAGS=-O3
  LDFLAGS=-lstdc++ -lc
else ifeq ($(COMPILER),clang)
  CXXFLAGS=-O3
  LDFLAGS=-lc
  CXX=clang++ -std=gnu++11
else ifeq ($(COMPILER),clang-3.1)
  CXXFLAGS=-O3
  LDFLAGS=-lc
  CXX=clang++-mp-3.1
else ifeq ($(COMPILER),clang-3.2)
  CXXFLAGS=-O3
  LDFLAGS=-lc
  CXX=clang++-mp-3.2
endif

export LIB
export LIB_ARCHIVE
export CFLAGS
export CXX
export CXXFLAGS
export LDFLAGS
export INCDIRS
export LIBDIRS

all : mill plot

%.o : %.cpp $(LIB_HEADERS)
	$(CXX) $(CFLAGS) $(CXXFLAGS) -c -I. $(INCDIRS) $<

$(PLOT_BIN) : $(PLOT_OBJ) $(LIB_ARCHIVE)
	$(CXX) $(CFLAGS) $(LDFLAGS) -o $@ -L. $(LIBDIRS) $< -l$(LIB) -lpng

$(MILL_BIN) : $(MILL_OBJ) $(LIB_ARCHIVE)
	$(CXX) $(CFLAGS) $(LDFLAGS) -o $@ -L. $(LIBDIRS) $< -l$(LIB) -lpng

$(STIPPLE_BIN) : $(STIPPLE_OBJ) $(LIB_ARCHIVE)
	$(CXX) $(CFLAGS) $(LDFLAGS) -o $@ -L. $(LIBDIRS) $< -l$(LIB) -lpng

$(TEST_MEIJER_BIN) : $(TEST_MEIJER_OBJ) $(LIB_ARCHIVE)
	$(CXX) $(CFLAGS) $(LDFLAGS) -o $@ -L. $(LIBDIRS) $< -l$(LIB) -lpng

$(LIB_ARCHIVE): $(LIB_OBJECTS)
	$(AR) -r $@ $^

clean :
	-rm $(PLOT_BIN) $(PLOT_OBJ) $(MILL_BIN) $(MILL_OBJ) $(STIPPLE_BIN) $(STIPPLE_OBJ) $(LIB_ARCHIVE) $(LIB_OBJECTS)
	$(MAKE) -C Tests clean

tests : $(LIB_ARCHIVE)
	$(MAKE) -C Tests tests

test : $(LIB_ARCHIVE)
	$(MAKE) -C Tests test
