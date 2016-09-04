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
#include <json/json.h>

#define BUFFER_SIZE 1000
#define MAXCLIENTS 20

int childCount = 0;

/*struct Question {
	int id;
	char content[200];
	char* correct_answer;
	char exam_id[2];
};

struct Exam {
	char id[2];
	char answers[5];
	char name[16];
};

struct User {
	char type[10];
	char login[9];
	char password[9];
};

struct UserExams {
	char exam_id[2];
	char user_login[9];
	char score[5];
};

struct User users[10];

struct Exam exams[10];
//int exam_count = 0;
struct Exam user_exam_names[10];
int user_exams_index = 0;

struct UserExams user_exams[10];


struct Question questions[10];
struct Question question_answers[10];
int question_index = 0;
//int user_exams_count = 0;
int user_exam_questions = 0;*/


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

/*static int callback0(void *NotUsed, int argc, char **argv, char **azColName) {
	struct User user;
   	sprintf(user.login, "%s", argv[0]);
   	printf("%s\n", argv[0]);
   	sprintf(user.password, "%s", argv[1]);
   	printf("%s\n", argv[1]);
   	sprintf(user.type, "%s", argv[2]);
   	users[0] = user;
   	return 0;
}

static int callback4_1(void *NotUsed, int argc, char **argv, char **azColName){
   	return 0;
}

static int callback4_2(void *NotUsed, int argc, char **argv, char **azColName){
	struct Exam exam;
   	sprintf(exam.id, "%s", argv[0]);
  	printf("%s\n", argv[0]);
  	exams[0] = exam;
   	return 0;
}

static int callback1(void *NotUsed, int argc, char **argv, char **azColName) {
	struct UserExams user_exam;
	struct Question question;
	sprintf(question.content, "%s", argv[1]);
	sprintf(question.exam_id, "%s", argv[2]);
	question.id = atoi(argv[3]);
	sprintf(user_exam.exam_id, "%s", argv[2]);
	sprintf(user_exam.user_login, "%s", argv[0]);
	printf("%s\n", argv[0]);
	user_exams[0] = user_exam;
	questions[0] = question;
   	return 0;
}

static int callback6(void *NotUsed, int argc, char **argv, char **azColName){
	struct Exam exam;
   	sprintf(exam.id, "%s", argv[1]);
   	sprintf(exam.name, "%s", argv[2]);
   	user_exam_names[user_exams_index] = exam;
   	user_exams_index++;
  	printf("%s\n", argv[1]);
  	printf("%s\n", argv[1]);
   	return 0;
}

static int callback2_1(void *NotUsed, int argc, char **argv, char **azColName) {
	struct Question question;

	questions[question_index].correct_answer = argv[0];
	printf("atoi: %s\n", argv[0]);
	printf("qq: %c\n", *questions[question_index].correct_answer)	;
   	return 0;
}

static int callback2_2(void *NotUsed, int argc, char **argv, char **azColName){
	user_exam_questions = atoi(argv[0]);
	printf("%d\n", atoi(argv[0]));
   	return 0;
}

static int callback2_3(void *NotUsed, int argc, char **argv, char **azColName){
	struct Question question;
	question.id = atoi(argv[0]);
	sprintf(question.content, "%s", argv[1]);
	sprintf(question.exam_id, "%s", argv[2]);
	questions[question_index] = question;
   	return 0;
}*/

