const co = require('co');
const net = require('net');
const {promptLogIn, promptStudentMenu, promptTeacherMenu, promptAddExam, promptExams} = require('./prompts');

let client = new net.Socket();
let pendingReuest = {};

function write(message) {
  let request = new Promise((resolve, reject) => {
    pendingReuest.resolve = resolve;
    pendingReuest.reject = reject;

    client.write(message);
  }).catch((err) => console.log(err));
  
  return request;
}

function sendLogIn(client, credentials) {
  let request = write(JSON.stringify(credentials))
    .then((loginData) => loginData);
  // let request = {userType: 'student'};
  return request;
};

const studentMenu = () =>
  co(function *() {
    const {choice} = yield promptStudentMenu();
    console.log({choice});
    yield {
      startExam: promptExams,
      examResults: () => { throw 'not implemented yet' }, 
    }[choice]()
  }).catch((err) => {
    console.log(err);
    studentMenu();
  });

const teacherMenu = () =>
  co(function *() {
    while(true) {
      const {choice} = yield promptTeacherMenu();
      yield {
        addExam: promptAddExam,
        seeExams: () => { throw 'not implemented yet' },
      }[choice]()
    }
  }).catch((err) => {
    console.log(err);
    teacherMenu();
  });

const main = () =>
  co(function *() {
    const credentials = yield promptLogIn();
    const {userType} = yield sendLogIn(client, credentials);
    yield {
      student: studentMenu,
      teacher: teacherMenu,
    }[userType]()
  }).catch((err) => {
    console.log(err, "Incorrect credentials! Try again!");
    main();
  });


function onConnectionSuccess() {
  // console.log('Successfully connected to server!\n');
  main();
}

client.on('data', onReceiveData);

function onReceiveData(jsonData) {
  const data = JSON.parse(jsonData);
  jsonData.status === 'dupa' ? 
    pendingReuest.reject(jsonData) : 
    pendingReuest.resolve(jsonData);
}

client.connect(6666, '127.0.0.1', onConnectionSuccess);