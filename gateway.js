function sendLogIn(credentials) {
  return new Promise((resolve, reject) => {
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
