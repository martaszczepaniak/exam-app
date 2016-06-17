const net = require('net');
let client = new net.Socket();

function onConnectionSuccess() {
	console.log('Successfully connected to server!');
	setTimeout(() => sendMessage('question1'), 500);
	setTimeout(() => sendMessage('question2'), 1000);
	setTimeout(() => sendMessage('elo'), 1500);
}

function onReceiveData(data) {
	switch(data.toString().substring(0,9)) {
	    case 'question1':
	    	console.log(`Received message ${data.toString()}\n`);
	        client.write('aok');
	        break;
	    case 'question2':
	    	console.log(`Received message ${data.toString()}\n`);
	        client.write('anothing');
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