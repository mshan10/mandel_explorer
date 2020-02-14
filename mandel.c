// ./mandel -s .0001 -m 1000 -x -0.74529 -y 0.113075 -H 1200 -W 1200

#include "bitmap.h"

#include <pthread.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <string.h>

struct thread_args {
	struct bitmap *bm; 
    double xmin; 
    double xmax; 
    double ymin; 
    double ymax; 
    int max;
    int threads_total;
	int thread_curr;
};

struct thread_args * thread_args_create(struct bitmap *bm, double xmin, double xmax, double ymin, double ymax, int max, int threads_total) {
	struct thread_args *t;

	t = malloc(sizeof *t);
	if(!t) return 0;
	t->bm = bm;
	t->xmin = xmin;
	t->xmax = xmax;
    t->ymin = ymin;
	t->ymax = ymax;
    t->max = max;
    t->threads_total = threads_total;
	return t;
}

int iteration_to_color( int i, int max );
int iterations_at_point( double x, double y, int max );
void  *compute_image( void *args );
void process_input(int argc, char *argv[]);

void show_help() {
	printf("Use: mandel [options]\n");
	printf("Where options are:\n");
	printf("-m <max>    The maximum number of iterations per point. (default=1000)\n");
	printf("-n <threads>The number of threads. (default=1)\n");
	printf("-x <coord>  X coordinate of image center point. (default=0)\n");
	printf("-y <coord>  Y coordinate of image center point. (default=0)\n");
	printf("-s <scale>  Scale of the image in Mandlebrot coordinates. (default=4)\n");
	printf("-W <pixels> Width of the image in pixels. (default=500)\n");
	printf("-H <pixels> Height of the image in pixels. (default=500)\n");
	printf("-o <file>   Set output file. (default=mandel.bmp)\n");
	printf("-h          Show this help text.\n");
	printf("\nSome examples are:\n");
	printf("mandel -x -0.5 -y -0.5 -s 0.2\n");
	printf("mandel -x -.38 -y -.665 -s .05 -m 100\n");
	printf("mandel -x 0.286932 -y 0.014287 -s .0005 -m 1000\n\n");
}

int main(int argc, char *argv[]) {
    process_input(argc, argv);
}

void process_input(int argc, char *argv[]) {
    char c;

    // These are the default configuration values used
	// if no command line arguments are given.

	const char *outfile = "mandel.bmp";
	double xcenter = 0;
	double ycenter = 0;
	double scale = 4;
	int    image_width = 500;
	int    image_height = 500;
	int    max = 1000;
    int    threads_total = 1;

	// For each command line argument given,
	// override the appropriate configuration value.

    while((c = getopt(argc,argv,"x:y:s:W:H:m:n:o:h"))!=-1) {
		switch(c) {
			case 'x':
				xcenter = atof(optarg);
				break;
			case 'y':
				ycenter = atof(optarg);
				break;
			case 's':
				scale = atof(optarg);
				break;
			case 'W':
				image_width = atoi(optarg);
				break;
			case 'H':
				image_height = atoi(optarg);
				break;
			case 'm':
				max = atoi(optarg);
				break;
            case 'n':
				threads_total = atoi(optarg);
				break;
			case 'o':
				outfile = optarg;
				break;
			case 'h':
				show_help();
				exit(1);
				break;
		}
	}

    // Display the configuration of the image.
	printf("mandel: x=%lf y=%lf scale=%lf max=%d outfile=%s\n",xcenter,ycenter,scale,max,outfile);

	// Create a bitmap of the appropriate size.
	struct bitmap *bm = bitmap_create(image_width,image_height);
    
	// Divide work into multiple threads
	pthread_t pt[threads_total];
	for(int i=0; i<threads_total; i++){
		// Construct thread arguments
		struct thread_args *ta = thread_args_create(bm,xcenter-scale,xcenter+scale,ycenter-scale,ycenter+scale,max,threads_total);
		ta->thread_curr = i;

		if(pthread_create(&pt[i],NULL,compute_image,ta)){
			perror("Error creating thread: ");
			exit( EXIT_FAILURE );
		}   
		
	}
	
	// Join threads_total
	for(int i=0; i<threads_total; i++){
		if(pthread_join(pt[i],NULL)){
			perror("Problem with pthread_join: "); 
		}
	}

	// Save the image in the stated file.
	if(!bitmap_save(bm,outfile)) {
		fprintf(stderr,"mandel: couldn't write to %s: %s\n",outfile,strerror(errno));
		exit(1);
	}

}

/*
Compute an entire Mandelbrot image, writing each point to the given bitmap.
Scale the image to the range (xmin-xmax,ymin-ymax), limiting iterations to "max"
*/

void * compute_image(void *argss) {
	int i,j;
	struct thread_args *args = argss;

	// Calculate dimensions to compute mandel for
	int width = bitmap_width(args->bm);
	int height = bitmap_height(args->bm);
	int thread_end;
	if (args->thread_curr + 1 == args->threads_total) {
		thread_end = height;
	} else {
		thread_end = (args->thread_curr + 1) * (height / args->threads_total);
	}
	int thread_height = height / args->threads_total;
	int thread_start = args->thread_curr * thread_height;

	// For every pixel in the image...
	for(j=thread_start; j< thread_end; j++) {

		for(i=0;i<width;i++) {

			// Determine the point in x,y space for that pixel.
			double x = args->xmin + i*(args->xmax-args->xmin)/width;
			double y = args->ymin + j*(args->ymax-args->ymin)/height;

			// Compute the iterations at that point.
			int iters = iterations_at_point(x,y,args->max);

			// Set the pixel in the bitmap.
			bitmap_set(args->bm,i,j,iters);
		}
	}
	
	return 0;
}

/*
Return the number of iterations at point x, y
in the Mandelbrot space, up to a maximum of max.
*/

int iterations_at_point(double x, double y, int max) {
	double x0 = x;
	double y0 = y;

	int iter = 0;

	while( (x*x + y*y <= 4) && iter < max ) {

		double xt = x*x - y*y + x0;
		double yt = 2*x*y + y0;

		x = xt;
		y = yt;

		iter++;
	}

	return iteration_to_color(iter,max);
}

/*
Convert a iteration number to an RGBA color.
Here, we just scale to gray with a maximum of imax.
Modify this function to make more interesting colors.
*/

int iteration_to_color(int i, int max) {
	int red = 100 * i*(i/1.394)/(max * 215);
	int green = 1000*i*(i/1.548)/(max * 220);
	int blue = i*460/255;
	return MAKE_RGBA(red,green,blue,0);
}
