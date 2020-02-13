<!DOCTYPE html>
<html>

<head>
	<title>ESP-MONITOR - DATA</title>
	<meta name="viewport" content="width=device-width, initial-scale=1">
	<meta charset="utf-8" />
	<link rel="icon" href="data:,">
	<link rel="stylesheet" type="text/css" href="style.css">
</head>

<body>
	<div class="topnav">
		<a href="index.tpl">Configuration</a>
		<a href="log.tpl">Log</a>
		<a class="active" href="sensor_data.tpl">Sensor data</a>
		<a href="flash/index.tpl">OTA Upload</a>
		<a href="wifi/wifi.tpl">Network Config</a>
		<div class="info">
			Firmware Version: <strong id="f_version"> %F_VERSION%</strong><br>
			Hardware Version: <strong id="h_version"> %H_VERSION%</strong><br>
			Sensor Type: <strong id="sensor_type"> %S_TYPE%</strong><br>
			Location: <strong id="location"> %S_LOCATION%</strong><br>
		</div>
	</div>

	<h1>ESP-MONITOR - DATA</h1>
			<script src="https://cdnjs.cloudflare.com/ajax/libs/Chart.js/2.9.3/Chart.bundle.min.js" integrity="sha256-TQq84xX6vkwR0Qs1qH5ADkP+MvH0W+9E7TdHJsoIQiM=" crossorigin="anonymous"></script>
			<canvas id="myChart" width="400" height="400"></canvas>
			<script type="text/javascript">
				window.chartColors = {
					red: 'rgb(255, 99, 132)',
					orange: 'rgb(255, 159, 64)',
					yellow: 'rgb(255, 205, 86)',
					green: 'rgb(75, 192, 192)',
					blue: 'rgb(54, 162, 235)',
					purple: 'rgb(153, 102, 255)',
					grey: 'rgb(201, 203, 207)'
				};
				var lbls = {};
				var data = { datasets: [] };
				var nid = 0;
				var timeFormat = 'MM/DD/YYYY HH:mm';
				var ctx = document.getElementById('myChart');
				var myChart = new Chart(ctx, {
					type: 'line',
					data: data,
					options: {
						responsive: true,
						title: {
							display: true,
							text: 'Chart.js Line Chart'
						},
						tooltips: {
							mode: 'index',
							intersect: false,
						},
						hover: {
							mode: 'nearest',
							intersect: true
						},
						scales: {
							xAxes: [{
								type: 'time',
								time: {
									parser: timeFormat,
									// round: 'day'
									tooltipFormat: 'll HH:mm:ss'
								},
								scaleLabel: {
									display: true,
									labelString: 'Date'
								}
							}],
							yAxes: [{
								scaleLabel: {
									display: true,
									labelString: 'value'
								}
							}]
						},
					}
				});
				var wsUri = "ws://" + window.location.host + "/websocket/sensors.cgi";

				function init() {
					websocket = new WebSocket(wsUri);
					websocket.onmessage = function (message) {
						id = message.data.split(":")[0];
						msgdata = parseFloat(message.data.split(":")[1].trim());
						if (!lbls.hasOwnProperty(id)) {
							lbls[id] = nid++;
							var colorNames = Object.keys(window.chartColors);
							var colorName = colorNames[data.datasets.length % colorNames.length];
							var newColor = window.chartColors[colorName];
							data.datasets.push({ label: id, data: [], backgroundColor: newColor, borderColor: newColor, fill: false });
						}
						data.datasets[lbls[id]].data.push({ x: new Date(), y: msgdata });
						myChart.update();
					};
				}

				window.addEventListener("load", init, false);
			</script>
</body>

</html>