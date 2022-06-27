# Richard Fernando Heise Ferreira (GRR20191053)

CFLAGS  = -Wall -g
CC = gcc 

#-----------------------------------------------------------------------------#
all : pingpong-tasks2

runPP3: pingpong-tasks3 
	./pingpong-tasks3 > minha_saida3.txt

runPP2: pingpong-tasks2
	./pingpong-tasks2 > minha_saida2.txt

pingpong-tasks2 : ppos_core.o
pingpong-tasks3 : ppos_core.o 
teste1 : queue.o ppos_core.o
contexts : contexts.o queue.o
testafila : testafila.o queue.o


#-----------------------------------------------------------------------------#

clean :
	$(RM) *.o

#-----------------------------------------------------------------------------#

purge:
	$(RM) contexts testafila teste1 *.o