<!DOCTYPE html>
<html>

<head>
	<title>ESP-MONITOR - Configuration</title>
	<meta name="viewport" content="width=device-width, initial-scale=1">
	<meta charset="utf-8" />
	<link rel="icon" href="data:,">
	<link rel="stylesheet" type="text/css" href="style.css">
</head>

<body>
	<div class="topnav">
		<a class="active" href="index.tpl">Configuration</a>
		<a href="log.tpl">Log</a>
		<a href="sensor_data.tpl">Sensor data</a>
		<a href="flash/index.tpl">OTA Upload</a>
		<a href="wifi/wifi.tpl">Wifi Config</a>
		<div class="info">
			Firmware Version: <strong id="f_version"> %F_VERSION%</strong><br>
			Hardware Version: <strong id="h_version"> %H_VERSION%</strong><br>
			Sensor Type: <strong id="sensor_type"> %S_SENSORS%</strong><br>
			Location: <strong id="location"> %S_LOCATION%</strong><br>
		</div>
	</div>

	<h1>ESP-MONITOR - Configuration</h1>
	<form method="POST" id="config_form" action="config.cgi">
		<label>Device Name</label> <input type="text" name="name" value="%S_NAME%" /> <br>
		<label>Device Location</label> <input type="text" name="loc" value="%S_LOCATION%" /> <br>
		<label>Device ID</label> <input type="number" name="id" value="%S_ID%"/> <br>
		<label>Refresh Rate</label> <input type="number" name="rate" value="%S_RATE%"/> <br>
		<label>MQTT Broker IP</label> <input type="text" name="mqtt" value="%S_MQTT%"/> <br>
		<fieldset>
			<legend>Sensor Type: </legend>
			<input type="checkbox" name="TH" value="TH" id="TH"><label for="TH">Temperature / Humidity</label><br>
			<input type="checkbox" name="SI" value="SI" id="SI"><label for="SI">Seismic / Temperature</label><br>
		</fieldset>
		<br>
		<input type="submit" value="Save Config and Restart" style="width: calc(50vw + 150px)">
	</form>
	<script type="text/javascript">
		if ("%S_SENSORS%".indexOf("SI") != -1) {
			document.getElementById("SI").checked = true;
		}
		if ("%S_SENSORS%".indexOf("TH") != -1) {
			document.getElementById("TH").checked = true;
		}
	</script>
</body>

</html>