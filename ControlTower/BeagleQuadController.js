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

// Handle key bindings for quadcopter control
// http://www.cambiaresearch.com/articles/15/javascript-char-codes-key-codes
document.onkeydown = function(e) {
	e = e || window.event;
	switch (e.which || e.keyCode) {
		// Left arrow = yaw left
		case 37:
			// alert("Left pressed!");
			socket.emit("leftKeyPressed",10);
			break;

		// Up arrow = throttle up
		case 38:
			// alert("Up pressed!");
			socket.emit("upKeyPressed",20);
			break;

		// Right arrow = yaw right
		case 39:
			// alert("Right pressed!");
			socket.emit("rightKeyPressed",10);
			break;

		// Down arrow = throttle down
		case 40:
			// alert("Down pressed!");
			socket.emit("downKeyPressed",20);
			break;

		// W key = pitch down
		case 87:
			// alert("W pressed!");
			socket.emit("wKeyPressed",0);
			break;

		// A key = roll left
		case 65:
			// alert("A pressed!");
			socket.emit("aKeyPressed",0);
			break;

		// S key = pitch up
		case 83:
			// alert("S pressed!");
			socket.emit("sKeyPressed",0);
			break;

		// D key = roll right
		case 68:
			// alert("D pressed!");
			socket.emit("dKeyPressed",0);
			break;

		default:
			return;
	}

	// Prevents scrolling of the page (default behavior)
	e.preventDefault();
}