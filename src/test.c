#include<sys/types.h>
#include<string.h>
#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>

void main(){
   char address[] = "/cgi/?name=ICWS";
   printf("%s",strtok(address,"?"));
}  