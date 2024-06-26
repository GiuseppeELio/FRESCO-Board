// Function to update PCool dashboard visibility
function updatePCoolDashboardVisibility() {
    fetch('/status')
        .then(response => response.text())
        .then(data => {
            const isPCoolEnabled = data === 'true';
            const pcoolDashboard = document.getElementById('pcool-dashboard');
            if (isPCoolEnabled) {
                pcoolDashboard.style.display = 'block'; // Show PCool dashboard
            } else {
                pcoolDashboard.style.display = 'none'; // Hide PCool dashboard
            }
        })
        .catch(error => console.error('Error:', error));
}

// Periodically update PCool dashboard status every 5 seconds
setInterval(updatePCoolDashboardVisibility, 5000);

// Initial status check
updatePCoolDashboardVisibility();

// Get current sensor readings when the page loads  
window.addEventListener('load', function() {
  getReadings();
  getReadings2(); // Add this line to also load the second set of readings
  getReadings3();
  getReadings4();
});

window.onload = function() {
  fetch('/getIPs')
    .then(response => response.text())
    .then(data => {
      document.getElementById('ip-display').innerHTML = data;
    });
};

function openWifiManager() {
    window.location.href = "/wifimanager";
}

document.getElementById("openWifiBtn").addEventListener("click", function () {
    console.log("Button Clicked!"); // Add this line
    window.open("wifimanager.html", "_self");
});

function openWifiManager() {
    window.location.href = "/settings";
}

document.getElementById("openSettingsBtn").addEventListener("click", function () {
    console.log("Button Clicked!"); // Add this line
    window.open("settings.html", "_self");
});

function openDatabase() {
    window.location.href = "/database";
}

document.getElementById("openDatabaseBtn").addEventListener("click", function () {
    console.log("Button Clicked!"); // Add this line
    window.open("database.html", "_self");
});

// For Powerchart.html
//window.addEventListener('load', function () {
//    // Initialize charts for Powerchart.html
//    getReadings3();
//});
//
//function openPowerChart() {
//        window.location.href = "/Powerchart";
//}
//    
//document.getElementById("openPowerBtn").addEventListener("click", function () {
//    console.log("Button Clicked!"); // Add this line
//    window.open("Powerchart.html", "_self");
//});
//
//
//
// Create Temperature Chart
Highcharts.setOptions({
    time: {
        timezone: 'Europe/London'
    }
});

var chartT = new Highcharts.Chart({
    chart:{
        renderTo:'chart-temperature'
    },
    series: [
        {
            name: 'T amb 1',
            type: 'line',
            color: '#101D42',
            lineWidth: 2,
        },
        {
            name: 'T amb 2',
            type: 'line',
            color: '#00A6A6',
            lineWidth: 2,
        },
        {
            name: 'T amb 3',
            type: 'line',
            color: '#8B2635',
            lineWidth: 2,
        },
        {
            name: 'T S1',
            type: 'line',
            color: 'darkgreen',
            lineWidth: 3,
            marker: {
                symbol: 'circle', // Change to a different symbol
                radius: 3,
                fillColor: 'darkgreen', // Change to dark green
            }
        },
        {
            name: 'T S2',
            type: 'line',
            color: 'violet',
            lineWidth: 3,
            marker: {
                symbol: 'square', // Change to a different symbol
                radius: 3,
                fillColor: 'violet', // Change to violet
            }
        },
        {
            name: 'T S3',
            type: 'line',
            color: 'orange',
            lineWidth: 3,
            marker: {
                symbol: 'triangle', // Change to a different symbol
                radius: 3,
                fillColor: 'orange', // Change to orange
            }
        },
        {
            name: 'T S4',
            type: 'line',
            color: 'cyan',
            lineWidth: 3,
            marker: {
                symbol: 'diamond', // Change to a different symbol
                radius: 3,
                fillColor: 'cyan', // Change to cyan
            }
        },
        {
            name: 'T Box',
            type: 'line',
            color: 'gray',
            marker: {
                symbol: 'star', // Change to a different symbol
                radius: 3,
                fillColor: 'gray', // Change to gray
            }
        },
        {
            name: 'Sky T',
            type: 'line',
            color: '#FF5733', // Choose a color for this series
            yAxis: 1, // Link this series to the second Y-axis
            data: [/* Your data for this series */]
        }
    ],
    title: {
        text: undefined
    },
    xAxis: {
        type: 'datetime',
        dateTimeLabelFormats: { second: '%H:%M:%S' }
    },
    yAxis: [
        {
            title: {
                text: 'Temperature (&#x2103;)'
            }
        },
        {
            title: {
                text: 'Sky Temperature (&#x2103;)' // Customize the label for the second Y-axis
            },
            opposite: true // This makes it display on the right side
        }
    ],
    credits: {
        enabled: false
    }
});

