#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include "server.h"

int main(int argc, char **argv){
    pthread_t t1;
    serverParam servidor, *servidor_1;
    int t,i;
    sigset_t alarm_sig;
    
	sigemptyset (&alarm_sig);
	for (i = SIGRTMIN; i <= SIGRTMAX; i++)
	    sigaddset (&alarm_sig, i);
	sigprocmask (SIG_BLOCK, &alarm_sig, NULL);
	
    servidor.argc = argc;
    servidor.argv = argv;

    servidor_1 = &servidor;

    
    //Criação da thread do Servidor
    t = pthread_create(&t1, NULL, server, (void*)servidor_1);
    if(t){
        printf("Erro criando a thread servidor!\n");
        return 0;
    }else{
        printf("Thread servidor criada!\n");
    }
  

    pthread_join(t1, NULL);
   
    return 0;
}
