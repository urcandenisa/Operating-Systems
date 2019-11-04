#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <sys/wait.h>
#include <time.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdbool.h>
#include "a2_helper.h"
int waitCond = 0;
bool end11 = false, once = false;
int ids[5], k;
int nrThreads;
typedef struct{
	int id;
	int id_proces;
	sem_t *logSem1;
	sem_t *logSem2;
} TH_STRUCT;

typedef struct{
	int value;
	sem_t *logSem;
	pthread_mutex_t *lock;
	pthread_cond_t *cond;
}THREAD_STRUCT;

sem_t *semafor1;
sem_t *semafor2;
sem_t *semafor3;
void *thread_function(void*arg){
	TH_STRUCT *s = (TH_STRUCT*)arg;

	if(s->id == 3 && s->id_proces == 5){
		info(BEGIN, 5, 3);
		sem_post(s->logSem1);
		sem_wait(s->logSem2);
		info(END, 5, 3);
		sem_post(semafor3);
		sem_post(semafor3);
	}else if(s->id == 1 && s->id_proces == 5){
		sem_wait(s->logSem1);
		info(BEGIN, 5, 1);
		info(END, 5, 1);
		sem_post(s->logSem2);
	}else if(s->id == 2 && s->id_proces == 5){
		sem_wait(semafor1);
		info(BEGIN, 5, 2);
		info(END, 5, 2);
		sem_post(semafor2);
	}else if(s->id == 4 && s->id_proces == 7){
		sem_wait(semafor3);
		info(BEGIN, 7, 4);
		info(END, 7, 4);
		sem_post(semafor1);
		sem_post(semafor1);
	}else if(s->id == 2 && s->id_proces == 7){
		sem_wait(semafor2);
		info(BEGIN, 7, 2);
		info(END, 7, 2);
	}else{
		info(BEGIN, s->id_proces, s->id);
		info(END, s->id_proces, s->id);
	}
	return NULL;
}

void *th_func(void *arg){
	TH_STRUCT *s = (TH_STRUCT*)arg;

	info(BEGIN, 5, 3);
	sem_post(s->logSem1);
	sem_wait(s->logSem2);
	info(END, 5, 3);



	return NULL;
}
/*
void *thread_function(void *arg){
	TH_STRUCT *s = (TH_STRUCT*)arg;
	pthread_mutex_lock(s->lock);
	while(waitCond == 0){
		pthread_cond_wait(s->cond, s->lock);
	}
	info(BEGIN, 5, s->id);
	info(END, 5, s->id);
	if(s->id == 2){
		sem_wait(semafor);
	}
	waitCond ++; 
	pthread_cond_signal(s->cond);
	pthread_mutex_unlock(s->lock);

	return NULL;
}

void *th_func(void*arg){
	TH_STRUCT *s = (TH_STRUCT*)arg;
	pthread_mutex_lock(s->lock);
	info(BEGIN, 5, 3);
	waitCond++;
	while(waitCond < 5){
		pthread_cond_wait(s->cond, s->lock);
	}
	info(END, 5, 3);
	pthread_mutex_unlock(s->lock);
	
	return NULL;
}
bool waitFor11 = false;
int nrThreads;
void *function1(void *arg){
	THREAD_STRUCT *s = (THREAD_STRUCT*)arg;
	sem_wait(s->logSem);
	info(BEGIN, 2, s->value);
	nrThreads ++;
	printf("NR THREADS : %d, thread running %d\n", nrThreads, s->value);
	if( s->value != 11 )
	{
			printf("NU E 11, intru aici cu %d\n", s->value);
			info(END, 2, s->value);
			nrThreads -- ;
			sem_post(s->logSem);
	}else{
		printf("E 11, intru aici, si am %d threaduri\n", nrThreads);
		while(nrThreads < 5){
			nrThreads++;
			sem_wait(s->logSem);
		}
		printf("ACUM AM %d THREADURI\n", nrThreads);

	//	sem_wait(s->logSem);
		info(END, 2, 11);
		sem_post(s->logSem);
		sem_post(s->logSem);
		sem_post(s->logSem);
		sem_post(s->logSem);
	}
//	info(END, 2, s->value);
//	nrThreads -- ;
//	sem_post(s->logSem);

	return NULL;
}*/

void *function(void*arg){
	THREAD_STRUCT *s = (THREAD_STRUCT*)arg;
	pthread_mutex_lock(s->lock);
	while(nrThreads == 0  || (nrThreads == 5 && end11 == false) ){
		pthread_cond_signal(s->cond);
		pthread_cond_signal(s->cond);
		pthread_cond_wait(s->cond, s->lock);
	}

	if(end11 == false){
		info(BEGIN, 2, s->value);
		ids[k++] = s->value;

	nrThreads ++;
	}
	if(once == false && end11 == true){
		for(int i = 0; i < 4; i++){
			info(END, 2, ids[i]);
		}
		once = true;
	}
    pthread_cond_broadcast(s->cond);
	pthread_mutex_unlock(s->lock);

	sem_wait(s->logSem);
	if(end11 == true){
		info(BEGIN, 2, s->value);
		nrThreads++;
   		info(END, 2, s->value);
		nrThreads --;
		pthread_cond_signal(s->cond);
		pthread_cond_signal(s->cond);
    }
    sem_post(s->logSem);
	return NULL;
}

