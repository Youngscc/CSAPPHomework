#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
# define LL long long

/*
	A����Ϊ����ÿһ���ֽ����з����ֽڣ����Ǵ�unsigned�����ĳһ���ֽڵ�ʱ����Ҫע�������λ������ֵ���پ���ǰ�߲���λ��ʱ����0��1�����ô���Ĭ�ϲ�0���ͻᵼ�������ǰ�����Ǹ�����������֮�����һ�������� 
*/

typedef unsigned packed_t;

int xbyte(packed_t word, int bytenum){
	int turnLeft = (3 - bytenum) << 3;
	int turnRight = 24;
	int num = word;
	return num << turnLeft >> turnRight;
}

int main(){
	printf("%d\n", xbyte(0xffffffff, 2));
	return 0;
}

