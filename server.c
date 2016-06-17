#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <err.h>

#define BUFFER_SIZE 100

char buffer[BUFFER_SIZE];

struct Exam_question {
  char number[2];
  char question[100];
  char possible_answers[100];
  char correct_answer;
};

struct Exam {
  struct Exam_question Q1;
  struct Exam_question Q2;
  struct Exam_question Q3;
  char exam_json[500];
};


void start_server(int sock) {
  int one = 1;
  struct sockaddr_in svr_addr;
  if (sock < 0)
    err(1, "can't open socket");
 
  setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int));
 
  int port = 6666;
  svr_addr.sin_family = AF_INET;
  svr_addr.sin_addr.s_addr = INADDR_ANY;
  svr_addr.sin_port = htons(port);
 
  if (bind(sock, (struct sockaddr *) &svr_addr, sizeof(svr_addr)) == -1) {
    close(sock);
    err(1, "Can't bind");
  }
 
  listen(sock, 5);
}

char* create_json_string(struct Exam Ex) {
  char* json = malloc(500* sizeof(char));
  sprintf(json, "{ \"question%s\": { \"number\": \"%s\", \"question\": \"%s\", \"possible_answers\": \"%s\", \"correct_answer\": \"%c\" } }", 
    Ex.Q1.number, Ex.Q1.number, Ex.Q1.question, Ex.Q1.possible_answers, Ex.Q1.correct_answer);
  return json;
}

void mainHandler(connection) {

  struct Exam_question Question1;

  strcpy( Question1.number, "1");
  strcpy( Question1.question, "What color is a chalkboard?");
  strcpy( Question1.possible_answers, "A - pink, B - green, C - yellow.");
  Question1.correct_answer = 'B';

  struct Exam Exam1;
  Exam1.Q1 = Question1;
  strcpy( Exam1.exam_json, create_json_string(Exam1));
  printf("%s\n", Exam1.exam_json );
	char response[BUFFER_SIZE];
	char message[20];
	char answer[5];
	char result[BUFFER_SIZE];
  
  // READ MESSAGE
  int received_msg_length = read(connection, buffer, BUFFER_SIZE);

	while(received_msg_length > 0) {
  	printf("buffer %s\n", buffer);
  	printf("buffer length %d\n", received_msg_length);

    
    sprintf(message, "%s", buffer);
    
    // ASSIGN HANDLER
    if(strcmp(message, "question") == 0) {
      	strcpy(response, Exam1.exam_json);
      	printf("response1:%s\n", response);
    } else if(message[0] == 'a') {
    	strcpy(answer, message);
    	memmove (answer, answer + 1, strlen(answer));
    	printf("msg:%s\n", answer);
    	strcpy(response, answer);
      strcpy(result, answer);
      printf("RESULT:%s\n", result);
      if(result[0] == 'B') {
        printf("correct!");
      }
      printf("response3:%s\n", answer);
    } else {
    	strcpy(response, message);
    	printf("response4:%s\n", message);
    }
    // RESPOND
    memset(buffer, 0, strlen(buffer));
    memset(message, 0, strlen(message));
    write(connection, response, strlen(response));
    // CHECK FOR NEW MESSAGES
    received_msg_length = read(connection, buffer, BUFFER_SIZE);
  }
}

int main() {

  int client_socket_connection;
  struct sockaddr_in cli_addr;
  socklen_t sin_len = sizeof(cli_addr);
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  
  start_server(sock);

  while (1) {
    client_socket_connection = accept(sock, (struct sockaddr *) &cli_addr, &sin_len);
    printf("got connection\n");
    
    mainHandler(client_socket_connection);

    close(client_socket_connection);
  }
}