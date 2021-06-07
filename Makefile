CC = g++ -std=c++11
OBJ = main.o Particle_IC_Constructor.o FileTreatment_Interpolation.o

OPENFLAG=-fopenmp
LIB = -L/home/return/gsl/lib -lgsl -lgslcblas
CFLAGS  = -I/home/return/gsl/include

main:$(OBJ)
	$(CC) $(OPENFLAG) -o main $(OBJ)  $(LIB)
	rm -f $(OBJ)
	

$(OBJ): %.o: %.cpp
	$(CC) $(CFLAGS) -c $<
	export LD_LIBRARY_PATH=/home/return/gsl/lib:$LD_LIBRARY_PATH
	
clean:
	rm -f main $(OBJ)
