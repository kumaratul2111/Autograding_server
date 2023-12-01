/* run using ./server <port> */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/time.h>
#include <arpa/inet.h>


void error(char *msg) {
  perror(msg);
  exit(1);
}

long total_service_time = 0;

void * read_file_and_send_result(void *arg)
{   

  struct timespec start_time;
  clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start_time);  
  
  char buffer[1024];
  int newsockfd = *(int *)(arg);
  printf("Start\t%d\t%ld\n", newsockfd, pthread_self());

   
  char intermediate_name[100]="";
  char c_file_name[100]="";
  char executable_file_name[100] = "";
  char compilation_file_name[100]="";
  char runtime_error_file_name[100]="";
  char output_file_name[100]="";
  char diff_file_name[100]="";
  char thread_id[100] = "";

  sprintf(thread_id,"%ld", pthread_self());

  strcat(intermediate_name, thread_id);
  strcat(c_file_name, strcat(intermediate_name, "_C_CODE.c"));
  intermediate_name[0] = '\0';
  strcat(intermediate_name, thread_id);
  strcat(executable_file_name, strcat(intermediate_name, "_C_CODE"));
  intermediate_name[0] = '\0';
  strcat(intermediate_name, thread_id);
  strcat(compilation_file_name, strcat(intermediate_name, "_COMPILATION_ERROR"));
  intermediate_name[0] = '\0';
  strcat(intermediate_name, thread_id);
  strcat(runtime_error_file_name, strcat(intermediate_name, "_RUNTIME_ERROR"));
  intermediate_name[0] = '\0';
  strcat(intermediate_name, thread_id);
  strcat(output_file_name, strcat(intermediate_name, "_OUTPUT.txt"));
  intermediate_name[0] = '\0';
  strcat(intermediate_name, thread_id);
  strcat(diff_file_name, strcat(intermediate_name, "_diff.txt"));


  //printf("%s\t%s\t%s\t%s\t%s\n",c_file_name, executable_file_name, compilation_file_name, runtime_error_file_name, output_file_name);

  //Various Commands that will be used
  char compilation_command[100] = "gcc ";
  char execute_command[100] = "./";
  char diff_command[100] = "diff actual_output.txt ";
  char remove_command[100] = "rm ";
  strcat(strcat(strcat(strcat(strcat(compilation_command, c_file_name), " -o "), executable_file_name)," 2>"),compilation_file_name);
  strcat(strcat(strcat(strcat(strcat(execute_command, executable_file_name), " 1>"), output_file_name), " 2>"), runtime_error_file_name);
  strcat(strcat(strcat(diff_command, output_file_name), " >"), diff_file_name);
  //printf("%s\n%s\n%s\n", compilation_command, execute_command, diff_command);

  //Reading C file from client
  memset(buffer, 0, 1024);
  int characters_read = read(newsockfd, buffer, 1024);
  int fd = open(c_file_name , O_WRONLY | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
  write(fd, buffer, characters_read);
  close(fd);
  printf("Mid\t%d\t%ld\n", newsockfd, pthread_self());
  //Compile and Run
  char message[5] = {0};

  if(characters_read == 0)
  {
    printf("File could not be read\n");
  }
  else if(system(compilation_command))
  {
    strcpy(message, "CERR");
    write(newsockfd, message, strlen(message)+1);
    memset(buffer, 0, 1024);
    int fdc = open("compilation_error", O_RDONLY);
    write(newsockfd, buffer, read(fdc, buffer, 1024));
    close(fdc);
    strcat(strcat(strcat(remove_command, c_file_name), " "),compilation_file_name);
    system(remove_command);
  }
  else if(system(execute_command))
  {
    strcpy(message, "RERR");
    write(newsockfd, message, strlen(message)+1);
    memset(buffer, 0, 1024);
    int fdr = open("runtime_error", O_RDONLY);
    write(newsockfd, buffer, read(fdr, buffer, 1024));
    close(fdr);
    strcat(strcat(strcat(strcat(strcat(strcat(strcat(strcat(strcat(remove_command, c_file_name), " "),compilation_file_name), " "), executable_file_name), " "), runtime_error_file_name), " "), output_file_name);
    system(remove_command);
  }
  else if(system(diff_command))
  {
    strcpy(message, "OERR");
    write(newsockfd, message, strlen(message)+1);
    memset(buffer, 0, 1024);
    int fdd = open("diff.txt", O_RDONLY);
    write(newsockfd, buffer, read(fdd, buffer, 1024));
    close(fdd);
    strcat(strcat(strcat(strcat(strcat(strcat(strcat(strcat(strcat(strcat(strcat(remove_command, c_file_name), " "),compilation_file_name), " "), executable_file_name), " "), runtime_error_file_name), " "), output_file_name), " "), diff_file_name);
    system(remove_command);
  }
  else
  {
    strcpy(message, "PASS");
    write(newsockfd, message, strlen(message)+1);
    strcat(strcat(strcat(strcat(strcat(strcat(strcat(strcat(strcat(strcat(strcat(remove_command, c_file_name), " "),compilation_file_name), " "), executable_file_name), " "), runtime_error_file_name), " "), output_file_name), " "), diff_file_name);
    system(remove_command);
  }

  struct timespec end_time;
  clock_gettime(CLOCK_THREAD_CPUTIME_ID, &end_time);

  long elapsed_seconds = end_time.tv_sec - start_time.tv_sec;
  long elapsed_nanoseconds = end_time.tv_nsec - start_time.tv_nsec;
  long elapsed_time = elapsed_seconds * 1000000000 + elapsed_nanoseconds;

  total_service_time += elapsed_time;

  printf("Done\t%d\t%ld\ttotal service time = %ld\n", newsockfd, pthread_self(), total_service_time);
  close(newsockfd);

}

int main(int argc, char *argv[]) {
  int sockfd,  portno, newsockfd[300] = {0}; 
  socklen_t clilen;
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
  listen(sockfd, 1); 
  clilen = sizeof(cli_addr);  
  int index = 0;
  while (1)
  {
    newsockfd[index] = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
    if (newsockfd[index] < 0)
      error("ERROR on accept");
    pthread_t t1 ;
    int rv = pthread_create(&t1, NULL, &read_file_and_send_result, (void *)&newsockfd[index]);
    if(rv != 0)
      printf("Thread could not be created\n");
    rv = pthread_detach(t1);
    index = (index+1)%300;
  }
  return 0;
}
