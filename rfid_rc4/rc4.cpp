//rc4 for rfid
#include<stdio.h>
#include<stdlib.h>
int word_encrypt(int round, int *KEY, int *plain, int *code, int *Z);
int one_word_encrypt(int *KEY, int *plain, int *code, int *Z);
int initial(int *S, int *KEY);
int generate_key_stream(int *S, int *Z, int *plain, int *code);
int encryption(int plain, int z);
void swap(int *a, int *b);
int atoh(char a);

int main()
{
	int i,j;
	int round=0;			//加密单位数（从低位加密计算，以word（2个Byte）为单位）
	int key_input=0;
	char text[2048];	//输入原文缓冲区
	int tmp[2048];
	int	KEY[4];			//输入密钥4组4bit
	int padNum=0;		//填充位数（4bit为1个单位）
	int charNum=0;		//输入原文位数（4bit为1个单位）
	int totalNum=0;		//填充后原文位数（4bit一个单位）
	int plain[2048];	//输入带加密原文
	int Z[2048];
	int code[2048];		//生成的密文

	printf("please input the plain text(16bit,in terms of hex): ");
	printf("example:if u want to encrypt \"0x12345\", please input 12345 \n");
	#if 0
	int plaintext;
	plaintext= 0x2542;
	printf("text: %x\n", plaintext);

	//将16bit的key_input分成四组4bit数据，然后赋值给KEY[i]
	for(i=0; i<4; i++)
	{
		plain[i]=( plaintext>>(4*i) )& 0x0f;
		printf("plain[%d] = %x \n", i, plain[i]);
	}
	one_word_encrypt(KEY, plain, code, Z);
	#endif
	//原文输入读作字符串后进行转换
	fgets (text, 256, stdin);
	i=0;
	while(text[i]!='\0')
	{
		i++;
	}
	charNum=i-1;
	printf("charNum = %d\n", charNum);
	printf("text: %s\n", text);

	printf("please input the key(16bit, in terms of hex): ");
	printf("example:if u want to using key \"0x1234\", please input 1234 \n");
	scanf("%x", &key_input);
	key_input= 0x1234;
	printf("key: %x\n", key_input);

	//将16bit的key_input分成四组4bit数据，然后赋值给KEY[i]
	for(i=0; i<4; i++)
	{
		KEY[i]=( key_input>>(4*i) )& 0x0f;
		printf("key[%d] = %x \n", i, KEY[i]);
	}
	printf("please input the number of encrypt words(integer): ");
	//scanf("%d",&round);
	printf("encrypt the last %d words\n", round);
	round =1;

	
	if(charNum %4)
	{
		padNum = 4- charNum%4;
	}
	else
	{
		padNum=0;
	}
	totalNum = charNum + padNum;
	printf("padNum %d totalNum %d\n", padNum, totalNum);
	//将16进制输入的text字符串分成四组4bit数据(两个word)，
	//如果不够，高位需补足添零。然后赋值给plain[i]
	//清零初始化
	for(i=0; i<2048; i++)
	{
		plain[i]=0;
	}
	//高位补零操作
	j=0;
	for(i=charNum-1; i>=0; i--)
	{
		plain[i]= atoh(text[j]);
		j++;
	}
	for(j= charNum; j< totalNum; j++)
	{
		plain[j]=0;
	}
#if 0
	// 补零，将字符串转换为数字
	for(i=0; i<padNum; i++)
	{
		plain[i]= 0;
	}
	j=0;
	for(i=padNum; i<totalNum; i++)
	{
		plain[i]= atoh(text[j]);
		j++;
	}
#endif
	
	for(i=0; i< totalNum; i++)
	{
		printf("plain[%d] = %x \n", i, plain[i]);
	}
	
	if(round==0 || round>totalNum/4)
	{
		round = totalNum/4;
	}
	word_encrypt(round, KEY, plain, code, Z);

	return 0;
}

//KEY[0...3]为输入密钥，plain[0...3]为输入原文，code[0...3]为加密生成的密文，Z[0...3]为计算过程中产生的密钥
//输入：KEY, plain,round
// 输出：code,Z
int word_encrypt(int roundNum, int *KEY, int *plain, int *code, int *Z)
{
	int round;
	for(round=0; round< roundNum; round++)
	{
		one_word_encrypt(KEY, plain+4*round, code, Z);
	}

	return 0;
}

//KEY[0...3]为输入密钥，plain[0...3]为输入原文，code[0...3]为加密生成的密文，Z[0...3]为计算过程中产生的密钥
//输入：KEY, plain
// 输出：code,Z
int one_word_encrypt(int *KEY, int *plain, int *code, int *Z)
{
	int i;
	int S[4];

	for(i=0; i<4; i++)
	{
		code[i] = 0;
		Z[i]=0;
	}

	//初始化产生序列S
	initial(S, KEY);

	//产生密钥流并加密
	generate_key_stream(S, Z, plain, code);

	printf("encrypt text:\n");
	for(i=3; i>=0; i--)
	{
		printf("%x", code[i]);
	}
	printf("\n");

	return 0;
}

//初始化函数
//KEY[0...3]为输入密钥
//输入：输入密钥KEY
//输出：S序列
int initial(int *S, int *KEY)
{
	int i,j;
	int tmp;


	for(i=0; i<4; i++)
	{
		S[i]=i;
	}

	j=0;

	for(i=0; i<4; i++)
	{
		j=(j+S[i]+KEY[i])% 4 ;
		swap(&S[i] , &S[j]);
	}

	printf("after initial\n");
	for(i=0; i<4; i++)
	{
		printf("S[%d]= %x ", i, S[i]);
	}
	printf("\n");

	
	return 0;
}

//密码流生成函数
int generate_key_stream(int *S, int *Z, int *plain, int *code)
{
	int i,j,k;
	i=0;
	j=0;
	k=0;
	int loop_times=0;
	
	while(loop_times<4)
	{
		i=(i+1)%4;

		j=(j+ S[i])%4;
		swap(&S[i], &S[j]);
		Z[loop_times]=S[(S[i]+S[j]) % 4];
#if 0
		printf("%d round\n", loop_times);
		for(k=0; k<4; k++)
		{
			printf("Z[%d]= %x ", k, Z[k]);
			printf("S[%d]= %x ", k, S[k]);
		}
		printf("\n");
#endif
		loop_times++;
	}

	for(i=0; i<4; i++)
	{
		code[i] = encryption(plain[i], Z[i]);
		printf("code[%d]= %x ", k, code[k]);
	}

	printf("after generate \n");
	for(i=0; i<4; i++)
	{
		printf("S[%d]= %x ", i, S[i]);
		printf("Z[%d]= %x ", i, Z[i]);
	}
	printf("\n");

	return 0;
}

//密文流产生函数
int encryption(int plain, int z)
{	
	return (plain^z);
}

//ascii to hex 函数
int atoh(char a)
{
	int retVal= -1;

	if(a>='0' && a<='9')
	{
		retVal = a - '0';
	}

	if(a>='a' && a<='f')
	{
		retVal = a-'a'+10;
	}

	if(a>='A' && a<='F')
	{
		retVal = a-'A'+10;
	}

	return retVal;
}
//swap 函数
void swap(int *a, int *b)
{
	int tmp;

	//printf("before swap :a= %d b=%d\n", *a, *b);

	tmp=*a;
	*a=*b;
	*b=tmp;

	//printf("after :a= %d b=%d\n", *a, *b);
}