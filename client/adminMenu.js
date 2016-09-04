const co = require('co');
const inquirer = require('inquirer');

const adminMenuPrompt = () =>
  inquirer.prompt([
    {
      type: 'list',
      name: 'choice',
      message: 'Pick action:',
      choices: [
        { name: 'Create students group.', value: 'createGroup' },
        { name: 'Add students to a group', value: 'addStudents' },
      ],
    },
  ]);

const createGroupPrompt = () =>
  inquirer.prompt([
    {
      type: 'input',
      name: 'group',
      message: 'Input group name:',
    }
  ]);

const addStudentsPrompt = () =>
  co(function* () {
    const students = yield inquirer.prompt([
      {
        type: 'checkbox',
        message: 'Select students(Press space to pick)',
        name: 'students',
        choices: ['bla', 'ble', 'buu'],
      }
    ]);
    return { students };
  });

const adminMenu = () =>
  co(function* () {
    while (true) {
      const { choice } = yield adminMenuPrompt();
      yield {
        createGroup: createGroupPrompt,
        addStudents: addStudentsPrompt,
      }[choice]();
    }
  }).catch((err) => {
    console.log(err);
    adminMenu();
  });

module.exports = { adminMenu };