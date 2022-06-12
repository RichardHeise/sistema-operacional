# Richard Fernando Heise Ferreira (GRR20191053)

CFLAGS  = -Wall 
CC = gcc 

#-----------------------------------------------------------------------------#
all : testafila

run: all
	./testafila	

testafila : testafila.o queue.o


#-----------------------------------------------------------------------------#

clean :
	$(RM) *.o

#-----------------------------------------------------------------------------#

purge:
	$(RM)  testafila *.o