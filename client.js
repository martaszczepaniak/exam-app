const co = require('co');
const {promptLogIn, promptStudentMenu, promptTeacherMenu} = require('./prompts');
const {sendLogIn} = require('./gateway');

const menu = (userType) =>
  co(function *() {
    yield {
      student: promptStudentMenu,
      teacher: promptTeacherMenu,
    }[userType]();
  }).catch((err) => {
    console.log(err);
    menu();
  });

const main = () =>
  co(function *() {
    const credentials = yield promptLogIn();
    const {userType} = yield sendLogIn(credentials);
    yield menu(userType);
  }).catch((err) => {
    console.log("Incorrect credentials! Try again!");
    main();
  });

main();
