const net = require('net');
const moment = require('moment');

let ongoingRequest = {};

const onReceiveData = (data) => {
	const jsonData = JSON.parse(data);
	if(jsonData.status !== 'ok') {
		ongoingRequest.reject(jsonData);
	} else {
		ongoingRequest.resolve(jsonData);
	}
};

let client = new net.Socket();

client.on('error', function(err) {
  console.log("Connection refused. " + err);
});

client.on('data', onReceiveData);



const request = (method, payload = {}) => {
  return new Promise((resolve, reject) => {
  	ongoingRequest.resolve = resolve;
  	ongoingRequest.reject = reject;
    client.write(JSON.stringify(Object.assign(payload, { method })));
  }).then((response) => { return response });
};

const sendLogIn = (credentials) => request('sendLogin', credentials);
const addExam = (exam) => request('createExam', exam);
const createGroup = (group) => request('createGroup', group);
const getGroups = () => request('getGroups');
const getStudents = () => request('getStudents');
const addStudentsToGroup = (data) => request('addStudentsToGroup', data);
const getExams = () => request('getExams');
const shareExam = (examId, groupId, duration) => request('shareExam', {examId, groupId, time: parseInt(moment().add(duration, 'minutes').format('X'))});
const getOpenUserExams = () => request('getOpenUserExams');
const getQuestions = (examId) => request('getQuestions', {examId});
const submitAnswers = (answers, examId) => request('submitAnswers', {answers, examId});
const getFinishedExams = () => request('getFinishedExams');

module.exports = {
  sendLogIn,
  client,
  addExam,
  createGroup,
  getGroups,
  getStudents,
  addStudentsToGroup,
  getExams,
  shareExam,
  getOpenUserExams,
  getQuestions,
  submitAnswers,
  getFinishedExams,
};