var chartT2 = new Highcharts.Chart({
    chart:{
        renderTo:'chart-humidity'
    },
    series: [
        {
            name: 'Hum #1',
            type: 'line',
            color: '#101D42',
            lineWidth: 3,
            marker: {
                symbol: 'circle',
                radius: 3,
                fillColor: '#101D42',
            }
        },
        {
            name: 'Hum #2',
            type: 'line',
            color: '#00A6A6',
            lineWidth: 3,
            marker: {
                symbol: 'square',
                radius: 3,
                fillColor: '#00A6A6',
            }
        },
        {
            name: 'Hum #3',
            type: 'line',
            color: '#8B2635',
            lineWidth: 3,
            marker: {
                symbol: 'triangle',
                radius: 3,
                fillColor: '#8B2635',
            }
        },
        
        {
            name: 'Sol. Irr.)',
            type: 'area',
            color: '#FF5733', // Choose a color for this series
            yAxis: 1, // Link this series to the second Y-axis
            data: [/* Your data for this series */]
        }
    ],
    title: {
        text: undefined
    },
    xAxis: {
        type: 'datetime',
        dateTimeLabelFormats: { second: '%H:%M:%S' }
    },
    yAxis: [
{
            title: {
                text: 'Relative Humidity (%)'
            },
            min: 0, // Set the minimum value for the Y-axis
            max: 100 // Set the maximum value for the Y-axis
        },
        {
            title: {
                text: 'Solar Irradiance (W/m^2)' // Customize the label for the second Y-axis
            },
            opposite: true // This makes it display on the right side
        }
    ],
    credits: {
        enabled: false
    }
});

var chartT3 = new Highcharts.Chart({
    chart:{
        renderTo:'chart-temperature_power'
    },
    series: [
        {
            name: 'T amb',
            type: 'line',
            color: '#101D42',
            lineWidth: 2,
        },
        {
            name: 'TP S1',
            type: 'line',
            color: 'darkgreen',
            lineWidth: 3,
            marker: {
                symbol: 'circle', // Change to a different symbol
                radius: 3,
                fillColor: 'darkgreen', // Change to dark green
            }
        },
        {
            name: 'TP S2',
            type: 'line',
            color: 'violet',
            lineWidth: 3,
            marker: {
                symbol: 'square', // Change to a different symbol
                radius: 3,
                fillColor: 'violet', // Change to violet
            }
        },
        {
            name: 'T S1',
            type: 'line',
            color: 'orange',
            lineWidth: 3,
            marker: {
                symbol: 'triangle', // Change to a different symbol
                radius: 3,
                fillColor: 'orange', // Change to orange
            }
        },
        {
            name: 'T S2',
            type: 'line',
            color: 'cyan',
            lineWidth: 3,
            marker: {
                symbol: 'diamond', // Change to a different symbol
                radius: 3,
                fillColor: 'cyan', // Change to cyan
            }
        },
    ],
    title: {
        text: undefined
    },
    xAxis: {
        type: 'datetime',
        dateTimeLabelFormats: { second: '%H:%M:%S' }
    },
    yAxis: [
        {
            title: {
                text: 'Temperature (&#x2103;)'
            }
        },
    ],
    credits: {
        enabled: false
    }
});
//
var chartT4 = new Highcharts.Chart({
    chart:{
        renderTo:'chart-power'
    },
    series: [
        {
            name: 'P S1',
            type: 'line',
            color: 'darkgreen',
            lineWidth: 3,
            marker: {
                symbol: 'circle', // Change to a different symbol
                radius: 3,
                fillColor: 'darkgreen', // Change to dark green
            }
        },
        {
            name: 'P S2',
            type: 'line',
            color: 'violet',
            lineWidth: 3,
            marker: {
                symbol: 'square', // Change to a different symbol
                radius: 3,
                fillColor: 'violet', // Change to violet
            }
        },
//        {
//            name: 'P S3',
//            type: 'line',
//            color: 'orange',
//            lineWidth: 3,
//            marker: {
//                symbol: 'triangle', // Change to a different symbol
//                radius: 3,
//                fillColor: 'orange', // Change to orange
//            }
//        },
//        {
//            name: 'P S4',
//            type: 'line',
//            color: 'cyan',
//            lineWidth: 3,
//            marker: {
//                symbol: 'diamond', // Change to a different symbol
//                radius: 3,
//                fillColor: 'cyan', // Change to cyan
//            }
//        },
    ],
    title: {
        text: undefined
    },
    xAxis: {
        type: 'datetime',
        dateTimeLabelFormats: { second: '%H:%M:%S' }
    },
    yAxis: [
        {
            title: {
                text: 'Power (W/m2)'
            }
        },
    ],
    credits: {
        enabled: false
    }
});

