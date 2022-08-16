# Richard Fernando Heise Ferreira (GRR20191053)

CFLAGS  = -Wall -g
CC = gcc 

#-----------------------------------------------------------------------------#
all : pingpong-prodcons

run: pingpong-prodcons
	./pingpong-prodcons

pingpong-prodcons: pingpong-prodcons.o queue.o ppos_core.o

pingpong: queue.o ppos_core.o pingpong.o


#-----------------------------------------------------------------------------#

clean :
	$(RM) *.o

#-----------------------------------------------------------------------------#

purge:
	$(RM) pingpong *.txt *.o