var gateway = `ws://${window.location.hostname}/wswifilog`;
var websocket;
var configData;
var statsData;
var editor
var autoscroll = true;
var clear = false;

require.config( { paths: { 'vs': 'https://microsoft.github.io/monaco-editor/node_modules/monaco-editor/min/vs' } });
require(['vs/editor/editor.main'], function() {
    editor = monaco.editor.create(document.getElementById('editor'),
    {
        theme: 'logview',
        language: 'log',
        automaticLayout: true
    });
});

function initWebSocket() {
  console.log('Trying to open a WiFi log WebSocket connection...');
  websocket = new WebSocket(gateway);
  websocket.onopen    = onOpen;
  websocket.onclose   = onClose; 
}

function onOpen(event) {
  console.log('Connection WiFi log opened');  
}
function onClose(event) {
  console.log('Connection WiFi log closed');
  setTimeout(initWebSocket, 2000);
}

window.addEventListener('load', onLoad);

var currentX = 0;

function UpdateConfig()
{
    var toSerial = document.getElementById("seriallog");
    toSerial.checked = configData.ToSerialPort;
    serialLogClick(toSerial);

    var toWifi = document.getElementById("wifilog");
    toWifi.checked = configData.ToWifi;
    wifiLogClick(toWifi);
    
    var debugLog = document.getElementById("debuglog");
    debugLog.checked = configData.LevelDebug;
    debugLogClick(debugLog);

    var loopLog = document.getElementById("looplog");
    loopLog.checked = configData.LevelLoop;
    loopLogClick(loopLog);

    var errorLog = document.getElementById("errorlog");
    errorLog.checked = configData.LevelError;
    errorLogClick(errorLog);

    var infoLog = document.getElementById("infolog");
    infoLog.checked = configData.LevelInfo;
    infoLogClick(infoLog);

    var timeTag = document.getElementById("timelog");
    timeTag.checked = configData.TagTime;
    timeLogClick(timeTag);

    var levelTag = document.getElementById("levellog");
    levelTag.checked = configData.TagLevel;
    levelLogClick(levelTag);
}

function AppendCommands(jsonArray)
{
    var comboOptions = document.getElementById("combo-options");

    for(var cmd in jsonArray)
    {
        var option = document.createElement('option');
        option.value = jsonArray[cmd];
        comboOptions.appendChild(option);
    }
}

function  UpdateLogLines(logLines)
{
    var lineCount = editor.getModel().getLineCount();
    var lastLineLength = editor.getModel().getLineMaxColumn(lineCount);
    var range = range = new monaco.Range(
        clear == false ? lineCount : 0,
        clear == false ? lastLineLength : 0,
        lineCount,
        lastLineLength);
    
    if(clear == true)
    {
        clear = false;
    }

    editor.executeEdits('', [{ range: range, text: logLines }]);

    // limit to 15000 lines.
    lineCount = editor.getModel().getLineCount();
    if(lineCount > 15000)
    {
        var numToRemove = lineCount - 15000;
        lastLineLength = lastLineLength = editor.getModel().getLineMaxColumn(numToRemove);
        range = range = new monaco.Range(
            0,
            0,
            numToRemove,
            lastLineLength);

        editor.executeEdits('', [{ range: range, text: null }]);
    }

    if(autoscroll)
    {
        lineCount = editor.getModel().getLineCount();
        editor.revealLine(lineCount);
    }
}

