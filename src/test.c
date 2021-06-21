#include<sys/types.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include <arpa/inet.h>

void main(){
   struct sockaddr_in dest;

   memset(&dest, '\0', sizeof(dest));
   dest.sin_addr.s_addr = inet_addr("twitter.com");
   dest.sin_port = 443;
   dest.sin_family = AF_INET;
   printf("%x %x %x %x" , dest.sin_family, dest.sin_port, dest.sin_addr, dest.sin_zero);
}  