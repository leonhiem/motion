<!DOCTYPE html>
<html lang="en">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta name="viewport" content="width=device-width">
<meta name="author" content="Simon Hiemstra">
<meta name="description" content="">
<meta http-equiv="cache-control" content="max-age=0" />
<meta http-equiv="cache-control" content="no-cache" />
<meta http-equiv="expires" content="0" />
<meta http-equiv="expires" content="Tue, 01 Jan 1980 1:00:00 GMT" />
<meta http-equiv="pragma" content="no-cache" />
<script type="text/javascript" src="https://ajax.googleapis.com/ajax/libs/jquery/1.8.2/jquery.min.js"></script>
<script type="text/javascript" src="resource/jquery.touchSwipe.min.js"></script>
<link rel="shortcut icon" href="/favicon.ico">
<link rel="apple-touch-icon" href="/apple-touch-icon.png">
<title>Schans Camera: Live</title>
<script type="text/javascript">
	
	var camera = 0;
	var position = 4;
	
	$(function() 
	{ 
		// Resetting the camera position upon loading
		$.get("http://10.0.0.12:3010/led4tog.cgi"); 
	});
	
	function cameraswitch()
	{ 
		camcontrol = document.getElementById("camcontrol");
		camera = camera + 1;
		switch(camera)
		{
			case 0: new_camera = 'http://10.0.0.12:3001'; camcontrol.style.display = 'block'; break; // Draaibare stal
			case 1: new_camera = 'http://10.0.0.12:3002'; camcontrol.style.display = 'none'; break; // Kalverstal
			case 2: new_camera = 'http://10.0.0.15';	break; // Andere kant
			case 3: new_camera = 'http://10.0.0.12:3003';	break; // robot1
			case 4: new_camera = 'http://10.0.0.12:3000';	break; // robot2
			case 5: camera = -1; cameraswitch(); break;
			default: camera = -1; cameraswitch(); break; 
		}
		
		document.getElementById('display').src = new_camera+'/videostream.cgi?user=admin&pwd=&resolution=32';
	}
	
	function camcontrol_lamp()
	{ $.get("http://10.0.0.12:3010/led1tog.cgi"); }

	function camcontrol_move(direction)
	{
		switch(direction)
		{
			case 'left': 
				if(position<6) position = position +1;
				$.get("http://10.0.0.12:3010/led"+position+"tog.cgi");
			break;
			case 'right':
				if(position>2) position = position -1;
				$.get("http://10.0.0.12:3010/led"+position+"tog.cgi");
			break;
		}
	}
	
</script>
<style type="text/css">

body 
{ width: 100%; 
  height: 100%; 
  text-align: center;
  margin: 0px; 
  background: #000000; }
  
h1 
{ font-family: "Verdana"; 
  font-size: 40px; 
  margin:0; 
  padding:0; }
  
hr 
{ height: 1px; 
  border: 0 none; 
  background: #000000; 
  width: 100%; }

div#onder
{ width:100%;
  height: 100%; }
  
div#screen
{ position: relative;
  width: 100%; 
  height: 100%; }
  
div#screen_left, div#screen_right
{ position: absolute;
  width: 5%; 
  height: 100%; 
  background: #000000;
  opacity: 0; }
div#screen_left:hover, div#screen_right:hover
{ opacity: 0.5; 
  cursor: pointer; }
div#screen_left
{ top: 0px; left: 0px; }
div#screen_right
{ top: 0px; right: 0px; }

img#display 
{ width: 100%; }

div#camcontrol
{ width:100%; 
  height: auto; 
  position: absolute;
  background: black;
  opacity: 0.5; }
  
div#camcontrol_left, div#camcontrol_right, div#camcontrol_lamp
{ width: 10%; height: 100%; color: white; font-size: 40px; font-family: Arial; line-height: 175%;   }
div#camcontrol_left:hover, div#camcontrol_right:hover, div#camcontrol_lamp:hover
{ background: #111; cursor: pointer; }

div#camcontrol_left  { float: left; }
div#camcontrol_right { float: right; }
div#camcontrol_lamp { float: left; margin-left: 35%; }

</style>
	
</head>
<body>
<div id="onder">
	<div id="screen">
		<div id="camcontrol">
			<div id="camcontrol_left" onclick="camcontrol_move('left')">L</div>
			<div id="camcontrol_right" onclick="camcontrol_move('right')">R</div>
			<div id="camcontrol_lamp" onclick="camcontrol_lamp()">L</div>
		</div>
		<div id="camera">
		<img id="display" src="http://10.0.0.12:3001/videostream.cgi?user=admin&pwd=&resolution=32" onclick="cameraswitch();">
		</div>
	</div>
	</table>
</div>
</body>
</html>
