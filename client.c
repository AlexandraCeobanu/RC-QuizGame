#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include<sqlite3.h>
#include<time.h>
#define port 2024
int secunde=5,flag=0;
char limita_depasita[5];
void printTimer()
{
  fflush(stdout);
    printf("\r ---%d----  Raspunsul este : ",secunde);
    fflush(stdout);
}
void delay(int s)
{
    sleep(s);
}
static void *counter()
{
  while(secunde>=0)
  {
     delay(1);
    printTimer();
    --secunde;
  }
  printf("\n");
  secunde=5;
}
int id_intrebare=1;
void inregistrare(int sock);
int main()
{
    int sock;
    int primul_jucator=0;
    struct sockaddr_in addr;
    socklen_t addr_size;
    pthread_t th;
    char buffer[300];
    if ((sock= socket (AF_INET, SOCK_STREAM, 0)) == -1) // creare socket
    {
      perror ("[server]Eroare la socket().\n");
      exit(1);
    }
    bzero (&addr, sizeof (addr)); // pregatirea structurilor de date
    addr.sin_family = AF_INET;	// stabilirea familiei de socket-uri;
    addr.sin_addr.s_addr = htonl (INADDR_ANY); // acceptam orice adresa
    addr.sin_port = htons (port); // utilizam un port utilizator
    if (connect (sock, (struct sockaddr *) &addr,sizeof (struct sockaddr)) == -1) // ne conectam la server
    {
      perror ("[client]Eroare la connect().\n");
      exit(1);
    }
    printf("Connected\n");
    inregistrare(sock);
    bzero(buffer,sizeof(buffer));
    read (sock, buffer, sizeof(buffer));
    if(strstr(buffer,"primul jucator")!=NULL)
   { 
    primul_jucator=1;
    bzero(buffer,sizeof(buffer));
    strcpy(buffer,"DA");
    write (sock, buffer, strlen(buffer));
    bzero(buffer,sizeof(buffer));
    read (sock, buffer, sizeof(buffer));
    printf("%s\n",buffer);
    bzero(buffer,sizeof(buffer));
    scanf("%s",buffer);
    write (sock, buffer, strlen(buffer));
  }
  else{
    bzero(buffer,sizeof(buffer));
    strcpy(buffer,"Se asteapta jucatori");
    write (sock, buffer, strlen(buffer));
    bzero(buffer,sizeof(buffer));
  }
  printf("Se asteapta jucatori\n");
  bzero(buffer,sizeof(buffer));
  read(sock, buffer, sizeof(buffer));
  printf("%s\n",buffer);
    bzero(buffer,sizeof(buffer));
    strcpy(buffer,"A inceput jocul");
    write (sock, buffer, strlen(buffer));
    bzero(buffer,sizeof(buffer));
    while(strstr(buffer,"exit")==NULL)
      {
        if(id_intrebare>3)
        {
          break;
        }
        bzero(buffer,sizeof(buffer));
        read (sock, buffer, sizeof(buffer));
        if(buffer!=NULL)
        printf("\n[Intrebare]:%s\n",buffer);
        bzero(buffer,sizeof(buffer));
        pthread_create(&th, NULL, &counter, NULL);
        scanf("\n%s",buffer);
        pthread_cancel(th);
        secunde=5;
        write (sock, buffer, strlen(buffer));
        bzero(limita_depasita,sizeof(limita_depasita));
        read(sock,limita_depasita,sizeof(limita_depasita));
        if(strstr(limita_depasita,"1")!=NULL && strstr(buffer,"exit")==NULL)
        printf("Ati depasit limita de timp!Raspunsul nu va fi luat in considerare\n");
        ++id_intrebare;
        sleep(1);
        if(strstr(buffer,"exit")==NULL)
        printf("Se asteapta sa raspunda ceilalti jucatori\n");
      }
      if(strstr(buffer,"exit")==NULL){
        bzero(buffer,sizeof(buffer));
        read (sock, buffer, sizeof(buffer));
        printf("[client]: Jocul s-a terminat\n");
        printf("\n[Clasamentul este]:\n");
        printf("%s",buffer);
        }
        else
        {
          printf("Jucator deconectat\n");
        }
        close(sock);
    return 0;
}
void inregistrare(int sock)
{
    char buffer[300];
    bzero(buffer,sizeof(buffer));
    read (sock, buffer, sizeof(buffer));
    printf("%s\n",buffer);
    bzero(buffer,sizeof(buffer));
    scanf("%s",buffer);
    write (sock, buffer, strlen(buffer));
    printf("Bine ati venit, %s\n",buffer);
}
