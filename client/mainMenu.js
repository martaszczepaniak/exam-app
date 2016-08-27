const co = require('co');
const inquirer = require('inquirer');
const { teacherMenu } = require('./teacherMenu');

const sendLogIn = () => ({ userType: 'teacher' });

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
    }[userType]();
  }).catch((err) => {
    console.log(err, 'Incorrect credentials! Try again!');
    mainMenu();
  });

module.exports = { mainMenu };
