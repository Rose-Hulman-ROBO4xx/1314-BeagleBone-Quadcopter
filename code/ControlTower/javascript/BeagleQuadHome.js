// Boilerplate code for socket io stuff
"use strict";

var socket;
var firstconnect = true;
var init_home=1;
var eventJoystickFlag = 0;

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

//**** WEB PAGE SETUP *****//
function dynamicLayout(){
	var browserWidth = getBrowserWidth();
	if(browserWidth < 750){
		changeLayout("thin");
	}
	if((browserWidth == 750) && (browserWidth <= 950)){
		changeLayout("wide");
	}
	if(browserWidth > 950){
		changeLayout("wider");
	}
}

function getBrowserWidth(){
	if(window.innerWidth){
		return window.innerWidth;
	}
	else if(document.documentElement && document.documentElement.clientWidth != 0){
		return document.documentElement.clientWidth;
	}
	else if(document.body){
		return document.clientWidth;
	}
	return 0;
}

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

//Add initial load to dynamicLayout
addEvent(window, 'load', dynamicLayout);
//Add resize event to dynamicLayout
addEvent(window, 'resize', dynamicLayout);

//CSS style sheet selection
function changeLayout(layout){
       ;
}       

//Tab switching functionality
function tabSwitch(newTab){
	if(init_home == 1){
		document.getElementById('home_images').style.display = 'none';
		init_home=0;
	}


	//TODO 
	//1) Make this so that there are only one set of tabs that are initially not displayed
	//2) Make those tabs change inner text. Ie: opensource --> instructions
	//3) Make the reference links of each point to the appropriate place. opensource links to opensource location

	var buildTabs = ["opensource_tab", "forum_tab", "purchase_tab"];
	var flightTabs = ["instructions_tab", "hud_tab", "diagnostics_tab"];
	if(newTab == "fly_tab"){
		document.getElementById(newTab).innerHTML = 'Flight!';
		document.getElementById(newTab).className = 'active';
		document.getElementById('build_tab').innerHTML = 'Build';
		document.getElementById('build_tab').className = 'inactive';
		flightTabs.forEach(function(entry){
			document.getElementById(entry).style.display = 'inline-block';
		});
		buildTabs.forEach(function(entry){
			document.getElementById(entry).style.display = 'none';
		});
	}
	else if(newTab == "build_tab"){
		document.getElementById(newTab).innerHTML = 'Building!';
		document.getElementById(newTab).className = 'active';
		document.getElementById('fly_tab').innerHTML = 'Fly';
		document.getElementById('fly_tab').className = 'inactive';
		buildTabs.forEach(function(entry){
			document.getElementById(entry).style.display = 'inline-block';
		});
		flightTabs.forEach(function(entry){
			document.getElementById(entry).style.display = 'none';
		});	
	}

		
}


// Callback on the client side for the test button
function testButtonClick() {
	console.log("Clicked the test button!")
	socket.emit("testButtonPress",0);
}

// Handle key bindings for quadcopter control
// http://www.cambiaresearch.com/articles/15/javascript-char-codes-key-codes
document.onkeydown = function(e) {
	if(document.getElementById('fly_tab').className == 'active'){
		e = e || window.event;
		switch (e.which || e.keyCode) {
			// Left arrow = yaw left
			case 37:
				// alert("Left pressed!");
				socket.emit("keyPress", "yaw", -10);
				break;

			// Up arrow = throttle up
			case 38:
				// alert("Up pressed!");
				socket.emit("keyPress","up", 20);
				break;

			// Right arrow = yaw right
			case 39:
				// alert("Right pressed!");
				socket.emit("keyPress", "yaw" ,10);
				break;

			// Down arrow = throttle down
			case 40:
				// alert("Down pressed!");
				socket.emit("keyPress", "up", -20);
				break;

			// W key = pitch down
			case 87:
				// alert("W pressed!");
				socket.emit("keyPress", "pitch", -5);
				break;

			// A key = roll left
			case 65:
				// alert("A pressed!");
				socket.emit("keyPress", "roll", -5);
				break;

			// S key = pitch up
			case 83:
				// alert("S pressed!");
				socket.emit("keyPress", "pitch", 5);
				break;

			// D key = roll right
			case 68:
				// alert("D pressed!");
				socket.emit("keyPress", "roll", 5);
				break;

			default:
				return;
		}	
	}


	// Prevents scrolling of the page (default behavior)
	e.preventDefault();
}

var controllers = {};
var rAF = window.mozRequestAnimationFrame || window.webkitRequestAnimationFrame || window.requestAnimationFrame;

function connecthandler(e){
	if(document.getElementById('fly_tab').className == 'active'){
		addgamepad(e.gamepad);
	}
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
	for(var i =0; i<gamepad.buttons.length; i++){
		var button = document.createElement("span");
		button.className = "button";
		button.innerHTML = i;
		buttons.appendChild(button);
	}
	controllerView.appendChild(buttons);
	var axes = document.createElement("div");
	axes.className = "axes";
	for(var i=0; i<gamepad.axes.length; i++){
		var axis = document.createElement("progress");
		axis.className = "axis";
		axis.setAttribute("max", "2");
		axis.setAttribute("value", "1");
		axis.innerHTML = i;
		axes.appendChild(axis);
	}
	controllerView.appendChild(axes);
	var flyTab = document.getElementById('fly_tab');
	flyTab.appendChild(controllerView);
	rAF(updateStatus_withTimeOut);
}

function updateStatus(){
	if(navigator.webkitGetGamepads){
		scangamepads();
	}
	for(var gamepad in controllers){
		var controller = controllers[gamepad];
		var d = document.getElementById("controller" + gamepad);
		var buttons = d.getElementsByClassName("button");
		for(var i=0; i<controller.buttons.length; i++){
			var b = buttons[i];
			var val = controller.buttons[i];
			var pressed = val == 1.0;
			if(typeof(val) == "object"){
				pressed = val.pressed;
				val = val.value;
			}
			if(pressed){
				b.className = "button pressed";
			}
			else{
				b.className = "button";
			}
			socket.emit("controllerButtonEvent", i, val);
		}
		var axes = d.getElementsByClassName("axis");
		for(var i =0; i < controller.axes.length; i++){
			var a = axes[i];
			a.innerHTML = i + ": " + controller.axes[i].toFixed(4);
			a.setAttribute("value", controller.axes[i] + 1);
			socket.emit("controllerAxesEvent", i, a.getAttribute("value"));
		}
	}
	rAF(updateStatus_withTimeOut);
}

function scangamepads(){
	if(document.getElementById('fly_tab').className == 'active'){
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

function updateStatus_withTimeOut(){
	setTimeout(updateStatus, 1000);
}
