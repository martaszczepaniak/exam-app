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
      choices: [
        {name: 'Start exam.', value: 'startExam'}, 
        {name: 'See exam results.', value: 'examResults'},
      ],
    },
  ];

  return inquirer.prompt(choiceList);
}

function promptTeacherMenu() {
	const choiceList = [
		{
			type: 'list',
			name: 'choice',
			message: 'Pick action:',
			choices: [
        {name: 'Add new exam.', value: 'addExam'},
        {name: 'See existing exams.', value: 'seeExams'},
      ]
		}
	]

  return inquirer.prompt(choiceList);
}

function promptAddExam() {
  return inquirer
    .prompt([{type: 'input', name: 'name', message: 'Exam name:'}])
}

function promptExams() {
  let exam_names = ['lala', 'kaka'];
  const choiceList = [
    {
      type: 'list', 
      name: 'choice' , 
      message: 'Available exams:', 
      choices: exam_names,
    }
  ]
  return inquirer.prompt(choiceList);
}

module.exports = {
  promptLogIn,
  promptStudentMenu,
  promptTeacherMenu,
  promptAddExam,
  promptExams,
};
