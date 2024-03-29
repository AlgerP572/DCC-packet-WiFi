var gateway = `ws://${window.location.hostname}/wstrackmeasuring`;
var websocket;
var configData;
var statsData;

function initWebSocket() {
  console.log('Trying to open a Track Measuring WebSocket connection...');
  websocket = new WebSocket(gateway);
  websocket.onopen    = onOpen;
  websocket.onclose   = onClose; 
}

function onOpen(event) {
  console.log('Connection Track Measuring opened');  
}
function onClose(event) {
  console.log('Connection Track Measuring closed');
  setTimeout(initWebSocket, 2000);
}

window.addEventListener('load', onLoad);

var chart;
var currentX = 0;

function plotSpeed(jsonValue) {

  var data = jsonValue.Data;
  console.log(data);
  var x = ++currentX; // (new Date()).getTime();
  var y = Number(data.Speed);

  // By definition running (will auto update on reconnect)
  document.getElementById("btnStart").innerHTML = "Stop";
  document.getElementById("angle").innerHTML = data.AxisAngle.toFixed(2);
  document.getElementById("measuredspeed").innerHTML = y.toFixed(2);
  document.getElementById("direction").innerHTML = y > 0 ? "Forward" : y< 0 ? "Backward" : "Parked";
  document.getElementById("absdistance").innerHTML = data.AbsDist.toFixed(2);
  document.getElementById("reldistance").innerHTML = data.RelDist.toFixed(2);
  document.getElementById("samplingrate").innerHTML = (1000000.0 / data.SamplingRate).toFixed(2);

  var scaleSpeed = (y  * 36 * Number(configData.ScaleList[configData.ScaleIndex].Scale)) / 10000; //[km/h]
  document.getElementById("scalespeed").innerHTML = scaleSpeed.toFixed(2);

     // if(chart.data.labels.length > 400) {
      chart.data.datasets[0].data.push({ x: x, y: y });
      chart.data.datasets[1].data.push({ x: x, y: data.ScaleSpeed });
      chart.data.datasets[2].data.push({ x: x, y: data.Accel });

      // useful for crosscheck of FW speed.
//   chart.data.datasets[2].data.push({ x: x, y: scaleSpeed });
       
    //} else {
    //  chartT.series[0].addPoint([x, y], true, false, true);
   // }
  chart.update();
}

function  UpdateUI()
{
  document.getElementById("wheeldiameter").innerHTML = configData.WheelDia.toFixed(2);
  document.getElementById("scale").innerHTML = configData.ScaleList[configData.ScaleIndex].Name;
  document.getElementById("scaleunit").innerHTML = "1:" + configData.ScaleList[configData.ScaleIndex].Scale;
  document.getElementById("reversedirection").innerHTML = configData.ReverseDir;

  chart.options.scales['y1'].title.text = 'Scale (' + configData.ScaleList[configData.ScaleIndex].Name + "  1:" + configData.ScaleList[configData.ScaleIndex].Scale + ') Speed [km/h]';
  chart.update();
}

function  UpdateFooter()
{
  document.getElementById("datetime").innerHTML = statsData.systime; 
  document.getElementById("ipaddress").innerHTML = statsData.ipaddress;
  document.getElementById("fwversion").innerHTML = statsData.version;
  
  document.getElementById("systemuptime").innerHTML = formatTime(Math.trunc(statsData.uptime/1000)); 
  document.getElementById("signalstrength").innerHTML = statsData.sigstrength  + " dBm";
  document.getElementById("ramflash").innerHTML = statsData.freemem + " / " + statsData.freedisk + " Bytes";

  document.getElementById("coretemp").innerHTML = statsData.temp.toFixed(2) + "\u00B0C"; 
  document.getElementById("accesspoint").innerHTML = statsData.apname;
  document.getElementById("batvoltage").innerHTML = statsData.ubat.toFixed(2) + " V";

  document.getElementById("extvoltage").innerHTML = statsData.uin.toFixed(2) + " V";
  document.getElementById("batcurrent").innerHTML = statsData.ibat.toFixed(2) + " mA";  
}

if (!!window.EventSource) {
  var source = new EventSource('/eventstrackmeasuring');

  source.addEventListener('open', function(e) {
    console.log("Track Measuring Events Connected");    
  }, false);

  source.addEventListener('error', function(e) {
    if (e.target.readyState != EventSource.OPEN) {
      console.log("Track Measuring Events Disconnected");      
    }
  }, false);

  source.addEventListener('message', function(e) {
    //console.log("Track Measuring message", e.data);
  }, false);

  source.addEventListener('SpeedData', function(e) {
    //console.log("Track Measuring new_readings", e.data);
    var myObj = JSON.parse(e.data);
//    console.log(myObj);
    plotSpeed(myObj);
  }, false);

  source.addEventListener('CfgData', function(e) {
    //console.log("Track Measuring new_readings", e.data);
    var myObj = JSON.parse(e.data);
    console.log(myObj);
    configData = myObj["Data"];
    UpdateUI();
  }, false);

  source.addEventListener('STATS', function(e) {
    //console.log("Track Measuring new_readings", e.data);
    var myObj = JSON.parse(e.data);
    console.log(myObj);
    statsData = myObj["Data"];
    UpdateFooter();
  }, false);
}

