#include <assert.h>
#include "imageprocessing.h"
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>


#define max(a,b) \
    ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a > _b ? _a : _b;})
#define min(a,b) \
    ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a < _b ? _a : _b;})
#define MAX_PROCESSES 4 // can be altered, but only to perfect squares
#define MAX_THREADS 2 // must be kept as 2


struct args {
    imagem* img;
    int x_i, x_f, y_i, y_f, N;
};


void remove_extension(char* dest, char* src) {
    /* Remove the extension from a file path and store
     * @param dest: location to store extension
     * @param src: file path
     */

    int position, len = strlen(src);

    // get position of extension beginning
    for (int i = len; i >= 0; i--)
        if (src[i] == '.') {
            position = i;
            break;
        }

    // store extension
    for (int i = position; i <= len; i++)
        dest[i - position] = src[i];

    // remove extension from source
    src[position] = 0;
}


void get_output_path(char* dest, char* src) {
    /* Save output file path
     * @param dest: location to save output file path
     * @param src: original file path
     */

    char ext[strlen(src)];

    strcpy(dest, src);
    remove_extension(ext, dest);
    strcat(dest, "-out");
    strcat(dest, ext);
}


void blur_pixel(imagem* img, int x, int y, int N) {
    /* Blur one pixel
     * @param img: pointer to image object
     * @param x: x coordinate of pixel
     * @param y: y coordinate of pixel
     * @param N: distance from pixel to average by
     */

    float r, g, b;
    unsigned int count;

    r = g = b = count = 0;

    // calculate sum of all surrounding pixels
    for (int i = max(0, x - N); i <= min(x + N, img->width - 1); i++) {
        for (int j = max(0, y - N); j <= min(y + N, img->height - 1); j++) {
            r += img->r[i + j * img->width];
            g += img->g[i + j * img->width];
            b += img->b[i + j * img->width];
            count++;
        }
    }

    // new value is the average of surrounding pixels
    img->r[x + y * img->width] = r/count;
    img->g[x + y * img->width] = g/count;
    img->b[x + y * img->width] = b/count;
}


void async_proc_blur(imagem* img, int N) {
    /* Blur image using concurrent processes
     * @param img: pointer to image object
     * @param N: distance from pixel to average by
     */

    int x_i, x_f, y_i, y_f, div = (int) sqrt(MAX_PROCESSES);
    pid_t pid;

    // each process handles one section of the picture
    for (int p = 0; p < MAX_PROCESSES; p++) {

        pid = fork();

        // process is child: blur section of the picture
        if (pid == 0) {
            x_i = (p % div) * img->width/div;
            x_f = ((p % div) + 1) * img->width/div;
            y_i = (p / div) * img->height/div;
            y_f = ((p / div) + 1) * img->height/div;
            for (int i = x_i; i < x_f; i++)
                for (int j = y_i; j < y_f; j++)
                    blur_pixel(img, i, j, N);
            exit(0);
        }
    }

    // wait all processes finish to return
    for (int p = 0; p < MAX_PROCESSES; p++)
        waitpid(-1, NULL, 0);

}

void* async_thrd_blur_aux(void* args) {
    /* Blur image using concurrent threads auxiliary
     * @param args: pointer to struct containing arguments
     */

    // unpack arguments
    struct args* args_s = (struct args*) args;
    imagem* img = args_s->img;
    int x_i = args_s->x_i, x_f = args_s->x_f, y_i = args_s->y_i, y_f = args_s->y_f, N = args_s->N;

    // blur section of the picture
    for (int i = x_i; i < x_f; i++)
        for (int j = y_i; j < y_f; j++)
            blur_pixel(img, i, j, N);

    return NULL;
}

void async_thrd_blur(imagem* img, int N) {
    /* Blur image using concurrent threads
     * @param img: pointer to image object
     * @param N: distance from pixel to average by
     */

    pthread_t threads[MAX_THREADS];
    struct args* args_s = malloc(MAX_THREADS * sizeof(struct args));
    int div = (int) sqrt(MAX_THREADS);

    // each thread handles one half of the picture
    for (int i = 0; i < MAX_THREADS; i++) {
        (args_s + i)->img = img;
        (args_s + i)->N = N;
        (args_s + i)->x_i = i* img->width/2;
        (args_s + i)->x_f = (i + 1) * img->width/2;
        (args_s + i)->y_i = 0;
        (args_s + i)->y_f = img->height;
        pthread_create(threads + i, NULL, async_thrd_blur_aux, (void*) (args_s + i));
    }

    // wait all threads finish to return
    for (int i = 0; i < MAX_THREADS; i++)
        pthread_join(threads[i], NULL);

    free(args_s);
}


void sync_blur(imagem* img, int N) {
    /* Blur image synchronously
     * @param img: pointer to image object
     * @param N: distance from pixel to average by
     */
    for (int i = 0; i < img->width; i++)
        for (int j = 0; j < img->height; j++)
            blur_pixel(img, i, j, N);
}


int main(int argc, char *argv[]) {

    // usage help
    if (argc != 4) {
        printf("usage: ./main <path> <N> <method>\n");
        printf("path: path/to/image\n");
        printf("N: blur intensity\n");
        printf("method: sync, proc or thrd\n");
        exit(1);
    }

    // output path
    char out[strlen(argv[1])];

    // blur intensity
    int N = atoi(argv[2]);

    // flags to map memory
    int PROT = PROT_READ | PROT_WRITE;
    int FLAGS = MAP_SHARED | MAP_ANONYMOUS;

    // image object
    imagem* img;

    // variables to measure time
    float seconds;
    clock_t start, end;

    // open image
    assert(img = mmap(NULL, sizeof(imagem), PROT, FLAGS, 0, 0));
    *img = abrir_imagem(argv[1]);

    // start timer
    start = clock();

    // blur picture using entered method
    if      (!strcmp(argv[3], "sync")) sync_blur(img, N);
    else if (!strcmp(argv[3], "proc")) async_proc_blur(img, N);
    else if (!strcmp(argv[3], "thrd")) async_thrd_blur(img, N);

    // end timer
    end = clock();

    // save output
    get_output_path(out, argv[1]);
    salvar_imagem(out, img);
    liberar_imagem(img);

    // output time
    seconds = (float) (end - start) / CLOCKS_PER_SEC;
    printf("Tempo: %.9fs\n", seconds);

    return 0;
}