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

const addExamPrompt = () =>
  inquirer.prompt([{ type: 'input', name: 'name', message: 'Exam name:' }]);


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
