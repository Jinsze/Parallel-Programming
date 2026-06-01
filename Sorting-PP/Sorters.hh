#ifndef SORTERS
#define SORTERS

#include <stdint.h>
#include <pthread.h>

class Sorter;

class SorterArgs {
public:
  SorterArgs(Sorter* s)
    : m_this(s)
  {}

  Sorter* m_this;
};

class Sorter {
public:
  virtual void sort(uint64_t* array, int array_size) {}

protected:
  virtual void* thread_body(void* args); //declare only

  static void* thread_create_helper(void* args) {
    SorterArgs* a = (SorterArgs*) args;
    return a->m_this->thread_body(args);
  }
};

class ShellSorter : public Sorter {
public:
  ShellSorter() {}

  void sort(uint64_t* array, int array_size);
};

class ParallelShellSorter : public Sorter {
public:
  ParallelShellSorter(int nthreads) {
    m_nthreads = nthreads;
  }
  void sort(uint64_t* array, int array_size);

private:
  void* thread_body(void* arg);

private:
  int m_nthreads;

  uint64_t* m_array;
  int m_size;
  pthread_barrier_t barrier;
};

class ParallelShellSorterArgs : public SorterArgs {
public:
  ParallelShellSorterArgs(ParallelShellSorter* s, int _tid)
    : SorterArgs(s), tid(_tid)
  {}

  int tid;
};

class RadixSorter : public Sorter {
public:
  RadixSorter() {}
  void sort(uint64_t* array, int array_size);
};

class ParallelRadixSorter : public Sorter {
public:
  ParallelRadixSorter(int nthreads) 
    : m_nthreads(nthreads)
  {}
  void sort(uint64_t* array, int array_size);

private:
  void* thread_body(void* arg);

private:
  int m_nthreads;
  uint64_t* m_array;
  uint64_t* temp_array;

  int m_size;

  int* nzeros;
  int* nones;

  pthread_barrier_t barrier;

};

class ParallelRadixSorterArgs : public SorterArgs {
public:
  ParallelRadixSorterArgs(ParallelRadixSorter* s, int _tid)
    : SorterArgs(s), tid(_tid)
  {}

  int tid;
};

#endif
