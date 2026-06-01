#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include <omp.h>

#include "kmeans.h"

extern double wtime(void);
extern int num_omp_threads;

/* Euclidean distance */
float euclid_dist_2(float *pt1, float *pt2, int dim) {
    float ans = 0.0;
    for (int i = 0; i < dim; i++)
        ans += (pt1[i] - pt2[i]) * (pt1[i] - pt2[i]);
    return ans;
}

/* Find nearest cluster */
int find_nearest_point(float *pt, int nfeatures, float **clusters, int nclusters) {
    int index = 0;
    float min_dist = euclid_dist_2(pt, clusters[0], nfeatures);

    for (int i = 1; i < nclusters; i++) {
        float dist = euclid_dist_2(pt, clusters[i], nfeatures);
        if (dist < min_dist) {
            min_dist = dist;
            index = i;
        }
    }
    return index;
}

/*----< serial_clustering() >---------------------------------------------*/
float** serial_clustering(float **feature,    /* in: [npoints][nfeatures] */
                          int     nfeatures,
                          int     npoints,
                          int     nclusters,
                          float   threshold,
                          int    *membership) /* out: [npoints] */
{

    float  **clusters;   /* out: [nclusters][nfeatures] */
    /* allocate space for returning variable clusters[] */
    clusters    = (float**) malloc(nclusters *             sizeof(float*));
    clusters[0] = (float*)  malloc(nclusters * nfeatures * sizeof(float));
    for (int i=1; i<nclusters; i++)
        clusters[i] = clusters[i-1] + nfeatures;

    /* randomly pick cluster centers */
    int n=0;
    for (int i=0; i<nclusters; i++) {
        //n = (int)rand() % npoints;
        for (int j=0; j<nfeatures; j++)
            clusters[i][j] = feature[n][j];
		n++;
    }

    // PUT YOUR CODE HERE
    int *new_centers_len = (int*) calloc(nclusters, sizeof(int));
    float **new_centers = (float**) malloc(nclusters * sizeof(float*));
    new_centers[0] = (float*) calloc(nclusters * nfeatures, sizeof(float));
    for (int i = 1; i < nclusters; i++)
        new_centers[i] = new_centers[i-1] + nfeatures;

    /* initialize membership */
    for (int i = 0; i < npoints; i++)
        membership[i] = -1;

    float delta;
    int loop = 0;

    do {
        delta = 0.0;

        /* reset */
        for (int i = 0; i < nclusters; i++) {
            new_centers_len[i] = 0;
            for (int j = 0; j < nfeatures; j++)
                new_centers[i][j] = 0.0;
        }

        /* assignment step */
        for (int i = 0; i < npoints; i++) {
            int index = find_nearest_point(feature[i], nfeatures, clusters, nclusters);

            if (membership[i] != index) {
                delta += 1.0;
                membership[i] = index;
            }

            new_centers_len[index]++;
            for (int j = 0; j < nfeatures; j++)
                new_centers[index][j] += feature[i][j];
        }

        /* update step */
        for (int i = 0; i < nclusters; i++) {
            if (new_centers_len[i] > 0) {
                for (int j = 0; j < nfeatures; j++)
                    clusters[i][j] = new_centers[i][j] / new_centers_len[i];
            }
        }

        delta /= npoints;
        loop++;

    } while (delta > threshold && loop < 1000);

    free(new_centers_len);
    free(new_centers[0]);
    free(new_centers);
    
    return clusters;
}


