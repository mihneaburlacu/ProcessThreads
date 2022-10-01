#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "a2_helper.h"
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>

#define NR_THREADS_PROCESS_3 4
#define NR_THREADS_PROCESS_7 45
#define NR_THREADS_PROCESS_5 6

typedef struct THREAD_STRUCT{
	//pthread_mutex_t *locker1; //lacatul 1
	//pthread_mutex_t *locker2; //lacatul 2
	sem_t *semaphoreFirst; //semaforul 1
	sem_t *semaphoreSecond; //semaforul 2
	int nr_thread; //numarul curent al thread-ului
	int nr_process; //numarul curent al procesului
}THREAD_STRUCT;

pthread_t threads_process_three[NR_THREADS_PROCESS_3];
pthread_t threads_process_seven[NR_THREADS_PROCESS_7];
pthread_t threads_process_five[NR_THREADS_PROCESS_5];

//pthread_mutex_t locker1;
//pthread_mutex_t locker2;
sem_t *semaphore1; //semafor cu nume 1
sem_t *semaphore2; //semafor cu nume 2

void *thread_function_process_three(void *parameter) 
{
	THREAD_STRUCT *th = (THREAD_STRUCT *) parameter;

	if((th->nr_thread == 2 || th->nr_thread == 3 || th->nr_thread == 4) && th->nr_process == 3) {
		sem_wait(th->semaphoreFirst);
	}

	if(th->nr_thread == 3 && th->nr_process == 3) {
		sem_wait(semaphore1);
	}

	info(BEGIN, th->nr_process, th->nr_thread);

	if(th->nr_thread == 1 && th->nr_process == 3) {
		sem_post(th->semaphoreFirst);
		sem_post(th->semaphoreFirst);
		sem_post(th->semaphoreFirst);
		sem_wait(th->semaphoreSecond);
	}

	info(END, th->nr_process, th->nr_thread);

	if(th->nr_thread == 2 && th->nr_process == 3) {
		sem_post(th->semaphoreSecond);
	}

	if(th->nr_thread == 3 && th->nr_process == 3) {
		sem_post(semaphore2);
	}

	return NULL;
}

void *thread_function_process_seven(void *parameter) 
{
	THREAD_STRUCT *th = (THREAD_STRUCT *) parameter;

	sem_wait(th->semaphoreFirst);

	info(BEGIN, th->nr_process, th->nr_thread);

	info(END, th->nr_process, th->nr_thread);

	sem_post(th->semaphoreFirst);
	
	return NULL;
}

void *thread_function_process_five(void *parameter) 
{
	THREAD_STRUCT *th = (THREAD_STRUCT *) parameter;

	if(th->nr_thread == 1 && th->nr_process == 5) {
		sem_wait(semaphore2);
	}

	info(BEGIN, th->nr_process, th->nr_thread);

	info(END, th->nr_process, th->nr_thread);

	if(th->nr_thread == 5 && th->nr_process == 5) {
		sem_post(semaphore1);
	}
	
	return NULL;
}

