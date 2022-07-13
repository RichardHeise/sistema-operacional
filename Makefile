# Richard Fernando Heise Ferreira (GRR20191053)

CFLAGS  = -Wall -g
CC = gcc 

#-----------------------------------------------------------------------------#
all : pingpong

run: pingpong
	./pingpong

pingpong: queue.o ppos_core.o pingpong.o


#-----------------------------------------------------------------------------#

clean :
	$(RM) *.o

#-----------------------------------------------------------------------------#

purge:
	$(RM) pingpong *.txt *.o