function onLoad(event) {
  initWebSocket();
  getConfig();

  // This requires the prvious two.
  initChart();  
}

function initChart(){
  ctx = document.getElementById("chart-speed-data").getContext("2d");
  chart = new Chart(ctx, {
    type: "scatter",
    exportEnabled: true,
    data: {
      datasets: [{
        label: "Measured Speed [mm/s]",
        borderWidth: 1,
        pointRadius: 2,
        backgroundColor: '#80C080',
        borderColor: '#80C080',
        yAxisID: 'y',
        showLine: true
      },
      {
        label:"Scale Speed [km/h]",
        borderWidth: 1,
        pointRadius: 2,
        backgroundColor: '#8080C0',
        borderColor: '#8080C0',
        yAxisID: 'y1',
        showLine: true
      },
    //   {
    //     label:"Scale FW [km/h]",
    //     borderWidth: 1,
    //     pointRadius: 2,
    //     backgroundColor: '#C08080',
    //     borderColor: '#C08080',
    //     yAxisID: 'y1',
    //     showLine: true
    //   },
      {
        label:"Accel [km/h s]",
        borderWidth: 1,
        pointRadius: 2,
        backgroundColor: '#C08080',
        borderColor: '#C08080',
        yAxisID: 'y2',
        showLine: true
      }],
    },
    options: {
      borderWidth: 3,
      borderColor: ["rgba(255, 99, 132, 128)"],
      scales: {
        x: {
            title: {
                display: true,
                text: 'Samples [count]'
                }
        }, 
        y: {
            title: {
                display: true,
                text: 'Measured Speed [mm/s]'
            },
            type: 'linear',
            display: true,
            position: 'left',
        },
        y1: {
            title: {
                display: true,
                text: 'Scale Speed [km/h]'
            },
            type: 'linear',
            display: true,
            position: 'right',
  
            // grid line settings
            grid: {
            drawOnChartArea: false, // only want the grid lines for one axis to show up
            },
        },
        y2: {
            title: {
                display: true,
                text: 'Scale Accel [km/h s]'
            },
            type: 'linear',
            display: true,
            position: 'right',
  
            // grid line settings
            grid: {
            drawOnChartArea: false, // only want the grid lines for one axis to show up
            },
        }
      }           
    },    
  });

    CanvasJSDataAsCSV(chart, "Track_Data", "buttonCsv");
}

function startMeasuring(sender)
{
  console.log("Click", sender);
	if (sender.innerHTML == "Start")
	{		
		sendMessage(websocket, "{\"Cmd\":\"SetSensor\", \"SubCmd\":\"RepRate\",\"Val\":500}");
		document.getElementById("btnStart").innerHTML = "Stop";
	}
	else
	{
		sendMessage(websocket,"{\"Cmd\":\"SetSensor\", \"SubCmd\":\"RepRate\",\"Val\":0}");
		document.getElementById("btnStart").innerHTML = "Start";
	}
}

function resetDistance(sender)
{
	sendMessage(websocket, "{\"Cmd\":\"SetSensor\", \"SubCmd\":\"ClearDist\"}");	
}

function getConfig()
{
  sendMessage(websocket, "{\"Cmd\":\"CfgData\", \"Type\":\"TODO:ID\", \"FileName\":\"phcfg.cfg\"}");
  sendMessage(websocket, "{\"Cmd\":\"STATS\"}");
}

function waitForOpenConnection(socket) {
  return new Promise((resolve, reject) => {
      const maxNumberOfAttempts = 10
      const intervalTime = 200 //ms

      let currentAttempt = 0
      const interval = setInterval(() => {
          if (currentAttempt > maxNumberOfAttempts - 1) {
              clearInterval(interval)
              reject(new Error('Maximum number of attempts exceeded'))
          } else if (socket.readyState === socket.OPEN) {
              clearInterval(interval)
              resolve()
          }
          currentAttempt++
      }, intervalTime)
  })
}

async function sendMessage (socket, msg)
{
  if (socket.readyState !== socket.OPEN) {
      try {
          await waitForOpenConnection(socket)
          socket.send(msg)
      } catch (err) { console.error(err) }
  } else {
      socket.send(msg)
  }
}

function formatTime(seconds) {
  return [
      parseInt(seconds / 60 / 60),
      parseInt(seconds / 60 % 60),
      parseInt(seconds % 60)
  ]
      .join(":")
      .replace(/\b(\d)\b/g, "0$1")
}
