<!DOCTYPE html>
<html>
<head>
	<title>ESP-MONITOR - LOG</title>
	<meta name="viewport" content="width=device-width, initial-scale=1">
	<meta charset="utf-8"/>
	<link rel="icon" href="data:,">
	<link rel="stylesheet" type="text/css" href="style.css">
	<script src="ansi_up.js" type="text/javascript"></script>
	<script type="text/javascript">

		var ansi_up = new AnsiUp;
		var wsUri = "ws://"+window.location.host+"/websocket/ws.cgi";
		var output;
	  
		function init()
		{
		  output = document.getElementById("output");
		  testWebSocket();
		}
	  
		function testWebSocket()
		{
		  websocket = new WebSocket(wsUri);
		  websocket.onopen = function(evt) { onOpen(evt) };
		  websocket.onclose = function(evt) { onClose(evt) };
		  websocket.onmessage = function(evt) { onMessage(evt) };
		  websocket.onerror = function(evt) { onError(evt) };
		}
	  
		function onOpen(evt)
		{
			// Get existing log and print it here.
		  writeToScreen("CONNECTED");
		}
	  
		function onClose(evt)
		{
		  writeToScreen("DISCONNECTED");
		}
	  
		function onMessage(evt)
		{
		  writeToScreen(ansi_up.ansi_to_html(evt.data));
		}
	  
		function onError(evt)
		{
		  writeToScreen('<span style="color: red;">ERROR:</span> ' + evt);
		}
	  
		function doSend(message)
		{
		  websocket.send(message);
		}
	  
		function writeToScreen(message)
		{
		  var pre = document.createElement("p");
		  pre.style.wordWrap = "break-word";
		  pre.innerHTML = message;
		  output.appendChild(pre);
		}
	  
		window.addEventListener("load", init, false);
	  
	  </script>
</head>
<body>
	<div class="topnav">
		<a href="index.html">Configuration</a>
		<a class="active" href="log.html">Log</a>
		<a href="sensor_data.html">Sensor data</a>
		<div class="info">
			Firmware Version: <strong id="f_version"> %F_VERSION%</strong><br>
			Hardware Version: <strong id="h_version"> %H_VERSION%</strong><br>
			Sensor Type: <strong id="sensor_type"> %S_TYPE%</strong><br>
			Location: <strong id="location"> %S_LOCATION%</strong><br>
		</div>
	</div> 
  
	<h1>ESP-MONITOR - LOG</h1>

	<pre id="output"></pre>
  
</body>
</html>
