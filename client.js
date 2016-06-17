const net = require('net');
const readline = require('readline');

const rl = readline.createInterface({
  input: process.stdin,
  output: process.stdout
});

let client = new net.Socket();

function isJson(str) {
    try {
        JSON.parse(str);
    } catch (e) {
        return false;
    }
    return true;
}

function onConnectionSuccess() {
	console.log('Successfully connected to server!');
	setTimeout(() => sendMessage('question'), 500);
	//setTimeout(() => sendMessage('elo'), 3000);
}

function onReceiveData(data) {
	if (isJson(data)) {
		let json = JSON.parse(data);
		let JSONdata;
		for (var property in json) {
    		if (json.hasOwnProperty(property)) {
        		JSONdata = property;
        		break;
    		}
		}
		console.log(JSONdata);
		switch(JSONdata) {
			case 'question1':
				let  message = JSON.parse(data);
				let questionContent = message.question1.question;
				let questionPossibleAnswers = message.question1.possible_answers;
				let questionNumber = message.question1.number;
				if(questionNumber == 1) {
			    	console.log(`Received message ${data.toString()}\n`);
			    	console.log(questionContent);
			        rl.question( `${questionContent} ${questionPossibleAnswers}\n` , (answer) => {
			        	client.write(`a${answer}`);
		  				rl.close();
					});
				}
			break;
			case 'user':
			break;
		}
	} else {
	    console.log(`Received message ${data.toString()}\n`);
	}
}

function sendMessage(message) {
	console.log(`Sent message ${message}`);
	client.write(message);
}

client.on('data', onReceiveData);

client.connect(6666, '127.0.0.1', onConnectionSuccess);