void mainHandler(connection) {
	//INITIALIZING BUFFER
	unsigned char response[BUFFER_SIZE];

	//int current_user = 0;
	

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

				break;
			}
			/*case '0': {

				char login[9];
				char password[9];

				memcpy(login, &buffer[9], 8);
				login[8] = '\0';

				memcpy(password, &buffer[17], 8);
				password[8] = '\0';

				char *sql;
				char s[100];
				sprintf(s, "SELECT * FROM Users WHERE login = %s;", login);
				sql = s;
				db_connection = sqlite3_exec(db, sql, callback0, 0, &zErrMsg);
			   	if( db_connection != SQLITE_OK ){
			   		fprintf(stderr, "SQL error: %s\n", zErrMsg);
			      	sqlite3_free(zErrMsg);
			   	}else{
			      	fprintf(stdout, "Table created successfully\n");
					if(strcmp(login, users[0].login) == 0 && 
						strcmp(password, users[0].password) == 0) {
						//current_user = user_index;
						sprintf(response, "{\"uuid\": \"%s\", \"status\": \"ok\", \"user_type\": \"%s\"}", 
							uuid, users[0].type);
						break;
					} else {
						sprintf(response, "{\"uuid\": \"%s\", \"status\": \"Incorrect credentials.\"}", uuid);
					}
			   	}
				break; 
			}
			case '1': {
				printf("case1");
				char *sql;
				char s[100];
				sprintf(s, "SELECT login, content, exam_id, min(id) FROM Questions, Users WHERE exam_id = 5 AND login = \"%s\";", users[0].login);
				sql = s;
				db_connection = sqlite3_exec(db, sql, callback1, 0, &zErrMsg);
				if(db_connection != SQLITE_OK) {
			   		fprintf(stderr, "SQL error: %s\n", zErrMsg);
			      	sqlite3_free(zErrMsg);
			   	} else {
			   		
				   	sprintf(response, "{\"uuid\": \"%s\", \"status\": \"ok\", \"question\": \"%s\"}", uuid, questions[question_index].content);
			   	}
				break;
			}
			case '2': {
				printf("qi: %d\n", question_index);
				printf("uq: %d\n", user_exam_questions);
				char answer = buffer[9];
				int score;
				char *sql;
				char s[100];
				sprintf(s, "SELECT correct_answer FROM Questions WHERE id = \"%d\";", questions[question_index].id);
				sql = s;
				db_connection = sqlite3_exec(db, sql, callback2_1, 0, &zErrMsg);
			   	if (db_connection != SQLITE_OK ) {
			   		fprintf(stderr, "SQL error: %s\n", zErrMsg);
			      	sqlite3_free(zErrMsg);
			   	} else {
			   		if(*questions[question_index].correct_answer == answer) {
			   			score++;
			   		}
			   		sprintf(s, "SELECT count(*) FROM Questions, Users WHERE exam_id = \"%s\" AND login = \"%s\";", 
			   			questions[question_index].exam_id, users[0].login);
			   		sql = s;
					db_connection = sqlite3_exec(db, sql, callback2_2, 0, &zErrMsg);
					printf("%d", question_index);
					printf("%d", user_exam_questions);
			   		if(question_index < user_exam_questions - 1) {
			   			sprintf(s, "SELECT id, content, exam_id FROM Questions WHERE id = \"%d\";", questions[question_index].id + 1);
						sql = s;
						question_index++;
						db_connection = sqlite3_exec(db, sql, callback2_3, 0, &zErrMsg);
					   	if (db_connection != SQLITE_OK ) {
					   		fprintf(stderr, "SQL error: %s\n", zErrMsg);
					      	sqlite3_free(zErrMsg);
					   	} else {
			   				sprintf(response, "{\"uuid\": \"%s\", \"status\": \"ok\", \"question\": \"%s\"}", 
			   				uuid, questions[question_index].content);
			   				printf("%s", questions[question_index].content);
			   			}
			   		} else {
						sprintf(response, "{\"uuid\": \"%s\", \"status\": \"ok\", \"score\": \"%d\"}", 
			   				uuid, score);
			   		}
			   	}

				break;
			}
			case '3': {
				int exam_id = buffer[9] - '0';
				char question_content[200];
				char correct_answer;
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
			      		sprintf(response, "{\"uuid\": \"%s\", \"status\": \"ok\", \"id\": \"%s\"}", uuid, exams[0].id);
			      	}
			   	}
				break;
			}
			case '6': {
				char *sql;
				char s[100];
				sprintf(s, "SELECT user_login, exam_id, name FROM User_Exams, Exams WHERE user_login = \"%s\" AND User_exams.exam_id = Exams.id AND score IS NULL OR score ='';", users[0].login);
				sql = s;
				db_connection = sqlite3_exec(db, sql, callback6, 0, &zErrMsg);
				if(db_connection != SQLITE_OK) {
			   		fprintf(stderr, "SQL error: %s\n", zErrMsg);
			      	sqlite3_free(zErrMsg);
			   	} else {
			   		char exam_names[800];

					sprintf(exam_names, "[\"%s\", \"%s\", \"%s\", \"%s\", \"%s\"]", user_exam_names[0].name, 
						user_exam_names[1].name ? user_exam_names[1].name : "0", user_exam_names[2].name ? user_exam_names[2].name : "0",
						user_exam_names[3].name ? user_exam_names[3].name : "0", user_exam_names[4].name ? user_exam_names[4].name : "0");
			   		sprintf(response, "{\"uuid\": \"%s\", \"status\": \"ok\", \"exam_names\": [ \"lalala\", \"kakaka\"], \"id\": 5 }", uuid);
			   	}
				break;
			}*/
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