/* run client using: ./client localhost <server_port> c_file numberofiteration sleeptime */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/time.h>

void error(char *msg) {
  perror(msg);
  exit(0);
}

int main(int argc, char *argv[]) {

  long int response_time, response_time_sum = 0, response_time_avg, total_time;
  int successful_response = 0;
  int sockfd, portno, n;
  
  struct sockaddr_in serv_addr;  
  struct hostent *server;  

  char buffer[1024] = {0};  
  if (argc < 3) {
    fprintf(stderr, "usage %s hostname port\n", argv[0]);
    exit(0);
  }
  portno = atoi(argv[2]);  
  server = gethostbyname(argv[1]);
  if (server == NULL) {
    fprintf(stderr, "ERROR, no such host\n");
    exit(0);
  }
  bzero((char *)&serv_addr, sizeof(serv_addr));  
  serv_addr.sin_family = AF_INET;  
  bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,
        server->h_length);
  serv_addr.sin_port = htons(portno);
  
  struct timeval Throughput_start, Throughput_end;
  gettimeofday(&Throughput_start, NULL);

    struct timeval start, end;
    gettimeofday(&start, NULL);

  int l;
  for(l=0; l<atoi(argv[4]); l++)
  {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);  
    if (sockfd < 0)
      error("ERROR opening socket");
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
      error("ERROR connecting");


   
    int fd = open(argv[3], O_RDONLY);
    write(sockfd, buffer, read(fd, buffer, 1024));
    close(fd);

    char message[5] = {0};
    read(sockfd, message, 5);
    if(!strcmp(message, "CERR"))
      printf("COMPILATION_ERROR\n");
    else if(!strcmp(message, "RERR"))
      printf("RUNTIME_ERROR\n");
    else if(!strcmp(message, "OERR"))
      printf("Output Error\n");
    else
      printf("PASS\n");

    if(strstr(message, "ERR"))
    {
      memset(buffer, 0, 1024);
      read(sockfd, buffer, 1024);
      printf("%s\n", buffer);
    }
    gettimeofday(&end, NULL);
    response_time = ((end.tv_sec * 1000000) + end.tv_usec) - ((start.tv_sec * 1000000) + start.tv_usec);
    response_time_sum += response_time ;
    successful_response++ ;
    sleep(atoi(argv[5]));
    close(sockfd);
  }
  gettimeofday(&Throughput_end, NULL);
  total_time = ((Throughput_end.tv_sec * 1000000) + Throughput_end.tv_usec) - ((Throughput_start.tv_sec * 1000000) + Throughput_start.tv_usec);

  printf("Successful responses=%d\n", successful_response);
  printf("Total response time in microseconds=%ld\n", response_time_sum);
  response_time_avg = response_time_sum/successful_response ;
  printf("Average response time in microseconds=%ld\n", response_time_avg);
  printf("Total time for completing the loop=%ld\n", total_time);
  float total_time_sec=total_time/1000000.0;
  printf("Throughput=%f\n", successful_response/total_time_sec);
  return 0;  
}
