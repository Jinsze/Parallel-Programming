
# Conjugate Gradient using MPI


## Files

- `main.c` - Main program and performance measurement.
- `cg.c` - Sequential and parallel CG implementations.
- `cg.h` - Data structures and function declarations.
- `Makefile` - Compilation instructions.

## Compilation

To remove the executable: `make clean`

To compile the program: `make`

This generates the executable: `cg`


## Running

Run the program using MPI: `mpirun -np <p> ./cg <N> <max_steps>`


where:

* `<p>` = number of MPI processes
* `<N>` = matrix dimension (generates an N × N matrix)
* `<max_steps>` = maximum number of Conjugate Gradient iterations

### Example

`mpirun -np 4 ./cg 4096 200`


This runs the Conjugate Gradient solver on a 4096 × 4096 matrix with a maximum of 200 iterations using 4 MPI processes.


## Output

The program reports:

* Sequential maximum error
* Parallel maximum error
* Matrix generation time
* Sequential solution time
* Parallel solution time
