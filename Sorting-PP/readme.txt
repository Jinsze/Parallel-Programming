# Sorting Lab (Pthread)

## Compile

make

## Run

### Sequential ShellSort

./shellsort -s <array_size> -r 1234

### Parallel ShellSort

./shellsort-parallel -s <array_size> -n <threads> -r 1234

### Sequential RadixSort

./radixsort -s <array_size> -r 1234

### Parallel RadixSort

./radixsort-parallel -s <array_size> -n <threads> -r 1234

## Parameters

-s : array size (e.g. 5000000, 10000000, 30000000)
-n : number of threads (1,2,4,8,16,32,64)
-r : random seed (use 1234 for consistency)

