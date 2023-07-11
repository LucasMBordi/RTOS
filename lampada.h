#include <stdio.h>
#include <stdlib.h>
#include <pthread.h> 
#include "gps.h"

// Variáveis globais
int tempo_off = 0;     // quanto tempo a lampada estara off
int tempo_on = 0;   // Parametro passado por LigarLampada, que é o tempo que  a lampada estara ligada sem relação ao GPS
int timer_on = 0;  // tempo que a lampada está acesa
int timerL = 0;    // variavel que recebe a(s) hora do gps no momento que ele foi acionado
int timerL_M = 0;  // variavel que recebe o(s) minutos do gps no momento que ele foi acionado
int Desliga = 0;   // variavel que indica se  a função DesligarLampada está em uso
int sig = 0;       // variavel que indica se a função LigarLampada está em  uso
int off = 0;       // Tempo que a lampada ficara desligada 
int intervalo = 0; // Variavel que indica o intervalo de horas que ficara ligado

pthread_mutex_t r_mutex = PTHREAD_MUTEX_INITIALIZER;   // mutex usado para escrita no buffer de retorno
pthread_cond_t r_cond = PTHREAD_COND_INITIALIZER;
int g_ret = 0;
char ret[256];
int w_ret = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void* light_off(void *arg){
    struct periodic_info info;
	make_periodic(1000000, &info);
	while(1){
		pthread_mutex_lock(&mutex);
		while((sig == 1) || (state_lon == 1)){
			
            pthread_cond_wait(&cond,&mutex);
        }
		while((off > 0) && (timer_on > 0)){
			timer_on--;
			off--;
			wait_period(&info);	
		}
		if((off == 0) || (timer_on == 0)){
		Desliga = 0;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
		pthread_exit(NULL);
		}
	}
	
}
void* lampada_on_noGPS(void *arg){
    struct periodic_info info;
	make_periodic(1000000, &info);
	pthread_mutex_lock(&mutex);
	while(1){
		while((Desliga == 1) ||	(state_lon == 1)){
            pthread_cond_wait(&cond,&mutex);
		}
		while(tempo_on > 0){
			timer_on++;
			tempo_on --;
            wait_period(&info);	
		}
		if(tempo_on == 0){
		sig = 0;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
		pthread_exit(NULL);
		}
	}
}
void* lampada_on_withGPS(void *arg){
	 struct periodic_info info;
	 make_periodic(1000000, &info); // período - 1 segundos
	 while(1){
		 pthread_mutex_lock(&mutex);
		 while((Desliga == 1) || (sig == 1)){
			 pthread_cond_wait(&cond, &mutex); 
		 }
		 while((g_time - timerL) != intervalo){
			   timer_on++;
			   wait_period(&info);
		 }
		if((g_time - timerL) == intervalo){
		    while((g_time_m - timerL_M) != 0){
		           timer_on++;
			       wait_period(&info);
		    }
       }
	
	    state_lon = 0;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
		pthread_exit(NULL);
		
			
	}
 }



int LigarLampada(int tempo){
	pthread_t t4;
	int rc;
	rc = pthread_create(&t4, NULL, lampada_on_noGPS, NULL); // Criação da thread que indica que a lampada foi acesa pela função LigarLampada
    if(rc){
          printf("Erro ao criar a thread lampada_on_noGPS!\n");
	}else{
          printf("Thread lampada_on_noGPS criada!\n");
	      tempo_on = tempo;
	      sig = 1;
          
        }

}
void DesligarLampada(int tempo){
	pthread_t t3;
	int rc;
	if(timer_on <= 0){
		 pthread_mutex_lock(&r_mutex);
		 snprintf(ret, sizeof(ret) - 1, "Erro, tempo menor ou igual a 0\n");
		 //printf("Erro, tempo menor ou igual a 0\n"); 
		  g_ret++;
                 w_ret = 1;
                 pthread_cond_signal(&r_cond);
                pthread_mutex_unlock(&r_mutex);
	}else{
	    rc = pthread_create(&t3, NULL, light_off, NULL); // Criação da thread que indica que a lampada foi desligada pela função DesligarLampada
	    if(rc){
            printf("Erro criando a thread light_off!\n");
            }else{
                      printf("Thread light_off criada!\n");
			off = tempo;   // Tempo em segundos que a lampada estará desligada
			Desliga = 1;   // Flag que indica que o comando DesligarLampada está rodando
          
                 }
	}
}
int VerificaEstado(){
	if((sig == 1)||(state_lon == 1)){
		return 1;
    }else{
        //printf("A lampada esta desligada");
		return -1;
    }
	//return s;
}
int VerificaTempo(){
	
	return timer_on;
}

int SetaIntervalo(int num){
	pthread_t t2,t1;
	int rc;
	rc = pthread_create(&t1, NULL, lampada_on_withGPS, NULL); // Criação da thread que indica que a lampada foi acesa pela função seta intervalo
	if(rc){
            printf("Erro criando a thread Lampada!\n");
        }else{
            printf("Thread Lampada criada!\n");
			timerL = g_time; // Pega o hora inicial do gps
			timerL_M = g_time_m; // Pega os minutos iniciais do gps
			state_lon = 1;    // Indica que a função SetaIntervalo que está rodando
			intervalo = num; // Numero de hora(s) que a lampada ficara ligada
          
        }
	rc = pthread_create(&t2, NULL, gps, NULL); // Criação da thread gps
	if(rc){
            printf("Erro criando a thread GPS!\n");
        }else{
            printf("Thread GPS criada!\n");
			
        }
}
