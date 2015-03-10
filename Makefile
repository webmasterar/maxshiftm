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

all: maxshiftm$(EXT)

maxshiftm$(EXT): maxshiftm.o
	gcc -o $(call FixPath,dist/maxshiftm)$(EXT) maxshiftm.c main.c -lm -I.

maxshiftm.o: main.c
	gcc -c main.c

clean:
	$(RM) main.o maxshiftm.o $(call FixPath,dist/maxshiftm)$(EXT)

