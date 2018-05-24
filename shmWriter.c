/* Shared Memory 개념
 * - 두개의 무관한 프로세스간에 서로 같은 logical memory에 접근할수 있게 해준다.
 * - 여러 프로세스 사이에서 데이터를 보내고 공유할 수 있는 효율적인 방법!
 * - Shared Memory는 아무런 동기화 기능을 제공하지 않는다.
 *   >> 동기화문제는 signal을 이용해서 해결할 수 있다!!
 *
 *
 */


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SHMSIZE 100	// shared memory의 size는 100byte

int main(void)
{
	void *shared_Mem = (void*) 0;
	int shmid;
	int *shmaddr;
	int i;

	
	// Step1. shmget
	// int shmget(key_t key, size_t size, int shmflg);
	// - 물리적인 shared memory를 생성한다.
	// - key : 공유 메모리를 구별하는 식별 번호
	//   size : 공유 메모리 크기 (stack과 heap사이의 unallocated memory 부분에 만든다)
	//   shmflg : 동작 옵션
	//      >> IPC_CREATE : key에 해당하는 공유메모리가 없다면 새로 생성
	//                      만약 있다면 무시하며 생성을 위해 접근 권한을 지정해주어야 한다.
	//         IPC_EXCL : 공유메모리가 이미 있다면, 실패로 반환하며 공유 메모리에 접근하지 못한다.
	//                    이 옵션이 없어야 기존 공유메모리에 접근할 수 있다.
	// - 리턴값
	//    >>  -1 : 실패자
	//       성공: 공유 메모리 식별자
	shmid = shmget((key_t)1234, sizeof(int)*SHMSIZE, 0666 | IPC_CREAT);
				      // 4 * 100 = 400
	if(shmid == -1)
	{
		fprintf(stderr, "shmget failed\n");
		exit(EXIT_FAILURE);
	}


	// Step2. shmat
	// void *shmat(int shmid, const void *shmaddr, int shmflg);
	// - 공유메모리를 마치 프로세스의 몸 안으로 첨부한다.
	// - shmid : 공유 메모리를 구별하는 식별 번호
	//   *shmaddr : 첨부되는 어드레스 주소, 일반적으로 NULL을 지정
	//   shmflg : 동작 옵션
	//     >> SHM_RDONLY : 공유메모리를 읽기 전용으로
	//        SHM_RND : shmaddr이 NULL이 아닌 경우일 때만 사용되며,
	//                  shmaddr을 반올림하여 메모리 페이지 경계에 맞춘다.
	// - 리턴값
	//    >>  (void *)-1 : 실패
	//              이외 : 프로세스에 첨부된 프로세스에서의 공유 메모리 주소
	shared_Mem = shmat(shmid, (void *)0, 0);		// 처음에 void로 선언했다.

	if(shared_Mem == (void *)-1)
	{
		fprintf(stderr, "shmat failed\n");
		exit(EXIT_FAILURE);
	}
	
	printf("Memory attached at 0x%p\n", shared_Mem); 	// 주소값을 잘 전달 받았는지 출력해보자
	shmaddr = (int *)shared_Mem;				// 보이드 포인터므로 메모리에
								//   접근할 수 없으므로 강제 형변환 

	// Step3. memory access
	// 데이터를 채워보자
	for(i=0; i<SHMSIZE;i++)
	{
		*(shmaddr+i) = i+1;
		printf("shmaddr:%p, data:%d\n", shmaddr+i, *(shmaddr+i));	// 데이터가 잘 써졌나 확인
	}

	sleep(4);	// 다른 프로세스에서 접근할 시간을 준다.


	// Step4. shmdt
	// int shmdt(const void *shmaddr);
	// - 프로세스에 첨부된 공유 메모리를 프로세스에서 분리합니다.
	// - *shmaddr : 분리할 공유 메모리 주소
	// - 리턴값
	//     >> 0 : 성공
	//       -1 : 실패
	if(shmdt(shared_Mem)==-1)
	{
		fprintf(stderr, "shmdt failed\n");
		exit(EXIT_FAILURE);
	}

	// Step5. shmctl : IPC_RMID
	// int shmctl(int shmid, int cmd, struct shmid_ds *buf);
	// - 공유 메모리에 대한 정보를 구하거나 변경 또는 제거한다.
	// - shmid : 공유 메모리 식별 번호
	//   cmd : 제어 명령
	//   shmid_ds *buf : 공유 메모리 정보를 구하기 위한 버퍼 포인터
	// - 리턴값
	//    >> 0 : 성공
	//      -1 : 실패
	if(shmctl(shmid, IPC_RMID,0)==-1)
	{
		fprintf(stderr,"shmctl(IPC_RMID) failed\n");
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
}

/* [출력결과]
 * pi@Hongsberry:~/RaspiEx/LinuxEx_Ras/Execute $ ./shmWriter 
 * Memory attached at 0x0x76f43000
 * shmaddr:0x76f43000, data:1
 * shmaddr:0x76f43004, data:2
 * shmaddr:0x76f43008, data:3
 * shmaddr:0x76f4300c, data:4
 * 		...
 * shmaddr:0x76f43180, data:97
 * shmaddr:0x76f43184, data:98
 * shmaddr:0x76f43188, data:99
 * shmaddr:0x76f4318c, data:100
 *
 */
