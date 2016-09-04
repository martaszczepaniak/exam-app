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
        { name: 'Share an exam.', value: 'shareExam' },
        { name: 'View students results.', value: 'seeResults' },
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

const existingExamsPrompt = () =>
  inquirer.prompt([
      {
        type: 'list', 
        name: 'choice' , 
        message: 'Pick an exam to share:', 
        choices: ['exam1', 'exam2', 'exam3'],
      }
  ]);

const shareExamsPrompt = () =>
  co(function* () {
    const pickedExam = yield existingExamsPrompt();

    const pickedGroup = yield inquirer.prompt([
      {
        type: 'list', 
        name: 'choice' , 
        message: 'Pick a group to share:', 
        choices: ['group1', 'group2', 'group3'],
      }
    ]);

    return { pickedExam, pickedGroup };
  });

const resultsPrompt = () =>
  co(function* () {
    const studentResults = [1,2,3];
    const pickedExam = yield existingExamsPrompt;
    console.log(studentResults);
    return { pickedExam };
  });

const teacherMenu = () =>
  co(function* () {
    while (true) {
      const { choice } = yield teacherMenuPrompt();
      yield {
        addExam: addExamPrompt,
        shareExam: shareExamsPrompt,
        seeResults: resultsPrompt,
      }[choice]();
    }
  }).catch((err) => {
    console.log(err);
    teacherMenu();
  });

module.exports = { teacherMenu };
