# N-Body Simulation (Sequential and MPI Parallel)

## Overview

This project implements a sequential and parallel N-body simulation using MPI.


## Compilation

To remove previously compiled files: `make clean`

To compile the program: `make`

This generates the executable: `nbody`


## Running the Program

Execute the program using:

`mpirun -np <nProcess> ./nbody <nParticle> <nTimestep> <sizeTimestep>`

### Parameters

* `<nProcess>` – number of MPI processes.
* `<nParticle>` – total number of particles in the simulation.
* `<nTimestep>` – number of simulation timesteps.
* `<sizeTimestep>` – timestep size used for numerical integration.

### Example

`mpirun -np 8 ./nbody 1024 500 1`

This runs the simulation with:

* 8 MPI processes
* 1024 particles
* 500 timesteps
* timestep size of 1

## Notes

The number of particles must be evenly divisible by the number of MPI processes:

`nParticle % nProcess == 0`

This requirement ensures that each MPI process is assigned the same number of particles.
