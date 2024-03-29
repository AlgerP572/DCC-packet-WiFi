// Get current sensor readings when the page loads
window.addEventListener('load', onLoad);
var chart;

//Plot temperature in the temperature chart
function plotTemperature(jsonValue) {

  var keys = Object.keys(jsonValue);
  console.log(keys);
  console.log(keys.length);

  var x = (new Date()).getTime();
  chart.data.labels.push(x);
  chart.data.datasets[0].data.push(jsonValue["sensor1"]);
  chart.data.datasets[1].data.push(jsonValue["sensor2"]);
  chart.data.datasets[2].data.push(jsonValue["sensor3"]);
  chart.data.datasets[3].data.push(jsonValue["sensor4"]);
  chart.update();
}

// Function to get current readings on the webpage when it loads for the first time
function getReadings(){
  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      var myObj = JSON.parse(this.responseText);
      console.log(myObj);
      plotTemperature(myObj);
    }
  };
  xhr.open("GET", "/readings", true);
  xhr.send();
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
   
 source.addEventListener('temperature', function(e) {
    console.log("temperature", e.data);
    document.getElementById("temp").innerHTML = e.data;
   }, false);
   
   source.addEventListener('humidity', function(e) {
    console.log("humidity", e.data);
    document.getElementById("hum").innerHTML = e.data;
   }, false);
   
   source.addEventListener('pressure', function(e) {
    console.log("pressure", e.data);
    document.getElementById("pres").innerHTML = e.data;
   }, false);
}

function onLoad(event) {
  initChart();
}

function initChart(){
  ctx = document.getElementById("chart-speed-data").getContext("2d");
  chart = new Chart(ctx, {
    type: "line",
    data: {
      datasets: [{
         label: "Measured Speed [mm / s]",
         borderWidth: 1,
         pointRadius: 2
        },
        {
         label: "Measured Speed [mm / s]",
         borderWidth: 1,
         pointRadius: 2
        },
        {
          label: "Measured Speed [mm / s]",
          borderWidth: 1,
          pointRadius: 2
        },
        {
          label: "Measured Speed [mm / s]",
          borderWidth: 1,
          pointRadius: 2
        }],
    },
    options: {
      borderWidth: 3,
      borderColor: ["rgba(255, 99, 132, 128)"]      
    },
  });
}