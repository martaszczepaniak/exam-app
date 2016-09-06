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
#include <time.h>
#include <sys/wait.h>

#define BUFFER_SIZE 1000
#define MAXCLIENTS 20

struct User {
	char type[10];
	char login[100];
	char password[100];
};

struct Question {
	char content[200];
	char correct_answer[1];
	char exam_id[2];
};

struct Exam {
	char name[16];
};

struct Answer {
	char answer[1];
	int question_id;
};


struct User current_user;
int childCount = 0;

unsigned char buffer[BUFFER_SIZE];

void sig_child(int s) {
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

static int loginCallback(void* NotUsed, int argc, char **argv, char **azColName) {
   	struct User user;
   	sprintf(user.login, "%s", argv[0]);
   	sprintf(user.password, "%s", argv[1]);
   	sprintf(user.type, "%s", argv[2]);
   	current_user = user;

   	return 0;
}

json_object *getGroupsJSONArray;
static int getGroupsCallback(void* NotUsed, int argc, char **argv, char **azColName) {
	json_object *groupJSON = json_object_new_object();
	json_object *groupId = json_object_new_string(argv[0]);
	json_object *groupName = json_object_new_string(argv[1]);
	json_object_object_add(groupJSON, "id", groupId);
	json_object_object_add(groupJSON, "name", groupName);
	json_object_array_add(getGroupsJSONArray, groupJSON);
  return 0;
}

json_object *getStudentsJSONArray;
static int getStudentsCallback(void* NotUsed, int argc, char **argv, char **azColName) {
	json_object *studentJSON = json_object_new_object();
	json_object *studentId = json_object_new_string(argv[0]);
	json_object_object_add(studentJSON, "id", studentId);
	json_object_array_add(getStudentsJSONArray, studentJSON);
  return 0;
}

json_object *getExamsJSONArray;
static int getExamsCallback(void* NotUsed, int argc, char **argv, char **azColName) {
	json_object *examJSON = json_object_new_object();
	json_object *examId = json_object_new_string(argv[0]);
	json_object *examName = json_object_new_string(argv[1]);
	json_object_object_add(examJSON, "id", examId);
	json_object_object_add(examJSON, "name", examName);
	json_object_array_add(getExamsJSONArray, examJSON);
  return 0;
}

json_object *getFinishedExamsJSONArray;
static int getFinishedExamsCallback(void* NotUsed, int argc, char **argv, char **azColName) {
	json_object *examJSON = json_object_new_object();
	json_object *examId = json_object_new_string(argv[0]);
	json_object *examName = json_object_new_string(argv[1]);
	json_object *score = json_object_new_string(argv[2]);
	json_object_object_add(examJSON, "id", examId);
	json_object_object_add(examJSON, "name", examName);
	json_object_object_add(examJSON, "score", score);
	json_object_array_add(getFinishedExamsJSONArray, examJSON);
  return 0;
}

json_object *getQuestionsJSONArray;
static int getQuestionsCallback(void* NotUsed, int argc, char **argv, char **azColName) {
	json_object *questionJSON = json_object_new_object();
	json_object *questionId = json_object_new_string(argv[0]);
	json_object *content = json_object_new_string(argv[1]);
	json_object_object_add(questionJSON, "id", questionId);
	json_object_object_add(questionJSON, "content", content);
	json_object_array_add(getQuestionsJSONArray, questionJSON);
  return 0;
}

struct Answer correct_answers[100];
int correct_answers_count;
static int getCorrectAnswersCallback(void* NotUsed, int argc, char **argv, char **azColName) {
	struct Answer answer;
	answer.question_id = atoi(argv[0]);
	sprintf(answer.answer, "%s", argv[1]);
	correct_answers[correct_answers_count] = answer;
	++correct_answers_count;
  return 0;
}

struct UserExam {
	char user_login[100];
};

struct UserExam groupUserExams[100];
int groupUserExamsCount;

static int getGroupUsersCallback(void* NotUsed, int argc, char **argv, char **azColName) {
	struct UserExam user_exam;
	sprintf(user_exam.user_login, "%s", argv[0]);
	groupUserExams[groupUserExamsCount] = user_exam;
	groupUserExamsCount++;
  return 0;
}

static int nullCallback(void* NotUsed, int argc, char **argv, char **azColName) {
  return 0;
}

void mainHandler(connection) {
	//INITIALIZING BUFFER
	unsigned char response[BUFFER_SIZE];
	// READ MESSAGE
	int received_msg_length = read(connection, buffer, BUFFER_SIZE);

	sqlite3 *db;
	char *zErrMsg = 0;
	int db_connection;

	/* Open database */
	db_connection = sqlite3_open("exam-app.db", &db);

	if(db_connection) {
	fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
	} else {
		fprintf(stderr, "Opened database successfully\n");
	}

	char *sql;
	char query[200];

	while(received_msg_length > 0) {
		json_object * jobj = json_tokener_parse(buffer);
		char method[100];
		json_object_object_foreach(jobj, key, val) {
			if (strcmp(key, "method") == 0) {
				strncpy(method, json_object_get_string(val), 100);
			}
		}
		//SEND LOGIN
		if(strcmp(method, "sendLogin") == 0) {
			char login[100];
			char password[100];

			json_object_object_foreach(jobj, key1, val1) {
				if (strcmp(key1, "login") == 0) {
					strncpy(login, json_object_get_string(val1), 100);
				} else if(strcmp(key1, "password") == 0) {
					strncpy(password, json_object_get_string(val1), 100);
				}
			}

			sprintf(query, "SELECT * FROM Users WHERE login = %s;", login);
			sql = query;
			db_connection = sqlite3_exec(db, sql, loginCallback, 0, &zErrMsg);
			if(db_connection != SQLITE_OK) {
				fprintf(stderr, "SQL error: %s\n", zErrMsg);
				sqlite3_free(zErrMsg);
				json_object *statusNotOk = json_object_new_string("Incorrect Credentials");
				json_object * jobj = json_object_new_object();
				json_object_object_add(jobj,"status", statusNotOk);
				sprintf(response, "%s", json_object_to_json_string(jobj));
			} else if(strcmp(login, current_user.login) == 0 && strcmp(password, current_user.password) == 0) {
			  json_object *statusOk = json_object_new_string("ok");
			  json_object *userType = json_object_new_string(current_user.type);
				json_object * jobj = json_object_new_object();
				json_object_object_add(jobj,"status", statusOk);
				json_object_object_add(jobj,"userType", userType);
				sprintf(response, "%s", json_object_to_json_string(jobj));
			} else {
				json_object *statusNotOk = json_object_new_string("Incorrect Credentials");
				json_object * jobj = json_object_new_object();
				json_object_object_add(jobj,"status", statusNotOk);
				sprintf(response, "%s", json_object_to_json_string(jobj));
			}
		} else if(strcmp(method, "createExam") == 0) {
			char examName[100];
			struct Question questions[100];
			json_object_object_foreach(jobj, key2, val2) {
				if (strcmp(key2, "name") == 0) {
					strncpy(examName, json_object_get_string(val2), 100);
				} else if(strcmp(key2, "questions") == 0) {
					int i;
					json_object *jvalue;
					for (i = 0; i < json_object_array_length(val2); i++) {
						jvalue = json_object_array_get_idx(val2, i);
						json_object_object_foreach(jvalue, key3, val3) {
							if (strcmp(key3, "question") == 0) {
								strncpy(questions[i].content, json_object_get_string(val3), 200);
							} else if (strcmp(key3, "correctAnswer") == 0) {
								strncpy(questions[i].correct_answer, json_object_get_string(val3), 1);
							}
						}
					}
				}
			}
			sprintf(query, "INSERT INTO Exams(name, owner_id) VALUES ('%s', '%s'); ", examName, current_user.login);
			sql = query;
			db_connection = sqlite3_exec(db, sql, nullCallback, 0, &zErrMsg);
			sqlite3_int64 exam_id = sqlite3_last_insert_rowid(db);
			int sizeOfQuestionsArray = sizeof(questions) / sizeof(questions[0]);
			int i;
			for(i = 0; questions[i].content[0] != '\0'; i++) {
				char *sql;
				char query[200];
				sprintf(query, "INSERT INTO Questions(content, correct_answer, exam_id) VALUES (\'%s\', \'%s\', \'%d\');", questions[i].content, questions[i].correct_answer, (int)exam_id);
				sql = query;
				db_connection = sqlite3_exec(db, sql, nullCallback, 0, &zErrMsg);
			}
			json_object *statusOk = json_object_new_string("ok");
			json_object * jobj = json_object_new_object();
			json_object_object_add(jobj,"status", statusOk);
			sprintf(response, "%s", json_object_to_json_string(jobj));
		} else if(strcmp(method, "createGroup") == 0) {
			char groupName[100];
			json_object_object_foreach(jobj, key4, val4) {
				if(strcmp(key4, "groupName") == 0) strncpy(groupName, json_object_get_string(val4), 100);
			}
			sprintf(query, "INSERT INTO Groups(name) VALUES (\'%s\'); ", groupName);
			sql = query;
			db_connection = sqlite3_exec(db, sql, nullCallback, 0, &zErrMsg);
			sqlite3_int64 exam_id = sqlite3_last_insert_rowid(db);
			json_object *statusOk = json_object_new_string("ok");
			json_object * jobj = json_object_new_object();
			json_object_object_add(jobj,"status", statusOk);
			sprintf(response, "%s", json_object_to_json_string(jobj));
		} else if(strcmp(method, "getGroups") == 0) {
			getGroupsJSONArray = json_object_new_array();
			sprintf(query, "SELECT * FROM Groups;");
			sql = query;
			db_connection = sqlite3_exec(db, sql, getGroupsCallback, 0, &zErrMsg);
			json_object *statusOk = json_object_new_string("ok");
			json_object *jobj = json_object_new_object();
			json_object_object_add(jobj,"status", statusOk);
			json_object_object_add(jobj,"groups", getGroupsJSONArray);
			sprintf(response, "%s", json_object_to_json_string(jobj));
		} else if(strcmp(method, "getStudents") == 0) {
			getStudentsJSONArray = json_object_new_array();
			sprintf(query, "SELECT * FROM Users WHERE users.type = 'student';");
			sql = query;
			db_connection = sqlite3_exec(db, sql, getStudentsCallback, 0, &zErrMsg);
			json_object *statusOk = json_object_new_string("ok");
			json_object *jobj = json_object_new_object();
			json_object_object_add(jobj,"status", statusOk);
			json_object_object_add(jobj,"students", getStudentsJSONArray);
			sprintf(response, "%s", json_object_to_json_string(jobj));
		} else if(strcmp(method, "getExams") == 0) {
			getExamsJSONArray = json_object_new_array();
			sprintf(query, "SELECT * FROM Exams WHERE Exams.owner_id = '%s';", current_user.login);
			sql = query;
			db_connection = sqlite3_exec(db, sql, getExamsCallback, 0, &zErrMsg);
			json_object *statusOk = json_object_new_string("ok");
			json_object *jobj = json_object_new_object();
			json_object_object_add(jobj,"status", statusOk);
			json_object_object_add(jobj,"exams", getExamsJSONArray);
			sprintf(response, "%s", json_object_to_json_string(jobj));
		} else if(strcmp(method, "getOpenUserExams") == 0) {
			getExamsJSONArray = json_object_new_array();
			sprintf(query, "SELECT Exams.id, Exams.name FROM Exams INNER JOIN User_exams ON Exams.id = User_exams.exam_id WHERE User_exams.user_login = %s AND User_exams.valid_until > '%d' AND User_exams.score IS NULL;", current_user.login, (int)time(NULL));
			sql = query;
			db_connection = sqlite3_exec(db, sql, getExamsCallback, 0, &zErrMsg);
			json_object *statusOk = json_object_new_string("ok");
			json_object *jobj = json_object_new_object();
			json_object_object_add(jobj,"status", statusOk);
			json_object_object_add(jobj,"exams", getExamsJSONArray);
			sprintf(response, "%s", json_object_to_json_string(jobj));
		} else if(strcmp(method, "getQuestions") == 0) {
			char examId[100];
			struct Question questions[100];
			json_object_object_foreach(jobj, key7, val7) {
				if (strcmp(key7, "examId") == 0) {
					strncpy(examId, json_object_get_string(val7), 100);
					printf("\n%s\n", examId);
				}
			}
			sprintf(query, "SELECT * FROM Questions WHERE Questions.exam_id = '%s';", examId);
			sql = query;
			getQuestionsJSONArray = json_object_new_array();
			db_connection = sqlite3_exec(db, sql, getQuestionsCallback, 0, &zErrMsg);
			json_object *statusOk = json_object_new_string("ok");
			json_object *jobj = json_object_new_object();
			json_object_object_add(jobj,"status", statusOk);
			json_object_object_add(jobj,"questions", getQuestionsJSONArray);
			sprintf(response, "%s", json_object_to_json_string(jobj));
		} else if(strcmp(method, "submitAnswers") == 0) {
			int examId;
			int answersCount = 0;
			int userExamId;
			struct Answer answers[100];
			int score = 0;

			json_object_object_foreach(jobj, key8, val8) {
				if(strcmp(key8, "examId") == 0) {
					examId = json_object_get_int(val8);
				} else if(strcmp(key8, "answers") == 0) {
					int i;
					for (i = 0; i < json_object_array_length(val8); i++) {
						struct Answer answer;
						json_object *jvalue = json_object_array_get_idx(val8, i);
						json_object_object_foreach(jvalue, key9, val9) {
							if (strcmp(key9, "questionId") == 0) {
								answer.question_id = json_object_get_int(val9);
							} else if (strcmp(key9, "answer") == 0) {
								strncpy(answer.answer, json_object_get_string(val9), 1);
							}
						}
						answers[answersCount] = answer;
						answersCount++;
					}
				}
			}

			correct_answers_count = 0;
			sprintf(query, "SELECT Questions.id, Questions.correct_answer FROM Questions WHERE exam_id = '%d';", examId);
			sql = query;
			db_connection = sqlite3_exec(db, sql, getCorrectAnswersCallback, 0, &zErrMsg);
			int i = 0;
			for(;i < answersCount; ++i) {
				int j = 0;
				for(;j < correct_answers_count; ++j) {
					if(answers[i].question_id == correct_answers[j].question_id) {
						printf("\nSCORE! answer: %s, correct_answer: %s\n", answers[i].answer, correct_answers[j].answer);
						if(strcmp(answers[i].answer, correct_answers[j].answer) == 0) ++score;
					}
				} 
			}
			sprintf(query, "UPDATE User_exams SET score = %d WHERE User_exams.user_login = '%s' AND exam_id = '%d';", score, current_user.login, examId);
			sql = query;
			db_connection = sqlite3_exec(db, sql, nullCallback, 0, &zErrMsg);
			json_object *statusOk = json_object_new_string("ok");
			json_object * jobj = json_object_new_object();
			json_object_object_add(jobj,"status", statusOk);
			sprintf(response, "%s", json_object_to_json_string(jobj));
		} else if(strcmp(method, "getFinishedExams") == 0) {
			getFinishedExamsJSONArray = json_object_new_array();
			sprintf(query, "SELECT Exams.id, Exams.name, User_exams.score FROM Exams INNER JOIN User_exams ON Exams.id = User_exams.exam_id WHERE User_exams.user_login = '%s' AND User_exams.score IS NOT NULL;", current_user.login);
			sql = query;
			db_connection = sqlite3_exec(db, sql, getFinishedExamsCallback, 0, &zErrMsg);
			json_object *statusOk = json_object_new_string("ok");
			json_object *jobj = json_object_new_object();
			json_object_object_add(jobj,"status", statusOk);
			json_object_object_add(jobj,"exams", getFinishedExamsJSONArray);
			printf("\n%s\n", json_object_to_json_string(jobj));
			sprintf(response, "%s", json_object_to_json_string(jobj));
		} else if(strcmp(method, "shareExam") == 0) {
			int exam_id;
			int group_id;
			char valid_until[100];
			json_object_object_foreach(jobj, key6, val6) {
				if (strcmp(key6, "groupId") == 0) {
					group_id = json_object_get_int(val6);
				} else if (strcmp(key6, "examId") == 0) {
					exam_id = json_object_get_int(val6);
				} else if (strcmp(key6, "time") == 0) {
					sprintf(valid_until, "%s", json_object_get_string(val6));
				}
			}
			groupUserExamsCount = 0;
			sprintf(query, "SELECT * FROM UserGroups WHERE UserGroups.group_id = '%d';", group_id);
			sql = query;
			db_connection = sqlite3_exec(db, sql, getGroupUsersCallback, 0, &zErrMsg);
			int i;
			for(i = 0; i < groupUserExamsCount; ++i) {
				sprintf(query, "INSERT INTO User_exams(exam_id, user_login, valid_until) VALUES (%d, %s, '%s'); ", exam_id, groupUserExams[i].user_login, valid_until);
				sql = query;
				db_connection = sqlite3_exec(db, sql, nullCallback, 0, &zErrMsg);
			}
			json_object *statusOk = json_object_new_string("ok");
			json_object *jobj = json_object_new_object();
			json_object_object_add(jobj,"status", statusOk);
			sprintf(response, "%s", json_object_to_json_string(jobj));
		} else if(strcmp(method, "addStudentsToGroup") == 0) {
			int studentIds[100];
			int groupId;
			int studentsCount;
			json_object_object_foreach(jobj, key5, val5) {
				if (strcmp(key5, "studentIds") == 0) {
					int i;
					studentsCount = json_object_array_length(val5);
					for (i = 0; i < json_object_array_length(val5); i++) {
						json_object *jvalue = json_object_array_get_idx(val5, i);
						studentIds[i] = json_object_get_int(jvalue);
					}
				} else if(strcmp(key5, "groupId") == 0) {
					groupId = json_object_get_int(val5);
				}
			}
			int i;
			for(i = 0; i < studentsCount; ++i) {
				sprintf(query, "INSERT INTO UserGroups(user_id, group_id) VALUES(%d, %d)", studentIds[i], groupId);
				sql = query;
				db_connection = sqlite3_exec(db, sql, nullCallback, 0, &zErrMsg);
			}
			json_object *statusOk = json_object_new_string("ok");
			json_object *jobj = json_object_new_object();
			json_object_object_add(jobj,"status", statusOk);
			sprintf(response, "%s", json_object_to_json_string(jobj));
		} else {
			printf("%s\n", method);
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