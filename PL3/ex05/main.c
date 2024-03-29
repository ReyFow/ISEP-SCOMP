#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>

typedef struct {
    int value;
} sharedValues;

#define DATA_SIZE sizeof(sharedValues)
#define FILE_NAME "/shmEx05"
#define NUMBER_OF_OPERATIONS 1000000

void handler(int signo, siginfo_t *sinfo, void *context){
    write(STDOUT_FILENO, "Catch USR1!\n", 13);
}

int main(void){

    int fd, status, i;
    pid_t childPid, parentPid = getpid();

    struct sigaction act;
    memset(&act, 0, sizeof(struct sigaction));
    sigemptyset(&act.sa_mask);
    act.sa_sigaction = handler;
    sigaction(SIGUSR1, &act, NULL);

    fd = shm_open(FILE_NAME, O_CREAT|O_EXCL|O_RDWR, S_IRUSR|S_IWUSR);
    if(fd < 0) {
		perror("Erro ao criar memoria partilhada");
        exit(-1);
	}

    if (ftruncate (fd, DATA_SIZE) < 0) {
        perror("Erro ao alocar espaço na memória");
        exit(-1);
    }
    
    sharedValues *shared_data = (sharedValues*) mmap(NULL, DATA_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

    shared_data -> value = 100;

    childPid = fork();
    if(childPid < 0){
        perror("Erro ao criar o processo");
        exit(-1);
    }else if(childPid == 0){
        childPid = getpid();
        for(i = 0; i < NUMBER_OF_OPERATIONS; i++){
            pause();
            printf("Child executing (pid: %d)\n", childPid);
            shared_data -> value++;
            shared_data -> value--;
            kill(parentPid, SIGUSR1);
        }

        if(munmap(shared_data, DATA_SIZE) < 0){
            perror("Erro ao remover mapping");
            exit(-1);
        }

        if(close(fd) < 0){
            perror("Erro ao fechar file descriptor");
            exit(-1);
        }

        exit(0);
    }
    sleep(1);
    for(i = 0; i < NUMBER_OF_OPERATIONS; i++){
        shared_data -> value++;
        shared_data -> value--;
        printf("Parent executing (pid: %d)\n", parentPid);
        kill(childPid, SIGUSR1);
        pause();
    }
    waitpid(childPid, &status, 0);
    printf("Valor final: %d\n", shared_data -> value);

    if(munmap(shared_data, DATA_SIZE) < 0){
        perror("Erro ao remover mapping");
        exit(-1);
    }

    if(close(fd) < 0){
        perror("Erro ao fechar file descriptor");
        exit(-1);
    }

    if (shm_unlink(FILE_NAME) < 0) {
        perror("Erro ao remover o Ficheiro!");
        exit(-1);
    }

    return 0;
}