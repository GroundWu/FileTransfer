#include<stdio.h>        // printf
#include<stdlib.h>       // exit
#include<string.h>       // bzero

int main()
{
    char s1[10];
    char s2[10];
    printf("s1 input:");
    scanf("%s",s1);
    printf("s2 input:");
    scanf("%s",s2);

    printf("s1:%s\n",s1);
    printf("s2:%s\n",s2);
}