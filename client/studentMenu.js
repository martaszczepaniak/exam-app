const co = require('co');
const inquirer = require('inquirer');

const studentMenuPrompt = () =>
  inquirer.prompt([
    {
      type: 'list',
      name: 'choice',
      message: 'Pick action:',
      choices: [
        { name: 'Start exam.', value: 'startExam' },
        { name: 'See exam results.', value: 'seeResults' },
      ],
    },
  ]);

const promptQuestion = () =>
	co(function* () {
		console.log("dupa");
    const answer = yield inquirer.prompt([
      {
				type: 'list', 
				name: 'choice' , 
				message: 'Your answer is:', 
				choices: ['A', 'B', 'C'],
			}
    ]);
		return { answer };
  });

const promptQuestions = () =>
	co(function* () {
		const answers = [];
		const questions = ['a', 'b', 'c'];
		for (question in questions) {
			answers[question] = yield promptQuestion();
		}
		return { answers };
  });

const seeExamResults = () =>
	co(function* () {
		console.log("examResults...");
	});

const studentMenu = () =>
  co(function* () {
    while (true) {
      const { choice } = yield studentMenuPrompt();
      yield {
        startExam: promptQuestions,
      }[choice]();
    }
  }).catch((err) => {
    console.log(err);
    studentMenu();
  });

module.exports = { studentMenu };