function plotTemperature(jsonValue) {

  var keys = Object.keys(jsonValue);
  console.log(keys);
  console.log(keys.length);

  for (var i = 0; i < keys.length; i++){
    var x = (new Date()).getTime();
    x += 1 * 60 * 60 * 1000;
    console.log(x);
    const key = keys[i];
    var y = Number(jsonValue[key]);
    console.log(y);

    if(chartT.series[i].data.length > 100) {
      chartT.series[i].addPoint([x, y], true, true, true);
    } else {
      chartT.series[i].addPoint([x, y], true, false, true);
    }

  }
}

function plotHum(jsonValue) {

  var keys = Object.keys(jsonValue);
  console.log(keys);
  console.log(keys.length);

  for (var i = 0; i < keys.length; i++){
    var x = (new Date()).getTime();
    x += 1 * 60 * 60 * 1000;
    console.log(x);
    const key = keys[i];
    var y = Number(jsonValue[key]);
    console.log(y);

    if(chartT2.series[i].data.length > 100) {
      chartT2.series[i].addPoint([x, y], true, true, true);
    } else {
      chartT2.series[i].addPoint([x, y], true, false, true);
    }

  }
}

function plotTempsPow(jsonValue) {

  var keys = Object.keys(jsonValue);
  console.log(keys);
  console.log(keys.length);

  for (var i = 0; i < keys.length; i++){
    var x = (new Date()).getTime();
    x += 1 * 60 * 60 * 1000;
    console.log(x);
    const key = keys[i];
    var y = Number(jsonValue[key]);
    console.log(y);

    if(chartT3.series[i].data.length > 100) {
      chartT3.series[i].addPoint([x, y], true, true, true);
    } else {
      chartT3.series[i].addPoint([x, y], true, false, true);
    }

  }
}
//
function plotPow(jsonValue) {

  var keys = Object.keys(jsonValue);
  console.log(keys);
  console.log(keys.length);

  for (var i = 0; i < keys.length; i++){
    var x = (new Date()).getTime();
    x += 1 * 60 * 60 * 1000;
    console.log(x);
    const key = keys[i];
    var y = Number(jsonValue[key]);
    console.log(y);

    if(chartT4.series[i].data.length > 100) {
      chartT4.series[i].addPoint([x, y], true, true, true);
    } else {
      chartT4.series[i].addPoint([x, y], true, false, true);
    }

  }
}

