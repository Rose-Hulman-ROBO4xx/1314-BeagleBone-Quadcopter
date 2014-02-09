var dirname = "/dev/ControlTower/Images/";

var image = "original.gif"
var movement = new Array();
	movement = [{key:"roll", speed:0, pressed:false},
				{key:"pitch", speed:0, pressed:false},
				{key:"yaw", speed:0, pressed:false},
				{key:"up", speed:0, pressed:false}
				];
				
				
				
				
function rotateQuad(direction, speed) {
		movement.forEach(function(entry){
			if(entry.key == direction){
				//update key press
				entry.pressed = true;
				entry.speed += speed;
			}
		updateQuadcopter();
		updateAnimation();
		console.log(entry.key + ":"+ entry.speed);
			
		});
		
}
	
	
function updateVals(speed) {
	movement.forEach(function(entry){
		if((entry.speed != 0)  && keyUnpressed(entry.key)){
			entry.speed = 0;
		}		
	});
}
	
function keyUnpressed(entry){
	return !entry.pressed;
	}
	
function updateQuadcopter(){
	movement.forEach(function(entry){
			;//write to file
		});
}	

function updateAnimation(){
	image ="./images/QC_";
	movement.forEach(function(entry){
		if(entry.speed != 0){
			image.concat(entry.key+"-"+entry.speed);
		}
		document.getElementById("quad_image").src=image+".gif";
		;//update css for forward, left, up
	});
}	

function writeToFile(){
	movement.forEach(function(entry){
		file.write(entry.key.join(':') + entry.key.speed.join('\\n'));
		});
}