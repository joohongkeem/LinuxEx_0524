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
	if(signo==SIGUSR2)
		puts("");
	else if(signo == SIGUSR1)
	{

		puts("----------------------------------------------------------");	
		puts("                    데이터 출력완료");
		puts("----------------------------------------------------------");
	}
}



int main(void)
{	
	int msgid;
	key_t keyValue;
	char buffer[BUFFSIZE];
	pid_t W_pid;
	pid_t R_pid;
	int sig;
	void *shared_Mem = (void*)0;
	int shmid;
	char *shmaddr;
	int i,j=0;

	// ftok함수를 사용하여 메시지 큐를 생성
	keyValue = ftok(PATH,PROJ_ID);
	msgid = msgget(keyValue, 0666 | IPC_CREAT);
	

	// 메시지 큐가 정상적으로 생성되지 않았을 경우 에러메시지 출력
	if(msgid == -1)
	{
		fprintf(stderr, "msgget failed with ERROR(%d)\n",errno);
		exit(EXIT_FAILURE);
	}
	

	// 시그널 핸들러 등록
	signal(SIGUSR1, sig_handler);
	signal(SIGUSR2, sig_handler);


	// 자신의 pid를 상대에게 전송 
	W_pid = getpid();
	if( msgsnd(msgid, (void *)&W_pid, sizeof(pid_t),0) == -1)
	{
		exit(EXIT_FAILURE);
	}
	

	// 상대가 받았다면, 시그널을 받을 때까지 대기 
	pause();
	puts("자신 pid 전송완료");

	
	// 상대의 pid를 받음
	if( msgrcv(msgid, (void *)&R_pid, sizeof(pid_t), 0, 0) == -1)
	{
		fprintf(stderr, "msgrcv failed with ERROR(%d)\n", errno);
		exit(EXIT_FAILURE);
	}
	puts("상대 pid 수신완료");

	// pid가 정상적으로 주고받아졌는지 확인
	printf("나의 pid : %d\n", getpid());
	printf("상대 pid : %d\n", R_pid);

	
	// 공유메모리 생성
	shmid = shmget((key_t)2004, sizeof(buffer) * SHMSIZE, 0666 | IPC_CREAT);	
	if(shmid == -1)
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
	
	
	// 공유메모리에 데이터 입
	puts("----------------------------------------------------------");
	puts("공유메모리에 데이터 입력하기(최대 20개, 마치려면 EXIT입력)");
	puts("----------------------------------------------------------");

	for(i=0; j<(SHMSIZE); i=i+(BUFFSIZE))
	{
		j++;
		memset(buffer, '\0', sizeof(buffer));
		printf("%2d번째 메시지 : ",j);
		scanf("%[^\n]",buffer);
		getchar();
		
		// EXIT를 입력하면 갯수를 특정 배열에 저장하고, break
		if(strncmp(buffer,"EXIT",4)==0) {*(shmaddr + SHMSIZE*BUFFSIZE) = j-1; break;}
	
		// 버퍼의 내용을 공유메모리에 저장
		strcpy(shmaddr+i,buffer);
		

		// 20개가 꽉찬다면 마찬가지로 갯수를 특정 배열에 저장
		if(j==SHMSIZE){*(shmaddr + SHMSIZE*BUFFSIZE) = j;}
		//printf("주소 : %d 값 : ",shmaddr+i);
		//puts(shmaddr+i);
		
	}

	kill(R_pid,SIGUSR1);
	
	// 다읽을때까지 대
	pause();
	//printf("\n 갯수 : %d\n", *(shmaddr + SHMSIZE));
	
	// 모두 정상적으로 됐으니 Detach
	if(shmdt(shared_Mem) == -1)
	{
		fprintf(stderr, "shmdt failed\n");
		exit(EXIT_FAILURE);
	}


	// 모두 Detatch됐으니 shared memory 삭제
	if(shmctl(shmid, IPC_RMID,0) == -1)
	{
		fprintf(stderr, "shmctl(IPC_RMID) failed\n");
		exit(EXIT_FAILURE);
	}
	

	exit(EXIT_SUCCESS);
}
