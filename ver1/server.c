// run using ./server <port> 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>

void error(char *msg) {
  perror(msg);
  exit(1);
}

int main(int argc, char *argv[]) {
  int sockfd, newsockfd, portno; 

  socklen_t clilen;  
  char buffer[1024];  
  struct sockaddr_in serv_addr, cli_addr;  
  int n;

  if (argc < 2) {
    fprintf(stderr, "ERROR, no port provided\n");
    exit(1);
  }

  sockfd = socket(AF_INET, SOCK_STREAM, 0); 
  if (sockfd < 0)
    error("ERROR opening socket");
  bzero((char *)&serv_addr, sizeof(serv_addr));  
  serv_addr.sin_family = AF_INET;  
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  portno = atoi(argv[1]);
  serv_addr.sin_port = htons(portno);   
  if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    error("ERROR on binding");
  listen(sockfd, 2);  
  clilen = sizeof(cli_addr);   
  while(1)
  {
    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
    if (newsockfd < 0)
      error("ERROR on accept");
    memset(buffer, 0, 1024);
    int characters_read = read(newsockfd, buffer, 1024);
    int fd = open("c_code.c", O_WRONLY | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
    write(fd, buffer, characters_read);
    close(fd);
    char message[5] = {0};
    if(system("gcc c_code.c -o c_code 2>compilation_error"))
    {
      strcpy(message, "CERR");
      write(newsockfd, message, strlen(message)+1);
      memset(buffer, 0, 1024);
      int fdc = open("compilation_error", O_RDONLY);
      write(newsockfd, buffer, read(fdc, buffer, 1024));
      close(fdc);
      system("rm c_code.c compilation_error");
    }
    else if(system("./c_code 1>output.txt 2>runtime_error"))
    {
      strcpy(message, "RERR");
      write(newsockfd, message, strlen(message)+1);
      memset(buffer, 0, 1024);
      int fdr = open("runtime_error", O_RDONLY);
      write(newsockfd, buffer, read(fdr, buffer, 1024));
      close(fdr);
      system("rm c_code.c c_code output.txt runtime_error compilation_error");
    }
    else if(system("diff output.txt actual_output.txt > diff.txt"))
    {
      strcpy(message, "OERR");
      write(newsockfd, message, strlen(message)+1);
      memset(buffer, 0, 1024);
      int fdd = open("diff.txt", O_RDONLY);
      write(newsockfd, buffer, read(fdd, buffer, 1024));
      close(fdd);
      system("rm c_code.c c_code output.txt runtime_error compilation_error diff.txt");
    }
    else
    {
      strcpy(message, "PASS");
      write(newsockfd, message, strlen(message)+1);
      system("rm c_code.c c_code output.txt runtime_error compilation_error diff.txt");
    }
    close(newsockfd);
  }
  return 0;
}
