ifdef SystemRoot
   RM = del /Q
   FixPath = $(subst /,\,$1)
   EXT = .exe
else
   ifeq ($(shell uname), Linux)
      RM = rm -f
      FixPath = $1
      EXT = 
   endif
endif

all: mw_maxshift$(EXT)

mw_maxshift$(EXT): mw_maxshift.o
	gcc -o $(call FixPath,dist/mw_maxshift)$(EXT) mw_maxshift.c main.c -I.

mw_maxshift.o: main.c
	gcc -c main.c

clean:
	$(RM) main.o mw_maxshift.o $(call FixPath,dist/mw_maxshift)$(EXT)
