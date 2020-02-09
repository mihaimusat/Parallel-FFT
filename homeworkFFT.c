/* Musat Mihai-Robert
 * Grupa 332CB
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <pthread.h>
#include <complex.h>

typedef double complex comp;

char* input_file; 
char* output_file;
int N, P;
comp* data;
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

// functie care calculeaza Fast Fourier Transform 
// si reprezinta locul de unde vor pleca thread-urile
void compute_fft(comp* data, comp* result, int N, int step) {
	
	if(step > N) {
		return;
	}
	
	if (step < N) {
		compute_fft(result, data, N, step * 2);
		compute_fft(result + step, data + step, N, step * 2);
 		
		int i;
		for (i = 0; i < N; i += 2 * step) {
			comp t = cexp(-I * M_PI * i / N) * result[i + step];
			data[i / 2] = result[i] + t;
			data[(i + N)/2] = result[i] - t;
		}
	}
}

// mai intai tratez cazul in care P = 1
// deci doar apelez functia pentru step = 1
void* threadFunction(void *var) {
	
	int tid = *(int*)var;
	compute_fft(data, result, N, 1);
	
	return NULL;
}

/* in cazul in care P = 2, inseamna ca
   trebuie sa ma duc un nivel mai jos in 
   recursivitate si fac step = 2
*/

// pornesc un thread din subarborele stang pentru P = 2
void* one(void *var) {
	
	int tid = *(int*)var;
	compute_fft(result, data, N, 2);

	return NULL;
}

// pornesc un thread din subarborele drept pentru P = 2
void* two(void *var) {
	
	int tid = *(int*)var;
	compute_fft(result + 1, data + 1, N, 2);

	return NULL;
}

/* in cazul in care P = 4, inseamna ca
   trebuie sa ma duc inca un nivel mai jos in 
   recursivitate si fac step = 4
*/

void* three(void *var) {
	
	int tid = *(int*)var;
	compute_fft(data, result, N, 4);

	return NULL;
}

void* four(void *var) {
	
	int tid = *(int*)var;
	compute_fft(data + 1, result + 1, N, 4);

	return NULL;
}

void* five(void *var) {
	
	int tid = *(int*)var;
	compute_fft(data + 2, result + 2, N, 4);

	return NULL;
}

void* six(void *var) {
	
	int tid = *(int*)var;
	compute_fft(data + 3, result + 3, N, 4);

	return NULL;
}

int main(int argc, char * argv[]) 
{
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
	data = malloc(sizeof(comp) * N);
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

	// initializarea vectorului de rezultate partiale
	// cu valorile din vectorul primit ca input
	for (i = 0; i < N; i++) {
		result[i] = data[i];
	}

	// initializarea vectorului pentru thread_id 
	pthread_t tid[P];
	int thread_id[P];
	for(i = 0; i < P; i++) {
		thread_id[i] = i;
	}

	// in functie de valoarea lui P (care poate sa fie 1, 2 sau 4)
	// pornesc thread-urile si unesc subarborii formati din recursivitate
	// pentru a ajunge la rezultatul final aflat in radacina arborelui construit
	switch(P) {

		case 1:
			pthread_create(&(tid[0]), NULL, threadFunction, &(thread_id[0]));
			pthread_join(tid[0], NULL);
			break;
		case 2:
			if(N > 1) {
				pthread_create(&(tid[0]), NULL, one, &(thread_id[0]));
				pthread_create(&(tid[1]), NULL, two, &(thread_id[1]));

				for(i = 0; i < P; i++) {
					pthread_join(tid[i], NULL);
				}
				
				// dupa ce am dat join, am grija sa unesc cei doi subarbori construiti
				for (i = 0; i < N; i += 2) {
					comp t = cexp(-I * M_PI * i / N) * result[i + 1];
					data[i / 2] = result[i] + t;
					data[(i + N)/2] = result[i] - t;
				}	
			}
			break;
		case 4:
			if(N > 2) {
				pthread_create(&(tid[0]), NULL, three, &(thread_id[0]));
				pthread_create(&(tid[1]), NULL, four, &(thread_id[1]));
				pthread_create(&(tid[2]), NULL, five, &(thread_id[2]));
				pthread_create(&(tid[3]), NULL, six, &(thread_id[3]));

				for(i = 0; i < P; i++) {
					pthread_join(tid[i], NULL);
				}
	
				// unesc mai intai toti subarborii din stanga
				for (i = 0; i < N; i += 4) {
					comp t = cexp(-I * M_PI * i / N) * data[i + 3];
					result[i / 2 + 1] = data[i + 1] + t;
					result[(i + N)/2 + 1] = data[i + 1] - t;
				}

				// apoi unesc si subarborii din dreapta
				for (i = 0; i < N; i += 4) {
					comp t = cexp(-I * M_PI * i / N) * data[i + 2];
					result[i / 2] = data[i] + t;
					result[(i + N)/2] = data[i] - t;
				}

				// la final, combin rezultatele obtinute
				for (i = 0; i < N; i += 2) {
					comp t = cexp(-I * M_PI * i / N) * result[i + 1];
					data[i / 2] = result[i] + t;
					data[(i + N)/2] = result[i] - t;
				}
			}
			break;
		
	}

	// scriere rezultate in fisierul de output
	fprintf(fout, "%d\n", N);
	for(i = 0; i < N; i++) {
		fprintf(fout, "%lf %lf\n", creal(data[i]), cimag(data[i]));
	}
	
	// inchidere fisiere
	fclose(fin);
	fclose(fout);

	// eliberare memorie folosita
	cleanMemory();

	return 0;
}
