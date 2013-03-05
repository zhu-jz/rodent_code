#
# Makefile
#

LIBS = -s -static -lm 

default: rodent

OFILES = rodent.o \
   
 
$(OFILES): 

rodent:  $(OFILES) 
	g++ -o rodent $(OFILES)  $(LIBS)

clean:
	rm -f *.o  rodent

.c.o:   main.c
	g++ -c -g $*.c  -s -w -Wfatal-errors -pipe -s -DNDEBUG -Ofast -march=athlon-xp -static -fno-rtti \
                       -finline-functions -fprefetch-loop-arrays -flto -fwhole-program


# for popcount (AMD)   =   -march=amdfam10 -mtune=amdfam10 -mpopcnt -DGCC_POPCOUNT
# for popcount (INTEL) =   -msse4.2 -march=corei7 -mtune=corei7 -mpopcnt -DGCC_POPCOUNT







