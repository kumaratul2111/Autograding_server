/* run using ./server <port> <thread_pool_size>*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/time.h>
#include <arpa/inet.h>

struct request{
  int request_id;
  int queue_index;
};

struct request autograding_request[1000];

int queue_size = 0;
int queue_front = 0;
int queue_rear = -1;

pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;

void error(char *msg) {
  perror(msg);
  exit(1);
}

void * read_file_and_send_result(void *arg)
{ 
  
  char buffer[1024];
  int request_id;
  
  while(1)
  {
    pthread_mutex_lock(&queue_mutex);
    while(queue_size == 0)
      pthread_cond_wait(&empty, &queue_mutex);
    request_id = autograding_request[queue_front].request_id;
    queue_size--;
    queue_front = (queue_front + 1) % 1000;
    pthread_mutex_unlock(&queue_mutex);
    char file_content[512] = {'0'};
    char new_content[512] = {'0'};

    printf("Start\t%d\t%ld\n", request_id, pthread_self());

    //Variables to store C file, compilation error, runtime error and output error.
    char intermediate_name[100]="";
    char c_file_name[100]="";
    char executable_file_name[100] = "";
    char compilation_file_name[100]="";
    char runtime_error_file_name[100]="";
    char output_file_name[100]="";
    char diff_file_name[100]="";
    char status_file_name[100]="";
    char thread_id[100] = "";
    char request_id_str[30]; 

    sprintf(request_id_str ,"%d", request_id);

    strcat(intermediate_name, request_id_str);
    strcat(c_file_name, strcat(intermediate_name, "_C_CODE.c"));
    intermediate_name[0] = '\0';
    strcat(intermediate_name, request_id_str);
    strcat(executable_file_name, strcat(intermediate_name, "_C_CODE"));
    intermediate_name[0] = '\0';
    strcat(intermediate_name, request_id_str);
    strcat(compilation_file_name, strcat(intermediate_name, "_COMPILATION_ERROR"));
    intermediate_name[0] = '\0';
    strcat(intermediate_name, request_id_str);
    strcat(runtime_error_file_name, strcat(intermediate_name, "_RUNTIME_ERROR"));
    intermediate_name[0] = '\0';
    strcat(intermediate_name, request_id_str);
    strcat(output_file_name, strcat(intermediate_name, "_OUTPUT.txt"));
    intermediate_name[0] = '\0';
    strcat(intermediate_name, request_id_str);
    strcat(diff_file_name, strcat(intermediate_name, "_diff.txt"));
    intermediate_name[0] = '\0';
    strcat(intermediate_name, request_id_str);
    strcat(status_file_name, strcat(intermediate_name, "_status.txt"));

    


    //printf("%s\n%s\n%s\n%s\n%s\n%s\n%s\n",c_file_name, executable_file_name, compilation_file_name, runtime_error_file_name, output_file_name, diff_file_name, status_file_name);

    //Various Commands that will be used
    char compilation_command[100] = "gcc ";
    char execute_command[100] = "./";
    char diff_command[100] = "diff actual_output.txt ";
    char remove_command[100] = "rm ";
    strcat(strcat(strcat(strcat(strcat(compilation_command, c_file_name), " -o "), executable_file_name)," 2>"),compilation_file_name);
    strcat(strcat(strcat(strcat(strcat(execute_command, executable_file_name), " 1>"), output_file_name), " 2>"), runtime_error_file_name);
    strcat(strcat(strcat(diff_command, output_file_name), " >"), diff_file_name);
    //printf("%s\n%s\n%s\n", compilation_command, execute_command, diff_command);

    sleep(15);
    memset(file_content, 0, 1024);
    memset(new_content, 0, 1024);
    FILE *file = fopen(status_file_name, "r");
    fgets(file_content, sizeof(file_content), file);
    memset(file_content, 0, 1024);
    int number;
    fscanf(file, "%d", &number);
    fclose(file);
    file = fopen(status_file_name, "w");
    fprintf(file, "WIP\n");
    fprintf(file, "%d\n", number);
    fclose(file);
    //Compile and Run
    char message[50] = {0};

    if(system(compilation_command))
    {
      FILE *file = fopen(status_file_name, "a");
      fprintf(file, "Compilation Error\n");
      memset(buffer, 0, 1024);
      int fdc = open(compilation_file_name, O_RDONLY);
      int cr = read(fdc, buffer, 1024);
      fwrite(buffer, 1, cr, file);
      close(fdc);
      fclose(file);
      strcat(strcat(strcat(remove_command, c_file_name), " "),compilation_file_name);
      system(remove_command);
    }
    else if(system(execute_command))
    {
      FILE *file = fopen(status_file_name, "a");
      fprintf(file, "Runtime Error\n");
      memset(buffer, 0, 1024);
      int fdr = open(runtime_error_file_name, O_RDONLY);
      int cr = read(fdr, buffer, 1024);
      fwrite(buffer, 1, cr, file);
      close(fdr);
      fclose(file);
      strcat(strcat(strcat(strcat(strcat(strcat(strcat(strcat(strcat(remove_command, c_file_name), " "),compilation_file_name), " "), executable_file_name), " "), runtime_error_file_name), " "), output_file_name);
      system(remove_command);
    }
    else if(system(diff_command))
    {
      FILE *file = fopen(status_file_name, "a");
      fprintf(file, "Output Error\n");
      memset(buffer, 0, 1024);
      int fdd = open(diff_file_name, O_RDONLY);
      int cr = read(fdd, buffer, 1024);
      fwrite(buffer, 1, cr, file);
      close(fdd);
      fclose(file);
      strcat(strcat(strcat(strcat(strcat(strcat(strcat(strcat(strcat(strcat(strcat(remove_command, c_file_name), " "),compilation_file_name), " "), executable_file_name), " "), runtime_error_file_name), " "), output_file_name), " "), diff_file_name);
      system(remove_command);
    }
    else
    {
      FILE *file = fopen(status_file_name, "a");
      fprintf(file, "Pass\n");
      strcat(strcat(strcat(strcat(strcat(strcat(strcat(strcat(strcat(strcat(strcat(remove_command, c_file_name), " "),compilation_file_name), " "), executable_file_name), " "), runtime_error_file_name), " "), output_file_name), " "), diff_file_name);
      fclose(file);
      system(remove_command);
    }
    sleep(10);

    memset(file_content, 0, 1024);
    memset(new_content, 0, 1024);
    int fds = open(status_file_name, O_RDONLY);
    int ch_read = read(fds, file_content, sizeof(file_content));
  
    strcat(new_content, "DONE\n");
    file_content[0] = ' ';
    file_content[1] = ' ';
    file_content[2] = ' ';
    
    strcat(new_content, file_content);
    
    close(fds);
    fds = open(status_file_name, O_WRONLY);

    write(fds, new_content, ch_read + 5);
    close(fds);
    printf("Done\t%d\t%ld\n", request_id, pthread_self());
  }
}

int main(int argc, char *argv[]) {
  int sockfd,  portno, newsockfd; 
  socklen_t clilen;
  struct sockaddr_in serv_addr, cli_addr;
  int n;
  int request_count = 0;
  char buffer[1024];
  if (argc < 3) {
    fprintf(stderr, "ERROR, no port provided\n");
    exit(1);
  }

  sockfd = socket(AF_INET, SOCK_STREAM, 0); 
  if (sockfd < 0)
    error("ERROR opening socket");
  bzero((char *)&serv_addr, sizeof(serv_addr)); // initialize serv_address bytes to all zeros
  serv_addr.sin_family = AF_INET; // Address Family of INTERNET
  serv_addr.sin_addr.s_addr = INADDR_ANY;  //Any IP address. 
  portno = atoi(argv[1]);
  serv_addr.sin_port = htons(portno);  
  if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    error("ERROR on binding");
  listen(sockfd, 1); 
  clilen = sizeof(cli_addr);  

  int number_of_threads = atoi(argv[2]);
  pthread_t thread[number_of_threads];

  //char flag = 'e';

  for(int i = 0; i < number_of_threads; i++)
  {
    thread[i] = pthread_create(&thread[i], NULL, &read_file_and_send_result, NULL);
    if(thread[i] != 0)
      printf("Thread could not be created\n");   
  }

  while(1)
  {
    char response[1024] = {'0'};
    printf("Waiting for a new connection\n");
    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
    if (newsockfd < 0)
      error("ERROR on accept");
    
    
    char req_count[30]="";
    char type_of_request[1] = {'0'};

    read(newsockfd, type_of_request, 1);


    if(type_of_request[0] == '0'){
      request_count++;
      
      char c_file_name[100]="";
      char status_file_name[100] = "";
      char intermediate_name[100]="";
      
      
      sprintf(req_count, "%d", request_count);
      strcat(intermediate_name, req_count);
      strcat(c_file_name, strcat(intermediate_name, "_C_CODE.c"));
      intermediate_name[0]='\0';
      strcat(intermediate_name, req_count);
      strcat(status_file_name, strcat(intermediate_name, "_status.txt"));
      
      memset(buffer, 0, 1024);
      int characters_read = read(newsockfd, buffer, 1024);
      int fd = open(c_file_name , O_WRONLY | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
      write(fd, buffer, characters_read);
      close(fd);
      // printf("The file has been read\n%s\n", buffer);
     

      pthread_mutex_lock(&queue_mutex);
      if (queue_size < 1000) {
          queue_size++;
          queue_rear = (queue_rear + 1) % 1000;
          autograding_request[queue_rear].request_id = request_count ;
          autograding_request[queue_rear].queue_index = queue_rear ;
          pthread_cond_signal(&empty);
      } else {
          printf("Queue is full. Connection dropped.\n");
          close(newsockfd);
      }
      pthread_mutex_unlock(&queue_mutex);

      FILE *file = fopen(status_file_name, "w");
      fprintf(file, "ACCEPTED\n");
      fprintf(file, "%d\n", queue_rear);
      fclose(file);

      memset(response, 0, 1024);
      strcat(response, "Your request has been accepted\nRequest ID : ");
      strcat(response, req_count);
      write(newsockfd, response, sizeof(response));
    }
    else{
      char request_id[50] = {'0'}; 
      int cr = read(newsockfd, request_id, 50);

      char status_file_name[100] = "";
      char intermediate_name[100]="";

      intermediate_name[0]='\0';
      strcat(intermediate_name, request_id);
      strcat(status_file_name, strcat(intermediate_name, "_status.txt"));

      FILE *file = fopen(status_file_name, "r");
      if (file == NULL) {
        write(newsockfd, "We do not have your request", 28);
      }
      memset(response, 0, 1024);
      int fds = open(status_file_name, O_RDONLY);
      int chr = read(fds, response, sizeof(response));
      close(fds);
      if(response[0] == 'W'){
        char file_content2[10] = {'0'};
        fgets(file_content2, sizeof(file_content2), file);
        memset(file_content2, 0, sizeof(file_content2));
        fgets(file_content2, sizeof(file_content2), file);

        int queue_position = atoi(file_content2);
        int position = queue_position - queue_front + 1;
        char position_str[10] = {'0'};
        sprintf(position_str, "%d", position);
        char temp_response[100] = "Your request is at queue position ";
        strcat(temp_response, position_str);
        strcat(temp_response, "\n");

        strcat(response, "\n");
        strcat(response, temp_response);

      }
      fclose(file);
      write(newsockfd, response, sizeof(response));
      
    }

  }
  return 0;
}