/*----< parallel_clustering() >---------------------------------------------*/
float** parallel_clustering(float **feature,    /* in: [npoints][nfeatures] */
                          int     nfeatures,
                          int     npoints,
                          int     nclusters,
                          float   threshold,
                          int    *membership) /* out: [npoints] */
{

	float  **clusters;					/* out: [nclusters][nfeatures] */     int      nthreads;

    nthreads = num_omp_threads; 

    /* allocate space for returning variable clusters[] */
    clusters    = (float**) malloc(nclusters *             sizeof(float*));
    clusters[0] = (float*)  malloc(nclusters * nfeatures * sizeof(float));
    for (int i=1; i<nclusters; i++)
        clusters[i] = clusters[i-1] + nfeatures;

    /* randomly pick cluster centers */
    int n=0;
    for (int i=0; i<nclusters; i++) {
        //n = (int)rand() % npoints;
        for (int j=0; j<nfeatures; j++)
            clusters[i][j] = feature[n][j];
		n++;
    }


    // PUT YOUR CODE HERE
    omp_set_num_threads(nthreads);

    /* global accumulators */
    int *new_centers_len = (int*) calloc(nclusters, sizeof(int));
    float **new_centers = (float**) malloc(nclusters * sizeof(float*));
    new_centers[0] = (float*) calloc(nclusters * nfeatures, sizeof(float));
    for (int i = 1; i < nclusters; i++)
        new_centers[i] = new_centers[i-1] + nfeatures;

    /* thread-local accumulators */
    float ***partial_new_centers = (float***) malloc(nthreads * sizeof(float**));
    int **partial_new_centers_len = (int**) malloc(nthreads * sizeof(int*));

    for (int t = 0; t < nthreads; t++) {
        partial_new_centers[t] = (float**) malloc(nclusters * sizeof(float*));
        partial_new_centers[t][0] = (float*) calloc(nclusters * nfeatures, sizeof(float));

        for (int i = 1; i < nclusters; i++)
            partial_new_centers[t][i] = partial_new_centers[t][i-1] + nfeatures;

        partial_new_centers_len[t] = (int*) calloc(nclusters, sizeof(int));
    }

    /* initialize membership */
    for (int i = 0; i < npoints; i++)
        membership[i] = -1;

    float delta;
    int loop = 0;

    do {
        delta = 0.0;

        /* reset global */
        for (int i = 0; i < nclusters; i++) {
            new_centers_len[i] = 0;
            for (int j = 0; j < nfeatures; j++)
                new_centers[i][j] = 0.0;
        }

        /* reset local */
        for (int t = 0; t < nthreads; t++) {
            for (int i = 0; i < nclusters; i++) {
                partial_new_centers_len[t][i] = 0;
                for (int j = 0; j < nfeatures; j++)
                    partial_new_centers[t][i][j] = 0.0;
            }
        }


        /* parallel region */
        int i, j, index;

        #pragma omp parallel \
            shared(feature, clusters, membership, partial_new_centers, partial_new_centers_len, npoints, nfeatures, nclusters) \
            private(i, j, index) \
            reduction(+:delta)
        {
            int tid = omp_get_thread_num();

            #pragma omp for schedule(static)
            for (i = 0; i < npoints; i++) {

                index = find_nearest_point(feature[i], nfeatures, clusters, nclusters);

                if (membership[i] != index) {
                    delta += 1.0;
                }

                membership[i] = index;

                partial_new_centers_len[tid][index]++;

                for (j = 0; j < nfeatures; j++)
                    partial_new_centers[tid][index][j] += feature[i][j];
            }
        }

        /* reduction (main thread) */
        for (int i = 0; i < nclusters; i++) {
            for (int t = 0; t < nthreads; t++) {
                new_centers_len[i] += partial_new_centers_len[t][i];
                for (int j = 0; j < nfeatures; j++)
                    new_centers[i][j] += partial_new_centers[t][i][j];
            }
        }

        /* update clusters */
        for (int i = 0; i < nclusters; i++) {
            if (new_centers_len[i] > 0) {
                for (int j = 0; j < nfeatures; j++)
                    clusters[i][j] = new_centers[i][j] / new_centers_len[i];
            }
        }

        delta /= npoints;
        loop++;

    } while (delta > threshold && loop < 1000);

    /* free memory */
    free(new_centers_len);
    free(new_centers[0]);
    free(new_centers);

    for (int t = 0; t < nthreads; t++) {
        free(partial_new_centers[t][0]);
        free(partial_new_centers[t]);
        free(partial_new_centers_len[t]);
    }
    free(partial_new_centers);
    free(partial_new_centers_len);

    return clusters;
}

