const co = require('co');
const inquirer = require('inquirer');
const { addExam, getExams, getGroups, shareExam } = require('./gateway');

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
        name: 'correctAnswer',
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
    yield addExam({name: examName, questions})
  });

const existingExamsPrompt = () =>
  co(function* () {
    const { exams } = yield getExams();

    return inquirer.prompt([
        {
          type: 'list', 
          name: 'examId' , 
          message: 'Pick an exam to share:', 
          choices: exams.map((exam) => ({ name: exam.name, value: exam.id })),
        }
    ]);
  });

const shareExamsPrompt = () =>
  co(function* () {
    const { examId } = yield existingExamsPrompt();
    const { groups } = yield getGroups();
    const { groupId, duration } = yield inquirer.prompt([
      { type: 'list', name: 'groupId', message: 'Pick a group to share to:', 
        choices: groups.map(({ name, id }) => ({ name, value: id })),
      },
      { type: 'input', name: 'duration', message: 'Exam duration in minutes:' }
    ]);
    yield shareExam(examId, groupId, duration);
  });

const resultsPrompt = () =>
  co(function* () {
    const pickedExam = yield existingExamsPrompt;
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
