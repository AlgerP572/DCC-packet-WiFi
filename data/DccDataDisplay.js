var gateway = `ws://${window.location.hostname}/wsdccdata`;
var websocket;
var configData;
var statsData;

function initWebSocket() {
  console.log('Trying to open a Dcc Data WebSocket connection...');
  websocket = new WebSocket(gateway);
  websocket.onopen    = onOpen;
  websocket.onclose   = onClose; 
}

function onOpen(event) {
  console.log('Connection Dcc Data opened');  
}
function onClose(event) {
  console.log('Connection Dcc Data closed');
  setTimeout(initWebSocket, 2000);
}

window.addEventListener('load', onLoad);

var chart;

function  UpdateDCCStats(jsonValue)
{
    var data = jsonValue.Stats;
    console.log(data);

    // These counts are for half-bits, so divide by two.
    document.getElementById("bitCount").innerHTML = "Bit Count / " + data.refreshTime + " sec= " + (data.count / 2).toFixed(0) + " (Zeros= " + (data.count0 / 2).toFixed(0) + ", Ones= " + (data.count1 / 2).toFixed(0) + "), Glitches= " + data.glitchCount;
        
    document.getElementById("packetStats").innerHTML = "Valid= " + data.packetCount + ", NMRA out of spec= " + data.outOfSpecRejectionCount + ", Checksum Error= " + data.checksumError + ", Lost= " + data.countLostPackets + ", Long= " + data.countLongPackets;
    document.getElementById("zeroBits").innerHTML = "0 half-bit length (us): " + (data.total0 / data.count0).toFixed(1) + " ("+ data.min0 + " - " + data.max0 + ") delta < " + data.max0BitDelta;
    document.getElementById("oneBits").innerHTML = "1 half-bit length (us): " + (data.total1 / data.count1).toFixed(1) + " ("+ data.min1 + " - " + data.max1 + ") delta < " + data.max1BitDelta;
    document.getElementById("cpu").innerHTML = "IRC Duration (us): " + (data.ircTime / data.count).toFixed(1) + " ("+ data.minIrcTime +"-"+ data.maxIrcTime +"),  CPU load:" + data.cpuLoad.toFixed(1) + "%"
}

function  UpdateDCCPackets(jsonValue)
{
  var data = jsonValue.DccBytes;
  console.log(data);

  document.getElementById("dccPacketOne").innerHTML = "1:  " + data.PacketZero;
  document.getElementById("dccPacketTwo").innerHTML = "2:  " + data.PacketOne;
  document.getElementById("dccPacketThree").innerHTML = "3:  " + data.PacketTwo;
  document.getElementById("dccPacketFour").innerHTML = "4:  " + data.PacketThree;
  document.getElementById("dccPacketFive").innerHTML = "5:  " + data.PacketFour;
  document.getElementById("dccPacketSix").innerHTML = "6:  " + data.PacketFive;
  document.getElementById("dccPacketSeven").innerHTML = "7:  " + data.PacketSix;
  document.getElementById("dccPacketEight").innerHTML = "8:  " + data.PacketSeven;

  var firstZeroBit = 0;

  // Zero is num intervals
  for(var j = 1; j < data.PacketZeroInterval[0]; j++)
  {
        if(data.PacketZeroInterval[j] <= 80)
            continue;
         firstZeroBit = j;
         break;
  }

  if(firstZeroBit < 18)
    return; // Invalid packet. Should never get here as DCC inspector does not allow this.

  // only display previous 8 bits of preamble.
  firstZeroBit++;
  var psuedoTime = 0;
  chart.data.datasets[0].data.length = 0;
  chart.data.datasets[0].data.push({ x: psuedoTime, y: -0.5});

  // Extract preamble timings.
  var i = 0;
  for(i = firstZeroBit - 17; i < firstZeroBit; i+=2)
  {    
    psuedoTime += data.PacketZeroInterval[i];    
    chart.data.datasets[0].data.push({ x: psuedoTime, y: -0.5});    
    chart.data.datasets[0].data.push({ x: psuedoTime, y: -4.5});

    psuedoTime += data.PacketZeroInterval[i + 1];
    chart.data.datasets[0].data.push({ x: psuedoTime, y: -4.5});
    chart.data.datasets[0].data.push({ x: psuedoTime, y: -0.5});
  }

  // First DCC byte (after preamble).
  firstZeroBit++;

    var dataByteCount = 1;
    while(firstZeroBit < data.PacketZeroInterval.length)
    {
        psuedoTime = 0;
        chart.data.datasets[dataByteCount].data.length = 0;
        chart.data.datasets[dataByteCount].data.push({ x: psuedoTime, y: -0.5 - dataByteCount * 5.0});

        for(i = firstZeroBit; i < firstZeroBit + 18; i+=2)
        {        
            psuedoTime += data.PacketZeroInterval[i];    
            chart.data.datasets[dataByteCount].data.push({ x: psuedoTime, y: -0.5 - dataByteCount * 5.0});    
            chart.data.datasets[dataByteCount].data.push({ x: psuedoTime, y: -4.5 - dataByteCount * 5.0});

            psuedoTime += data.PacketZeroInterval[i + 1];
            chart.data.datasets[dataByteCount].data.push({ x: psuedoTime, y: -4.5 - dataByteCount * 5.0});
            chart.data.datasets[dataByteCount].data.push({ x: psuedoTime, y: -0.5 - dataByteCount * 5.0});
        }

        // Next DCC byte.
        firstZeroBit+=18;
        dataByteCount++;
    }

    for(var j = dataByteCount; j < chart.data.datasets.length; j++)
    {
        chart.data.datasets[j].data.length = 0;
    }

  chart.update();
}

