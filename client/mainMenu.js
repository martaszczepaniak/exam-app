const co = require('co');
const inquirer = require('inquirer');
const { teacherMenu } = require('./teacherMenu');
const { studentMenu } = require('./studentMenu');
const { adminMenu } = require('./adminMenu');

const sendLogIn = () => ({ userType: 'admin' });

const loginPrompt = () =>
  inquirer.prompt([
    { type: 'input', name: 'login', message: 'Your login:' },
    { type: 'password', name: 'password', message: 'Your password:' },
  ]);

const mainMenu = () =>
  co(function* () {
    const credentials = yield loginPrompt();
    const { userType } = yield sendLogIn(credentials);
    yield {
      teacher: teacherMenu,
      student: studentMenu,
      admin: adminMenu,
    }[userType]();
  }).catch((err) => {
    console.log(err, 'Incorrect credentials! Try again!');
    mainMenu();
  });

module.exports = { mainMenu };
