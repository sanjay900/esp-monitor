<html>
<head><title>Upgrade firmware</title>
<link rel="stylesheet" type="text/css" href="../style.css">
<script type="text/javascript" src="../140medley.min.js"></script>
<script type="text/javascript">

var xhr=j();
var valid_apps = {};

function openAjaxSpinner() {
    $("#spinner").style.display = 'block';
}

function closeAjaxSpinner() {
    $("#spinner").style.display = 'none';
}

function doCommonXHR(donecallback, successcallback) {
	xhr.onreadystatechange=function() {
        if (xhr.readyState==4) {
            if (xhr.status>=200 && xhr.status<=400) {
                var json_response = JSON.parse(xhr.responseText);
                if (json_response.success == false) {
                    $("#remark").innerHTML="<strong style=\"color: red;\">Error: "+xhr.responseText+"</strong>";
                    closeAjaxSpinner();
                } else {
                    if (successcallback && typeof(successcallback) === "function") {successcallback(json_response);}
                }
            }
            else
            {
                $("#remark").innerHTML="<strong style=\"color: red;\">Error!  Check connection.  HTTP status: " + xhr.status + "</strong>";
                closeAjaxSpinner();
            }
            if (donecallback && typeof(donecallback) === "function") {donecallback();}
        }
	}
}

function doReboot(callback) {
	$("#remark").innerHTML="Sending reboot command...";
    openAjaxSpinner();
	xhr.open("GET", "./reboot");
    doCommonXHR(callback, function(json_response) {
        $("#remark").innerHTML="Command sent.  Rebooting!...";
        // todo: use Ajax to test if connection restored before reloading page...
        window.setTimeout(function() {
            location.reload(true);
        }, 4000);
    });
	xhr.send();
	return false;
}

function doVerify(partition, callback) {
	$("#remark").innerHTML="Verifying App in partition: " + partition + "...";
    openAjaxSpinner();
	xhr.open("GET", "./flashinfo.json?verify=1" + ((partition)?("&partition=" + partition):("")));
    doCommonXHR(callback, function(flinfo) {
        for(var i=0;i<flinfo.app.length;i++)
        {
            if (typeof flinfo.app[i].valid !== "undefined") {
                valid_apps[flinfo.app[i].name] = flinfo.app[i].valid;
            }
        }
        $("#remark").innerHTML="Finished verify.  " + partition + " is <b>" + ((valid_apps[partition])?("Valid"):("Not valid")) + "</b>";
        doInfo(function() {
            closeAjaxSpinner();
        });
    });
	xhr.send();
	return false;
}

function doSetBoot(partition, callback) {
	$("#remark").innerHTML="Setting Boot Flag to partition: " + partition + "...";
    openAjaxSpinner();
	xhr.open("GET", "./setboot" + ((partition)?("?partition=" + partition):("")));
    doCommonXHR(callback, function(json_response) {
        doInfo(function() {
            $("#remark").innerHTML="Finished setting Boot Flag to " + partition + ".  <b>Now press Reboot!</b>";
            closeAjaxSpinner();
        });
    });
	xhr.send();
	return false;
}

function doEraseFlash(partition, callback) {
	$("#remark").innerHTML="Erasing data in: " + partition + "...";
    openAjaxSpinner();
	xhr.open("GET", "./erase" + ((partition)?("?partition=" + partition):("")));
    doCommonXHR(callback, function(json_response) {
        doInfo(function() {
            $("#remark").innerHTML="Finished erasing data in: " + partition + ".  <b>Must reboot to reformat it!</b>";
            closeAjaxSpinner();
        });
    });
	xhr.send();
	return false;
}

function doUpgrade(partition, callback) {
	var f=$("#file").files[0];
	if (typeof f === 'undefined') {
		$("#remark").innerHTML="<strong style=\"color: red;\">Can't read file!  Browse for .bin file.</strong>";
		return
	}
	$("#remark").innerHTML="Uploading App to partition: " + partition;
    openAjaxSpinner();
	xhr.open("POST", "./upload" + ((partition)?("?partition=" + partition):("")));
    doCommonXHR(callback, function(json_response) {
        doInfo(function() {
            $("#remark").innerHTML="Upload flash done and Boot Set. <b>Now Reboot!</b>";   
            closeAjaxSpinner();
            // doReboot();
        });
    });
    xhr.send(f);
	return false;
}

function humanFileSize(B,i){var e=i?1e3:1024;if(Math.abs(B)<e)return B+" B";var a=i?["kB","MB","GB","TB","PB","EB","ZB","YB"]:["KiB","MiB","GiB","TiB","PiB","EiB","ZiB","YiB"],t=-1;do B/=e,++t;while(Math.abs(B)>=e&&t<a.length-1);return B.toFixed(3)+" "+a[t]}

