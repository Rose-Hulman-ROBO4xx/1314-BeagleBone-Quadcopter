var fs = require('fs');

var dirName = '/dev/QuadCopter';

fs.unlinkSync('/dev/');
console.log('Succesfully connected');

fs.existsSync(dirName);
console.log('Succesfully connected');

fs.writeFileSync('pitch.txt', '0', 'utf8');

console.log('Succesfully write to file');
 

/*
Our code
*/

//**** WEB PAGE SETUP *****//
function dynamicLayout() {
    var browserWidth = getBrowserWidth();
    if (browserWidth < 750) {
        changeLayout("thin");
    }
    if ((browserWidth >= 750) && (browserWidth <= 950)) {
        changeLayout("wide");
    }
    if (browserWidth > 950) {
        changeLayout("wider");
    }
}

function getBrowserWidth() {
    if (window.innerWidth) {
        return window.innerWidth;
    } else if (document.documentElement && document.documentElement.clientWidth != 0) {
        return document.documentElement.clientWidth;
    } else if (document.body) {
        return document.clientWidth;
    }
    return 0;
}

//**** Event Handling ****//

//Adds an event to the events queue
function addEvent(obj, type, fn) {
    if (obj.addEventListener) {
        obj.addEventListener(type, fn, false);
    } else if (obj.attachEvent) {
        obj["e" + type + fn] = fn;
        obj[type + fn] = function () {
            obj["e" + type + fn](window.event);
        }
        obj.attachEvent("on" + type, obj[type + fn]);
    }
}


function setupEvents() {
    document.getElementById('heads_up_display_tab').onclick() = tabSwitch('hud_tab', 'heads_up_display');
    document.getElementById('diagnostics_tab').onclick() = tabSwitch('diagnostics_tab', 'diagnostics');
    document.getElementById('opensource_tab').onclick() = tabSwitch('opensource_tab', 'opensource');

}

setupEvents();

//Add initial load to dynamicLayout
addEvent(window, 'load', dynamicLayout);
//Add resize event to dynamicLayout
addEvent(window, 'resize', dynamicLayout);

//CSS style sheet selection

//Tab switching functionality
function tabSwitch(tab_id, new_content) {

    var contents = ["instructions_tab", "hud_tab", "diagnostics_tab", "opensource_tab", "forum_tab", "purchase_tab"];
    if (tab_id == "fly_tab") {
        document.getElementById(tab_id).innerHTML = 'Flight!';
        document.getElementById('build_tab').innerHTML = 'Build';
        document.getElementById('instructions_tab').style.display = 'inline-block';
        document.getElementById('hud_tab').style.display = 'inline-block';
        document.getElementById('diagnostics_tab').style.display = 'inline-block';
        document.getElementById('opensource_tab').style.display = 'none';
        document.getElementById('forum_tab').style.display = 'none';
        document.getElementById('purchase_tab').style.display = 'none';
        document.getElementById('fly_tab').className = 'active';
        document.getElementById('build_tab').className = 'inactive';
    } else if (tab_id == "build_tab") {
        document.getElementById('fly_tab').innerHTML = 'Fly';
        document.getElementById('fly_tab').className = 'inactive';
        document.getElementById('build_tab').innerHTML = 'Building!';
        document.getElementById('build_tab').className = 'active';
        document.getElementById('instructions_tab').style.display = 'none';
        document.getElementById('hud_tab').style.display = 'none';
        document.getElementById('diagnostics_tab').style.display = 'none';
        document.getElementById('opensource_tab').style.display = 'inline-block';
        document.getElementById('forum_tab').style.display = 'inline-block';
        document.getElementById('purchase_tab').style.display = 'inline-block';
    }
	else if(tab_id == "heads_up_display_content"){
		document.getElementByClass('movements').style.display = 'inline-block';
	}

}

    // Callback on the client side for the test button
    function print(message) {
        console.log("Clicked the test button!");
        socket.emit("testButtonPress", 0);
    }

    // Handle key bindings for quadcopter control
    // http://www.cambiaresearch.com/articles/15/javascript-char-codes-key-codes
    document.onkeydown = function (e) {
        e = e || window.event;
        switch (e.which || e.keyCode) {
            // Left arrow = yaw left
            case 37:
                // alert("Left pressed!");
                socket.emit("leftKeyPressed", 10);
				rotateQuad("images", "");
                break;

                // Up arrow = throttle up
            case 38:
                // alert("Up pressed!");
                socket.emit("upKeyPressed", 20);
                break;

                // Right arrow = yaw right
            case 39:
                // alert("Right pressed!");
                socket.emit("rightKeyPressed", 10);
                break;

                // Down arrow = throttle down
            case 40:
                // alert("Down pressed!");
                socket.emit("downKeyPressed", 20);
                break;

                // W key = pitch down
            case 87:
                // alert("W pressed!");
                socket.emit("wKeyPressed", 0);
                break;

                // A key = roll left
            case 65:
                // alert("A pressed!");
                socket.emit("aKeyPressed", 0);
                break;

                // S key = pitch up
            case 83:
                // alert("S pressed!");
                socket.emit("sKeyPressed", 0);
                break;

                // D key = roll right
            case 68:
                // alert("D pressed!");
                socket.emit("dKeyPressed", 0);
                break;

            default:
                return;
        }

        // Prevents scrolling of the page (default behavior)
        e.preventDefault();
    }