// Boilerplate code for socket io stuff
"use strict";

var PRESSED = 0;
var UNPRESSED = 1;

var TRUE = 1;
var FALSE = 0;

var FLAGS = 4;
var YAW = 3;
var PITCH = 2;
var ROLL = 1;
var THROTTLE = 0;

var CONTROLLER_ROLL = 2;
var CONTROLLER_YAW = 0;
var CONTROLLER_PITCH = 3;
var CONTROLLER_THROTTLE = 1;
var CONTROLLER_STARTBUTTON = 9;

var CONTROLS_ACTIVE = FALSE;

var START_STATE = TRUE;
var HOVER_STATE = FALSE;

var buttonFlag = UNPRESSED;

var setupAxes = false;
var activeButtons = [0, 9];
var axesPreVals = new Array();

var socket;
var firstconnect = true;
var init_home=1;
var eventJoystickFlag = 0;
var controlStringArray = ["-32768", "0", "0", "0"];

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


//**** Event Handling ****//

//Adds an event to the events queue
function addEvent(obj, type, fn){
	if(obj.addEventListener){
		obj.addEventListener(type, fn, false);
	}
	else if(obj.attachEvent){
		obj["e"+type+fn] = fn;
		obj[type+fn] = function(){
			obj["e"+type+fn](window.event);
		}
		obj.attachEvent("on"+type, obj[type+fn]);
	}
}

//Tab switching functionality
function tabSwitch(newTab){
	if(newTab == "fly_tab"){
		document.getElementById(newTab).innerHTML = 'Flight!';
		CONTROLS_ACTIVE = 1;
	}
	
}


// Handle key bindings for quadcopter control
// http://www.cambiaresearch.com/articles/15/javascript-char-codes-key-codes
// TODO: tie these in with the update CSS
// 		could modularize with the controller CSS updating code
// TODO: remove magic numbers
// 		standardize JSON requesting
document.onkeydown = function(e) {
		e = e || window.event;
		switch (e.which || e.keyCode) {
			//case 32:
				//controlDataString(FLAG, 0);
				//break;
			//left arrow = yaw left
			case 37:
				// alert("Left pressed!");
				controlDataString(YAW, 32768);
				//socket.emit("keyPress", "yaw", 32768);
				break;

			// Up arrow = throttle up
			case 38:
				// alert("Up pressed!");
				controlDataString(THROTTLE, 32768);
				//socket.emit("keyPress","up", 32768);
				break;

			// Right arrow = yaw right
			case 39:
				// alert("Right pressed!");
				controlDataString(YAW, -32768);
				//socket.emit("keyPress", "yaw" , -32768);
				break;

			// Down arrow = throttle down
			case 40:
				// alert("Down pressed!");
				controlDataString(THROTTLE, -32768);
				//socket.emit("keyPress", "up", -32768);
				break;

			// W key = pitch down
			case 87:
				// alert("W pressed!");
				controlDataString(PITCH, 32768);
				//socket.emit("keyPress", "pitch", 32768);
				break;

			// A key = roll left
			case 65:
				// alert("A pressed!");
				controlDataString(ROLL, 32768);
				//socket.emit("keyPress", "roll", 32768);
				break;

			// S key = pitch up
			case 83:
				// alert("S pressed!");
				controlDataString(PITCH, -32768);
				//socket.emit("keyPress", "pitch", -32768);
				break;

			// D key = roll right
			case 68:
				controlDataString(ROLL, -32768);
				// alert("D pressed!");
				//socket.emit("keyPress", "roll", -32768);
				break;

			default:
				return;
		}	


	sendControlData();

	// Prevents scrolling of the page (default behavior)
	e.preventDefault();
}