function doInfo(callback) {
    var dataformats = {0: "ota_data", 1: "phy_init", 2: "nvs", 3: "coredump", 0x81: "fat", 0x82: "spiffs"};
	xhr.open("GET", "./flashinfo.json");
    doCommonXHR(callback, function(flinfo) {
        var tablerows = "";
        for(var i=0;i<flinfo.app.length;i++)
        {
            if (typeof flinfo.app[i].valid !== "undefined") {
                valid_apps[flinfo.app[i].name] = flinfo.app[i].valid;
            }
            tablerows += "<tr>"
                         + "<td>" + flinfo.app[i].name + "</td>"
                         + "<td>" + humanFileSize(flinfo.app[i].size,0) + "</td>"
                         + "<td>" + flinfo.app[i].version + "</td>"
                         + "<td>" + ((valid_apps[flinfo.app[i].name])?("&#10004;"):(('<input type="submit" value="Verify ' + flinfo.app[i].name + '!" onclick="doVerify( \'' + flinfo.app[i].name + '\')" />'))) + "</td>"
                         + "<td>" + ((flinfo.app[i].running)?("&#10004;"):("")) + "</td>"
                         + '<td>' + ((flinfo.app[i].bootset)?("&#10004;"):('<input type="submit" value="Set Boot ' + flinfo.app[i].name + '!" onclick="doSetBoot( \'' + flinfo.app[i].name + '\')" />')) + '</td>'
                         + '<td>' + (((flinfo.app[i].ota) && !(flinfo.app[i].running))?('<input type="submit" value="Upload to ' + flinfo.app[i].name + '!" onclick="doUpgrade( \'' + flinfo.app[i].name + '\')" />'):('')) + '</td>'
                         + "</tr>";
        };
        $('#tbodyApp').innerHTML=tablerows;
        tablerows = "";
        for(var i=0;i<flinfo.data.length;i++)
        {
            tablerows += "<tr>"
                         + "<td>" + flinfo.data[i].name + "</td>"
                         + "<td>" + humanFileSize(flinfo.data[i].size,0) + "</td>"
                         + "<td>" + dataformats[flinfo.data[i].format] + "</td>"
                         + '<td>' + ('<input type="submit" value="Erase ' + flinfo.data[i].name + '!" onclick="doEraseFlash( \'' + flinfo.data[i].name + '\')" />') + '</td>'
                         + "</tr>";
        };
        $('#tbodyData').innerHTML=tablerows;
    });
	xhr.send();
	return false;
}

window.onload=function(e) {
    openAjaxSpinner();
	doInfo(function(){
        closeAjaxSpinner();
        $("#remark").innerHTML="Ready.";
    }
    );
}

</script>
<style>
table, td, th {
    border: 1px solid black;
}
table {
    border-collapse: collapse;
    width: 100%;
}
</style>
</head>
<body>
<div class="topnav">
    <a href="/index.tpl">Configuration</a>
    <a href="/log.tpl">Log</a>
    <a href="/sensor_data.tpl">Sensor data</a>
    <a class="active" href="/flash/index.tpl">OTA Upload</a>
    <a href="/wifi/wifi.tpl">Wifi Config</a>
    <div class="info">
        Firmware Version: <strong id="f_version"> %F_VERSION%</strong><br>
        Hardware Version: <strong id="h_version"> %H_VERSION%</strong><br>
        Sensor Type: <strong id="sensor_type"> %S_TYPE%</strong><br>
        Location: <strong id="location"> %S_LOCATION%</strong><br>
    </div>
</div> 
<div id="main">
<h1>Upgrade Firmware</h1>

<p><table>
<caption><b>App Partitions</b></caption>
     <thead>
          <tr>
               <th>Partition Name</th>
               <th>Partition Size</th>
               <th>App Version</th>
               <th>App Valid</th>
               <th>App Running</th>
               <th>App Set Boot</th>
               <th>App Upload <br/><input type="file" id="file" /></th>
           </tr>
     </thead>
     <tbody id="tbodyApp">
     </tbody>
</table></p>

<p><table>
<caption><b>Data Partitions</b></caption>
     <thead>
          <tr>
               <th>Partition Name</th>
               <th>Partition Size</th>
               <th>Data Type</th>
               <th>Erase</th>
           </tr>
     </thead>
     <tbody id="tbodyData">
     </tbody>
</table></p>
<input type="submit" value="Reboot!" onclick="doReboot()" />
<div><p id="remark">Loading...</p></div>
<div id="spinner"></div>
</div>
</body>
</html>