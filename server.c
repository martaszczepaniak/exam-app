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
#include <sqlite3.h>

#define BUFFER_SIZE 1000
#define MAXCLIENTS 20

int childCount = 0;

/*struct Question {
	char content[200];
	char correct_answer;
};

struct Exam {
	int current_question;
	struct Question questions[5];
	char answers[5];
	char name[16];
};

struct User {
	char type[10];
	char login[9];
	char password[9];
};

struct UserExams {
	struct Exam exam;
	struct User user;
	int score;
};

struct User users[10];

struct Exam exams[10];
int exam_count = 0;

struct UserExams user_exams[10];
int user_exams_count = 0;*/

char current_user_login[50];
char current_user_pass[50];
char current_user_type[20];
char exam_id[5];
char question_content[200];
char correct_answer;

unsigned char buffer[BUFFER_SIZE];


void sig_child(int s)
{
	while ( waitpid(-1, 0, WNOHANG) > 0 )
	childCount --;
}

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

static int callback0(void *NotUsed, int argc, char **argv, char **azColName){
   sprintf(current_user_login, "%s", argv[0]);
   printf("%s\n", argv[0]);
   sprintf(current_user_pass, "%s", argv[1]);
   printf("%s\n", argv[1]);
   sprintf(current_user_type, "%s", argv[2]);
   return 0;
}

static int callback4_1(void *NotUsed, int argc, char **argv, char **azColName){
   	return 0;
}

static int callback4_2(void *NotUsed, int argc, char **argv, char **azColName){
   	sprintf(exam_id, "%s", argv[0]);
  	printf("%s\n", argv[0]);
   	return 0;
}

static int callback1(void *NotUsed, int argc, char **argv, char **azColName) {
	sprintf(question_content, "%s", argv[0]);
	printf("%s\n", argv[0]);
   	return 0;
}