// Function to get current readings on the webpage when it loads for the first time
function getReadings(){
  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 100) {
      var myObj = JSON.parse(this.responseText);
      console.log(myObj);      
      plotTemperature(myObj);
    }
  }; 
  xhr.open("GET", "/readings", true);
  xhr.send();
}

function getReadings2(){
  var xhr2 = new XMLHttpRequest();
  xhr2.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 100) {
      var myObj2 = JSON.parse(this.responseText);
      console.log(myObj2);      
      // Do something with myObj2
      plotHum(myObj2);
    }
  }; 
  xhr2.open("GET", "/readings2", true); // Assuming "/readings2" is the endpoint for the second JSON
  xhr2.send();
}

function getReadings3() {
  var xhr3 = new XMLHttpRequest();
  xhr3.onreadystatechange = function () {
    if (this.readyState == 4 && this.status == 100) {
      var myObj3 = JSON.parse(this.responseText);
      console.log(myObj3);
      // Do something with myObj2
      plotTempsPow(myObj3);
    }
  };
  xhr3.open("GET", "/readings3", true);
  xhr3.send();
}

//
function getReadings4(){
  var xhr4 = new XMLHttpRequest();
  xhr4.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 100) {
      var myObj4 = JSON.parse(this.responseText);
      console.log(myObj4);      
      // Do something with myObj4
      plotPow(myObj4);
    }
  }; 
  xhr4.open("GET", "/readings4", true); // Assuming "/readings4" is the endpoint for the second JSON
  xhr4.send();
}

if (!!window.EventSource) {
  var source = new EventSource('/events');
  
  source.addEventListener('open', function(e) {
    console.log("Events Connected");
  }, false);

  source.addEventListener('error', function(e) {
    if (e.target.readyState != EventSource.OPEN) {
      console.log("Events Disconnected");
    }
  }, false);
  
  source.addEventListener('message', function(e) {
    console.log("message", e.data);
  }, false);
  
  source.addEventListener('new_readings', function(e) {
    console.log("new_readings", e.data);
    var myObj = JSON.parse(e.data);
    console.log(myObj);
    plotTemperature(myObj);
  }, false);
}

if (!!window.EventSource) {
  var source2 = new EventSource('/events2');
  
  source2.addEventListener('open', function(f) {
    console.log("Events Connected");
  }, false);

  source2.addEventListener('error', function(f) {
    if (f.target.readyState != EventSource.OPEN) {
      console.log("Events Disconnected");
    }
  }, false);
  
  source2.addEventListener('message', function(f) {
    console.log("message", f.data);
  }, false);
  
   source2.addEventListener('new_readings2', function(f) {
   console.log("new_readings2", f.data);
   var myObj2 = JSON.parse(f.data);
   console.log(myObj2);
   plotHum(myObj2);
  }, false);
}

if (!!window.EventSource) {
  var source3 = new EventSource('/events3');
  
  source3.addEventListener('open', function(f) {
    console.log("Events Connected");
  }, false);

  source3.addEventListener('error', function(f) {
    if (f.target.readyState != EventSource.OPEN) {
      console.log("Events Disconnected");
    }
  }, false);
  
  source3.addEventListener('message', function(f) {
    console.log("message", f.data);
  }, false);
  
   source3.addEventListener('new_readings3', function(f) {
   console.log("new_readings3", f.data);
   var myObj3 = JSON.parse(f.data);
   console.log(myObj3);
   plotTempsPow(myObj3);
  }, false);
}

if (!!window.EventSource) {
  var source4 = new EventSource('/events4');
  
  source4.addEventListener('open', function(f) {
    console.log("Events Connected");
  }, false);

  source4.addEventListener('error', function(f) {
    if (f.target.readyState != EventSource.OPEN) {
      console.log("Events Disconnected");
    }
  }, false);
  
  source4.addEventListener('message', function(f) {
    console.log("message", f.data);
  }, false);
  
   source4.addEventListener('new_readings4', function(f) {
   console.log("new_readings4", f.data);
   var myObj4 = JSON.parse(f.data);
   console.log(myObj4);
   plotPow(myObj4);
  }, false);
}