function  UpdateUI()
{  
  chart.options.scales['y1'].title.text = 'Scale (' + configData.ScaleList[configData.ScaleIndex].Name + "  1:" + configData.ScaleList[configData.ScaleIndex].Scale + ') Speed [km/h]';
  chart.update();
}

function  UpdateFooter()
{
  document.getElementById("datetime").innerHTML = statsData.systime; 
  document.getElementById("ipaddress").innerHTML = statsData.ipaddress;
  document.getElementById("fwversion").innerHTML = statsData.version;
  
  document.getElementById("systemuptime").innerHTML = formatTime(Math.trunc(statsData.uptime/1000)) + ' / ' + statsData.cpuload.toFixed(2) + '%'; 
  document.getElementById("signalstrength").innerHTML = statsData.sigstrength  + " dBm";
  document.getElementById("ramflash").innerHTML = statsData.freemem + " / " + statsData.freeflash + " Bytes";

  document.getElementById("coretemp").innerHTML = statsData.temp.toFixed(2) + "\u00B0C"; 
  document.getElementById("accesspoint").innerHTML = statsData.apname;
  document.getElementById("spiffs").innerHTML = statsData.freedisk + " / " + statsData.totaldisk + "Bytes";
}

if (!!window.EventSource) {
  var source = new EventSource('/eventsdccdata');

  source.addEventListener('open', function(e) {
    console.log("Track Dcc Data Connected");    
  }, false);

  source.addEventListener('error', function(e) {
    if (e.target.readyState != EventSource.OPEN) {
      console.log("Track Dcc Data Disconnected");      
    }
  }, false);

  source.addEventListener('DccStats', function(e) {    
    var myObj = JSON.parse(e.data);
    UpdateDCCStats(myObj);
  }, false);

  source.addEventListener('PacketBytes', function(e) {
    var myObj = JSON.parse(e.data);
    UpdateDCCPackets(myObj);
  }, false);

  source.addEventListener('CfgData', function(e) {
    var myObj = JSON.parse(e.data);
    console.log(myObj);
    configData = myObj["Data"];
    UpdateUI();
  }, false);

  source.addEventListener('STATS', function(e) {
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
        label: "Preamble",
        borderWidth: 1,
        pointRadius: 2,
        backgroundColor: '#80C080',
        borderColor: '#80C080',
        yAxisID: 'y',
        showLine: true
      },
      {
        label:"Address",
        borderWidth: 1,
        pointRadius: 2,
        backgroundColor: '#8080C0',
        borderColor: '#8080C0',
        yAxisID: 'y',
        showLine: true
      },   
      {
        label:"Instruction",
        borderWidth: 1,
        pointRadius: 2,
        backgroundColor: '#C08080',
        borderColor: '#C08080',
        yAxisID: 'y',
        showLine: true
      },
      {
        label:"Error",
        borderWidth: 1,
        pointRadius: 2,
        backgroundColor: '#80C0C0',
        borderColor: '#80C0C0',
        yAxisID: 'y',
        showLine: true
      },
      {
        label:"Byte4",
        borderWidth: 1,
        pointRadius: 2,
        backgroundColor: '#C080C0',
        borderColor: '#C080C0',
        yAxisID: 'y',
        showLine: true
      },
      {
        label:"Byte5",
        borderWidth: 1,
        pointRadius: 2,
        backgroundColor: '#C0C080',
        borderColor: '#C0C080',
        yAxisID: 'y',
        showLine: true
      },
      {
        label:"Byte6",
        borderWidth: 1,
        pointRadius: 2,
        backgroundColor: '#C0A080',
        borderColor: '#C0A080',
        yAxisID: 'y',
        showLine: true
      }],
    },
    options: {
      animation: false,
      borderWidth: 3,
      borderColor: ["rgba(255, 99, 132, 128)"],
      scales: {
        x: {
            title: {
                display: true,
                text: 'Interval time [\u03BCs]'
                }
        }, 
        y: {
            title: {
                display: true,
                text: 'Relative Logic level [hi/lo]'
            },
            type: 'linear',
            display: true,
            position: 'left',
        }
      }           
    },    
  });

    CanvasJSDataAsCSV(chart, "Track_Data", "buttonCsv");
}

function capture(sender)
{  
  console.log("Click", sender);
  sendMessage(websocket, "{\"Cmd\":\"SetSensor\", \"SubCmd\":\"Capture\"}");	
}

function continuous(sender)
{
    console.log("Click", sender);
	sendMessage(websocket, "{\"Cmd\":\"SetSensor\", \"SubCmd\":\"Continuous\"}");	
}

function stop(sender)
{
    console.log("Click", sender);
	sendMessage(websocket, "{\"Cmd\":\"SetSensor\", \"SubCmd\":\"Stop\"}");	
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
