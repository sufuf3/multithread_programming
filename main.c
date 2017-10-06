#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <pthread.h>
#include <sched.h>

#include "primitives.h"
#include "raytracing.h"

#define OUT_FILENAME "out.ppm"

#define ROWS 512
#define COLS 512

static void write_to_ppm(FILE *outfile, uint8_t *pixels,
                         int width, int height)
{
    fprintf(outfile, "P6\n%d %d\n%d\n", width, height, 255);
    fwrite(pixels, 1, height * width * 3, outfile);
}

static double diff_in_second(struct timespec t1, struct timespec t2)
{
    struct timespec diff;
    if (t2.tv_nsec-t1.tv_nsec < 0) {
        diff.tv_sec  = t2.tv_sec - t1.tv_sec - 1;
        diff.tv_nsec = t2.tv_nsec - t1.tv_nsec + 1000000000;
    } else {
        diff.tv_sec  = t2.tv_sec - t1.tv_sec;
        diff.tv_nsec = t2.tv_nsec - t1.tv_nsec;
    }
    return (diff.tv_sec + diff.tv_nsec / 1000000000.0);
}

int main()
{
    uint8_t *pixels;
    light_node lights = NULL;
    rectangular_node rectangulars = NULL;
    sphere_node spheres = NULL;
    color background = { 0.0, 0.1, 0.1 };
    struct timespec start, end;

#include "use-models.h"

    /* allocate by the given resolution */
    pixels = malloc(sizeof(unsigned char) * ROWS * COLS * 3);
    if (!pixels) exit(-1);

    int input_status = 0;
    //pthread_t *nthread;
    printf(" Please input the status:\n "
           " 1) One thread four cores.\n "
           " 2) Two threads two cores.\n "
           " 3) Two threads one core.\n "
           " Please choose the number:");
    scanf( " %d",  &input_status);
    if (input_status < 1 || input_status > 3) {
        printf("Wrong input!");
    }
    printf("\n\n# Rendering scene\n");
    /* do the ray tracing with the given geometry */
    clock_gettime(CLOCK_REALTIME, &start);
    //TODO
    /*This is a domain function for this program.
     * Please trace the parameter of it and create your threads to do the function*/
    if (input_status == 1) {
        /*pthread_t * thread_num = ( pthread_t *) malloc ( 1 * sizeof( pthread_t));
        rayargs** pr = (rayargs **) malloc( 1 * sizeof(  rayargs * ));
        for( int num = 0; num < 4; num++) {
            printf("Acore: %d \n", num);
            pr[0] = ray(pixels, background,
                               rectangulars, spheres, lights, &view, ROWS, COLS, num, 4);
            pthread_create(&thread_num[0], NULL, (void *) &raytracing, (void *) pr[0]);
            printf("Bcore: %d \n", num);
            cpu_set_t cpus;
            CPU_ZERO(&cpus);
            CPU_SET(num, &cpus);
            pthread_setaffinity_np(thread_num[0], sizeof(cpu_set_t), &cpus);
            printf("core: %d\n", num);
        }
        pthread_join(thread_num[0], NULL);*/
    } else if (input_status == 2) {
        pthread_t * thread_num = ( pthread_t *) malloc ( 2 * sizeof( pthread_t));
        rayargs** pr = (rayargs **) malloc( 2 * sizeof(  rayargs * ));
        for( int num = 0; num < 2; num++) {
            pr[num] = ray(pixels, background,
                          rectangulars, spheres, lights, &view, ROWS, COLS, num, 2);
            pthread_create(&thread_num[num], NULL, (void *) &raytracing, (void *) pr[num]);
            cpu_set_t cpus;
            CPU_ZERO(&cpus);
            CPU_SET(num, &cpus);
            pthread_setaffinity_np(thread_num[num], sizeof(cpu_set_t), &cpus);
        }
        for( int num = 0; num < 2; num++) {
            pthread_join(thread_num[num], NULL);
        }
    } else if (input_status == 3) {
        printf("Not yet");
        //pthread_t * thread_num = ( pthread_t *) malloc ( 2 * sizeof( pthread_t));
    } else {
        printf("There are something worng!");
    }

    /*raytracing(pixels, background,
               rectangulars, spheres, lights, &view, ROWS, COLS);*/
    clock_gettime(CLOCK_REALTIME, &end);
    {
        FILE *outfile = fopen(OUT_FILENAME, "wb");
        write_to_ppm(outfile, pixels, ROWS, COLS);
        fclose(outfile);
    }

    delete_rectangular_list(&rectangulars);
    delete_sphere_list(&spheres);
    delete_light_list(&lights);
    free(pixels);
    printf("Done!\n");
    printf("Execution time of raytracing() : %lf sec\n", diff_in_second(start, end));
    return 0;
}
