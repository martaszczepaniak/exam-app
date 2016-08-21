const inquirer = require('inquirer');

function promptLogIn() {
  const questionList = [
    {type: 'input', name: 'login', message: 'Your login:'},
    {type: 'password', name: 'password', message: 'Your password:'}
  ];

  return inquirer.prompt(questionList);
};

function promptStudentMenu() {
  const choiceList = [
    {
      type: 'list',
      name: 'choice' ,
      message: 'Pick action:',
      choices: ['Start exam.', 'See exam results.']
    }
  ];

  return inquirer.prompt(choiceList);
}

function promptTeacherMenu() {
	const choiceList = [
		{
			type: 'list',
			name: 'choice',
			message: 'Pick action:',
			choices: ['Add new exam.', 'See existing exams.']
		}
	]

  return inquirer.prompt(choiceList);
}
module.exports = {
  promptLogIn,
  promptStudentMenu,
  promptTeacherMenu,
};
