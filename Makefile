# Richard Fernando Heise Ferreira (GRR20191053)

CFLAGS  = -Wall -g
CC = gcc 

#-----------------------------------------------------------------------------#
all : teste1

run: all
	./teste1

teste1 : queue.o ppos_core.o
contexts : contexts.o queue.o
testafila : testafila.o queue.o


#-----------------------------------------------------------------------------#

clean :
	$(RM) *.o

#-----------------------------------------------------------------------------#

purge:
	$(RM) contexts testafila teste1 *.o