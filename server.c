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

#define BUFFER_SIZE 1000

struct Question {
	char content[200];
	char correct_answer;
};

struct Exam {
	int current_question;
	struct Question questions[5];
	char answers[5];
};

struct User {
	char type[10];
	char login[9];
	char password[9];
	struct Exam exam;
};

struct User_exams {
	struct Exam exam;
	struct User user;
	int score;
};

struct User users[10];
struct User_exams user_exams;

unsigned char buffer[BUFFER_SIZE];

void start_server(int sock) {
	int one = 1;
	struct sockaddr_in svr_addr;
	if (sock < 0) err(1, "can't open socket");

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

char* create_response_with_question(char* response, char* uuid, char* question) {
	sprintf(response, 
		"{\"uuid\": \"%s\", \"status\": \"ok\", \"question\": \"%s\"}",
		uuid, question
	);
	return response;
}

struct Exam create_exam() {
	struct Question question1 = {"What color is a chalkboard? A - pink, B - green, C - yellow.", 'B'};
	struct Question question2 = {"What color is a dog? A - pink, B - green, C - yellow.", 'A'};
	struct Exam exam = {0};
	exam.questions[0] = question1;
	exam.questions[1] = question2;
	return exam;
}

void mainHandler(connection) {
	//INITIALIZING BUFFER
	unsigned char response[BUFFER_SIZE];

	int current_user = 0;
	struct Exam current_exam;

	// READ MESSAGE
	int received_msg_length = read(connection, buffer, BUFFER_SIZE);

	while(received_msg_length > 0) {
		char uuid[9];
		memcpy(uuid, &buffer[1], 8);
		uuid[8] = '\0';

		// ASSIGN HANDLER
		switch(buffer[0]) {
			case '0': {
				char login[9];
				char password[9];

				memcpy(login, &buffer[9], 8);
				login[8] = '\0';

				memcpy(password, &buffer[17], 8);
				password[8] = '\0';
				int user_index;
				for(user_index = 0; user_index < 10; user_index++) {
					if(strcmp(login, users[user_index].login) == 0 && 
						strcmp(password, users[user_index].password) == 0) {
						current_user = user_index;
						sprintf(response, "{\"uuid\": \"%s\", \"status\": \"ok\", \"user_type\": \"%s\"}", 
							uuid, users[user_index].type);
						break;
					} else {
						//printf("\n\n%s\n", users[1].login);
						sprintf(response, "{\"uuid\": \"%s\", \"status\": \"Incorrect credentials.\"}", uuid);
					}
				}
				break; 
			}
			case '1': {
				if(current_user != 0) {
					users[current_user].exam = user_exams.exam;
					current_exam = users[current_user].exam;

					create_response_with_question(response, uuid, 
						current_exam.questions[current_exam.current_question].content);
				} else {
					sprintf(response, "{\"uuid\": \"%s\", \"status\": \"Unauthorized access.\"}", uuid);
				}
				break;
			}
			case '2': {
				if(current_user != 0) {
					char answer = buffer[9];
					current_exam.answers[current_exam.current_question] = answer;
					printf("%d\n",  current_exam.current_question);
					current_exam.current_question++;

					if (current_exam.current_question < (sizeof(current_exam.questions)/sizeof(current_exam.questions[0]))) {
						create_response_with_question(response, uuid, 
						current_exam.questions[current_exam.current_question].content);
					} else {
						int score = 0;
						int question_index;
						for(question_index = 0; question_index < 5; question_index++) {
							if(current_exam.questions[question_index].correct_answer == current_exam.answers[question_index])
								score++;
						}
						sprintf(response, "{\"uuid\": \"%s\", \"status\": \"ok\", \"score\": \"%d\"}", uuid, score);
					}
				}
				break;
			}
			case '3': {
				struct Question question;

				memcpy(question.content, &buffer[9], 100);
				question.content[100] = '\0';

				question.correct_answer = buffer[109];
				int question_index;
				for(question_index = 0; question_index < 5; question_index++) {
					if(user_exams.exam.questions[question_index].content[0] == '\0') {
						strcpy(user_exams.exam.questions[question_index].content, question.content);
						user_exams.exam.questions[question_index].correct_answer = question.correct_answer;
						if(question_index < 4) {
							sprintf(response, "{\"uuid\": \"%s\", \"status\": \"ok\"}", uuid);
						} else {
							sprintf(response, "{\"uuid\": \"%s\", \"status\": \"not ok\"}", uuid);
						}
						break;
					}
				}
				break;
			}
			default:
				strcpy(response, "Incorrect action number");
		}

		// RESPOND
		write(connection, response, strlen(response));
		
		//RESET buffer
		memset(buffer, 0, strlen(buffer));
		memset(response, 0, strlen(response));
		
		// CHECK FOR NEW MESSAGES
		received_msg_length = read(connection, buffer, BUFFER_SIZE);
	}
}

int main() {
	struct User user0;

	strcpy(user0.type, "student");
	strcpy(user0.login, "login123");
	strcpy(user0.password, "password");

	struct User user1;

	strcpy(user1.type, "teacher");
	strcpy(user1.login, "teacher1");
	strcpy(user1.password, "password");

	users[1] = user0;
	users[2] = user1;

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