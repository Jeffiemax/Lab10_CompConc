#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

/*Implementar uma variação na implementação do problema 
produtor/consumidor,usando semáforos.*/

//Variável global
#define N 5 //tamanho do buffer
int TC, TP; //valores das Threads consumidoras e produtoras
pthread_t condt; 

// Variaveis globais
sem_t slotCheio, slotVazio;//condicao
sem_t mutexProd, mutexCons;//exclusao mutua
int Buffer[N]; //espaco de dados compartilhados
int in=0, out=0; //variaveis de estado

//inicializa o buffer
void IniciaBuffer(int n) {
  int i;
  for(i=0; i<n; i++)
    Buffer[i] = 0; //zerando o vetor
}

//imprime o buffer
void ImprimeBuffer(int n) {
  int i;
  for(i=0; i<n; i++)
    printf("%d ", Buffer[i]);
  printf("\n");
}

void Preenche(int item, int id) {

    printf("P[%d] quer preencher\n", id);
    //aguarda slot vazio
    sem_wait(&slotVazio);
    if(in == 1){
        printf("P[%d] bloqueado\n", id);
        sem_wait(&mutexProd); //bloqueia produtor quando um já está preenchendo
    }
    in++;

    //exclusao mutua entre produtores
    printf("P[%d] está preenchendo\n", id);
    for(int i=0; i<N; i++){
        Buffer[i] = id; //preenche o vetor com o valor do id da thread
    }
   
    ImprimeBuffer(N); 
    //sinaliza que o vetor está cheio
    sem_post(&slotCheio); //sinaliza que está vazio e incrementa o semaforo
    in = 0;
    sem_post(&mutexProd);
    printf("P[%d] saindo\n", id);
}

int Esvaziar (int id ) {
    
    //aguarda slot cheio
    printf("C[%d] quer consumir\n", id);
     //aguarda slot cheio
    sem_wait(&slotCheio);
    if(out == 1){
        printf("C[%d] bloqueado\n", id); //bloqueia consumidor quando um já está esvaziando
        sem_wait(&mutexCons);
    }
    out++;
    //exclusao mutua entre consumidores
    
    printf("C[%d] está esvaziando\n", id);
    IniciaBuffer(N); // esvazia o vetor (preenche ele com zeros)
    ImprimeBuffer(N);
    //sinaliza que o vetor está vazio
    sem_post(&slotVazio); //sinaliza que está vazio e incrementa o semaforo
    out = 0;
    sem_post(&mutexCons); 
    printf("C[%d] saindo\n", id);
}

//thread produtora
void *produtor(void * arg) {
    int *id = (int *) arg;
    //printf("Sou a thread produtora %d\n", *id);
    while(1) {
        //produz um elemento....
        Preenche(*id, *id);
        sleep(1);
    }
    free(arg);
    pthread_exit(NULL);
    }

//thread consumidora
void *consumidor(void * arg) {
    int elemento ;
    int *id = (int *) arg;
    //printf("Sou a thread consumidora %d\n", *id);
    while(1) {
        elemento = Esvaziar(*id);
        sleep(1); //faz o processamento do item 
    }
    free(arg);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]){

    if(argc < 3){
        fprintf(stderr,"Digite %s < num de Threads consumidora > < num de Threads produtora >\n", argv[0]);
    }
    TC = atoi(argv[1]);
    TP = atoi(argv[1]);

    int i;
    pthread_t tid[TC + TP];
    int *id[TC + TP];

      //aloca espaco para os IDs das threads
    for(i=0; i<TC+TP;i++) {
        id[i] = malloc(sizeof(int));
        if(id[i] == NULL) exit(-1);
        *id[i] = i+1;
    }

    //inicializa o Buffer
    IniciaBuffer(N); 

    sem_init(&mutexCons, 0, 1);
    sem_init(&mutexProd, 0, 1);
    sem_init(&slotCheio, 0, 0); 
    sem_init(&slotVazio, 0, 1);
 
    //cria as threads consumidoras
    for(i=0; i<TC; i++) {    
        in = (in + 1) % N;
        if(pthread_create(&tid[i], NULL, consumidor, (void *) id[i])) exit(-1);
    }
     //cria as threads produtoras
    for(i=0; i<TP; i++) {
        if(pthread_create(&tid[i], NULL, produtor, (void *) id[i])) exit(-1);
    }

    pthread_exit(NULL);
    return 1;
}