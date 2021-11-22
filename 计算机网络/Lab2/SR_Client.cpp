// GBN_client.cpp : �������̨Ӧ�ó������ڵ㡣
//
#include <stdlib.h>
#include <WinSock2.h>
#include <time.h>
#include <stdio.h>
#include <fstream>
#pragma comment(lib,"ws2_32.lib")
#define SERVER_PORT 12340 //�������ݵĶ˿ں�
#define SERVER_IP "127.0.0.1" // �������� IP ��ַ
const int BUFFER_LENGTH = 1026;
const int PACKET_NUM = 25;
const int SEQ_SIZE = 20;//���ն����кŸ�����Ϊ 1~20
const int RECV_WIND_SIZE = 10;//���մ��ڴ�СΪ 10��SR��Ӧ������մ��ںͷ��ʹ��ڵĴ�С��С�ڵ��������кŸ���

int curSeq;                 //��ǰ���ݰ��� seq
int curAck;                 //��ǰ�ȴ�ȷ�ϵ� ack
int totalPacket;            //��Ҫ���͵İ�����
BOOL ack[SEQ_SIZE];//���շ�����

int totalSeq;   //�ѷ��͵İ�������
int totalAck;   //ȷ���յ���ack���İ�������
/****************************************************************/
/* -time �ӷ������˻�ȡ��ǰʱ��
-quit �˳��ͻ���
-testgbn [X][Y] ���� GBN Э��ʵ�ֿɿ����ݴ���
[X] [0,1] ģ�����ݰ���ʧ�ĸ���
[Y] [0,1] ģ�� ACK ��ʧ�ĸ���
*/
/****************************************************************/
void printTips() {
	printf("*****************************************\n");
	printf("| -time to get current time             |\n");
	printf("| -quit to exit client                  |\n");
	printf("| -testsr [X] [Y] to test the gbn       |\n");
	printf("*****************************************\n");
}
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
// Method: seqIsAvailable
// FullName: seqIsAvailable
// Access: public
// Returns: bool
// Qualifier: ��ǰ���к�seq�Ƿ�ɽ���
//************************************
bool seqRecvAvailable(int seq) {
	int step;
	step = seq - curAck;
	step = step >= 0 ? step : step + SEQ_SIZE;
	//���к��Ƿ��ڵ�ǰ���մ���֮��
	if (step >= RECV_WIND_SIZE) {
		return false;
	}
	return true;
}
int main(int argc, char* argv[])
{
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
		return 1;
	}
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		printf("Could not find a usable version of Winsock.dll\n");
		WSACleanup();
	}
	else {
		printf("The Winsock 2.2 dll was found okay\n");
	}
	SOCKET socketClient = socket(AF_INET, SOCK_DGRAM, 0);
	SOCKADDR_IN addrServer;
	addrServer.sin_addr.S_un.S_addr = inet_addr(SERVER_IP);
	addrServer.sin_family = AF_INET;
	addrServer.sin_port = htons(SERVER_PORT);
	//���ջ�����
	char buffer[BUFFER_LENGTH];
	ZeroMemory(buffer, sizeof(buffer));
	int len = sizeof(SOCKADDR);
	//Ϊ�˲���������������ӣ�����ʹ�� -time ����ӷ������˻�õ�ǰʱ��
	//ʹ�� -testgbn [X] [Y] ���� GBN ����[X]��ʾ���ݰ���ʧ����
	// [Y]��ʾ ACK ��������
	printTips();
	int ret;
	int interval = 1;//�յ����ݰ�֮�󷵻� ack �ļ����Ĭ��Ϊ 1 ��ʾÿ�������� ack��0 ���߸�������ʾ���еĶ������� ack
	char cmd[128];
	float packetLossRatio = 0; //Ĭ�ϰ���ʧ�� 0.2
	float ackLossRatio = 0; //Ĭ�� ACK ��ʧ�� 0.2
	//��ʱ����Ϊ������ӣ�����ѭ����������
	srand((unsigned)time(NULL));

	//���������ݶ����ڴ�
	std::ifstream icin;
	icin.open("../test.txt");
	char data[1024 * PACKET_NUM];
	ZeroMemory(data, sizeof(data));
	icin.read(data, 1024 * PACKET_NUM);
	icin.close();
	totalPacket = sizeof(data) / 1024;
	int recvSize;
	while (true) {
		gets_s(buffer);
		ret = sscanf(buffer, "%s%f%f", &cmd, &packetLossRatio, &ackLossRatio);
		//��ʼ SR ���ԣ�ʹ�� SR Э��ʵ�� SR �ɿ��ļ�����
		if (!strcmp(cmd, "-testsr")) {
			for (int i = 0; i < SEQ_SIZE; ++i) {
				ack[i] = FALSE;
			}
			FILE* fcli = fopen("clientfile.txt", "w+");
			printf("%s\n", "Begin to test GBN protocol, please don't abort the process");
			printf("The loss ratio of packet is %.2f,the loss ratio of ack is % .2f\n", packetLossRatio, ackLossRatio);
			int waitCount = 0;
			int stage = 0;
			curAck = 0;
			BOOL b;
			unsigned char u_code;//״̬��
			unsigned short seq;//�������к�
			unsigned short recvSeq;//���մ��ڴ�СΪ 1����ȷ�ϵ����к�
			unsigned short waitSeq;//�ȴ������к�
			sendto(socketClient, "-testsr", strlen("-testgbn") + 1, 0, (SOCKADDR*)&addrServer, sizeof(SOCKADDR));
			while (true)
			{
				//�ȴ� server �ظ����� UDP Ϊ����ģʽ
				recvfrom(socketClient, buffer, BUFFER_LENGTH, 0, (SOCKADDR*)&addrServer, &len);
				if (strcmp(buffer, "All Finished!\n") == 0) {
					fclose(fcli);
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
						sendto(socketClient, buffer, 2, 0, (SOCKADDR*)&addrServer, sizeof(SOCKADDR));
						stage = 1;
						recvSeq = 0;
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
					//�ڴ����ڼ�����
					if (seqRecvAvailable(seq)) {
						fwrite(&buffer[1], sizeof(char), strlen(buffer) - 1, fcli);
						//�������
						printf("%s\n",&buffer[1]);
						recvSeq = seq;
						ack[recvSeq - 1] = TRUE;
						buffer[0] = seq;
						buffer[1] = '\0';
						if (seq - 1 == curAck) {
							int nxt;
							for (int i = 0; i < RECV_WIND_SIZE; ++i) {
								nxt = (curAck + i) % SEQ_SIZE;
								if (!ack[nxt])break;
							}
							curAck = (nxt + 1) % SEQ_SIZE;
						}
					}
					else {
						//�����ǰһ������û���յ�����ȴ� Seq Ϊ 1 �����ݰ��������򲻷��� ACK����Ϊ��û����һ����ȷ�� ACK��
						recvSeq = seq;
						buffer[0] = recvSeq;
						buffer[1] = '\0';
					}
					b = lossInLossRatio(ackLossRatio);
					if (b) {
						printf("The ack of %d loss\n", (unsigned char)buffer[0]);
						continue;
					}
					sendto(socketClient, buffer, 2, 0, (SOCKADDR*)&addrServer, sizeof(SOCKADDR));
					printf("send a ack of %d\n", (unsigned char)buffer[0]);
					break;
				}
				Sleep(500);
			}
		}
		//printf("%s\n", buffer);
		sendto(socketClient, buffer, strlen(buffer) + 1, 0, (SOCKADDR*)&addrServer, sizeof(SOCKADDR));
		ret = recvfrom(socketClient, buffer, BUFFER_LENGTH, 0, (SOCKADDR*)&addrServer, &len);
		printf("%s\n", buffer);
		if (!strcmp(buffer, "Good bye!")) {
			break;
		}
		printTips();
	}
	//�ر��׽���
	closesocket(socketClient);
	WSACleanup();
	return 0;
}