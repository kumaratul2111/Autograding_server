/* run client using: ./client localhost <server_port> <type_of_request> request_id|c_file sleeptime timeout */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/time.h>
#include <signal.h>


long int response_time, response_time_sum = 0, response_time_avg=0, total_time;
int successful_response = 0;
int timeout;
struct timeval Throughput_start, Throughput_end;
int timeout_status = 0;
int total_timeout = 0;


void error(char *msg) {
  perror(msg);
  exit(0);
}

void alarm_handler(int signum) {
  printf("Time out\n");
  timeout_status = 1 ;
  // printf("%d", signum);
  response_time = timeout * 1000000;
  response_time_sum += response_time ;
  total_timeout++;
  sleep(2);
}

int main(int argc, char *argv[]) {

  signal(SIGALRM, alarm_handler);
  timeout = atoi(argv[6]);
  int sockfd, portno, n;
  
  struct sockaddr_in serv_addr; //Socket address structure
  struct hostent *server; //return type of gethostbyname

  char buffer[1024] = {0}; //buffer for message
  char response[1024] = {0}; //response from server

  if (argc < 3) {
    fprintf(stderr, "usage %s hostname port\n", argv[0]);
    exit(0);
  }

  portno = atoi(argv[2]); // 2nd argument of the command is port number

  /* create socket, get sockfd handle */

  server = gethostbyname(argv[1]);
  //finds the IP address of a hostname. 
  //Address is returned in the 'h_addr' field of the hostend struct

  if (server == NULL) {
    fprintf(stderr, "ERROR, no such host\n");
    exit(0);
  }


  bzero((char *)&serv_addr, sizeof(serv_addr)); // set server address bytes to zero

  serv_addr.sin_family = AF_INET; // Address Family is IP

  bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,
        server->h_length);
  /*Copy server IP address being held in h_addr field of server variable
  to sin_addr.s_addr of serv_addr structure */

  //convert host order port number to network order
  serv_addr.sin_port = htons(portno);
  
  gettimeofday(&Throughput_start, NULL);
  char type_of_request[2];
  strcpy(type_of_request, argv[3]);

  // int l;
  // for(l=0; l<atoi(argv[4]); l++)
  // {
 
    
  sockfd = socket(AF_INET, SOCK_STREAM, 0); //create the half socket. 
  
  //AF_INET means Address Family of INTERNET. SOCK_STREAM creates TCP socket (as opposed to UDP socket)
  //printf("%d\n", getpid());
  //printf("%d\n", sockfd);
  
  if (sockfd < 0)
    error("ERROR opening socket");
  

  
  timeout_status = 0;
  alarm(atoi(argv[6]));
  if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    error("ERROR connecting");

  struct timeval start, end;
  gettimeofday(&start, NULL);
  
  //Sending type of request
  write(sockfd, type_of_request, 1);
  
  if(type_of_request[0] == '0'){
    int fd = open(argv[4], O_RDONLY);
    write(sockfd, buffer, read(fd, buffer, 1024));
    close(fd);
  }
  else{
    write(sockfd, argv[4], sizeof(argv[4]));
  }
  read(sockfd, response, 1024);
  printf("%s\n", response);
 

  alarm(0);
  //Reading local time
  gettimeofday(&end, NULL);
 
  response_time = ((end.tv_sec * 1000000) + end.tv_usec) - ((start.tv_sec * 1000000) + start.tv_usec);
  response_time_sum += response_time ;
  
  sleep(atoi(argv[5]));
  close(sockfd);

  gettimeofday(&Throughput_end, NULL);
  total_time = ((Throughput_end.tv_sec * 1000000) + Throughput_end.tv_usec) - ((Throughput_start.tv_sec * 1000000) + Throughput_start.tv_usec);

  printf("Total response time in microseconds=%ld\n", response_time_sum);
  return 0;  
}
