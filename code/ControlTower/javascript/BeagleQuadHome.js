// Boilerplate code for socket io stuff
"use strict";

var PRESSED = 0;
var UNPRESSED = 1;

var FLAGS = 4;
var YAW = 3;
var PITCH = 2;
var ROLL = 1;
var THROTTLE = 0;

var buttonFlag = UNPRESSED;

var setupAxes = false;
var activeButtons = [0];
var axesPreVals = new Array();

var socket;
var firstconnect = true;
var init_home=1;
var eventJoystickFlag = 0;
var controlStringArray = [0, 0, 0, 0, 0];

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
				controlDataString(YAW, 16384);
				//socket.emit("keyPress", "yaw", 16384);
				break;

			// Up arrow = throttle up
			case 38:
				// alert("Up pressed!");
				controlDataString(THROTTLE, 16384);
				//socket.emit("keyPress","up", 16384);
				break;

			// Right arrow = yaw right
			case 39:
				// alert("Right pressed!");
				controlDataString(YAW, -16384);
				//socket.emit("keyPress", "yaw" , -16384);
				break;

			// Down arrow = throttle down
			case 40:
				// alert("Down pressed!");
				controlDataString(THROTTLE, -16384);
				//socket.emit("keyPress", "up", -16384);
				break;

			// W key = pitch down
			case 87:
				// alert("W pressed!");
				controlDataString(PITCH, 16384);
				//socket.emit("keyPress", "pitch", 16384);
				break;

			// A key = roll left
			case 65:
				// alert("A pressed!");
				controlDataString(ROLL, 16384);
				//socket.emit("keyPress", "roll", 16384);
				break;

			// S key = pitch up
			case 83:
				// alert("S pressed!");
				controlDataString(PITCH, -16384);
				//socket.emit("keyPress", "pitch", -16384);
				break;

			// D key = roll right
			case 68:
				controlDataString(ROLL, -16384);
				// alert("D pressed!");
				//socket.emit("keyPress", "roll", -16384);
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
				//socket.emit("keyPress", "yaw", 16384);
				break;

			// Up arrow = throttle up
			case 38:
				// alert("Up pressed!");
				controlDataString(THROTTLE, 0);
				//socket.emit("keyPress","up", 16384);
				break;

			// Right arrow = yaw right
			case 39:
				// alert("Right pressed!");
				controlDataString(YAW, 0);
				//socket.emit("keyPress", "yaw" , -16384);
				break;

			// Down arrow = throttle down
			case 40:
				// alert("Down pressed!");
				controlDataString(THROTTLE, 0);
				//socket.emit("keyPress", "up", -16384);
				break;

			// W key = pitch down
			case 87:
				// alert("W pressed!");
				controlDataString(PITCH, 0);
				//socket.emit("keyPress", "pitch", 16384);
				break;

			// A key = roll left
			case 65:
				// alert("A pressed!");
				controlDataString(ROLL, 0);
				//socket.emit("keyPress", "roll", 16384);
				break;

			// S key = pitch up
			case 83:
				// alert("S pressed!");
				controlDataString(PITCH, 0);
				//socket.emit("keyPress", "pitch", -16384);
				break;

			// D key = roll right
			case 68:
				controlDataString(ROLL, 0);
				// alert("D pressed!");
				//socket.emit("keyPress", "roll", -16384);
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
	controllers[gamepad.index] = gamepad;
	var controllerView = document.createElement("div");
	controllerView.setAttribute("id", "controller" + gamepad.index);
	var controllerHeader = document.createElement("h1");
	controllerHeader.appendChild(document.createTextNode("User Game Controller: " + gamepad.id));
	controllerView.appendChild(controllerHeader);
	var buttons = document.createElement("div");
	buttons.className = "buttons";
	for(var i in activeButtons){
		var button = document.createElement("span");
		button.className = "button";
		button.innerHTML = i;
		buttons.appendChild(button);
	}
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
}

function updateStatus(){

	console.log("here");
	if(navigator.webkitGetGamepads){
		scangamepads();
	}
	for(var gamepad in controllers){
		var controller = controllers[gamepad];
		var d = document.getElementById("controller" + gamepad);
		var buttons = d.getElementsByClassName("button");
		for(var i in activeButtons){
			console.log("buttons");
			var b = buttons[i];
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
				//socket.emit("controllerButtonEvent", i, val);
			}
		}
		var axes = d.getElementsByClassName("axis");
		//TODO: change this to show the degree of roll, pitch, yaw...
		for(var i =0; i < controller.axes.length; i++){
			//TODO: do this better...
			console.log("axes");
			if(setupAxes == false){
				setupAxes = true;
				for(var i = 0; i < controller.axes.length; i++){
					axesPreVals[i] = 0;
				}
			}
			if(Math.abs(axesPreVals[i] - controller.axes[i]+1)> 600){
					var a = axes[i];
					a.innerHTML = i + ": " + controller.axes[i].toFixed(4);
					a.setAttribute("value", controller.axes[i] + 1);
					controllerAxesEvent(i, (a.getAttribute("value")-1)*32768);
					//socket.emit("controllerAxesEvent", i, (a.getAttribute("value")-1)*32768);
					axesPreVals[i] = controller.axes[i]+1;
			}
	}

	}
	sendControlData();
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
	console.log(index, value);
	switch(index){
		//TODO: implement flag control
//		case FLAG:
//			controlStringArray[FLAG] = value;
//			break;
		case YAW:
			controlStringArray[YAW] = value;
			break;
		case PITCH:
			controlStringArray[PITCH] = value;
			break;
		case ROLL:
			controlStringArray[ROLL] = value;
			break;
		case THROTTLE:
			controlStringArray[THROTTLE] = value;
			break;
	}
}


function controllerButtonEvent(button, value){
	switch(button){
		//case 0:
			//controlDataString(FLAG, value);
			//break;
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
	console.log("hereAxes");
	switch(axes){
		case 0:
			controlDataString(ROLL, value);
			break;
		case 1:
			controlDataString(PITCH, value);
			break;
		case 2:
			controlDataString(YAW, value);
			break;
		case 3:
			controlDataString(THROTTLE, value);
	}
}

function sendControlData(){
	socket.emit("controlEvent", controlStringArray);
}

function startController(){
	setInterval(updateStatus(), 3000);
}
