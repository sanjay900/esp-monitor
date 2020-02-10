<!DOCTYPE html>
<html>
<head>
	<title>ESP-MONITOR - Configuration</title>
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
			document.getElementById("txt").value = allText;
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
		
		function sendForm() {
			var formData = document.getElementById("txt").value;
			
			var xmlhttp = new XMLHttpRequest();
			xmlhttp.open('POST','config.txt', true);
			xmlhttp.onreadystatechange = function() {
				if (xmlhttp.readyState == 4 && xmlhttp.status == 200) {
					console.log(xmlhttp.responseXML);
				}
			};
			
			xmlhttp.setRequestHeader('Content-Type', 'text/plain');
			
			xmlhttp.send(formData);
			
			xmlhttp.onload = function() {
				if (xmlhttp.status != 200) { // HTTP error?
					// handle error
					alert( 'Error: ' + xmlhttp.status);
					return;
				}
				if (xmlhttp.readyState === xmlhttp.DONE) {
					//alert("File uploaded successfully, Rebooting...");
					if (xmlhttp.status === 200) {
						alert(xmlhttp.responseText);
						//setTimeout(location.reload(), 8000);
					}
				}
				alert("File uploaded, Rebooting...");
			  // get the response from xhr.response
			};

			xmlhttp.onerror = function() {
			  // handle non-HTTP error (e.g. network down)
			  alert( 'Error: ' + xmlhttp.status);
			};
			
				
		  return false; // Prevent page from submitting.
		}
	</script>
</head>
<body onload="loadConfigDoc();">
	<div class="topnav">
		<a class="active" href="index.tpl">Configuration</a>
		<a href="log.tpl">Log</a>
		<a href="sensor_data.tpl">Sensor data</a>
		<div class="info">
			Firmware Version: <strong id="f_version"> %F_VERSION%</strong><br>
			Hardware Version: <strong id="h_version"> %H_VERSION%</strong><br>
			Sensor Type: <strong id="sensor_type"> %S_TYPE%</strong><br>
			Location: <strong id="location"> %S_LOCATION%</strong><br>
		</div>
	</div> 

	<h1>ESP-MONITOR - Configuration</h1>
	<form method="POST" id="config_form" target="_parent">
		<textarea id="txt" style="width: 550px; height: 500px;">{{xmlString}}</textarea><br><br>
		<input type="submit" value="Save Config and Restart" onclick="return sendForm();">
	</form>
</body>
</html>
