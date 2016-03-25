MF=     Makefile

CC=     g++

CFLAGS= -g -D_USE_64 -msse3 -O3 -fomit-frame-pointer -funroll-loops

LFLAGS= -std=c++11 -DNDEBUG -I .

EXE=    maxshiftm

SRC=    main.cpp MaxShiftM.cpp

OBJ=    main.o MaxShiftM.o

HD=     maxshiftm.h Makefile

#
# No need to edit below this line 
#

.SUFFIXES: 
.SUFFIXES: .cpp .o

.cpp.o: 
	$(CC) $(CFLAGS) -c $(LFLAGS) $< 

all:    $(EXE) 

$(EXE): $(OBJ)
	$(CC) $(CFLAGS) $(LFLAGS) $(SRC) -o $@

$(OBJ): $(HD) 

clean: 
	rm -f $(OBJ) $(EXE) *~
