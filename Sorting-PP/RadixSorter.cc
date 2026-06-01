#include "Sorters.hh"

#include <vector>
#include <algorithm>

// default implementation of Sorter::thread_body
void* Sorter::thread_body(void* args)
{
    return NULL; // does nothing by default
}


void RadixSorter::sort(uint64_t* array, int n)
{
    uint64_t* temp = new uint64_t[n];

    for (int shift = 0; shift < 64; shift += 8) {
        int count[256] = {0};

        // count
        for (int i = 0; i < n; i++) {
            int byte = (array[i] >> shift) & 0xFF;
            count[byte]++;
        }

        // prefix sum
        for (int i = 1; i < 256; i++) {
            count[i] += count[i - 1];
        }

        // sort
        for (int i = n - 1; i >= 0; i--) {
            int byte = (array[i] >> shift) & 0xFF;
            temp[--count[byte]] = array[i];
        }

        // copy back
        for (int i = 0; i < n; i++) {
            array[i] = temp[i];
        }
    }

    delete[] temp;
}

void ParallelRadixSorter::sort(uint64_t* array, int n)
{
    m_array = array;
    m_size = n;

    temp_array = new uint64_t[n];

    nzeros = new int[m_nthreads];
    nones  = new int[m_nthreads];

    pthread_barrier_init(&barrier, NULL, m_nthreads);

    pthread_t threads[m_nthreads];
    ParallelRadixSorterArgs* args[m_nthreads];

    for (int i = 0; i < m_nthreads; i++) {
        args[i] = new ParallelRadixSorterArgs(this, i);
        pthread_create(&threads[i], NULL, thread_create_helper, args[i]);
    }

    for (int i = 0; i < m_nthreads; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_barrier_destroy(&barrier);

    delete[] temp_array;
    delete[] nzeros;
    delete[] nones;
}

void* ParallelRadixSorter::thread_body(void* arg)
{
    ParallelRadixSorterArgs* args = (ParallelRadixSorterArgs*)arg;
    int tid = args->tid;

    int chunk = m_size / m_nthreads;
    int start = tid * chunk;
    int end = (tid == m_nthreads - 1) ? m_size : start + chunk;

    for (int bit = 0; bit < 64; bit++) {

        // STEP 1: COUNT
        int zero = 0, one = 0;

        for (int i = start; i < end; i++) {
            if ((m_array[i] >> bit) & 1)
                one++;
            else
                zero++;
        }

        nzeros[tid] = zero;
        nones[tid] = one;

        pthread_barrier_wait(&barrier);

        // STEP 2: PREFIX SUM
        int index0 = 0;
        int index1 = 0;

        for (int i = 0; i < tid; i++) {
            index0 += nzeros[i];
            index1 += nones[i];
        }

        int total_zeros = 0;
        for (int i = 0; i < m_nthreads; i++) {
            total_zeros += nzeros[i];
        }

        index1 += total_zeros;

        pthread_barrier_wait(&barrier);

        // STEP 3: DISTRIBUTE
        for (int i = start; i < end; i++) {
            if ((m_array[i] >> bit) & 1)
                temp_array[index1++] = m_array[i];
            else
                temp_array[index0++] = m_array[i];
        }

        pthread_barrier_wait(&barrier);

        // STEP 4: SWAP
        pthread_barrier_wait(&barrier);

        if (tid == 0) {
            uint64_t* tmp = m_array;
            m_array = temp_array;
            temp_array = tmp;
        }

        pthread_barrier_wait(&barrier);
    }

    return NULL;
}