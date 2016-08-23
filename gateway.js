function sendLogIn(client, credentials) {
  return new Promise((resolve, reject) => {
  	client.write(JSON.stringify(credentials));
    if(credentials.login == 'michal') {
      resolve({status: 'ok', userType: 'student'});
    } else {
      reject({status: 'dupa'});
    }
  });
};

module.exports = {
  sendLogIn,
};
