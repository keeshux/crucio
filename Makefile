CRUCIO_NAME=crucio
CRUCIOTEX_NAME=cruciotex

CPP=g++
OPTIMIZE=-O3
#BENCHMARK=-DUSE_BENCHMARK
#PROFILE=-pg
#DEBUG=-ggdb
SRC_DIR=src
EXT_DIR=ext

vpath %.h = $(SRC_DIR)
vpath %.cc = $(SRC_DIR)

CPPFLAGS=-Wall -ansi -pedantic -I$(EXT_DIR)/include $(OPTIMIZE) \
	$(BENCHMARK) $(PROFILE) $(DEBUG)

OBJS=Dictionary.o Grid.o Walk.o Model.o Compiler.o Backjumper.o \
	LetterCompiler.o WordCompiler.o Output.o \
	crucio.o cruciotex.o

all: $(CRUCIO_NAME) $(CRUCIOTEX_NAME)

CRUCIO_OBJS=Dictionary.o Grid.o Walk.o Model.o Compiler.o Backjumper.o \
	LetterCompiler.o WordCompiler.o Output.o crucio.o

$(CRUCIO_NAME): $(CRUCIO_OBJS)
	$(CPP) -o $(CRUCIO_NAME) $(CRUCIO_OBJS)

CRUCIOTEX_OBJS=Output.o cruciotex.o

$(CRUCIOTEX_NAME): $(CRUCIOTEX_OBJS)
	$(CPP) -o $(CRUCIOTEX_NAME) $(CRUCIOTEX_OBJS)

$(OBJS): %.o: %.cc %.h
	$(CPP) $(CPPFLAGS) -c $< -o $@

clean:
	rm -f *.o $(CRUCIO_NAME) $(CRUCIOTEX_NAME)

depend:
	$(CPP) -MM $(SRC_DIR)/*.cc >depend

remake: clean $(CRUCIO_NAME) $(CRUCIOTEX_NAME)

include depend
