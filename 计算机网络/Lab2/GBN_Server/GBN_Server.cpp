#include <stdlib.h>
#include <time.h>
#include <WinSock2.h>
#include <fstream>
#pragma comment(lib,"ws2_32.lib")
#define SERVER_PORT 12340 //�˿ں�
#define SERVER_IP "0.0.0.0" //IP ��ַ
const int PACKET_NUM = 25;
const int BUFFER_LENGTH = 1026; //��������С������̫���� UDP ������֡�а�����ӦС�� 1480 �ֽڣ�
const int SEND_WIND_SIZE = 10;//���ʹ��ڴ�СΪ 10��GBN ��Ӧ���� W + 1 <=N��W Ϊ���ʹ��ڴ�С��N Ϊ���кŸ�����
//����ȡ���к� 0...19 �� 20 ��
//��������ڴ�С��Ϊ 1����Ϊͣ-��Э��
const int SEQ_SIZE = 20; //���кŵĸ������� 0~19 ���� 20 ��
//���ڷ������ݵ�һ���ֽ����ֵΪ 0�������ݻᷢ��ʧ�ܣ���Ϊ0ֱ�Ӵ����ַ����Ľ�β��
//��˽��ն����к�Ϊ 1~20���뷢�Ͷ�һһ��Ӧ
BOOL ack[SEQ_SIZE];//�յ� ack �������Ӧ 0~19 �� ack
int curSeq;//��ǰ���ݰ��� seq
int curAck;//��ǰ�ȴ�ȷ�ϵ� ack
int totalSeq;//�յ��İ�������
int totalPacket;//��Ҫ���͵İ�����
int totalAck;//�Ѿ�ȷ�ϵ�����,�����ж��Ƿ����ֹͣ����
//************************************
// Method: lossInLossRatio
// FullName: lossInLossRatio
// Access: public
// Returns: BOOL
// Qualifier: ���ݶ�ʧ���������һ�����֣��ж��Ƿ�ʧ,��ʧ�򷵻�TRUE�����򷵻� FALSE
// Parameter: float lossRatio [0,1]
//************************************
BOOL lossInLossRatio(float lossRatio) {
	int lossBound = (int)(lossRatio * 100);
	int r = rand() % 100;
	if (r < lossBound) {
		return TRUE;
	}
	return FALSE;
}
//************************************
// Method: getCurTime
// FullName: getCurTime
// Access: public
// Returns: void
// Qualifier: ��ȡ��ǰϵͳʱ�䣬������� ptime ��
// Parameter: char * ptime
//************************************
void getCurTime(char* ptime) {
	char buffer[128];
	memset(buffer, 0, sizeof(buffer));
	time_t c_time;
	struct tm* p;
	time(&c_time);
	p = localtime(&c_time);
	sprintf_s(buffer, "%d/%d/%d %d:%d:%d", p->tm_year + 1900, 
		p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
	strcpy_s(ptime, sizeof(buffer), buffer);
}
//************************************
// Method: seqIsAvailable
// FullName: seqIsAvailable
// Access: public
// Returns: bool
// Qualifier: ��ǰ���к� curSeq �Ƿ����
//************************************
bool seqIsAvailable() {
	int step;
	step = curSeq - curAck;
	step = step >= 0 ? step : step + SEQ_SIZE;//���к���ѭ��ʹ�õ�
	//���к��Ƿ��ڵ�ǰ���ʹ���֮��
	if (step >= SEND_WIND_SIZE) {
		return false;
	}
	if (ack[curSeq]) {
		return true;
	}
	return false;
}
//************************************
// Method: timeoutHandler
// FullName: timeoutHandler
// Access: public 
// Returns: void
// Qualifier: ��ʱ�ش������������������ڵ�����֡��Ҫ�ش�
//************************************
void timeoutHandler() {
	printf("Timer out error.\n");
	int index;
	int res = 0;
	for (int i = 0; i < SEND_WIND_SIZE; ++i) {
		//���Ѿ�����ȥ�ģ���û�յ�ack�ģ�������Ϊ��û��
		index = (i + curAck) % SEQ_SIZE;
		if (ack[index] == FALSE)res++;
		ack[index] = TRUE;
	}
	totalSeq -= res;
	curSeq = curAck;
}
//************************************
// Method: ackHandler
// FullName: ackHandler
// Access: public
// Returns: void
// Qualifier: �յ� ack���ۻ�ȷ�ϣ�ȡ����֡�ĵ�һ���ֽ�
//���ڷ�������ʱ����һ���ֽڣ����кţ�Ϊ 0��ASCII��ʱ����ʧ�ܣ���˼�һ�ˣ��˴���Ҫ��һ��ԭ
// Parameter: char c
//************************************
void ackHandler(char c) {
	unsigned char index = (unsigned char)c - 1; //���кż�һ
	printf("Recv a ack of %d\n", index);
	if (curAck <= index) {
		for (int i = curAck; i <= index; ++i) {
			ack[i] = TRUE;
			totalAck++;
		}
		curAck = (index + 1) % SEQ_SIZE;
	}
	else {
		//�����������������ԭ���£�һ���Ƿ�����������һ�����������к�ѭ��ʹ��
		//�����ǵ����������ü���ack�����˵ĸ��ʽϵͣ��������߲�ֵ�ж����������
		//ack ���������ֵ���ص��� curAck �����
		if (curAck - index > SEND_WIND_SIZE) {
			for (int i = curAck; i < SEQ_SIZE; ++i) {
				ack[i] = TRUE;
				totalAck++;
			}
			for (int i = 0; i <= index; ++i) {
				ack[i] = TRUE;
				totalAck++;
			}
		}
		curAck = index + 1;
	}
}
//������
int main(int argc, char* argv[]) {
	//�����׽��ֿ⣨���룩
	WORD wVersionRequested;
	WSADATA wsaData;
	//�׽��ּ���ʱ������ʾ
	int err;
	//�汾 2.2
	wVersionRequested = MAKEWORD(2, 2);
	//���� dll �ļ� Scoket ��
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) {
		//�Ҳ��� winsock.dll
		printf("WSAStartup failed with error: %d\n", err);
		return -1;
	}
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		printf("Could not find a usable version of Winsock.dll\n");
		WSACleanup();
	}
	else {
		printf("The Winsock 2.2 dll was found okay\n");
	}

	//˫���䣬���Է�������ҲҪ���ö�����
	float packetLossRatio = 0;
	float ackLossRatio = 0;
	srand((unsigned)time(NULL));

	SOCKET sockServer = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	//�����׽���Ϊ������ģʽ
	int iMode = 1; //1����������0������
	ioctlsocket(sockServer, FIONBIO, (u_long FAR*) & iMode);//����������
	SOCKADDR_IN addrServer; //��������ַ
	//addrServer.sin_addr.S_un.S_addr = inet_addr(SERVER_IP);
	addrServer.sin_addr.S_un.S_addr = htonl(INADDR_ANY);//���߾���
	addrServer.sin_family = AF_INET;
	addrServer.sin_port = htons(SERVER_PORT);
	err = bind(sockServer, (SOCKADDR*)&addrServer, sizeof(SOCKADDR));
	if (err) {
		err = GetLastError();
		printf("Could not bind the port %d for socket.Error code is % d\n", SERVER_PORT, err);
		WSACleanup();
		return -1;
	}
	SOCKADDR_IN addrClient; //�ͻ��˵�ַ
	int length = sizeof(SOCKADDR);
	char buffer[BUFFER_LENGTH]; //���ݷ��ͽ��ջ�����
	ZeroMemory(buffer, sizeof(buffer));
	//���������ݶ����ڴ�
	std::ifstream icin;
	icin.open("../test.txt");
	char data[1024 * PACKET_NUM];
	ZeroMemory(data, sizeof(data));
	icin.read(data, 1024 * PACKET_NUM);
	icin.close();
	totalPacket = sizeof(data) / 1024;
	//printf("%d\n", totalPacket);
	int recvSize;
	for (int i = 0; i < SEQ_SIZE; ++i) {
		ack[i] = TRUE;
	}
	while (true) {
		//���������գ���û���յ����ݣ�����ֵΪ-1
		recvSize = recvfrom(sockServer, buffer, BUFFER_LENGTH, 0, ((SOCKADDR*)&addrClient), &length);
		if (recvSize < 0) {
			Sleep(200);
			continue;
		}
		printf("recv from client: %s\n", buffer);
		if (strcmp(buffer, "-time") == 0) {
			getCurTime(buffer);
		}
		else if (strcmp(buffer, "-quit") == 0) {
			strcpy_s(buffer, strlen("Good bye!") + 1, "Good bye!");
		}
		else if (strcmp(buffer, "-testgbn") == 0) {
			//���� gbn ���Խ׶�
			//���� server��server ���� 0 ״̬���� client ���� 205 ״̬�루server���� 1 ״̬��
			//server �ȴ� client �ظ� 200 ״̬�룬����յ���server ���� 2 ״̬������ʼ�����ļ���������ʱ�ȴ�ֱ����ʱ
			//���ļ�����׶Σ�server ���ʹ��ڴ�С��Ϊ
			for (int i = 0; i < SEQ_SIZE; ++i) {
				ack[i] = TRUE;
			}
			ZeroMemory(buffer, sizeof(buffer));
			int recvSize;
			int waitCount = 0;
			printf("Begin to test GBN protocol,please don't abort the process\n");
			//������һ�����ֽ׶�
			//���ȷ�������ͻ��˷���һ�� 205 ��״̬�루���Լ�����ģ���ʾ������׼�����ˣ����Է�������
			//�ͻ����յ� 205 ֮��ظ�һ�� 200 ��״̬�룬��ʾ�ͻ���׼�����ˣ����Խ���������
			//�������յ� 200 ״̬��֮�󣬾Ϳ�ʼʹ�� GBN ����������
			printf("Shake hands stage\n");
			int stage = 0;
			bool runningFlag = true;
			int isFinished = 0;
			while (runningFlag) {
				if (isFinished) {
					break;
				}
				switch (stage) {
				case 0://���� 205 �׶�
					buffer[0] = 205;
					sendto(sockServer, buffer, strlen(buffer) + 1, 0, (SOCKADDR*)&addrClient, sizeof(SOCKADDR));
					Sleep(100);
					stage = 1;
					break;
				case 1://�ȴ����� 200 �׶Σ�û���յ��������+1����ʱ������˴Ρ����ӡ����ȴ��ӵ�һ����ʼ
					recvSize = recvfrom(sockServer, buffer, BUFFER_LENGTH, 0, ((SOCKADDR*)&addrClient), &length);
					if (recvSize < 0) {
						++waitCount;
						if (waitCount > 20) {
							runningFlag = false;
							printf("Timeout error\n");
							break;
						}
						Sleep(500);
						continue;
					}
					else {
						if ((unsigned char)buffer[0] == 200) {
							printf("Begin a file transfer\n");
							printf("File size is %dB, each packet is 1024B and packet total num is % d\n", sizeof(data), totalPacket);
							curSeq = 0;
							curAck = 0;
							totalSeq = 0;
							totalAck = 0;
							waitCount = 0;
							stage = 2;
						}
					}
					break;
				case 2://���ݴ���׶�
					if (seqIsAvailable() && totalSeq<totalPacket) {//�ڶ����ж��Ǵ����Ѿ���������Ϊ����ack��ʧ����δȷ�ϵ��������ʱ����ʱ����Ҫ����
						//���͸��ͻ��˵����кŴ� 1 ��ʼ
						buffer[0] = curSeq + 1;
						ack[curSeq] = FALSE;
						//���ݷ��͵Ĺ�����Ӧ���ж��Ƿ������
						//Ϊ�򻯹��̴˴���δʵ��
						memcpy(&buffer[1], data + 1024 * totalSeq, 1024);
						printf("send a packet with a seq of %d\n", curSeq);
						sendto(sockServer, buffer, BUFFER_LENGTH, 0, (SOCKADDR*)&addrClient, sizeof(SOCKADDR));
						++curSeq;
						curSeq %= SEQ_SIZE;
						++totalSeq;
						Sleep(500);
					}
					//�ȴ� Ack����û���յ����򷵻�ֵΪ-1��������+1
					recvSize = recvfrom(sockServer, buffer, BUFFER_LENGTH, 0, ((SOCKADDR*)&addrClient), &length);
					if (recvSize < 0) {
						waitCount++;
						//20 �εȴ� ack ��ʱ�ش�
						if (waitCount > 20)
						{
							timeoutHandler();
							waitCount = 0;
						}
					}
					else {
						//�յ� ack
						ackHandler(buffer[0]);
						waitCount = 0;
						//printf("%d\n", totalAck);
						if (totalAck == totalPacket ) {
							isFinished = 1;
							strcpy(buffer, "All Finished!\n");
						}
					}
					Sleep(500);
					break;
				}
			}
		}
		else if (strcmp(buffer, "-testgbn_send") == 0) {//˫������������������������Ѿ����Թ��ˣ�����������Է�������
			FILE* fser = fopen("serverfile.txt", "w+");
			int iMode = 0; //1����������0������
			ioctlsocket(sockServer, FIONBIO, (u_long FAR*)& iMode);//��������
			printf("%s\n", "Begin to test GBN protocol, please don't abort the process");
			printf("The loss ratio of packet is %.2f,the loss ratio of ack is %.2f\n", packetLossRatio, ackLossRatio);
			int waitCount = 0;
			int stage = 0;
			BOOL b;
			unsigned char u_code;//״̬��
			unsigned short seq;//�������к�
			unsigned short recvSeq;//���մ��ڴ�СΪ 1����ȷ�ϵ����к�
			unsigned short waitSeq;//�ȴ������к�
			int len = sizeof(SOCKADDR);
			while (true)
			{
				//�ȴ� server �ظ����� UDP Ϊ����ģʽ
				recvfrom(sockServer, buffer, BUFFER_LENGTH, 0, (SOCKADDR*)&addrClient, &len);
				if (strcmp(buffer, "All Finished!\n") == 0) {
					fclose(fser);//д��֮��ر��ļ�
					break;
				}
				switch (stage) {
				case 0://�ȴ����ֽ׶�
					u_code = (unsigned char)buffer[0];
					if ((unsigned char)buffer[0] == 205)
					{
						printf("Ready for file transmission\n");
						buffer[0] = 200;
						buffer[1] = '\0';
						sendto(sockServer, buffer, 2, 0, (SOCKADDR*)&addrClient, sizeof(SOCKADDR));
						stage = 1;
						recvSeq = 0;
						waitSeq = 1;
					}
					break;
				case 1://�ȴ��������ݽ׶�
					seq = (unsigned short)buffer[0];
					//�����ģ����Ƿ�ʧ
					b = lossInLossRatio(packetLossRatio);
					if (b) {
						printf("The packet with a seq of %d loss\n", seq);
						continue;
					}
					printf("recv a packet with a seq of %d\n", seq);
					//������ڴ��İ�����ȷ���գ�����ȷ�ϼ���
					if (!(waitSeq - seq)) {
						++waitSeq;
						fwrite(&buffer[1], sizeof(char), strlen(buffer) - 1, fser);
						if (waitSeq == 21) {
							waitSeq = 1;
						}
						//�������
						printf("%s\n",&buffer[1]);
						buffer[0] = seq;
						recvSeq = seq;
						buffer[1] = '\0';
					}
					else {
						//�����ǰһ������û���յ�����ȴ� Seq Ϊ 1 �����ݰ��������򲻷��� ACK����Ϊ��û����һ����ȷ�� ACK��
						if (!recvSeq) {
							continue;
						}
						buffer[0] = recvSeq;
						buffer[1] = '\0';
					}
					b = lossInLossRatio(ackLossRatio);
					if (b) {
						printf("The ack of %d loss\n", (unsigned char)buffer[0]);
						continue;
					}
					sendto(sockServer, buffer, 2, 0, (SOCKADDR*)&addrClient, sizeof(SOCKADDR));
					printf("send a ack of %d\n", (unsigned char)buffer[0]);
					break;
				}
				Sleep(500);
			}
		}
		//printf("%s\n", buffer);
		sendto(sockServer, buffer, strlen(buffer) + 1, 0, (SOCKADDR*)&addrClient, sizeof(SOCKADDR));
		Sleep(500);
	}
	//�ر��׽��֣�ж�ؿ�
	closesocket(sockServer);
	WSACleanup();
	return 0;
}