document.onkeyup = function(e){
		e = e || window.event;
		switch (e.which || e.keyCode) {
			//case 32:
				//controlDataString(FLAG, 0);
				//break;
			//left arrow = yaw left
			case 37:
				// alert("Left pressed!");
				controlDataString(YAW, 0);
				//socket.emit("keyPress", "yaw", 32768);
				break;

			// Up arrow = throttle up
			case 38:
				// alert("Up pressed!");
				controlDataString(THROTTLE, 0);
				//socket.emit("keyPress","up", 32768);
				break;

			// Right arrow = yaw right
			case 39:
				// alert("Right pressed!");
				controlDataString(YAW, 0);
				//socket.emit("keyPress", "yaw" , -32768);
				break;

			// Down arrow = throttle down
			case 40:
				// alert("Down pressed!");
				controlDataString(THROTTLE, 0);
				//socket.emit("keyPress", "up", -32768);
				break;

			// W key = pitch down
			case 87:
				// alert("W pressed!");
				controlDataString(PITCH, 0);
				//socket.emit("keyPress", "pitch", 32768);
				break;

			// A key = roll left
			case 65:
				// alert("A pressed!");
				controlDataString(ROLL, 0);
				//socket.emit("keyPress", "roll", 32768);
				break;

			// S key = pitch up
			case 83:
				// alert("S pressed!");
				controlDataString(PITCH, 0);
				//socket.emit("keyPress", "pitch", -32768);
				break;

			// D key = roll right
			case 68:
				controlDataString(ROLL, 0);
				// alert("D pressed!");
				//socket.emit("keyPress", "roll", -32768);
				break;

			default:
				return;
		}	


	sendControlData();

	// Prevents scrolling of the page (default behavior)
	e.preventDefault();
}

var controllers = {};
var rAF = window.mozRequestAnimationFrame || window.webkitRequestAnimationFrame || window.requestAnimationFrame;

function connecthandler(e){
	addgamepad(e.gamepad);
}

function addgamepad(gamepad){
	if(gamepad.id.indexOf('Controller') < 0){
		return;
	}
	controllers[gamepad.index] = gamepad;
	var controllerView = document.createElement("div");
	controllerView.setAttribute("id", "controller" + gamepad.index);
	var controllerHeader = document.createElement("h1");
	controllerHeader.appendChild(document.createTextNode("User Game Controller: " + gamepad.id));
	controllerView.appendChild(controllerHeader);
	var buttons = document.createElement("div");
	buttons.className = "buttons";
	activeButtons.forEach(function(i){
		var button = document.createElement("span");
		button.className = "button";
		button.innerHTML = i;
		buttons.appendChild(button);
	});
	controllerView.appendChild(buttons);
	var axes = document.createElement("div");
	axes.className = "axes";
	//TODO: pretty this CSS to show pitch, yaw, roll
	for(var i=0; i<gamepad.axes.length; i++){
		var axis = document.createElement("progress");
		axis.className = "axis";
		axis.setAttribute("max", "2");
		axis.setAttribute("value", "1");
		axis.innerHTML = i;
		axes.appendChild(axis);
	}
	//TODO change the above to make it faster
	controllerView.appendChild(axes);
	var controller= document.getElementById('gamepads');
	controller.appendChild(controllerView);
	startController();
//	console.log("Started Controller");
}

function updateStatus(){
	var count = -1;

	//console.log("updating Status");
	if(navigator.webkitGetGamepads){
		scangamepads();
	}
	for(var gamepad in controllers){
		var controller = controllers[gamepad];
		var d = document.getElementById("controller" + gamepad);
		var buttons = d.getElementsByClassName("button");
		activeButtons.forEach(function(i){
			count++;
			var b = buttons[count];
			var val = controller.buttons[i];
			var pressed = val == 1.0;
			if(typeof(val) == "object"){
				pressed = val.pressed;
				val = val.value;
			}
			if(pressed){
//TODO: make the buttonFlag a toggle switch
//TODO: make a button wait till not pressed clause

					b.className = "button pressed";
					controllerButtonEvent(i, val);
					//socket.emit("controllerButtonEvent", i, val);
			}
			else{
				b.className = "button";
				controllerButtonEvent(i, val)
				//socket.emit("controllerButtonEvent", activeButtons[i], val);
			}
		});
		var axes = d.getElementsByClassName("axis");
		//TODO: change this to show the degree of roll, pitch, yaw...
//		if(CONTROLS_ACTIVE==TRUE){
		for(var i =0; i < controller.axes.length; i++){
			//TODO: do this better...
//			console.log("axes");
					var a = axes[i];
					a.innerHTML = i + ": " + controller.axes[i].toFixed(4);
					a.setAttribute("value", controller.axes[i] + 1);
					controllerAxesEvent(i, (a.getAttribute("value")-1)*-32768);
	}

	}
//	}
	sendControlData();
	console.log("exiting updateStatus");
}
		

