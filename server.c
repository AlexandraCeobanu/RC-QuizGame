#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <signal.h>
#include <pthread.h>
#include <sqlite3.h>
#define port 2024
sqlite3 *datab;
int handle;
static void *treat(void * arg);
void pune_intrebare(void * arg);
void mesaj_jucatori();
void inregistrare(int sock, int id_thread);
void reset();
void castigator();
void clasament_f();
void baza_date();
typedef struct thData{
	int idThread; //id-ul thread-ului tinut in evidenta de acest program
	int cl; //descriptorul intors de accept
}thData;
pthread_t th[100];
thData *td[100]; //parametru functiei executata de thread //Identificatorii thread-urilor care se vor crea
struct jucator{
    char nume[100];
    int scor;
    int activ;
    int id_jucator;
}jucatori[100];
int cnt=0,nr_intrebare;
int main()
{
    baza_date();
	int i=0;
    int socketServer,socketClient;
    struct sockaddr_in server,client;
    socklen_t addr_size;
    char buffer[300];
    if ((socketServer= socket (AF_INET, SOCK_STREAM, 0)) == -1) // creare socket
    {
      perror ("[server]Eroare la socket().\n");
      exit(1);
    }
    int optval=1;
    setsockopt(socketServer, SOL_SOCKET, SO_REUSEADDR,&optval,sizeof(optval));
    bzero (&server, sizeof (server)); // pregatirea structurilor de date
    server.sin_family = AF_INET;	// stabilirea familiei de socket-uri;
    server.sin_addr.s_addr = htonl (INADDR_ANY); // acceptam orice adresa
    server.sin_port = htons (port); // utilizam un port utilizator
    if(bind (socketServer, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1) // atasam socketul
    {
      perror ("[server]Eroare la bind().\n");
      exit(2);
    }
    if (listen (socketServer, 10) == -1) // punem serverul sa asculte daca vin clienti sa se conecteze
    {
      perror ("[server]Eroare la listen().\n");
      exit(2);
    }
    printf("Listening....\n");
    //pthread_create(&th[i], NULL, &treat, td);
    while(1)
    {
        addr_size=sizeof(client);
        socketClient=accept (socketServer, (struct sockaddr *) &client, &addr_size); //acceptam un client
        if (socketClient < 0)
	    {
	        perror ("[server]Eroare la accept().\n");
            exit(2);
	    }
        printf("Client connected\n");
        inregistrare(socketClient,i);
        bzero(buffer,sizeof(buffer));
        strcpy(buffer,"primul jucator");
        write (socketClient, buffer, strlen(buffer));
        bzero(buffer,sizeof(buffer));
        read (socketClient, buffer, sizeof(buffer));
        bzero(buffer,sizeof(buffer));
        strcpy(buffer,"Introduceti numarul de jucatori: ");
        write (socketClient, buffer, strlen(buffer));
        bzero(buffer,sizeof(buffer));
        read (socketClient, buffer, sizeof(buffer));
        printf("[server]: Numarul de jucatori este %s\n",buffer);
        int nr_jucatori=atoi(buffer);
        td[i]=(struct thData*)malloc(sizeof(struct thData));
        td[i]->idThread=i;
	    td[i]->cl=socketClient;
        i=i+1;
         while(i<nr_jucatori)
    {
        int socketClient;
        addr_size=sizeof(client);
        socketClient=accept (socketServer, (struct sockaddr *) &client, &addr_size); //acceptam un client
        if (socketClient < 0)
	        {
	            perror ("[server]Eroare la accept().\n");
                exit(2);
	        }
        printf("Client connected\n");
        td[i]=(struct thData*)malloc(sizeof(struct thData));
        td[i]->idThread=i;
	    td[i]->cl=socketClient;
        inregistrare(socketClient,td[i]->idThread);
        bzero(buffer,sizeof(buffer));
        char text[20];
        sprintf(text,"Al %d jucator   ",i);
        strcpy(buffer,text);
        buffer[strlen(buffer)]='\0';
        write (socketClient, buffer, strlen(buffer));
        bzero(buffer,sizeof(buffer));
        bzero(buffer,sizeof(buffer));
        read (socketClient, buffer, sizeof(buffer));
        i=i+1;
    }
    mesaj_jucatori();
        nr_intrebare=1;
        while(nr_intrebare<=3) // 3 numarul de intrebari din baza de date
    {
        for(int j=0;j<i;j++)
        {
            if(jucatori[j].activ==1)
            {pthread_create(&th[j], NULL, &treat, td[j]);
            pthread_join(th[j],NULL);
            sleep(2);}
        }
        ++nr_intrebare;
    }
    castigator();
    reset();
    i=0;
    }
    sqlite3_close(datab);
    return 0;
}
void baza_date()
{
     handle = sqlite3_open("baza_date.db", &datab);
    if (handle != SQLITE_OK)
    {
        printf("[BD]: Nu am reusit sa deschid baza de date!  \n");
        exit (1); //select din baza de date
    }
}
void reset()
{
    for(int j=0;j<cnt;j++)
    {
    bzero(jucatori[j].nume,sizeof(jucatori[j].nume));
    jucatori[j].scor=0;
    jucatori[j].id_jucator=0;
    jucatori[j].activ=0;
    free(td[j]);
    }
    cnt=0;
}
void mesaj_jucatori()
{
    char buffer[300];
    int j;
    bzero(buffer,sizeof(buffer));
    strcpy(buffer,"Jocul a inceput");
    for(j=0;j<cnt;j++)
        {
            //printf("S-a trimis mesajul jucatorului %d/n",j);
            write (td[j]->cl, buffer, strlen(buffer));
            bzero(buffer,sizeof(buffer));
            read(td[j]->cl, buffer, sizeof(buffer));
        }
}
void clasament_f()
{
    for(int i=0;i<cnt-1;i++)
    {
        for(int j=i+1;j<cnt;j++)
        {
            if(jucatori[i].scor<jucatori[j].scor)
            {
                    int aux=jucatori[i].scor;
                    jucatori[i].scor=jucatori[j].scor;
                    jucatori[j].scor=aux;
                    char nume[40];
                    strcpy(nume,jucatori[i].nume);
                    strcpy(jucatori[i].nume,jucatori[j].nume);
                    strcpy(jucatori[j].nume,nume);
            }
        }

    }
}
void castigator()
{
    int max_puncte=0;
    char clasament[300]="",loc[20],puncte[10];
    clasament_f();
    for(int i=0;i<cnt;i++)
    {
        sprintf(loc,"%d",(i+1));
        strcat(clasament,loc);
        strcat(clasament,")");
        strcat(clasament,jucatori[i].nume);
        strcat(clasament,"-->");
        sprintf(puncte,"%d",jucatori[i].scor);
        strcat(clasament,puncte);
        strcat(clasament," puncte");
        if(i==0)
        {
            strcat(clasament,"  CASTIGATOR");
        }
        strcat(clasament,"\n");
        bzero(loc,sizeof(loc));
        bzero(puncte,sizeof(puncte));
    }
    
    for(int i=0;i<cnt;i++)
    {
         write (td[i]->cl, clasament, strlen(clasament));
         close(td[i]->cl);
         printf("[server]: Client deconectat\n");
    }

}
static void *treat(void * arg)
{		
		struct thData tdL; 
        char buffer[300];
		tdL= *((struct thData*)arg);	
		printf ("[thread]- %d - Asteptam mesajul...\n", tdL.idThread);
		fflush (stdout);	
		pune_intrebare((struct thData*)arg);
        printf(" scor %d\n",jucatori[tdL.idThread].scor);
		return(NULL);	
  		
}
void inregistrare(int sock,int id_thread)
{
    char buffer[300];
    strcpy(buffer,"Introduceti numele:");
    write (sock, buffer, strlen(buffer));
    bzero(buffer,sizeof(buffer));
    read (sock, buffer, sizeof(buffer));
    printf("[server]: Numele jucatorului este %s\n",buffer);
    strcpy(jucatori[cnt].nume,buffer);
    jucatori[id_thread].scor=0;
    jucatori[id_thread].id_jucator=id_thread;
    jucatori[id_thread].activ=1;
    cnt=cnt+1;
}
void pune_intrebare(void *arg)
{
    char buffer[300];
    sqlite3_stmt *stmt; // pointer catre prima linie din baza de date
    struct thData tdL; 
	tdL= *((struct thData*)arg);
    int sock=tdL.cl;
    int thread=tdL.idThread;
    char *sql = sqlite3_mprintf("SELECT id_intrebare,intrebare,var1,var2,var3,var_corecta,punctaj FROM INTREBARI");
    if(sqlite3_prepare_v2(datab,sql,-1,&stmt,0)!=SQLITE_OK)//select din baza de date
        {
            printf("[BD]: Nu am reusit sa execut select! %s \n",sqlite3_errmsg(datab));
            exit (1);
        }
        const unsigned char * intrebare,*var1,*var2,*var3;
        int punctaj,id_intrebare,var_corecta;
        while(sqlite3_step(stmt)!=SQLITE_DONE)
        {   
            id_intrebare=sqlite3_column_int(stmt,0);
            if(id_intrebare==nr_intrebare)
            {
            intrebare=sqlite3_column_text(stmt,1);
            var1=sqlite3_column_text(stmt,2);
            var2=sqlite3_column_text(stmt,3);
            var3=sqlite3_column_text(stmt,4);
            var_corecta=sqlite3_column_int(stmt,5);
            punctaj=sqlite3_column_int(stmt,6);
            bzero(buffer,sizeof(buffer));
            strcpy(buffer,intrebare);
            strcat(buffer,"\n");
            strcat(buffer,var1);
            strcat(buffer,"\n");
            strcat(buffer,var2);
            strcat(buffer,"\n");
            strcat(buffer,var3);
            if (write (sock, buffer, strlen(buffer)) < 0)
            {
                perror ("[server]Eroare la write() spre client.\n");
                exit(2);
            }   
            bzero(buffer,sizeof(buffer));
            fd_set readfds; //multimea descriptorilor activi 
            struct timeval timee;
            timee.tv_sec = 7;
            timee.tv_usec = 0;
            FD_ZERO (&readfds); // initial multimea este vida
            FD_SET(sock, &readfds); //includem in multime socketul curent
            select((sock+1), &readfds, NULL, NULL, &timee);
            if(FD_ISSET(sock, &readfds)==0)
            {
                printf("Timpul de raspuns a fost depasit!\n");
                if (read (sock, buffer, sizeof(buffer)) < 0)
                {
                    perror ("[server]Eroare la read() din client.\n");
                    exit(2);
                }
                if(strstr(buffer,"exit")!=NULL)
                {
                    jucatori[thread].activ=0;
                    close(sock);
                }
                bzero(buffer,sizeof(buffer));
                strcpy(buffer,"1");
                write (sock, buffer, strlen(buffer));
            }
            else{
            if (read (sock, buffer, sizeof(buffer)) < 0)
                {
                    perror ("[server]Eroare la read() din client.\n");
                    exit(2);
                }
           printf("[Raspuns][Thread %d]:%s,\n",thread,buffer);
           char raspuns[30];
            strcpy(raspuns,"0");
            write (sock, raspuns, strlen(raspuns));
            bzero(raspuns,sizeof(raspuns));
            strcpy(raspuns,buffer);
            if(var_corecta==atoi(buffer))
            {
               // printf("raspuns corect\n");
                jucatori[thread].scor=jucatori[thread].scor+punctaj;
            }
            else
            if(strstr(raspuns,"exit")!=NULL)
            {
                jucatori[thread].activ=0;
                close(sock);
            }}
            FD_CLR (sock, &readfds);
            break;
            }
        }
        sqlite3_finalize(stmt);
}