void mainHandler(connection) {
	//INITIALIZING BUFFER
	unsigned char response[BUFFER_SIZE];

	int current_user = 0;

	// READ MESSAGE
	int received_msg_length = read(connection, buffer, BUFFER_SIZE);

	sqlite3 *db;
	char *zErrMsg = 0;
	int db_connection;

	/* Open database */
	db_connection = sqlite3_open("exam-app.db", &db);

	if(db_connection){
	fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
	}else{
		fprintf(stderr, "Opened database successfully\n");
	}

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

				char *sql;
				char s[100];
				//sql = "SELECT * FROM Users WHERE login = 100;";
				sprintf(s, "SELECT * FROM Users WHERE login = %s;", login);
				sql = s;
				db_connection = sqlite3_exec(db, sql, callback0, 0, &zErrMsg);
			   	if( db_connection != SQLITE_OK ){
			   		fprintf(stderr, "SQL error: %s\n", zErrMsg);
			      	sqlite3_free(zErrMsg);
			   	}else{
			      	fprintf(stdout, "Table created successfully\n");
			      	if(strcmp(login, current_user_login) == 0 && 
						strcmp(password, current_user_pass) == 0) {
						sprintf(response, "{\"uuid\": \"%s\", \"status\": \"ok\", \"user_type\": \"%s\"}", 
							uuid, current_user_type);
						break;
					} else {
						printf("%s\n", current_user_login);
						printf("%s\n", current_user_pass);
						sprintf(response, "{\"uuid\": \"%s\", \"status\": \"Incorrect credentials.\"}", uuid);
					}
			   	}
				break; 
			}
			case '1': {
				int current_question = 1;
				char *sql;
				char s[100];
				//sql = "SELECT * FROM Users WHERE login = 100;";
				sprintf(s, "SELECT content FROM Questions WHERE exam_id = 5 AND id = %d;", current_question);
				sql = s;
				db_connection = sqlite3_exec(db, sql, callback1, 0, &zErrMsg);
				if(db_connection != SQLITE_OK) {
			   		fprintf(stderr, "SQL error: %s\n", zErrMsg);
			      	sqlite3_free(zErrMsg);
			   	} else {
			   		sprintf(response, "{\"uuid\": \"%s\", \"status\": \"ok\", \"question\": \"%s\"}", uuid, question_content);
			   		current_question++;
			   	}
				break;
			}
			case '3': {
				int exam_id = buffer[9] - '0';

				memcpy(question_content, &buffer[10], 100);
				question_content[100] = '\0';

				correct_answer = buffer[110];

				char *sql;
				char s[100];
				sprintf(s, "INSERT INTO Questions(content, correct_answer, exam_id) VALUES (\'%s\', \'%c\', \'%d\' );", question_content, correct_answer, exam_id);
				sql = s;
				db_connection = sqlite3_exec(db, sql, callback0, 0, &zErrMsg);
				if( db_connection != SQLITE_OK ){
			   		fprintf(stderr, "SQL error: %s\n", zErrMsg);
			      	sqlite3_free(zErrMsg);
			   	} else {
			      	fprintf(stdout, "Table created successfully\n");
			      	sprintf(response, "{\"uuid\": \"%s\", \"status\": \"ok\", \"id\": \"%d\"}", uuid, exam_id);
			   	}
				break;
			}
			case '4': {
				char exam_name[20];
				exam_name[16] = '\0';
				memcpy(exam_name, &buffer[9], 16);
				printf("%s\n", exam_name);
				char *sql;
				char *sql1;
				char sql_cmd1[100];
				char sql_cmd2[100];
				sprintf(sql_cmd1, "INSERT INTO Exams(name) VALUES (\'%s\');", exam_name);
				sql = sql_cmd1;
				db_connection = sqlite3_exec(db, sql, callback4_1, 0, &zErrMsg);
			   	if( db_connection != SQLITE_OK ) {
			   		fprintf(stderr, "SQL error: %s\n", zErrMsg);
			      	sqlite3_free(zErrMsg);
			   	} else {
			      	fprintf(stdout, "Table created successfully\n");
			      	sprintf(sql_cmd2, "SELECT id FROM Exams WHERE name = \"%s\";", exam_name);
					sql1 = sql_cmd2;
			      	db_connection = sqlite3_exec(db, sql1, callback4_2, 0, &zErrMsg);
				   	if( db_connection != SQLITE_OK ) {
				   		fprintf(stderr, "SQL error: %s\n", zErrMsg);
				      	sqlite3_free(zErrMsg);
				   	} else {
			      		sprintf(response, "{\"uuid\": \"%s\", \"status\": \"ok\", \"id\": \"%s\"}", uuid, exam_id);
			      	}
			   	}
				break;
			}
			// case '1': {
			// 	if(current_user != 0) {
			// 		users[current_user].exam = user_exams.exam;
			// 		current_exam = users[current_user].exam;

			// 		create_response_with_question(response, uuid, 
			// 			current_exam.questions[current_exam.current_question].content);
			// 	} else {
			// 		sprintf(response, "{\"uuid\": \"%s\", \"status\": \"Unauthorized access.\"}", uuid);
			// 	}
			// 	break;
			// }
			// case '2': {
			// 	if(current_user != 0) {
			// 		char answer = buffer[9];
			// 		current_exam.answers[current_exam.current_question] = answer;
			// 		printf("%d\n",  current_exam.current_question);
			// 		current_exam.current_question++;

			// 		if (current_exam.current_question < (sizeof(current_exam.questions)/sizeof(current_exam.questions[0]))) {
			// 			create_response_with_question(response, uuid, 
			// 			current_exam.questions[current_exam.current_question].content);
			// 		} else {
			// 			int score = 0;
			// 			int question_index;
			// 			for(question_index = 0; question_index < 5; question_index++) {
			// 				if(current_exam.questions[question_index].correct_answer == current_exam.answers[question_index])
			// 					score++;
			// 			}
			// 			sprintf(response, "{\"uuid\": \"%s\", \"status\": \"ok\", \"score\": \"%d\"}", uuid, score);
			// 		}
			// 	}
			// 	break;
			// }
			/*case '3': {
				struct Question question;
				int exam_id = buffer[9] - '0';

				memcpy(question.content, &buffer[10], 100);
				question.content[100] = '\0';

				question.correct_answer = buffer[110];

				int question_index;
				for(question_index = 0; question_index < 5; question_index++) {
					if(exams[exam_id].questions[question_index].content[0] == '\0') {
						if(question_index < 4) {
							exams[exam_id].questions[question_index] = question;
							sprintf(response, "{\"uuid\": \"%s\", \"status\": \"ok\", \"id\": \"%d\"}", uuid, exam_id);
						} else {
							sprintf(response, "{\"uuid\": \"%s\", \"status\": \"not ok\"}", uuid);
						}
						break;
					}
				}
				break;
			}
			case '4': {
				struct Exam new_exam;
				memcpy(new_exam.name, &buffer[9], 16);
				new_exam.name[16] = '\0';
				new_exam.current_question = 0;
				exams[exam_count] = new_exam;
				struct UserExams teacher_user_exams = {new_exam, users[current_user]};
				user_exams[user_exams_count] = teacher_user_exams;

				sprintf(response, "{\"uuid\": \"%s\", \"status\": \"ok\", \"id\": \"%d\"}", uuid, exam_count);
				exam_count++;
				user_exams_count++;
				break;
			}
			case '5': {
				struct Exam teacher_exams[10];
				int teacher_exams_count = 0;

				int i;
				for(i = 0; i < 10; i++) {
					if(strcmp(user_exams[i].user.login, users[current_user].login) == 0) {
						teacher_exams[teacher_exams_count] = user_exams[i].exam;
						teacher_exams_count++;
					}
				}

				char exam_names[800];

				sprintf(exam_names, "[\"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\"]", 
					teacher_exams[0].name, teacher_exams[1].name, teacher_exams[2].name, teacher_exams[3].name, teacher_exams[4].name,
					teacher_exams[5].name, teacher_exams[6].name, teacher_exams[7].name, teacher_exams[8].name, teacher_exams[9].name);

				sprintf(response, "{\"uuid\": \"%s\", \"status\": \"ok\", \"exam_names\": %s}", uuid, exam_names);
				break;
			}*/
			//default:
				//strcpy(response, "Incorrect action number");
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
	int client_socket_connection;
	struct sockaddr_in cli_addr;
	socklen_t sin_len = sizeof(cli_addr);
	int sock = socket(AF_INET, SOCK_STREAM, 0);

	start_server(sock);

	while (1) {
		while (childCount >= MAXCLIENTS)
			sleep(1);
		client_socket_connection = accept(sock, (struct sockaddr *) &cli_addr, &sin_len);
		printf("got connection\n");

		if (client_socket_connection > 0) {
			int pid;
			if ((pid=fork()) == 0) {
				close(sock);
				mainHandler(client_socket_connection);
				childCount --;
			}
			else if (pid > 0) {
				childCount ++;
				close(client_socket_connection);
			}
			else
				perror("fork");
		}
  	}
}