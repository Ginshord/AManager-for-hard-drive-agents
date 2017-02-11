#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <ifaddrs.h>
#include <time.h> //get time since 1970 1.1

#include <signal.h>
#include <sys/stat.h>

#include <pthread.h> //pthread_create
#include <sys/utsname.h> //call  uname() to get local os info

#define BUFSIZE 32  // if too long  like  1024  then at manager side, it will make port string length 996.
long n;
int sockfd;
long sent_bytes;
char buf[BUFSIZE];
void *CmdAgent(void *x);
void setBaconID(void);
char* getIpaddr(void);
void setBaconIP(void);
void setBaconPort(void);
void bacon2String(void) ;
void setBaconStartUpTime(void);
void BeaconSender(struct sockaddr_in);
void GetLocalOS(char OS[16], int *valid);
void GetLocalTime(int *time, int *valid);
struct BACON
{
    int 	ID;          // randomly generated during startup
    int 	StartUpTime; // the time when the client starts
    char*  IP[4];	  // the IP address of this client
    int	CmdPort;     //  the client listens to this port for cmd
};

void error(char *msg) {
    perror(msg);
    exit(1);
}

int send_packets(char *buffer, int buffer_len)// 1 fixed sec to send bacon msg to udp server.
{
    while (1) {
      sent_bytes = send(sockfd, buffer, buffer_len, 0);
      if (sent_bytes < 0)
      {
          fprintf(stderr, "cannot send. The Manager Server may not open\n");
          return -1;
      }
      sleep(5);
    }
    return -1;
}

struct BACON baconMsg;
char* baconString;
int main(int argc,  char *argv[]) {
    pthread_t thread1;
    baconString = malloc(1024);
    unsigned int addr = inet_addr(argv[1]);
    struct hostent *server = gethostbyaddr((char*)&addr, 4, AF_INET);
    unsigned int svrAddr = *(unsigned int *) server->h_addr_list[0];
    unsigned short svrPort = atoi(argv[2]);//port no.
    //printf("%s\n",argv[1]);
    //printf("%d\n",*(unsigned int *) server->h_addr_list[0]);
    struct sockaddr_in sin;
    memset (&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = svrAddr; //printf("%d\n",svrAddr);
    sin.sin_port = htons(svrPort); //printf("%d\n",svrPort);

    if (argc != 3) {
        fprintf(stderr,"usage: %s <hostname> <port>\n", argv[0]);
        exit(0);
    }
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host as %s\n", argv[1]);
        exit(0);
    }
    setBaconIP();
    setBaconID();
    setBaconPort();
    setBaconStartUpTime();
    bacon2String();
    if (pthread_create(&thread1, NULL,CmdAgent, NULL)!=0)  //create thread to call CmdAgent.
      printf("error\n");

    BeaconSender(sin);

    pthread_join(thread1,NULL);
    close(sockfd);
    return 0;
}
//------------------------------------BaconSender----------------------------
void BeaconSender(struct sockaddr_in sin){//Send the following message to manager every minute using UDP datagram
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)//UDP send BACON
    {
        perror("socket");
        printf("Failed to create socket\n");
        abort ();
    }

    if (connect(sockfd, (struct sockaddr *) &sin, sizeof(sin)) < 0)
    {
        fprintf(stderr, "Cannot connect to server\n");
        abort();
    }
    //printf("connection succeed!\n plz type msg:");

    //--------------------------------SEND BECON--------------------------------

    //fgets(buf, BUFSIZE, bac); //stdin  <---------->  changed into bacon structure
    if(send_packets(baconString, BUFSIZE)==-1)
      printf("error in send_packets");

}

