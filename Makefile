CRUCIO_NAME=crucio
CRUCIOTEX_NAME=cruciotex
FILLIN_NAME=fill-in

CPP=g++
OPTIMIZE=-O3 -DCRUCIO_C_ARRAYS
#BENCHMARK=-DCRUCIO_BENCHMARK
#PROFILE=-pg
#DEBUG=-ggdb
SRC_DIR=src
EXT_DIR=ext

vpath %.h = $(SRC_DIR)
vpath %.cc = $(SRC_DIR)

CPPFLAGS=-Wall -ansi -pedantic -I$(EXT_DIR)/include $(OPTIMIZE) \
	$(BENCHMARK) $(PROFILE) $(DEBUG)

OBJS=common.o Grid.o Walk.o Backjumper.o Model.o \
    Dictionary.o WordSet.o LanguageMatcher.o SolutionMatcher.o \
    Compiler.o LetterCompiler.o WordCompiler.o Output.o \
	crucio.o cruciotex.o

all: $(CRUCIO_NAME) $(CRUCIOTEX_NAME) $(FILLIN_NAME)

CRUCIO_OBJS=common.o Grid.o Walk.o Backjumper.o Model.o \
    Dictionary.o WordSet.o LanguageMatcher.o SolutionMatcher.o \
    Compiler.o LetterCompiler.o WordCompiler.o Output.o \
    crucio.o

$(CRUCIO_NAME): $(CRUCIO_OBJS)
	$(CPP) -o $(CRUCIO_NAME) $(CRUCIO_OBJS)

CRUCIOTEX_OBJS=WordSet.o Output.o cruciotex.o

$(CRUCIOTEX_NAME): $(CRUCIOTEX_OBJS)
	$(CPP) -o $(CRUCIOTEX_NAME) $(CRUCIOTEX_OBJS)

FILLIN_OBJS=common.o Grid.o FillIn.o fill-in.o

$(FILLIN_NAME): $(FILLIN_OBJS)
	$(CPP) -o $(FILLIN_NAME) $(FILLIN_OBJS)

$(OBJS): %.o: %.cc %.h
	$(CPP) $(CPPFLAGS) -c $< -o $@

clean:
	rm -f *.o $(CRUCIO_NAME) $(CRUCIOTEX_NAME) $(FILLIN_NAME)

depend:
	$(CPP) -MM $(SRC_DIR)/*.cc >depend

remake: clean $(CRUCIO_NAME) $(CRUCIOTEX_NAME) $(FILLIN_NAME)

include depend
