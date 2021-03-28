//�������������ϵͳ���ڶ�����ҵ

# include <stdio.h>
# define LL long long

/*
	-------------------   2.59   -------------------
*/

int didit(int x,int y){
	return ( (x & 0xFF) | (y & 0xFFFFFF00) );
}

void solve1(){
	int x=0x89ABCDEF,y=0x76543210;
	printf("0x%X\n",didit(x,y));
}

/*
	-------------------   2.63   -------------------
*/

unsigned srl(unsigned x,int k){
	/* Perform shift arithmetically */
	unsigned xsra = (int) x >> k;
	int w = sizeof(int) << 3;
	unsigned sum = ( 1 << (w-k) ) - 1;
	return xsra & sum;
}

int sra(int x,int k){
	/* Perform shift logically */
	int xsrl = (unsigned) x >> k;
	int w = sizeof(int) << 3;
	int p = 1 << (w - k - 1);
	p = (p & xsrl);
	p = ~(p - 1);
	return p | xsrl;
}

void solve2(){
	int x = -2;
	printf("%u %d\n",srl(x,1),sra(x,1));
}

/*
	-------------------   2.67   -------------------
*/

/*
    A��һ������ʱ���Ƶ�λ�����ܳ�����ǰ�������͵�λ�� 
*/

int int_size_is_32(){
	int set_msb = 1 << 31;
	int beyond_msb = set_msb << 1;
	return set_msb && !beyond_msb;
}

int int_size_is_32_for16bit(){
	int set_msb = 1 << 15;
	set_msb <<= 15;
	set_msb <<= 1;
	int beyond_msb = set_msb << 1;
	return set_msb && !beyond_msb;
}

void solve3(){
	if (int_size_is_32()) printf("It's 32-bit\n");
	else printf("It's not 32-bit\n");
}

/*
	-------------------   2.71   -------------------
*/

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

void solve4(){
	printf("%d\n", xbyte(0xffffffff, 2));
}

/*
	-------------------   2.75   -------------------
*/

int signed_high_prod(int x, int y){
	return (int) ((1LL*x*y) >> 32);
}

unsigned unsigned_high_prod(unsigned x, unsigned y){
	unsigned x_sgn = x >> 31;
	unsigned y_sgn = y >> 31;
	return signed_high_prod(x,y) + x_sgn*y + y_sgn*x;
}

void solve5(){
	unsigned x=0xFFFFFFFF;
	unsigned y=0xF1F1F1F1;
	printf("%u\n",unsigned_high_prod(x,y));
}

/*
	-------------------   2.79   -------------------
*/

int divide_power2(int x,int k){
	int sgn = x >> 31;
	x += sgn & ((1 << k) - 1);
	return x >> k;
}

int mul3div4(int x){
	return divide_power2( (x << 1) + x , 2);
}

void solve6(){
	int x = -2147483647;
	printf("%d\n",mul3div4(x));
}

/*
	-------------------   2.83   -------------------
*/

/*
	A��������x = 0.yyyyyyyy��������ôx<<k = y.yyyyyyyy����=x*(2^k)����x*(2^k)=x+y����x((2^k)-1)=y��x=y/((2^k)-1);
	x=y/((2^k)-1)
	
	B��
		a.  x = 101 / 111 = 5/7
		b.	x = 0110 / 1111 = 2/5
		c.  x = 010011 / 111111 = 19/63
*/

int main(){
//	solve1();
//	solve2();
//	solve3();
//	solve4();
//	solve5();
//	solve6();
} 
