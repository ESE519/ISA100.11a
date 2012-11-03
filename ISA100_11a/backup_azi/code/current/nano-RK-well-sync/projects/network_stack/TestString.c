#include<stdio.h>
#include<limits.h>
#include<string.h>
#include<math.h> 

long n;

//char ch[100 * (long)pow(2,20)] = {65};
char ch[104857600] = {65};

int main()
{
	// n = 100 * (long)pow(2,20);
	//printf("%ld\n", n);
	ch[104857599] = 0;
	printf("Size  = %d\n", strlen(ch));
	
	return 0;
}

