const net = require('net');
const inquirer = require('inquirer');
const randomString = require('randomstring');
const _ = require('lodash');

let client = new net.Socket();

let messages = {};

function promptLogIn() {
	const questionList = [
		{type: 'input', name: 'login', message: 'Your login:'},
		{type: 'password', name: 'password', message: 'Your password:'}
	]

	inquirer
		.prompt(questionList)
		.then(credentials => logIn(credentials));
}

function promptStudentsChoice() {
	const choiceList = [
		{
			type: 'list', 
			name: 'choice' , 
			message: 'Pick action:', 
			choices: ['Start exam.', 'See exam results.']
		}
	]

	inquirer
		.prompt(choiceList)
		.then((answer) => (getExam()));
}

function promptTeachersChoice() {
	const choiceList = [
		{
			type: 'list', 
			name: 'choice' , 
			message: 'Pick action:', 
			choices: ['Add new exam.', 'See existing exams.']
		}
	]

	inquirer
		.prompt(choiceList)
		.then(({choice}) => {
			if(choice === choiceList[0].choices[0]) {
				promptAddExam();	
			} else {
				getExams();
			}
		});
}

function promptAddExam() {
	inquirer
		.prompt([{type: 'input', name: 'name', message: 'Exam name:'}])
		.then(answer => {
			addExam(answer)
		})
}

function promptAddQuestion(id) {
	const questionList = [
		{type: 'input', name: 'question', message: 'Input content of question and possible answers:'},
		{
			type: 'list', 
			name: 'correctAnswer', 
			message: 'Correct answer:', 
			choices: ['A', 'B', 'C']
		}
	]

	inquirer
		.prompt(questionList)
		.then(answer => {
			console.log(answer);
			addQuestion(answer, id);
		});
}

function promptNextQuestionChoice(id) {
	const choiceList = [
		{
			type: 'list', 
			name: 'choice' , 
			message: 'Add next question?', 
			choices: ['Yes.', 'No. Save Exam.']
		}
	]

	inquirer
		.prompt(choiceList)
		.then(({choice}) => {
			if(choice === choiceList[0].choices[0]) {
				promptAddQuestion(id);	
			} else {
				promptTeachersChoice();
			}
		})
}

function promptQuestion(question) {
	const choiceList = [
		{
			type: 'list', 
			name: 'choice' , 
			message: 'Your answer is:', 
			choices: ['A', 'B', 'C']
		}
	]
	console.log(`\n\n${question}`);
	inquirer.prompt(choiceList).then(
		({choice}) => sendMessage('answer', choice))
}

function onConnectionSuccess() {
	// console.log('Successfully connected to server!\n');
	promptLogIn();
}

function onReceiveData(jsonData) {
	console.log(jsonData.toString());

	const data = JSON.parse(jsonData);
	const {uuid, status, user_type} = data;
	const {action} = messages[uuid];

	if(status !== 'ok') console.log(`\n ${status} \n`);

	switch(action) {
		case 'logIn': {
			if (status === 'ok') {
				switch(user_type) {
					case 'student':
						promptStudentsChoice();
					break;
					case 'teacher':
						promptTeachersChoice();
					break;
				}
			} else {
				promptLogIn();
			}
			break;
		}
		case 'getExam': {
			if(status === 'ok') {
				const {question} = data;
				promptQuestion(question);
			}
			break;
		}
		case 'answer': {
			if(status === 'ok') {
				const {question, score} = data;
				if(score) {
					console.log(`\nYour score is: ${score}\n`); 
					promptStudentsChoice();
				} else { 
					promptQuestion(question); 
				}
			}
			break;		
		}
		case 'addQuestion': {
			if(status === 'ok') {
				const {id} = data;
				promptNextQuestionChoice(id);
				console.log('next')
			} else {
				console.log('It was 5th question. Exam was added.');	
			} 
			break;
		}
		case 'addExam': {
			if(status === 'ok') {
				const {id} = data;
				promptAddQuestion(id);
			}
		}
		case 'getExams': {
			if(status === 'ok') {
				let {exam_names} = data;
				exam_names = exam_names.map(name => name.trim()).filter(name => name);
				const choiceList = [
					{
						type: 'list', 
						name: 'exam' , 
						message: 'Check scores for exam:', 
						choices: exam_names,
					}
				]
				inquirer.prompt(choiceList);
			}
		}
	}
	delete(messages[uuid]);
}

const actions = {
	logIn: 0,
	getExam: 1,
	answer: 2,
	addQuestion: 3,
	addExam: 4,
	getExams: 5,
}

function sendMessage(action, payload = '') {	
	const UUID = randomString.generate(8);
	const message = `${actions[action]}${UUID}${payload}`;

	client.write(message);
	messages[UUID] = {action, message};
	console.log(`Sent message ${message}`);
}

function logIn({login, password}) {
	const payload = `${_.padEnd(login, 8)}${_.padEnd(password, 8)}`;
	sendMessage('logIn', payload);
}

function getExam() {
	sendMessage('getExam');
}

function addExam({name}) {
	const payload = _.padEnd(name, 16);
	sendMessage('addExam', payload);
}

function addQuestion({question, correctAnswer}, id) {
	const payload = `${id}${_.padEnd(question, 100)}${correctAnswer}`;
	sendMessage('addQuestion', payload);
}

function getExams() {
	sendMessage('getExams');
}

client.on('data', onReceiveData);

client.connect(6666, '127.0.0.1', onConnectionSuccess);