void *func(void *arg){
	THREAD_STRUCT *s = (THREAD_STRUCT*)arg;
	pthread_mutex_lock(s->lock);
	info(BEGIN, 2, 11);
	nrThreads ++;
	while(nrThreads < 5) 
		pthread_cond_wait(s->cond, s->lock);
	info(END, 2, 11);
	end11 = true;
	nrThreads --;
	pthread_mutex_unlock(s->lock);
	return NULL;
}


int main(){
    init();  
    sem_unlink("semaforDeni");
    sem_unlink("semaforDeni2");
    semafor1 = sem_open("semaforDeni", O_CREAT, 0644, 0);
    semafor2 = sem_open("semaforDeni2", O_CREAT, 0644, 0);
    semafor3 = sem_open("semaforDeni3", O_CREAT, 0644, 0);
   	pid_t pid1 = -1;
    info(BEGIN, 1, 0);
    if((pid1 = fork()) == 0){
    	info(BEGIN, 2, 0);

    	//aici e p2
    	sem_t logSem;
    	sem_init(&logSem, 0, 5);
    	pthread_t tids[45];
    	pthread_mutex_t locck = PTHREAD_MUTEX_INITIALIZER;
   				pthread_cond_t connd = PTHREAD_COND_INITIALIZER;
    	THREAD_STRUCT param[45];
    
    	for(int i = 1; i <= 44; i++){
    		param[i].value = i;
    		param[i].lock = &locck;
    		param[i].cond = &connd;
    		param[i].logSem = &logSem;
    		if( i == 11)
    			pthread_create(&tids[i], NULL, func, &param[i]);
    		else
    		pthread_create(&tids[i], NULL, function, &param[i]);
    		
    	}

    	for(int i = 1; i <= 44; i++){
    		pthread_join(tids[i], NULL);
    	}

    	sem_destroy(&logSem);

    	pid_t pid2 = -1;
    	if((pid2 = fork()) == 0){
    		info(BEGIN, 4, 0);
    		pid_t pid4 = -1;
    		if((pid4 = fork()) == 0){

    			info(BEGIN, 5, 0);
  				sem_t logSem1;
   				sem_t logSem2;
   				
   					sem_init(&logSem1, 0, 0);
   				sem_init(&logSem2, 0, 0);
    			pid_t pid5 = -1;
    			if((pid5 = fork()) == 0){	
	
    				info(BEGIN, 7, 0);
    				//aici e p7
    				pthread_t threads[4];
    				TH_STRUCT par[4];
    					TH_STRUCT params[6];
   				pthread_t tid[6];
   				
   				for(int i = 1; i <= 5; i++){
   					params[i].id = i;
   					params[i].id_proces = 5;
   					params[i].logSem2 = &logSem2;
   					params[i].logSem1 = &logSem1;
   					//if( i == 3){
   					//	pthread_create(&tid[i], NULL, th_func, &params[i]);
   					//}else{
   						pthread_create(&tid[i], NULL, thread_function, &params[i]);
   					//}
   				}
    				//sem_t logSem1;
   					//sem_t logSem2;

   					//sem_init(&logSem1, 0, 0);
   					//sem_init(&logSem2, 0, 0);

    				for(int i = 1; i <= 4; i++){
    					par[i].id = i;
    					par[i].id_proces = 7;
    					par[i].logSem1 = &logSem1;
    					par[i].logSem2 = &logSem2;
    					pthread_create(&threads[i], NULL, thread_function, &par[i]);

    				}
					sem_destroy(&logSem1);
   					sem_destroy(&logSem2);

   				//	for(int i = 1; i <= 4; i++){
    			//		pthread_join(threads[i], NULL);}

    				pid_t pid7 = -1;
    				if((pid7 = fork()) == 0){
    					info(BEGIN, 8, 0);
    				
   						info(END, 8, 0);
    				}
    				else{
    					waitpid(pid7, NULL, 0);
    					//wait(NULL);
    					info(END, 7, 0);
    				}
    				
    				for(int i = 1; i <= 5; i++){
    					if(i<=4)
    					pthread_join(threads[i], NULL);
   					pthread_join(tid[i], NULL);
   					}
    				
    				
   					
    			}
    			else{
	   				waitpid(pid5, NULL, 0);
	//				wait(NULL);
    				
    				info(END, 5, 0);
    			}
    		}else{
    			waitpid(pid4, NULL, 0);
    			info(END, 4, 0);
    		}
    	}
    	else{
    	waitpid(pid2, NULL, 0);
    	info(END, 2, 0);
    	}
    }	
    else{
    	pid_t pid3 = -1;
    	if((pid3 = fork()) == 0){
    		info(BEGIN, 3, 0);
    		pid_t pid6 = -1;
    		if((pid6 = fork()) == 0){
    			info(BEGIN, 6, 0);
    			info(END, 6, 0);
    		}else{
    			waitpid(pid3, NULL, 0);
    			info(END, 3, 0);
    		}	
    	}else{
    		waitpid(pid1, NULL, 0);
  			info(END, 1, 0);
    	}
    	
 	}
    return 0;
}
