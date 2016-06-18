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
			choices: ['Add exam.', 'See students results.']
		}
	]

	inquirer
		.prompt(choiceList)
		.then((answer) => (promptAddQuestion()));
}

function promptAddQuestion() {
	const questionList = [
		{type: 'input', name: 'question', message: 'Input content of question and possible answers:'},
		{type: 'input', name: 'correctAnswer', message: 'Input correct answer(e.g. A):'}
	]

	inquirer
		.prompt(questionList)
		.then(answer => {
			console.log(answer);
			addExam(answer);
		});
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
	console.log(question);
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
	}
	delete(messages[uuid]);
}

const actions = {
	logIn: 0,
	getExam: 1,
	answer: 2,
	addExam: 3,
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

function addExam({question, correctAnswer}) {
	const payload = `${_.padEnd(question, 100)}${correctAnswer}`;
	sendMessage('addExam', payload);
}

client.on('data', onReceiveData);

client.connect(6666, '127.0.0.1', onConnectionSuccess);