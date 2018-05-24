#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#include "shm.h"

void sig_handler(int signo)
{
	if(signo == SIGUSR1)
		puts("         [..........입력완료!..........]");	
}

int main(void)
{	
	int msgid;
	key_t keyValue;
	char buffer[BUFFSIZE];
	pid_t W_pid;
	pid_t R_pid;
	int sig;
	void *shared_Mem = (void*) 0;
	int shmid;
	char *shmaddr;
	int i;
	int j=1;
	FILE *fwp;

	fwp = fopen("Backup.txt", "w+");
	if(fwp == NULL)
	{
		puts("File open ERROR");
		exit(EXIT_FAILURE);
	}

	// ftok함수를 사용하여 메시지 큐를 생성
	keyValue = ftok(PATH,PROJ_ID);
	msgid = msgget(keyValue, 0666 | IPC_CREAT);
	

	// 메시지 큐가 정상적으로 만들어지지 않았으면 에러메시지 출력
	if(msgid == -1)
	{
		fprintf(stderr, "msgget failed with ERROR(%d)\n",errno);
		exit(EXIT_FAILURE);
	}


	// 시그널 핸들러 등록.
	signal(SIGUSR1, sig_handler);


	// 상대의 pid를 받음
	if( msgrcv(msgid, (void *)&W_pid, sizeof(pid_t), 0, 0) == -1)
	{
		fprintf(stderr, "msgrcv failed with ERROR(%d)\n", errno);
		exit(EXIT_FAILURE);
	}
	puts("상대 pid 수신완료");

	
	// 잘받았으니 signal 전송
	kill(W_pid,SIGUSR2);


	// 자신의 pid를 상대에게 전송
	R_pid = getpid();
	if( msgsnd(msgid, (void *)&R_pid, sizeof(pid_t),0) == -1)
	{
		fprintf(stderr, "msgsnd failed\n");
		exit(EXIT_FAILURE);
	}
	puts("자신 pid 전송완료");

	// pid가 정상적으로 주고받아졌는지 확인
	printf("나의 pid : %d\n", getpid());
	printf("상대 pid : %d\n", W_pid);
	
	
	// 공유메모리 생성
	shmid = shmget((key_t)2004, sizeof(buffer) * SHMSIZE, 0666 | IPC_CREAT);
	if( shmid == -1)
	{
		fprintf(stderr, "shmget failed\n");
		exit(EXIT_FAILURE);
	}


	// 공유메모리 attach
	shared_Mem = shmat(shmid, (void *)0, 0);
	if(shared_Mem == (void *)-1)
	{
		fprintf(stderr, "shmat failed\n");
		exit(EXIT_FAILURE);
	}
	shmaddr = (char *)shared_Mem;
	
	// 상대의 입력이 끝날때까지 대기
	puts("---------------------------------------------------");
	puts("          공유메모리의 데이터 출력하기");
	puts("---------------------------------------------------");
	puts("         [..........입력대기중.........]");
	pause();
	puts("---------------------------------------------------");


	// 공유메모리의 데이터 출력
	
	for(j=1; j<=*(shmaddr + SHMSIZE*BUFFSIZE) ; j++)
	{
		printf("[%2d번째 데이터] ",j);
		puts(shmaddr+BUFFSIZE*(j-1));
		fprintf(fwp,"%s\n",shmaddr+BUFFSIZE*(j-1));
	}

	kill(W_pid,SIGUSR1);
	

	// 공유메모리 Detach
	if(shmdt(shared_Mem)== -1)
	{
		fprintf(stderr, "shmdt failed\n");
		exit(EXIT_FAILURE);
	}

	puts("---------------------------------------------------");

	fclose(fwp);
	exit(EXIT_SUCCESS);
}
