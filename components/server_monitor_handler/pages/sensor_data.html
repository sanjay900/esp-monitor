<!DOCTYPE html>
<html>
<head>
	<title>ESP-MONITOR - DATA</title>
	<meta name="viewport" content="width=device-width, initial-scale=1">
	<meta charset="utf-8"/>
	<link rel="icon" href="data:,">
	<link rel="stylesheet" type="text/css" href="style.css">
	<script type="text/javascript">
		function loadConfigDoc() {
		  var xmlhttp = new XMLHttpRequest();
		  xmlhttp.onreadystatechange = function() {
			if (this.readyState == 4 && this.status == 200) {
					loadData(this);
			}
		  };
		  xmlhttp.open("GET", "config.txt" , true);
		  xmlhttp.send();
		}

		function loadData(txt) {
			var allText = txt.responseText;
			var pos = allText.indexOf("f_version=") + 10;
			var end = allText.indexOf(";", pos);
			document.getElementById("f_version").innerHTML = allText.slice(pos, end);
			pos = allText.indexOf("h_version=") + 10;
			end = allText.indexOf(";", pos);
			document.getElementById("h_version").innerHTML = allText.slice(pos, end);
			pos = allText.indexOf("sensor_type=") + 12;
			end = allText.indexOf(";", pos);
			document.getElementById("sensor_type").innerHTML = allText.slice(pos, end);
			pos = allText.indexOf("location=") + 9;
			end = allText.indexOf(";", pos);
			document.getElementById("location").innerHTML = allText.slice(pos, end);
		}
	</script>
</head>
<body onload="drawGraph(); loadConfigDoc();">
	<div class="topnav">
		<a href="index.html">Configuration</a>
		<a href="log.html">Log</a>
		<a class="active" href="sensor_data.html">Sensor data</a>
		<div class="info">
			Firmware Version: <strong id="f_version"> %F_VERSION%</strong><br>
			Hardware Version: <strong id="h_version"> %H_VERSION%</strong><br>
			Sensor Type: <strong id="sensor_type"> %S_TYPE%</strong><br>
			Location: <strong id="location"> %S_LOCATION%</strong><br>
		</div>
	</div> 
  
	<h1>ESP-MONITOR - DATA</h1>
	
	<p>Temperature: <strong> %TEMPERATURE% &#x2103;</strong><br>
	<p>Humidity: <strong> %HUMIDITY% %</strong><br><br><br>

<canvas id="myCanvas" width="570" height="300"></canvas>
<menu id="menu_items" >
<div >
<span style="left:60px;"><input type="checkbox" id="c0" value="Connect" checked><label for="c0" style="color:#ff0000">Temperature</label></span>
<span style="left:110px;"><input type="checkbox" id="c1" value="Connect"><label for="c1" style="color:#00ff00">Humidity</label></span>
<span style="left:410px;">Division:<select id="dur"><option value="100">100ms</option><option value="200">200ms</option><option value="500">500ms</option><option value="1000">1sec</option></select></span>
</div>
</menu>
<script type="text/javascript">
function GetArduinoInputs(){ //DEMO
	updateGraph(Math.random()*150,150+Math.random()*150);
	setTimeout('GetArduinoInputs()',document.getElementById('dur').value);
}
 
var canvas = document.getElementById('myCanvas');
var ctx = canvas.getContext('2d');
myCanvas.style.border = "red 1px solid";
var aval0=[], aval1=[]; 

for(cnt=0; cnt<18; cnt++){
	aval0[cnt]=0;
	aval1[cnt]=0;
}

function drawGraph(){initGraph(); updateGraph(0,0); GetArduinoInputs();}

function initGraph(){
	ctx.fillStyle="#ff0000";
	ctx.font = "26px serif";
	ctx.textAlign = "center";
	ctx.textBaseline = "bottom";
	ctx.fillText("Data Display Graph",290,30);
	ctx.fillStyle="#000000";
	ctx.font = "14px serif";
	ctx.textAlign = "left";
	ctx.textBaseline = "bottom";
	ctx.fillText("0",540,300);
	ctx.fillText("128",540,275);
	ctx.fillText("256",540,250);
	ctx.fillText("384",540,225);
	ctx.fillText("512",540,200);
	ctx.fillText("640",540,175);
	ctx.fillText("768",540,150);
	ctx.fillText("896",540,125);
	ctx.fillText("1024",540,100);
}
		
function updateGraph(val0, val1){
	ctx.clearRect(0, 30, canvas.width-30, canvas.height);
	aval0.shift(); aval0.push(val0*2/10.23);
	aval1.shift(); aval1.push(val1*2/10.23);
	if(document.getElementById('c0').checked)drawPlot(0);
	if(document.getElementById('c1').checked)drawPlot(1);
}

function drawPlot(barSel){
	ctx.save();
	ctx.beginPath();
	ctx.lineWidth = 1;
	ctx.strokeStyle = '#888888';
	for(cnt=1; cnt<9; cnt++){
		ctx.setLineDash([1,1]);
		ctx.moveTo(0,75+cnt*25);
		ctx.lineTo(525,75+cnt*25);
		ctx.stroke();
	}
	for(cnt=1; cnt<18; cnt++){
		ctx.moveTo(0+cnt*30,90);
		ctx.lineTo(0+cnt*30,300);
		ctx.stroke();
	}
	ctx.restore();
	ctx.beginPath();
	switch(barSel){
		case 0 : ctx.moveTo(0,300-aval0[0]); break;
		case 1 : ctx.moveTo(0,300-aval1[0]); break;
		default : ctx.moveTo(0,300-aval0[0]); 
	}
	for(cnt=1; cnt<18; cnt++){
		switch(barSel){
			case 0 : ctx.lineTo(cnt*30,300-aval0[cnt]); ctx.strokeStyle = '#ff0000'; break;
			case 1 : ctx.lineTo(cnt*30,300-aval1[cnt]); ctx.strokeStyle = '#00ff00'; break;
			default : ctx.lineTo(cnt*30,300-aval0[cnt]); ctx.strokeStyle = '#00ffff';
		}
		ctx.lineWidth = 3;
		ctx.stroke();
	}
}
</script>
</body>
</html>
