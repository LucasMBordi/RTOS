#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>


//Inclusão do embarcado
#include "lampada.h"

#define TOTAL 5

//struct utilizada para passar os parâmetros de inicialização do servidor pela  pthread_create: host e porta.
typedef struct serverParam {
    int argc;
    char **argv;
} serverParam;

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

struct nodo {
    int newsockfd;
};

struct nodo nodo[5];
long g_cid;

//thread responsável por enviar as respostas ao cliente
void *transmitir(void *arg){
    // estrutura para fazer a thread ser periódica
    char info[256];
    int rc, cont = g_ret;

    bzero(ret, sizeof(ret));
    bzero(info, sizeof(info));

    while(1){
         //envia a string ao cliente sempre que houver alguma alteração em ret
         pthread_mutex_lock(&r_mutex);
         while(w_ret == 0){
             pthread_cond_wait(&r_cond, &r_mutex);
         }
         if((g_ret - cont) == 1){
             rc = write(nodo[g_cid].newsockfd, ret, 256);
             bzero(ret, sizeof(ret));
             if(rc <= 0){
                 printf("Thread Comunicação - Erro escrevendo no socket!\n");
             }
         } else if((g_ret - cont) > 1){
             printf("Strings de retorno foram perdidas!\n"); 
         }
         w_ret = 0;
         cont = g_ret;
         
         pthread_mutex_unlock(&r_mutex);
        
    }
}

void *cliente(void *arg) {
    long cid = (long)arg;
    int i, n,aux;
    char buffer[256], t[256];
	char *token;
	int msg = 0;
	int h,min,seg,resto,segundos;
    while (1) {
        bzero(buffer,sizeof(buffer));
        n = read(nodo[cid].newsockfd,buffer,70);
        printf("Recebeu: %s - %lu\n", buffer,strlen(buffer));
        if (n <= 0) {
            printf("Erro lendo do socket id %lu!\n", cid);
            close(nodo[cid].newsockfd);
            nodo[cid].newsockfd = -1;
            pthread_exit(NULL);
        }
	    pthread_mutex_lock(&m);
        g_cid = cid;
	    strcpy(t,buffer);
	    token = strtok(t, " ");
        if(strcmp(buffer,"Verifica Estado\n") == 0){
		    aux = VerificaEstado();
                if(aux == 1){
		                pthread_mutex_lock(&r_mutex);
		                snprintf(ret, sizeof(ret) - 1, "A lampada esta ligada ! \n");
		                g_ret++;
                        w_ret = 1;
                        pthread_cond_signal(&r_cond);
                        pthread_mutex_unlock(&r_mutex);
            
                   }else{
			           pthread_mutex_lock(&r_mutex);
		              snprintf(ret, sizeof(ret) - 1, "A lampada esta desligada ! \n");
		              g_ret++;
                      w_ret = 1;
                      pthread_cond_signal(&r_cond);
                      pthread_mutex_unlock(&r_mutex);
                //printf("A lampada esta desligada \n");
                 }
            
	  }  
	  else if(strcmp(token,"Ligar") == 0){
               token = strtok(NULL, " ");
               if(strcmp(token,"Lampada") == 0){
                    token = strtok(NULL, " ");
                    msg = atoi(token);
                    LigarLampada(msg);
               }else{
		            pthread_mutex_lock(&r_mutex);
		            snprintf(ret, sizeof(ret) - 1, "Comando inexistente, tente novamente! \n");
		            g_ret++;
                     w_ret = 1;
                     pthread_cond_signal(&r_cond);
                     pthread_mutex_unlock(&r_mutex);
                //printf("Comando inexistente ! \n");
            }
        }
       else if(strcmp(token,"Desligar") == 0){
               token = strtok(NULL, " ");
               if(strcmp(token,"Lampada") == 0){
                    token = strtok(NULL, " ");
                    msg = atoi(token);
                    DesligarLampada(msg);
               }else{
		            pthread_mutex_lock(&r_mutex);
		            snprintf(ret, sizeof(ret) - 1, "Comando inexistente, tente novamente! \n");
		            g_ret++;
                    w_ret = 1;
                    pthread_cond_signal(&r_cond);
                    pthread_mutex_unlock(&r_mutex);
                //printf("Comando inexistente ! \n");
               }
        }
       else if(strcmp(token,"Seta") == 0){
              token = strtok(NULL, " ");
              if(strcmp(token,"Intervalo") == 0){
                   token = strtok(NULL, " ");
                   msg = atoi(token);
                   SetaIntervalo(msg);
              }else{
	 	           pthread_mutex_lock(&r_mutex);
		           snprintf(ret, sizeof(ret) - 1, "Comando inexistente, tente novamente! \n");
		           g_ret++;
                   w_ret = 1;
                   pthread_cond_signal(&r_cond);
                   pthread_mutex_unlock(&r_mutex);
                
              }
        }
       else if(strcmp(buffer,"Verifica Tempo\n") == 0){
			      segundos = VerificaTempo();
			      h = segundos / 3600;
	   		     resto = segundos % 3600;
			     min = resto/ 60;
	             seg = resto % 60;
		         pthread_mutex_lock(&r_mutex);
		         snprintf(ret, sizeof(ret) - 1, "A lampada está ligada a %i hora(s) %i minuto(s) e %i segundo(s) \n",h,min,seg);
		         g_ret++;
                 w_ret = 1;
                 pthread_cond_signal(&r_cond);
                 pthread_mutex_unlock(&r_mutex);
               // printf("A lampada está ligada a %i hora(s) %i minuto(s) e %i segundo(s) \n",h,min,seg);
            
	}else{
            pthread_mutex_lock(&r_mutex);
	        snprintf(ret,sizeof(ret) - 1,"Comando inexistente ! \n");
			g_ret++;
			w_ret = 1;
			pthread_cond_signal(&r_cond);
            pthread_mutex_unlock(&r_mutex);
			for (i = 0;i < TOTAL; i++) {
                if ((i != cid) && (nodo[i].newsockfd != -1)) {
                    n = write(nodo[i].newsockfd,buffer,70);
                    if (n <= 0) {
                    printf("Erro escrevendo no socket id %i!\n", i);
                  
                    }
                }
            }  
	}
     pthread_mutex_unlock(&m);
		   
           		   
   }
}
       

void* server(void* arg) {
    
    struct serverParam *servidor1 = (serverParam*)arg;
    int argc = servidor1->argc;
    char **argv = servidor1->argv;
   

    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clilen;
    int sockfd, portno;
    pthread_t t,t1;
    long i;

    if (argc < 2) {
        printf("Erro, porta nao definida!\n");
        printf("./SOFTWARE PORTA");
        exit(1);
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        printf("Erro abrindo o socket!\n");
        exit(1);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
        printf("Erro fazendo bind!\n");
        exit(1);
    }
    listen(sockfd,5);

    for (i = 0; i < TOTAL; i++) {
      nodo[i].newsockfd = -1;
    }
    while (1) {
        for (i = 0; i < TOTAL; i++) {
          if (nodo[i].newsockfd == -1) break;
        }
        nodo[i].newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr,&clilen);
        // MUTEX LOCK - GERAL
        pthread_mutex_lock(&m);
        if (nodo[i].newsockfd < 0) {
            printf("Erro no accept!\n");
            exit(1);
        }
        pthread_create(&t, NULL, cliente, (void *)i);
		pthread_create(&t1, NULL, transmitir, NULL);
        pthread_mutex_unlock(&m);
        // MUTEX UNLOCK - GERAL
    }
    //    close(sockfd);
    return 0; 
}
