 /*
 * [메시지큐]
 * - 하나의 프로세스로 부터 다른 프로세스로 '데이터의 블록'을 전달하는 IPC의 하나
 * - 주요 특징
 *   >> 각 데이터 블록은 type을 가짐
 *   >> 받아들이는 프로세스는 다른 형태 값을 가지는 데이터 블록을 독립적으로 받아들임.
 * - 장점
 *   >> 메시지를 전달
 *   >> 명명 파이프의 동기화와 방지 문제 예방
 * - 단점
 *   >> 각 데이터 블록에 부과되는 최대 크기에 제한
 *   >> 모든 큐에서 모든 블록의 최대 크기에 제한
 *
 * [mesget function] 
 * - int megget(key_t key, int msgflg);   - open과 비슷
 *   >> 주요기능 : 메시지 큐 생성, 액세스
 *   >> key : 메시지 큐 명명
 *      msgflg
 *         - IPC_PRIVATE : 프로세스 전용 큐
 *         - IPC_CREAT : 허용 플래그 포함. or 연산(0666|IPC_CREAT)
 *   >> return value
 *     - 성공 : 큐 식별자
 *     - 실패 : -1
 *
 * Ex)
 * - qid = msgget(IPC_PRIVATE, IPC_CREAT|0666);
 * - qid = msgget(1234,IPC_CREAT|0644);
 * - qid = msgget(1234,0644);
 *
 *
 * [msgsnd function]
 * - int msgsnd(int msqid, const void *msg_ptr, size_t msg_sz, int msgflg);  - write와 비슷
 *   >> 주요기능 : 메시지 추가
 *   >> msgid : 메시지 큐 식별자
 *      msg_ptr : 전달되는 메시지에 대한 포인터
 *      msg_sz : 메시지 크기
 *      msgflg : 큐가 가득 차거나 큐에 저장되는 메시지에 대한 시스템 범위의 제한에 도달 시
 *               발생 이벤트 제어
 *            - IPC_NOWAIT : 메시지 전달하지 않고 즉시 반환
 *            - null : 큐의 공간이 유효할때까지 대기
 *   >> return value
 *     - 성공 : 0
 *     - 실패 : 1
 *
 * [msgrcv function]
 * - int msgrcv(int msqid, void *msg_ptr, size_t msg_sz, long int msgtype, int msgflg); - read와 비슷
 *   >> 주요기능 : 메시지 큐로부터 메시지를 구함
 *   >> msqid : 메시지 큐 식별자
 *      msg_ptr : 전달된 메시지에 대한 포인터
 *      msg_sz : 메시지 크기
 *      msgtype : 간단한 형태의 우선권 지정
 *          - =0 : 큐에서 처음으로 유효한 메시지
 *          - >0 : 같은 메시지 형태를 가지는 첫 번째 메시지
 *          - <0 : 같은 형태의 메시지 or msgtype의 절대값보다 적은 첫 번째 메시지
 *      msg_sz : 메시지가 msg_sz보다 크면
 *          - msgflg에 MSG_NOERROR 지정시 truncate(길이를 줄이다) 하고 읽는다 (잔여 메시지는 제거)
 *	    - MSG_NOERROR가 없다면 E2BIG 에러. (메시지 큐에 그대로 있다)
 *      msgflg : 예외 처리
 *          - IPC_NOWAIT : -1 즉시 반환
 *          - null : 적절한 형태의 메시지가 전달될 때까지 대기
 *   >> return value
 *     - 성공 : 버퍼에 저장된 바이트 수 반환하고, msg_ptr에서 가리키는 사용자 할당의 버퍼로 복사되며
 *              데이터는 메시지 큐로부터 삭제
 *     - 에러 : -1 반환
 *
 *  [msgctl function]
 *  - int msgctl(int msqid, int command, struct msqid_ds *buf);	- fcntl과 비슷
 *    >> 주요기능 : 메시지 큐 제어함수
 *    >> msqid : 메시지 큐 식별자
 *       command : 수행할 동작
 *          - IPC_STAT : 메시지 큐와 관련된 값을 반영하기 위해 msqid_ds 구조체에서 데이터 설정
 *          - IPC_SET : 프로세스가 허용 권한을 가진다면 메시지 큐와 관련되는 값을
 *                      msqid_ds데이터 구조체에서 제공되는 것을 설정
 *          - IPC_RMID : 삭제
 *    >> return value
 *      - 성공 : 0
 *      - 실패 : -1
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>



struct PERSON
{
	long int msg_type;
	char name[10];
	unsigned char age;
	int id;
	char choice;
};


int main()
{
	int running = 1;
	int msgid;
	struct PERSON data;
	long int msg_to_receive = 0;
	int i=0;
	FILE *fwp;
	

	fwp = fopen("mymsg.txt","w+");
	if(fwp==NULL){puts("File open ERROR"); exit(EXIT_FAILURE);}
	msgid = msgget((key_t)65536,0666 | IPC_CREAT); 
	// 강사님 git참고하면, ftok를 사용해서 msgget하는 방식 나와있다.

	if(msgid == -1)
	{
		fprintf(stderr, "msgget failed with error : %d\n",errno);
		exit(EXIT_FAILURE);
	}

	
	//fwp = fopen("mymsg.txt","w+");
	//if(fwp==NULL){puts("File open ERROR"); exit(EXIT_FAILURE);}

	while(data.choice != 'N')
	{
		memset(data.name,'\0',sizeof(data.name));
		if( msgrcv(msgid, (void *)&data,sizeof(data),msg_to_receive,0) == -1 ) // receive
		{
			fprintf(stderr, "msgrcv failed with error : %d\n", errno);
			exit(EXIT_FAILURE);
		}

		printf("[%d번째]\n",++i);
		printf("  ID : %06d\n",data.id); 
		printf("NAME : %s\n", data.name);
		printf(" AGE : %d\n", data.age);
		//printf(" CHO : %c\n", data.choice);

		fprintf(fwp, "[ID]%08d  [이름]%-11s[나이]%d\n",data.id,data.name,data.age);
	}
	
	
	fclose(fwp);
	if(msgctl(msgid, IPC_RMID,0) == -1) 
	{
		fprintf(stderr, "msgctl(IPC_RMID) failed\n");
		exit(EXIT_FAILURE);
	}

	
	exit(EXIT_SUCCESS);
}

