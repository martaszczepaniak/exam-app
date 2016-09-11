const { mainMenu } = require('./mainMenu');
const { client } = require('./gateway');

client.connect(6666, '127.0.0.1', mainMenu);

process.on("SIGINT", () => {
  console.log("Loging out...");
  process.exit(0);
});