void create_hierarchy() 
{
	pid_t pidP2 = -1; //identificatorul unic pt procesul 2
	pid_t pidP3 = -1;
	pid_t pidP4 = -1;
	pid_t pidP5 = -1;
	pid_t pidP6 = -1;
	pid_t pidP7 = -1;

	pidP2 = fork(); //procesul 2
	if(pidP2== 0) {
		info(BEGIN, 2, 0);

		pidP3 = fork(); //procesul 3
		if(pidP3 == 0) {
			info(BEGIN, 3, 0);

			sem_t semaphoreFirst;
			sem_t semaphoreSecond;

			THREAD_STRUCT parameters[NR_THREADS_PROCESS_3];
			sem_init(&semaphoreFirst, 0, 0);
			sem_init(&semaphoreSecond, 0, 0);

			for(int i = 1; i <= NR_THREADS_PROCESS_3; i++) {
				parameters[i - 1].nr_thread = i;
				parameters[i - 1].nr_process = 3;
				parameters[i - 1].semaphoreFirst = &semaphoreFirst;
				parameters[i - 1].semaphoreSecond = &semaphoreSecond;
			}

			for(int i = 1; i <= NR_THREADS_PROCESS_3; i++) { //4 thread-uri pt procesul 3
				pthread_create(&threads_process_three[i - 1], NULL, thread_function_process_three, &parameters[i - 1]);
			}

			for(int i = 1; i <= NR_THREADS_PROCESS_3; i++) {
				pthread_join(threads_process_three[i - 1], NULL);
			}

			info(END, 3, 0);
			exit(3);

			sem_destroy(&semaphoreFirst);
			sem_destroy(&semaphoreSecond);
		}
		else {
			pidP5 = fork(); //procesul 5
			if(pidP5 == 0) {

				info(BEGIN, 5, 0);

				THREAD_STRUCT parametersProcessFive[NR_THREADS_PROCESS_5];

				for(int i = 1; i <= NR_THREADS_PROCESS_5; i++) {
					parametersProcessFive[i - 1].nr_thread = i;
					parametersProcessFive[i - 1].nr_process = 5;
				}

				for(int i = 1; i <= NR_THREADS_PROCESS_5; i++) { //6 thread-uri pt procesul 5
					pthread_create(&threads_process_five[i - 1], NULL, thread_function_process_five, &parametersProcessFive[i - 1]);
				}

				for(int i = 1; i <= NR_THREADS_PROCESS_5; i++) {
					pthread_join(threads_process_five[i - 1], NULL);
				}

					info(END, 5, 0);
					exit(5);

			}
			else {
				pidP6 = fork(); //procesul 6
				if(pidP6 == 0) {
					info(BEGIN, 6, 0);

					info(END, 6, 0);
					exit(6);
				}
			}

		}
		//wait for process 3, 5, 6
		waitpid(pidP3, NULL, 0);
		waitpid(pidP5, NULL, 0);
		waitpid(pidP6, NULL, 0);

		info(END, 2, 0);
		exit(2);
	}
	else {
		pidP4 = fork(); //procesul 4
		if(pidP4 == 0) {
			info(BEGIN, 4, 0);

			info(END, 4, 0);
			exit(4);
		}
		else {
			pidP7 = fork(); //procesul 7
			if(pidP7 == 0) {
				sem_t semaphoreOne;

				info(BEGIN, 7, 0);
				sem_init(&semaphoreOne, 0, 4);
				THREAD_STRUCT parametersProcessSeven[NR_THREADS_PROCESS_7];
				/*
				for(int i = 1; i <= NR_THREADS_PROCESS_7; i++) {
					parametersProcessSeven[i].nr_thread = i;
					parametersProcessSeven[i].nr_process = 7;
					parametersProcessSeven[i].semaphoreFirst = &semaphoreOne;
				}
				*/

				for(int i = 1; i <= NR_THREADS_PROCESS_7; i++) { //45 thread-uri pentru procesul 7
					parametersProcessSeven[i - 1].nr_thread = i;
					parametersProcessSeven[i - 1].nr_process = 7;
					parametersProcessSeven[i - 1].semaphoreFirst = &semaphoreOne;
					pthread_create(&threads_process_seven[i - 1], NULL, thread_function_process_seven, &parametersProcessSeven[i - 1]);
				}

				for(int i = 1; i <= NR_THREADS_PROCESS_7; i++) {
					pthread_join(threads_process_seven[i - 1], NULL);
				}

				sem_destroy(&semaphoreOne);

				info(END, 7, 0);
				exit(7);

			}
		}
	}
	//wait for process 2, 4, 7
	waitpid(pidP2, NULL, 0);
	waitpid(pidP4, NULL, 0);
	waitpid(pidP7, NULL, 0);

}

int main()
{
    init();

    info(BEGIN, 1, 0); //procesul 1

	semaphore1 = sem_open("semaphore_first", O_CREAT, 0644, 0); //semaforul cu nume 1
	semaphore2 = sem_open("semaphore_second", O_CREAT, 0644, 0); //semaforul cu nume 2
    
	create_hierarchy();

	sem_close(semaphore1);
	sem_close(semaphore2);

    info(END, 1, 0);
    exit(1);

    return 0;
}
