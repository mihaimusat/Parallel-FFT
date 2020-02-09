/* Musat Mihai-Robert
 * Grupa 332CB
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <pthread.h>
#include <complex.h>

#define min(x, y) (((x) < (y)) ? (x) : (y))

typedef double complex comp;

char* input_file; 
char* output_file;
int N, P;
double* data;
comp* result;

// functie prin care preiau argumentele programului in ordinea ceruta :
// argv[1] = input_file, argv[2] = output_file, argv[3] = numarul de thread-uri (P)
void getArgs(int argc, char **argv) {
	
	if (argc < 4) {
		printf("Not enough parameters: ./program input_file output_file P\n");
        	exit(1);
    	}

	// alocare memorie pentru input_file si output_file
    	input_file = malloc(strlen(argv[1]) + 1);
    	output_file = malloc(strlen(argv[2]) + 1);

	// verificare daca alocarea s-a facut cu succes
	if (input_file == NULL || output_file == NULL) {
        	printf("malloc failed!");
        	exit(1);
    	}

	// copiere argv[1] in input_file si argv[2] in output_file
    	memcpy(input_file, argv[1], strlen(argv[1]) + 1);
    	memcpy(output_file, argv[2], strlen(argv[2]) + 1);

    	P = atoi(argv[3]);
}

// functie prin care eliberez memoria alocata
void cleanMemory() {
	
	free(data);
	free(result);
	free(input_file);
	free(output_file);
}

// functie care calculeaza Fourier Transform
// si care reprezinta locul de unde incepe executia pentru thread-uri
void* threadFunction(void *var) {
	
	int tid = *(int*)var;
	
	// calculare indici de start si end conform laboratorului
    	int start = tid * ceil((double)N / P);
    	int end = min(N, (tid + 1) * ceil((double)N / P));	

	// variabile auxiliare pentru o scriere mai lizibila
	double double_pi = 2.0 * M_PI;
    	double angle, cosinus, sinus;
    	double inv = 1.0 / N;

	int i, j;

	// paralelizez for-ul exterior deoarece este mai eficient
	// si ma folosesc de faptul ca e ^ (i * x) = (cos x) + i * (sin x)
    	for(i = start; i < end; i++) {
        	result[i] = 0;
        	for(j = 0; j < N; j++) {
            		angle = double_pi * i * j * inv;
            		cosinus = cos(angle);
            		sinus = sin(angle); 
            		result[i] += data[j] * cosinus - I * data[j] * sinus;
        	}
    	}

	return NULL;
}

int main(int argc, char **argv) {
	
	// obtinere argumente pentru program 
	getArgs(argc, argv);
	
	FILE* fin;
        FILE* fout;

	// deschidere fisiere de input si output
	fin = fopen(input_file, "r");
	fout = fopen(output_file, "w");

	// citire numar de elemente ale vectorului de input
	fscanf(fin, "%d", &N);

	// alocare memorie pentru vectorii folositi
	// data = vector de input 
	// result = vector de output
	data = malloc(sizeof(double) * N);
	result = malloc(sizeof(comp) * N);

	// verificare daca alocarea s-a facut cu succes
	if (data == NULL || result == NULL) {
        	printf("malloc failed!");
        	exit(1);
    	}

	// citire vector de input
	int i;
	for(i = 0; i < N; i++) {
		fscanf(fin, "%lf", &data[i]);
	}

	// initializarea vectorului pentru thread_id 
	pthread_t tid[P];
	int thread_id[P];
	for(i = 0; i < P; i++) {
		thread_id[i] = i;
	}

	// creare thread-uri
	for(i = 0; i < P; i++) {
		pthread_create(&(tid[i]), NULL, threadFunction, &(thread_id[i]));
	}

	// apelare join pentru thread-urile create
	for(i = 0; i < P; i++) {
		pthread_join(tid[i], NULL);
	}

	// scriere rezultate in fisierul de output
	fprintf(fout, "%d\n", N);
	for(i = 0; i < N; i++) {
		fprintf(fout, "%lf %lf\n", creal(result[i]), cimag(result[i]));
	}
	
	// inchidere fisiere
	fclose(fin);
	fclose(fout);

	// eliberare memorie folosita
	cleanMemory();

	return 0;
}
