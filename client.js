const net = require('net');
const readline = require('readline');

const rl = readline.createInterface({
  input: process.stdin,
  output: process.stdout
});

let client = new net.Socket();

function onConnectionSuccess() {
	console.log('Successfully connected to server!');
	setTimeout(() => sendMessage('question'), 500);
}

function onReceiveData(data) {
	let  message = JSON.parse(data);
	let questionContent = message.question1.question;
	let questionNumber = message.question1.number;
	switch(questionNumber) {
	    case '1':
	    	console.log(`Received message ${data.toString()}\n`);
	    	console.log(questionContent);
	        rl.question( questionContent, (answer) => {
	        	console.log(answer);
	        	//client.write('aB');
  				rl.close();
			});
	        break;
	    default:
	        console.log(`Received message ${data.toString()}\n`);
	    }
}

function sendMessage(message) {
	console.log(`Sent message ${message}`);
	client.write(message);
}

client.on('data', onReceiveData);

client.connect(6666, '127.0.0.1', onConnectionSuccess);