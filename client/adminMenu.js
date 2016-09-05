const co = require('co');
const inquirer = require('inquirer');
const { createGroup, getGroups, getStudents, addStudentsToGroup } = require('./gateway');

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
  co(function* () {
    const newGroup = yield inquirer.prompt([
      {
        type: 'input',
        name: 'groupName',
        message: 'Input group name:',
      }
    ]);
    yield createGroup(newGroup);
  });

const addStudentsPrompt = () =>
  co(function* () {
    const { groups } = yield getGroups();
    const { groupId } = yield inquirer.prompt([
      { type: 'list', name: 'groupId', message: 'Available groups:', 
        choices: groups.map(({ name, id }) => ({ name, value: id })),
      },
    ]);
    const { students } = yield getStudents();
    const { studentIds } = yield inquirer.prompt([
      {
        type: 'checkbox',
        message: 'Select students(Press space to pick)',
        name: 'studentIds',
        choices: students.map(({ id }) => ({ name: parseInt(id) })),
      }
    ]);
    yield addStudentsToGroup({studentIds, groupId});
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