//---------------------------------Get IP addr---------------------------------
char* getIpaddr(void){
  int count=0;
  struct ifaddrs * ifAddrStruct=NULL;
     void * tmpAddrPtr=NULL;

     getifaddrs(&ifAddrStruct);

     while (ifAddrStruct!=NULL)
     {
         if (ifAddrStruct->ifa_addr->sa_family==AF_INET)
         {   // check it is IP4
             // is a valid IP4 Address
             tmpAddrPtr = &((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;
             static char addressBuffer[INET_ADDRSTRLEN];
             inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);

             if(count==1) return (char*)addressBuffer; //return en0 IPV4 address
             count+=1;
         }
         ifAddrStruct = ifAddrStruct->ifa_next;
     }
     return "abc";
}


//----------------------------Assign Bacon struct value------------------------
void setBaconID(){
  srand(time(NULL)); // randomize seed
  baconMsg.ID = rand()%5000; //no. within 5000 drive
}
void setBaconStartUpTime(){
    baconMsg.StartUpTime =(int)time(NULL);
}
void setBaconIP(){
  char* IPV4=getIpaddr();
  const char * split = ".";
  char * p;
  p = strtok (IPV4,split);
  int count=0;
  while(p!=NULL&&count<4) {
    baconMsg.IP[count]=p;
    //printf ("%s\n",p);
    p = strtok(NULL,split);
    count+=1;
  }
}
void setBaconPort(){
  srand(time(NULL)); // randomize seed
  baconMsg.CmdPort = rand()%5000 + 1024; //port no. between 1024-6024
}
//----------------------------- make BACON struct into char*------------------
void bacon2String(void) {
  // sprintf(idString, "%d", baconMsg.ID)
  // sprintf(timeString, "%d", baconMsg.CmdPort)
  // sprintf(cmdPortSting, "%d", baconMsg.StartUpTime)
  char* idString = malloc(16);
  char* timeString = malloc(16);
  char* cmdPortSting = malloc(16);
  snprintf(idString, 16, "%d", baconMsg.ID);//printf("%d\n", baconMsg.ID);
  snprintf(cmdPortSting, 16, "%d", baconMsg.CmdPort);//printf("%d\n", baconMsg.CmdPort);
  snprintf(timeString, 16, "%d", baconMsg.StartUpTime);//printf("%d\n", baconMsg.StartUpTime);
  // printf("IP:");
  // for (int i = 0; i < 4; i++) {
  //   printf("%s.\n", baconMsg.IP[i]);
  // }
  strcat(baconString,idString);strcat(baconString,",");strcat(baconString,timeString);strcat(baconString,",");
  strcat(baconString,cmdPortSting);
  strcat(baconString,",");
  for (int i=0; i<4; i++){
      strcat(baconString,baconMsg.IP[i]);
      if(i!=3) strcat(baconString,".");
  }
}

//----------------------------------CmdAgent: thread to receive cmd--------------------------------
void *CmdAgent(void *x){    //Receive and execute remote commands and send results back using TCP socket.
  struct utsname buffer;
  int svrSock; /* parent socket */
  int childfd; /* child socket */
  unsigned short svrPort = baconMsg.CmdPort; /* port to listen on 7777*/
  unsigned int clientlen; /* byte size of client's address */
  struct sockaddr_in serveraddr; /* server's addr */
  struct sockaddr_in clientaddr; /* client addr */
  struct hostent *hostp; /* client host info */
  char buf[BUFSIZE]; /* message buffer */
  char *hostaddrp; /* dotted decimal host addr string */
  int optval;
  long n; /* message byte size */

  optval = 1;
  setsockopt(svrSock, SOL_SOCKET, SO_REUSEADDR,
       (const void *)&optval , sizeof(int));
  //printf("jsdkfjklsjdkj\n");
  //printf("-----%d\n",svrPort);
  memset (&serveraddr, 0, sizeof (serveraddr));
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
  serveraddr.sin_port = htons(svrPort);


  svrSock = socket(AF_INET, SOCK_STREAM, 0);
  if (svrSock < 0)
      error("ERROR opening socket");
  if (bind(svrSock, (struct sockaddr *) &serveraddr,sizeof(serveraddr)) < 0)
      error("ERROR on binding");
  if (listen(svrSock, 5) < 0) /* allow 5 requests to queue up */
      error("ERROR on listen");

  clientlen = sizeof(clientaddr);
  while (1) {
      printf("listen to manager connection and continue to send UDP msg to manager\n");
      childfd = accept(svrSock, (struct sockaddr *) &clientaddr, &clientlen);
      //printf("aha\n");
      if (childfd < 0)
          error("ERROR on accept");
      printf("Manager already connected\n");
      /*
       * gethostbyaddr: determine who sent the message
       */
      hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr,
                            sizeof(clientaddr.sin_addr.s_addr), AF_INET);
      if (hostp == NULL)
          error("ERROR on gethostbyaddr");
      hostaddrp = inet_ntoa(clientaddr.sin_addr);
      if (hostaddrp == NULL)
          error("ERROR on inet_ntoa\n");
      printf("Agent server established connection with manager %s (%s)\n",
             hostp->h_name, hostaddrp);

      /*
       * read: read input string from the client
       */
      bzero(buf, BUFSIZE);
      n = read(childfd, buf, BUFSIZE);
      if (n < 0)
          error("ERROR reading from socket");

      printf("Receiving the command is : %s \n", buf);
      //printf("server received %ld bytes\n", n);

      char target[100];
      strncpy(target, buf, 10);
      target[10] = '\0';
      //printf("%s\n",target);
      if(strcmp("GetLocalOS", target) == 0){ //GetLocalOS
          //printf("send os info back\n");
          // if (uname(&buffer) != 0) {    //uname () works on mac / linux / FREEBSD   but windows
          //   perror("uname");
          //   exit(EXIT_FAILURE);
          // }
          //printf("system name = %s\n", buffer.sysname);
          //printf("node name   = %s\n", buffer.nodename);
          //printf("release     = %s\n", buffer.release);
          //printf("version     = %s\n", buffer.version);
          //printf("machine     = %s\n", buffer.machine);
          char OS[16];
          int valid = 0;
          GetLocalOS(OS,&valid);
          if(valid == 1){
            n = write(childfd, OS, 16);
            if (n < 0)
                error("ERROR writing to socket");
          }else{
            printf("fail to GetLocalOS info\n");
          }

      }else{//GetLocalTime()
        time_t rawtime;
        struct tm * timeinfo;
        time ( &rawtime );
        timeinfo = localtime ( &rawtime );
        char* time = malloc(60);
        char* hour = malloc(16);
        char* min = malloc(16);
        char* sec = malloc(16);
        snprintf(hour, 16, "%d", timeinfo->tm_hour);//printf("%d\n", baconMsg.ID);
        snprintf(min, 16, "%d", timeinfo->tm_min);//printf("%d\n", baconMsg.CmdPort);
        snprintf(sec, 16, "%d", timeinfo->tm_sec);//printf("%d\n", baconMsg.StartUpTime);
        //printf("%s\n",sec);
        strcat(time,hour);strcat(time,":");strcat(time,min);strcat(time,":");strcat(time,sec);
        strcat(time,"\0");
        n = write(childfd, time, 8);
         if (n < 0)
           error("ERROR writing to socket");
      }
      close(childfd);
    }//while
}//end CmdAgent

void GetLocalOS(char OS[16], int *valid){
  char* ret = strncpy(OS, "MacOS 10.12.2\0", 16);
  if(ret != NULL){
    *valid = 1;
  }else{
    *valid = 0;
  }
 }
void GetLocalTime(int *time, int *valid){
//no need to use such int pointer to pass value.
 }
