// Boilerplate code for socket io stuff

"use strict";

var socket;
var firstconnect = true;
var init_home=1;

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
