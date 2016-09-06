const co = require('co');
const inquirer = require('inquirer');
const { getExam, getOpenUserExams, getQuestions, submitAnswers, getFinishedExams } = require('./gateway');


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
	inquirer.prompt([
    {
			type: 'list', 
			name: 'answer' , 
			message: 'Your answer is:', 
			choices: ['A', 'B', 'C'],
		}
  ]);

const yourExamsPrompt = () =>
  co(function* () {
    const { exams } = yield getOpenUserExams();
    if(exams.length) {
      const { examId } = yield inquirer.prompt([
        {
          type: 'list',
          name: 'examId',
          message: 'Your exams:',
          choices: exams.map((exam) => ({ name: exam.name, value: exam.id })),
        }
      ]);
      const { questions } = yield getQuestions(examId);
      const answers = [];
      for (let question in questions) {
        console.log(`Question ${question}: ${questions[question].content}`);
        const { answer } = yield promptQuestion();
        answers.push({questionId: questions[question].id, answer: answer})
      }

      yield submitAnswers(answers, examId);  
    } else {
      console.log('You have no exams!');
    }
  })
	
const seeExamResults = () =>
	co(function* () {
		const { exams } = yield getFinishedExams();
    exams.forEach((exam, index) => {
      console.log(`\n${index + 1}: ${exam.name}, Score: ${exam.score}\n`);
    })
	});

const studentMenu = () =>
  co(function* () {
    while (true) {
      const { choice } = yield studentMenuPrompt();
      yield {
        startExam: yourExamsPrompt,
        seeResults: seeExamResults,
      }[choice]();
    }
  }).catch((err) => {
    console.log(err);
    studentMenu();
  });

module.exports = { studentMenu };