#!/usr/bin/node
// From Getting Started With node.js and socket.io 
// http://codehenge.net/blog/2011/12/getting-started-with-node-js-and-socket-io-v0-7-part-2/
// This is a general server for the various web frontends
"use strict";

var port = 1337, // Port to listen on
    http = require('http'),
    url = require('url'),
    fs = require('fs'),
    //b = require('bonescript'),
    child_process = require('child_process'),
    server,
    connectCount = 0,	// Number of connections to server
    errCount = 0;	// Counts the AIN errors.

var controlDataFile = '/tmp/BeagleQuad_ControlData.txt';

function writeToFile(input){
	fs.open(controlDataFile, 'a', undefined, function(error, fd){
		if(error){
			if(fd){
				fs.close();
			}
			console.log('Error opening fifo: ' +error);
		return;
		}
		fs.write(fd, input, 0, input.length, null, function(error, written, buffer){
			if(fd){
				fs.close(fd);
			}
			if(error){
				console.log('Error writing to fifo: ' + error);
			}
			else{
				if(written != input.length){
					console.log("Error: Did not write all data to fifo stream");
				}
			}
		});
	});
}

// Initialize various IO things.
function initIO() {
}

initIO();

// Create a server and get it listening
server = http.createServer(function (req, res) {
    var path = url.parse(req.url).pathname;
    console.log("path: " + path);
    if (path === '/') {
        path = '/html/BeagleQuadHome.html';
    }

    fs.readFile(__dirname + path, function (err, data) {
        if (err) { return send404(res); }
        res.write(data, 'utf8');
        res.end();
    });
});

server.listen(port);
console.log("Listening on " + port);

// Set up socket io
var io = require('socket.io').listen(server);
io.set('log level', 2);

// See https://github.com/LearnBoost/socket.io/wiki/Exposed-events
// for Exposed events

// on a 'connection' event
io.sockets.on('connection', function (socket) {

    console.log("Connection " + socket.id + " accepted.");

    /*
        Server side callback for our key bindings
    */
    socket.on('keyPress', function (key, value) {
	//console.log("CONSOLE LOG... " + key + ":"+value);
	writeToFile(key+":"+value);
    });


    // Boilerplate to deal with connection managing
    socket.on('disconnect', function () {
        console.log("Connection " + socket.id + " terminated.");
        connectCount--;
        console.log("connectCount = " + connectCount);
    });
    connectCount++;
    console.log("connectCount = " + connectCount);
});

// Send a 404 page not found when an issue occurs
function send404(response) {
    response.writeHead(404);
    response.write('404');
    response.end();
}
