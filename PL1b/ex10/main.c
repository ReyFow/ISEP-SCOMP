#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

volatile sig_atomic_t USR1_counter = 0;

void handle_USR1(int signo, siginfo_t *sinfo, void *context){
  USR1_counter++;
  char msg [80];
  sprintf(msg, "SIGUSR1 signal captured: USR1 counter = %d\n", USR1_counter);
  write(STDOUT_FILENO, msg, strlen(msg));
  sleep(3);
}

int main(void){
  int i;
  pid_t childPid, parentPid;
  struct sigaction act;

  memset(&act, 0, sizeof(struct sigaction));
  sigfillset(&act.sa_mask); 
  act.sa_sigaction = handle_USR1;
  act.sa_flags = SA_SIGINFO;
  sigaction(SIGUSR1, &act, NULL);

  childPid = fork();
  if(childPid < 0){
    perror("Erro ao criar o processo");
    exit(-1);
  }else if(childPid == 0){
    parentPid = getppid();
    struct timespec sleep_time = {0, 10000000};
    for (i = 0; i < 12; i++) {
      kill(parentPid, SIGUSR1);
      nanosleep(&sleep_time, NULL);
    }
    kill(parentPid, SIGINT);
    exit(0);
  }
  for( ; ; ){
    printf("Im working!\n");
    sleep(1);
  }
  return 0;
}

/*
  a)✅

  b)
    input:
        kill -SIGUSR1 parentPid

    output:
        Im working!
        Im working!
        Im working!
        SIGUSR1 signal captured: USR1 counter = 1
        Im working!
        Im working!
  c)
    output:
        ./main
        SIGUSR1 signal captured: USR1 counter = 1
        Im working!
        SIGUSR1 signal captured: USR1 counter = 2
        Im working!
        SIGUSR1 signal captured: USR1 counter = 3
        Im working!
        SIGUSR1 signal captured: USR1 counter = 4
        Im working!
        SIGUSR1 signal captured: USR1 counter = 5
        Im working!
        SIGUSR1 signal captured: USR1 counter = 6
        Im working!
        SIGUSR1 signal captured: USR1 counter = 7
        Im working!
        SIGUSR1 signal captured: USR1 counter = 8
        Im working!
        SIGUSR1 signal captured: USR1 counter = 9
        Im working!
        SIGUSR1 signal captured: USR1 counter = 10
        Im working!
        SIGUSR1 signal captured: USR1 counter = 11
        Im working!
        SIGUSR1 signal captured: USR1 counter = 12
        Im working!
        make: *** [Makefile:11: run] Interrupt
    
  d)
    output:
        ./main
        SIGUSR1 signal captured: USR1 counter = 1
        make: *** [Makefile:11: run] Interrupt
    A:
      Uma vez que o handler leva mais de 1 segundo para ser executado e está a bloquear todos os sinais, incluindo o SIGUSR1, 
      qualquer sinal que seja enviado enquanto o handler está em execução será bloqueado o que pode resultar em perda de sinais, 
      ou seja, estes não serão entregues ao processo.
*/