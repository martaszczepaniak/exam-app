const co = require('co');
const inquirer = require('inquirer');

const teacherMenuPrompt = () =>
  inquirer.prompt([
    {
      type: 'list',
      name: 'choice',
      message: 'Pick action:',
      choices: [
        { name: 'Add new exam.', value: 'addExam' },
        { name: 'See existing exams.', value: 'seeExams' },
      ],
    },
  ]);

const addQuestionPrompt = () =>
  inquirer
    .prompt([
      {
        type: 'input',
        name: 'question',
        message: 'Input content of question and possible answers:',
      },
      {
        type: 'list',
        name: 'correctAnwer',
        message: 'Correct answer:',
        choices: [
          { name: 'A', value: 'A' },
          { name: 'B', value: 'B' },
          { name: 'C', value: 'C' },
        ],
      },
    ]);

const addExamPrompt = () =>
  co(function* () {
    const { examName } = yield inquirer.prompt([
      { type: 'input', name: 'examName', message: 'Exam name:' },
    ]);
    let addNextQuestion = true;
    const questions = [];

    while (addNextQuestion) {
      const question = yield addQuestionPrompt();
      questions.push(question);
      const { nextQuestion } = yield inquirer.prompt([
        { type: 'confirm', name: 'nextQuestion', message: 'Add another question?' },
      ]);
      addNextQuestion = nextQuestion;
    }

    return { examName, questions };
  });

const teacherMenu = () =>
  co(function* () {
    while (true) {
      const { choice } = yield teacherMenuPrompt();
      yield {
        addExam: addExamPrompt,
      }[choice]();
    }
  }).catch((err) => {
    console.log(err);
    teacherMenu();
  });

module.exports = { teacherMenu };