function scangamepads(){
	var gamepads = navigator.webkitGetGamepads();
	for(var i =0; i < gamepads.length; i++){
		if(gamepads[i]){
			if(!(gamepads[i].index in controllers)){
				addgamepad(gamepads[i]);
			}
			else{
				controllers[gamepads[i].index] = gamepads[i];
			}
		}
	}
}

function disconnecthandler(e){
	removegamepad(e.gamepad);
}

function removegamepad(gamepad){
	var d = document.getElementById("controller" + gamepad.index);
	document.body.removeChild(d);
	delete controller[gamepad.index];
}


window.addEventListener("gamepadconnected", connecthandler);
window.addEventListener("gamepaddisconnected", disconnecthandler);
if(navigator.webkitGetGamepads){
	setInterval(scangamepads, 5000);
}

function controlDataString(index, value){
//	console.log(index, value);
	value = value.toString();
	switch(index){
		//TODO: implement flag control
//		case FLAG:
//			controlStringArray[FLAG] = parseInt(value);
//			break;
		case THROTTLE:
			controlStringArray[THROTTLE] = parseInt(value);
			break;
		case ROLL:
			controlStringArray[ROLL] = parseInt(value);
			break;
		case PITCH:
			controlStringArray[PITCH] = parseInt(value);
			break;
		case YAW:
			controlStringArray[YAW] = parseInt(value);
			break;
	}
}


function controllerButtonEvent(button, value){
	switch(button){
		//case 0:
		//	if(HOVER_STATE==TRUE){
		//		hover();		
		//		HOVER_STATE = FALSE;
		//	}
		//	else{
		//		CONTROLS_ACTIVE = TRUE;
		//		HOVER_STATE = TRUE;
		//	}
		//	break;
		//case 9:
		//	if(START_STATE==FALSE){
		//		land();
		//		START_STATE=TRUE;
		//	}
		//	else{
		//		CONTROLS_ACTIVE = TRUE;
		//		START_STATE=FALSE;
		//	}
		//	break;

		//case 6:
		//	controlDataString(THROTTLE, value);
		//	break;
		//case 7:
		//	controlDataString(THROTTLE, value);
		//	break;
		//
	}


}

//TODO implement different control methods
//	i.e. first person shooter, regular QC...etc
function controllerAxesEvent(axes, value){
//	console.log("hereAxes");
	switch(axes){
		case CONTROLLER_ROLL:
			controlDataString(ROLL, value);
			break;
		case CONTROLLER_PITCH:
			controlDataString(PITCH, value);
			break;
		case CONTROLLER_YAW:
			controlDataString(YAW, value);
			break;
		case CONTROLLER_THROTTLE:
			controlDataString(THROTTLE, value);
	}
}

function sendControlData(){
//	console.log("Sending Control Data");
	socket.emit("controlEvent", controlStringArray);
//	console.log("leaving sendControlData");
}

function startController(){
setInterval(updateStatus, 100);
}

function hover(){
	CONTROLS_ACTIVE = FALSE;
	controlDataString(ROLL, 0);
	controlDataString(PITCH, 0);
	controlDataString(YAW,0);
	sendControlData();
}

function land(){
	CONTROLS_ACTIVE = FALSE;
	var throttlePrev = controlStringArray[THROTTLE];
	while(throttlePrev > 1000){
		controlStringArray = [throttlePrev, "0", "0", "0"];
		sendControlData();
		throttlePrev = throttlePrev/2;
	}
}