function UpdateDisplayOptions(options)
{

    if(options.hasOwnProperty("Serial"))
    {
        var debugLog = document.getElementById("seriallog");
        debugLog.checked = options.Serial;
    }
    if(options.hasOwnProperty("Wifi"))
    {
        var debugLog = document.getElementById("wifilog");
        debugLog.checked = options.Wifi;
    }

    if(options.hasOwnProperty("Debug"))
    {
        var debugLog = document.getElementById("debuglog");
        debugLog.checked = options.Debug;
    }
    if(options.hasOwnProperty("Loop"))
    {
        var debugLog = document.getElementById("looplog");
        debugLog.checked = options.Loop;
    }
    if(options.hasOwnProperty("Error"))
    {
        var debugLog = document.getElementById("errorlog");
        debugLog.checked = options.Error;
    }
    if(options.hasOwnProperty("Info"))
    {
        var debugLog = document.getElementById("infolog");
        debugLog.checked = options.Info;
    }

    if(options.hasOwnProperty("TimeStamps"))
    {
        var debugLog = document.getElementById("timelog");
        debugLog.checked = options.TimeStamps;
    }
    if(options.hasOwnProperty("PrintLevel"))
    {
        var debugLog = document.getElementById("levellog");
        debugLog.checked = options.PrintLevel;
    }
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
  var source = new EventSource('/eventswifilog');

  source.addEventListener('open', function(e) {
    console.log("WiFi log Events Connected");    
  }, false);

  source.addEventListener('error', function(e) {
    if (e.target.readyState != EventSource.OPEN) {
      console.log("WiFi log Events Disconnected");      
    }
  }, false);

  source.addEventListener('message', function(e) {
    //console.log("WiFi log message", e.data);
  }, false);

  source.addEventListener('CfgData', function(e) {
    //console.log("Track Measuring new_readings", e.data);
    var myObj = JSON.parse(e.data);
    console.log(myObj);
    configData = myObj["Data"];
    UpdateConfig();
  }, false);

  source.addEventListener('APPENDCOMMANDS', function(e)
  {
    var myObj = JSON.parse(e.data);
    console.log(myObj);
    configData = myObj["Data"];
    AppendCommands(configData.commands)
    
  }, false); 

  source.addEventListener('DISPLAYUPDATE', function(e)
  {
    var myObj = JSON.parse(e.data);
    console.log(myObj);
    optionsData = myObj["Data"];
    UpdateDisplayOptions(optionsData);
    
  }, false);

  source.addEventListener('STATS', function(e) {
    //console.log("WiFi log new_readings", e.data);
    var myObj = JSON.parse(e.data);
    console.log(myObj);
    statsData = myObj["Data"];
    UpdateFooter();
  }, false);

  source.addEventListener('WebEventLog', function(e) {
    //console.log("WiFi log new_readings", e.data);
    var myObj = JSON.parse(e.data);
    console.log(myObj);

    UpdateLogLines(myObj.Lines.lines);
  }, false);
}

function onLoad(event) {
  initWebSocket();
  getConfig();
}

function serialLogClick(sender)
{
    var command = 'serial:' + (sender.checked ? 'on' : 'off');
    sendMessage(websocket, "{\"Cmd\":\"LogCmd\", \"SubCmd\":\"" + command + "\"}");
}

function wifiLogClick(sender)
{
    var command = 'wifi:' + (sender.checked ? 'on' : 'off');
    sendMessage(websocket, "{\"Cmd\":\"LogCmd\", \"SubCmd\":\"" + command + "\"}");
}

function debugLogClick(sender)
{
    var command = 'debug:' + (sender.checked ? 'on' : 'off');
    sendMessage(websocket, "{\"Cmd\":\"LogCmd\", \"SubCmd\":\"" + command + "\"}");
}

function loopLogClick(sender)
{
    var command = 'loop:' + (sender.checked ? 'on' : 'off');
    sendMessage(websocket, "{\"Cmd\":\"LogCmd\", \"SubCmd\":\"" + command + "\"}");
}

function errorLogClick(sender)
{
    var command = 'error:' + (sender.checked ? 'on' : 'off');
    sendMessage(websocket, "{\"Cmd\":\"LogCmd\", \"SubCmd\":\"" + command + "\"}");
}

function infoLogClick(sender)
{
    var command = 'info:' + (sender.checked ? 'on' : 'off');
    sendMessage(websocket, "{\"Cmd\":\"LogCmd\", \"SubCmd\":\"" + command + "\"}");
}

function timeLogClick(sender)
{
    var command = 'timestamps:' + (sender.checked ? 'on' : 'off');
    sendMessage(websocket, "{\"Cmd\":\"LogCmd\", \"SubCmd\":\"" + command + "\"}");
}

function levelLogClick(sender)
{
    var command = 'printlevel:' + (sender.checked ? 'on' : 'off');
    sendMessage(websocket, "{\"Cmd\":\"LogCmd\", \"SubCmd\":\"" + command + "\"}");
}

function autoScrollClick(sender)
{
    console.log("AutoScrollClick", sender);    
    autoscroll = sender.checked;
}

function clearClick(sender)
{
    console.log("ClearClick", sender);    
    clear = true;
}

function send(sender)
{
    console.log("Click", sender);
    var cmd = document.getElementById("command").value;
    
	sendMessage(websocket, "{\"Cmd\":\"LogCmd\", \"SubCmd\":\"" + cmd + "\"}");	
}

function getConfig()
{
  sendMessage(websocket, "{\"Cmd\":\"CfgData\", \"Type\":\"TODO:ID\", \"FileName\":\"log.cfg\"}");
  sendMessage(websocket, "{\"Cmd\":\"APPENDCOMMANDS\"}");
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
