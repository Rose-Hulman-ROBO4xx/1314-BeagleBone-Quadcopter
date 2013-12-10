// Boilerplate code for socket io stuff
var socket;
var firstconnect = true;

function connect() {
	if (firstconnect) {
  		socket = io.connect(null);
    	firstconnect = false;
 	} else {
    	socket.socket.reconnect();
  	}
}

function disconnect() {
	socket.disconnect();
}

connect();

/*
Our code
*/

// Callback on the client side for the test button
function testButtonClick() {
	console.log("Clicked the test button!")
	socket.emit("testButtonPress",0);
}
