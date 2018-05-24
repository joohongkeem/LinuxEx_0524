/* [System V IPC]
 * - 식별자(identifier) 와 키(Key)
 *   >> 메시지 큐, 세마포어, 공유 메모리 등의 IPC구조는 고유한 식별자에 의해 구별
 *   >> 키는 IPC구조를 생성할 때 (msgget, semget, shmget를 호출) 지정
 *      (키는 커널에 의해 식별자로 변환됨)
 * - <sys/ipc.h> 참조
 *
 *		     (약속,만남)
 * [server와 client의 rendezvous]
 * - server가 키값을 IPC_PRIVATE로 지정하여 새로운 IPC구조 생성하고 식별자를 저장,
 *   client는 저장된 값을 읽어 식별자 값을 얻는다.(file이용, parent-child간)
 * - server와 client가 약속된 키 값을 사용
 * - pathname과 proj를 약속하여 key_t ftok(char *pathname, char proj)함수를 이용하여
 *   키 값 생성
 *
 * - char *path	 	 key_t key    msgget()  int id
 *   ----------- ftok() ------------- semget() ---------
 *   char proj			      shmget()
 *
 * - IPC_PRIVATE로 생성된 키
 *   >> file에 저장해 공유 가능
 *   >> fork를 통해 공유가능 (exec시에 arg로 전달)
 * - 미리 약속된 키를 인지
 * - 특정 path name과 int형의 id로 각기 키를 생성 (같은 키가 생성됨)
 *
 * [ftok]                            (얻다)
 * - allows independent processes to derive the same key
 *   >> Based on a known pathname
 *   >> The file corresponding to the pathname must exist and 
 *      accessible that want to access an IPC object
 *
 * - #include <sys/ipc.h>
 *   key t_ftok(const char *path, int id);
 *
 *   >> The path name is a name of file in file system
 *   >> The id allows several IPC objects of the same type to be keyed form a single pahtname
 *   >> Return a key if successful, otherwise return -1 and set errno.
 *
 * [IPC 구조 관련 명령]
 * - ipcs : report IPC facility status
 *     ex) % ipcs [-q -m -s]
 * - ipcrm : remove IPC facility
 *     ex) % ipcrm [-q msgid] or [-Q msgkey]
 *         % ipcrm [-m shmid] or [-M shmkey]
 *         % ipcrm [-s semid] or [-S semkey]
 *
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


#include <sys/msg.h>


struct msqid_ds{
	struct ipc_perm msg_perm;
	struct msg *msg_first;
	struct msg *msg_last;
	unsigned short msg_cbytes;
	unsigned short msg_qnum;
	unsigned short msg_qbytes;
	unsigned short msg_lspid;
	unsigned short msg_lrpid;
	time_t msg_stime;
	time_t msg_rtime;
	time_t msg_ctime;
};


