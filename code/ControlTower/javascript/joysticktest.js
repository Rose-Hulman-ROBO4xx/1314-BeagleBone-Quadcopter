var joystick = new (require('joystick'))(0, 3500, 350); //install by "npm install joystick"

joystick.on('axis', joystick_logger);
var a1 = 0;
var a2 = 0;
var a3 = 0;
var a4 = 0;
function joystick_logger(message){
	if (message['number']==0){
		a1 = message['value']
	}
	if (message['number']==1){
		a2 = message['value']
	}
	if (message['number']==2){
		a3 = message['value']
	}
	if (message['number']==4){
		a4 = message['value']
	}
	console.log(a1+","+a2+","+a3+","+a4)
}
