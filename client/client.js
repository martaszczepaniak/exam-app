const { mainMenu } = require('./mainMenu');
const { client } = require('./gateway');

client.connect(6666, '127.0.0.1', mainMenu);