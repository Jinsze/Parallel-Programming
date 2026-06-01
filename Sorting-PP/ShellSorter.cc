#include "Sorters.hh"

#include <vector>
#include <algorithm>
#include <pthread.h>

// default implementation of Sorter::thread_body
void* Sorter::thread_body(void* args)
{
    return NULL; // does nothing by default
}

void ShellSorter::sort(uint64_t* array, int n)
{
    for (int gap = n / 2; gap > 0; gap /= 2) {
        for (int i = gap; i < n; i++) {
            uint64_t temp = array[i];
            int j = i;

            while (j >= gap && array[j - gap] > temp) {
                array[j] = array[j - gap];
                j -= gap;
            }

            array[j] = temp;
        }
    }
}

void ParallelShellSorter::sort(uint64_t* array, int n)
{
    m_array = array;
    m_size = n;

    pthread_t threads[m_nthreads];
    pthread_barrier_init(&barrier, NULL, m_nthreads);

    ParallelShellSorterArgs* args[m_nthreads];

    for (int i = 0; i < m_nthreads; i++) {
        args[i] = new ParallelShellSorterArgs(this, i);
        pthread_create(&threads[i], NULL, thread_create_helper, args[i]);
    }

    for (int i = 0; i < m_nthreads; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_barrier_destroy(&barrier);
}

void* ParallelShellSorter::thread_body(void* arg)
{
    ParallelShellSorterArgs* args = (ParallelShellSorterArgs*)arg;
    int tid = args->tid;

    for (int gap = m_size / 2; gap > 0; gap /= 2) {

        // Each thread handles different subsequences
        for (int start = tid; start < gap; start += m_nthreads) {

            // Insertion sort on this subsequence
            for (int i = start + gap; i < m_size; i += gap) {
                uint64_t temp = m_array[i];
                int j = i;

                while (j >= gap && m_array[j - gap] > temp) {
                    m_array[j] = m_array[j - gap];
                    j -= gap;
                }

                m_array[j] = temp;
            }
        }

        // Synchronize threads before next gap
        pthread_barrier_wait(&barrier);
    }

    return NULL;
}
