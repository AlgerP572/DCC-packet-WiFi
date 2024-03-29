var gateway = `ws://${window.location.hostname}/wsspeedmagic`;
var websocket;
var configData;
var statsData;

var	locoAddr = -1;
var locoAddrValid = false;

var throttleDef = {"DataType" : "ThrottleDef", "Version" : "1.0", "ProfName":"new profile", "VMax":50, "AtStep":100, "SpeedSteps":128, "GraphData" : {}, "FileName":""};
var throttleProfileData;
var trackLen = 13500;
var maxTestSpeed = 50;
var fwTrim = 0;
var bwTrim = 0;

var cvVal_2 = 0;
var cvVal_3 = 0;
var cvVal_4 = 0;
var cvVal_5 = 0;
var cvVal_6 = 0;
var cvVal_29 = 0;

function initWebSocket() {
  console.log('Trying to open a Speed Magic WebSocket connection...');
  websocket = new WebSocket(gateway);
  websocket.onopen    = onOpen;
  websocket.onclose   = onClose; 
}

function onOpen(event) {
  console.log('Connection Speed Magic opened');  
}
function onClose(event) {
  console.log('Connection Speed Magic closed');
  setTimeout(initWebSocket, 2000);
}

window.addEventListener('load', onLoad);

var profileChart;
var currentX = 0;

function plotSpeedTable(jsonValue)
{
    var data = jsonValue.Data;
    console.log(data);    

    for(let i = 0; i <= data.NumSteps; i++)
    {
        profileChart.data.datasets[1].data[i] = { x: i, y: data.fw[i]};
        profileChart.data.datasets[2].data[i] = { x: i, y: data.bw[i]};

        var scaleSpeed = (data.fw[i] * 36 * Number(configData.ScaleList[configData.ScaleIndex].Scale)) / 10000; //[km/h]
        profileChart.data.datasets[3].data[i] = { x: i, y: scaleSpeed};

        scaleSpeed = (data.bw[i] * 36 * Number(configData.ScaleList[configData.ScaleIndex].Scale)) / 10000; //[km/h]
        profileChart.data.datasets[4].data[i] = { x: i, y: scaleSpeed};
    }

    // new data set reset speed table
    profileChart.data.datasets[5].data = [];

    var fwFit = regression.polynomial(profileChart.data.datasets[1].data, { order: 7, precision: 16 });
    profileChart.data.datasets[6].data = fwFit.points;

    var bwFit = regression.polynomial(profileChart.data.datasets[2].data, { order: 7, precision: 16 });
    profileChart.data.datasets[7].data = bwFit.points;
     
    profileChart.update();

    if(data.Result != 0)
    { 
        if(data.Result == 1)       
            alert("Measurement complete.");
        if(data.Result == 2)       
            alert("Only one direction achieved vMax.");
        if(data.Result == 3)       
            alert("Could not complete speed steps in tracklength distance. Rerun test with longer tracklegnth");
        if(data.Result == 4)       
            alert("User stop.");
    }
}

function  UpdateUI()
{
  document.getElementById("speedstep").innerHTML = configData.SpeedStep;

  document.getElementById("wheeldiameter").innerHTML = configData.WheelDia.toFixed(2);  
  document.getElementById("scaleunit").innerHTML = "1:" + configData.ScaleList[configData.ScaleIndex].Scale;
 
  profileChart.options.scales['y1'].title.text = 'Scale (' + configData.ScaleList[configData.ScaleIndex].Name + "  1:" + configData.ScaleList[configData.ScaleIndex].Scale + ') Speed [km/h]';
  profileChart.update();
}

function plotSpeed(jsonValue) {
    var data = jsonValue.Data;
    console.log(data);    

    document.getElementById("speedstep").innerHTML = data.SpeedStep;
    document.getElementById("direction").innerHTML = ((data.DirF & 0x20) == 0x20) ? "For" : "Rev";   
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
  var source = new EventSource('/eventsspeedmagic');

  source.addEventListener('open', function(e) {
    console.log("Speed Magic Events Connected");    
  }, false);

  source.addEventListener('error', function(e) {
    if (e.target.readyState != EventSource.OPEN) {
      console.log("Speed Magic Events Disconnected");      
    }
  }, false);

  source.addEventListener('message', function(e) {
    //console.log("Speed Magic message", e.data);
  }, false);

  source.addEventListener('SpeedData', function(e) {
    //console.log("Track Measuring new_readings", e.data);
    var myObj = JSON.parse(e.data);
//    console.log(myObj);
    plotSpeed(myObj);
  }, false);

  source.addEventListener('SpeedTableData', function(e) {
    //console.log("Speed Magic new_readings", e.data);
    var myObj = JSON.parse(e.data);
//    console.log(myObj);
    plotSpeedTable(myObj);
  }, false);

  source.addEventListener('CfgData', function(e) {
    //console.log("Speed Magic new_readings", e.data);
    var myObj = JSON.parse(e.data);
    console.log(myObj);
    configData = myObj["Data"];
    UpdateUI();
  }, false);

  source.addEventListener('STATS', function(e) {
    //console.log("Speed Magic new_readings", e.data);
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
  initProfileChart();  
}

function initProfileChart(){
  ctx = document.getElementById("chart-speed-data").getContext("2d");
  profileChart = new Chart(ctx, {
    type: "scatter",   
    data: {
      datasets: [{
        label: "Throttle Profile [km/h]",
        borderWidth: 1,
        pointRadius: 0,
        backgroundColor: '#C08080',
        borderColor: '#C08080',
        xAxisID: 'x',
        yAxisID: 'y1',
        showLine: true,
        data: []
      },
      {
        label: "fw Speed [mm/s]",
        borderWidth: 1,
        pointRadius: 1,
        backgroundColor: '#80C080',
        borderColor: '#80C080',
        xAxisID: 'x',
        yAxisID: 'y',
        showLine: false,
        data: []
      },
      {
        label: "bw Speed [mm/s]",
        borderWidth: 1,
        pointRadius: 1,
        backgroundColor: '#8080C0',
        borderColor: '#8080C0',
        xAxisID: 'x',
        yAxisID: 'y',
        showLine: false,
        data: []
      },
      {
        label:"fw Speed [km/h]",
        borderWidth: 1,
        pointRadius: 1,
        backgroundColor: '#80C0C0',
        borderColor: '#80C0C0',
        xAxisID: 'x',
        yAxisID: 'y1',
        showLine: true,
        data: []
      },
      {
        label:"bw Speed [km/h]",
        borderWidth: 1,
        pointRadius: 1,
        backgroundColor: '#C080C0',
        borderColor: '#C080C0',
        xAxisID: 'x',
        yAxisID: 'y1',
        showLine: true,
        data: []
      },
      {
        label:"Speed Table [0-255]",
        borderWidth: 1,
        pointRadius: 1,
        backgroundColor: '#C0C080',
        borderColor: '#C0C080',
        xAxisID: 'x1',
        yAxisID: 'y2',
        showLine: true,
        data: []
      },
      {
        label: "fw fit [mm/s]",
        borderWidth: 1,
        pointRadius: 0,
        backgroundColor: '#80C080',
        borderColor: '#80C080',
        xAxisID: 'x',
        yAxisID: 'y',
        showLine: true,
        data: []
      },
      {
        label: "bw fit [mm/s]",
        borderWidth: 1,
        pointRadius: 0,
        backgroundColor: '#8080C0',
        borderColor: '#8080C0',
        xAxisID: 'x',
        yAxisID: 'y',
        showLine: true,
        data: []
      }],
    },
    options: {
      borderWidth: 3,
      borderColor: ["rgba(255, 99, 132, 128)"],
      scales: {
        x: {
            title: {
                display: true,
                text: 'Throttle Step [count]'
            },
            min: 0,
            max: 128
        },
        x1: {
            title: {
                display: true,
                text: 'Speed Table [count]'
            },
            position: 'top',
            grid: {
                display: false
            },
            min: 0,
            max: 28
        },
        y: {
            title: {
                display: true,
                text: 'Measured Speed [mm/s]'
            },
            type: 'linear',
            display: true,
            position: 'left',
            min: 0
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
                text: 'Motor DAC value [0 - 255]'
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

  CanvasJSDataAsCSV(profileChart, "Profile_Data", "testCsv");
  SpeedDataToProfile(profileChart, "Throttle_Data", "saveProfile");
}

function loadJMRIDlg(sender)
{
    console.log("Click", sender);
    document.getElementById("load-btn").click();
}

function loadJMRIFile(sender)
{
    console.log("Load JMRI File", sender);
    loadJMRIDecoder(sender);
}

function loadJMRIDecoder(sender)
{
	fileName = sender.files[0];
	var reader = new FileReader();
	var parser;
    reader.onload = function()
    {
        try 
        {
			parser = new DOMParser();
			locoDef = parser.parseFromString(reader.result,"text/xml");
		}
		catch(err) 
		{
			fileName = "";
			alert("Configuration data not valid");
			return;
		}
        validLocoDef = displayDecoder(locoDef);
//		setButtonStatus();
    };
    xmlFile = fileName;
    reader.readAsText(fileName);	
}

function displayDecoder(xmlNode)
{
	var isLoco = xmlNode.getElementsByTagName("locomotive-config").length > 0;
	var varVals = xmlNode.getElementsByTagName("varValue");
	var hasSpeedTable = -1;
	var cvVal = [];
	if (varVals.length > 0)
	{
		console.log(varVals.length);
		var attr = findXMLAttribute(varVals, "item", "Speed Table Definition");
		console.log(attr);
		if (attr)
			hasSpeedTable = attr.getAttribute("value");
		if (hasSpeedTable == 1)
		{
//			cvVal = xmlNode.getElementsByTagName("CVvalue");
//			speedTableProfileGraph.LineGraphs[0].DataElements.length = 0;//delete old data
//			var newEl = JSON.parse(JSON.stringify(DataElementTemplate));
//			newEl.x = 0;
//			newEl.y = 0;
//			speedTableProfileGraph.LineGraphs[0].DataElements.push(newEl);
//			console.log(xmlNode);
//			for (var i = 0; i < 28; i++) //28 table entries by definition
//			{
//				var cv = findXMLAttribute(cvVal, "name", (i+67).toString());
//				if (cv)
//				{
//					newEl = JSON.parse(JSON.stringify(DataElementTemplate));
//					newEl.x = i+1;
//					newEl.y = cv.getAttribute("value");
//					speedTableProfileGraph.LineGraphs[0].DataElements.push(newEl);
//				}
//			}
		}
	}
//	drawProfileBox(canvasElementSpeedTable, speedTableProfileGraph);
	if (isLoco)
	{
		document.getElementById("jmrifilename").innerHTML = xmlNode.getElementsByTagName("locomotive")[0].getAttribute("fileName");
		document.getElementById("dccloco").innerHTML = xmlNode.getElementsByTagName("locomotive")[0].getAttribute("id");
		document.getElementById("jmriaddr").innerHTML = xmlNode.getElementsByTagName("locomotive")[0].getAttribute("dccAddress");
		document.getElementById("decbrand").innerHTML = findCVVal(xmlNode, 8);
		document.getElementById("decfamily").innerHTML = xmlNode.getElementsByTagName("decoder")[0].getAttribute("family");
		document.getElementById("decmodel").innerHTML = xmlNode.getElementsByTagName("decoder")[0].getAttribute("model");

		document.getElementById("cvn02").innerHTML = findCVVal(xmlNode, 2);
        document.getElementById("cvn03").innerHTML = findCVVal(xmlNode, 3);
        document.getElementById("cvn04").innerHTML = findCVVal(xmlNode, 4);

		document.getElementById("cvn05").innerHTML = findCVVal(xmlNode, 5);
		document.getElementById("cvn06").innerHTML = findCVVal(xmlNode, 6);
		document.getElementById("cvn29").innerHTML = findCVVal(xmlNode, 29);

		cvVal_2 = findCVVal(xmlNode, 2);
		cvVal_5 = findCVVal(xmlNode, 5);
		cvVal_6 = findCVVal(xmlNode, 6);
		cvVal_29 = findCVVal(xmlNode, 29);
		
		var vMax = xmlNode.getElementsByTagName("locomotive")[0].getAttribute("maxSpeed");		
		document.getElementById("jmrivmax").innerHTML = vMax;
//		setThrottleProfile(document.getElementById("thscalespeed"));
		var newAddr = xmlNode.getElementsByTagName("locomotive")[0].getAttribute("dccAddress");
		locoAddr = -1;
		reqDCCAssign(newAddr);
        locoAddr = newAddr;
	}
	else
		console.log("Not Locomotive data");
	return isLoco;
}

function loadThrottleDlg(sender)
{
    console.log("Click", sender);
    document.getElementById("load-profile-btn").click();
}

function loadThrottle(sender)
{
    console.log("Load Throttle File", sender);
    loadThrottleFile(sender);
}

function loadThrottleFile(sender)
{
	fileName = document.getElementById("load-profile-btn").files[0];
//	console.log(fileName);
	var reader = new FileReader();
	
    reader.onload = function()
    {
        try 
        {
			throttleDef = JSON.parse(reader.result);
			throttleDef.GraphData.DimY = "km/h";
			throttleDef.FileName = fileName.name;
		}
		catch(err) 
		{
			throttleDef.fileName = "";
			alert("Not a valid throttle profile");
			return;
		}
        validThrottleDef = throttleDef.DataType == "ThrottleDef";
        if (!validThrottleDef)
        {
			throttleDef.fileName = "";
			alert("Not a valid throttle profile");
			return;
		}
		throttleProfileData = JSON.parse(JSON.stringify(throttleDef.GraphData));
        console.log(throttleProfileData);

        for(let i = 0; i < 128; i++)
        {
            var targetSpeed = getSpeedForThrottleStep(i, 127, throttleProfileData, false); // false returns km / h
            profileChart.data.datasets[0].data[i] = { x: i, y: targetSpeed};
        }        
        profileChart.update();
        
        console.log(throttleDef);
        document.getElementById("vMax").value = throttleDef.VMax;
        maxTestSpeed = throttleDef.VMax;
    };

    jsonFile = fileName;
    reader.readAsText(fileName);    
}

function reqDCCAssign(forAddress)
{
	if (forAddress >= 0)
		websocket.send("{\"Cmd\":\"SetSensor\", \"SubCmd\":\"SetDCC\"," + " \"Addr\":" + forAddress.toString()+ " }");
	else
		websocket.send("{\"Cmd\":\"SetSensor\", \"SubCmd\":\"SetDCC\"}");
}


function findXMLAttribute(inCollection, thisAttr, attrVal)
{
	for (var i = 0; i < inCollection.length; i++)
	{
//		console.log(i, inCollection[i].getAttribute(thisAttr));
		if (inCollection[i].getAttribute(thisAttr) == attrVal)
			return inCollection[i];
	}
	return null;
}

function findCVVal(xmlNode, cvNr)
{
	var cvVals = xmlNode.getElementsByTagName("CVvalue");
	for (var i = 0; i < cvVals.length; i++)
		if (cvVals[i].getAttribute("name") == cvNr) 
			return cvVals[i].getAttribute("value");
	return -1;
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

function setTrackLength(sender)
{
	trackLen = verifyNumber(sender.value, configData.TrackLen); 
}

function setTestSpeed(sender)
{
	maxTestSpeed = verifyNumber(sender.value, configData.MaxTestSpeed);	
}

function startSpeedTest(sender)
{

	if (locoAddr < 0)
    {
		alert("No DCC Address assigned! Verify connection to WiThrottle Server and reload JMRI file");
		//document.getElementById("btnLoadDecoder").files = [];
	}		
	else
	{

		if ((cvVal_2 != 0) || (cvVal_5 != 0) || (cvVal_6 != 0) || ((cvVal_29 & 0x10) != 0))
		{
			var dispText = "Verify and confirm CV settings before running the speed test.\nCV 2 expected 0 is " + cvVal_2 + "\nCV 5 expected 0 is " + cvVal_5 + "\nCV 6 expected 0 is " + cvVal_5 + "\nCV 29 Bit 4 expected 0 is " + ((cvVal_29 & 0x10)>>4) +  "\nProceed anyway?";
			if (confirm(dispText) == false)
				return;
		}

		var trkLen = trackLen;
		if (isNaN(trkLen))
		{
			alert("No Test Track length specified");
			return;
		}
		var techSpeed = (1000 * maxTestSpeed / 3.6) / configData.ScaleList[configData.ScaleIndex].Scale;
		if (isNaN(techSpeed))
		{
			alert("No maximum speed specified");
			return;
		}
		
		websocket.send("{\"Cmd\":\"SetSensor\", \"SubCmd\":\"RunTest\",\"TrackLen\":" + trkLen.toString() + ", \"Addr\":" + locoAddr.toString()+ ", \"VMax\":" + techSpeed.toString() + ", \"Mode\":" + 127 + "}");
	}
}

function stopSpeedTest(sender)
{
	websocket.send("{\"Cmd\":\"SetSensor\", \"SubCmd\":\"StopTest\"}");
}

function verifyNumber(inpValue, defValue)
{
	var numVal;
	if (isNaN(inpValue))
	{
		alert(inpValue + " is not a valid number. Please verify");
		return defValue;	
	}
	else
		return parseFloat(inpValue);
}

function verifyNumArray(inpValue, sepChar)
{
	var newInp = inpValue.split(sepChar);
	var newRes = [];
	for (var i=0; i < newInp.length; i++)
	{
		var hlpRes = parseInt(newInp[i]);
		if (!isNaN(hlpRes))
			newRes.push(hlpRes);
	}
	return newRes;
}

function calcTable(sender)
{
    console.log(sender);
	if (profileChart.data.datasets[0].data.length < 1 &&
        profileChart.data.datasets[1].data.length < 1 &&
        profileChart.data.datasets[2].data.length < 1)
	{
        alert("Run profile test or load profile csv file.");
        return;
    }
    if(throttleProfileData == undefined)
    {
        alert("Load the throttle profile to use for the speed table.");
        return;
    }

    var avgDevF = 0; //avg deviation between forward and backward
    var avgDevB = 0;
	var avgVal = 0;
	var maxVal = 0;
    var trimSteps = 0;

//		speedTableProfileGraph.LineGraphs[0].DataElements = [];
    profileChart.data.datasets[5].data = [];	
    profileChart.data.datasets[5].data[0] = { x: 0, y: 0};
	
    
	for (var j = 1; j < 28; j++)
	{
		var targetSpeed = getSpeedForThrottleStep(j, 27, throttleProfileData, true); // [mm/s]

        var avgDataSet = [];
        for(var i = 0; i < profileChart.data.datasets[6].data.length; i++)
        {
            avgDataSet.push({x: profileChart.data.datasets[6].data[i].x, y: (profileChart.data.datasets[6].data[i].y + profileChart.data.datasets[7].data[i].y) / 2});
        }

		var targetLevel = getMotorValueForSpeed(targetSpeed, avgDataSet, profileChart.data.datasets[6].data, profileChart.data.datasets[7].data);
		var x = j;
		var y =(targetLevel[0] + targetLevel[1]) / 2.0;
        
		
		if (y > maxVal)
			maxVal = y;
        if(targetLevel[2] > 0 && targetLevel[3] > 0)
        {
            var trim = (targetLevel[2] + targetLevel[3]) / 2.0;
            avgVal += trim;
		    avgDevF += targetLevel[2];            
            avgDevB += targetLevel[3]; //add deviation between fw and bw for trim calculation
            trimSteps++;
        }
        
		console.log(targetSpeed, targetLevel[0], targetLevel[1], y, targetLevel[2], targetLevel[3], avgDevF, avgDevB, avgVal);
        profileChart.data.datasets[5].data[j] = { x: x, y: Math.round(y)};
	}

    avgVal = avgVal / trimSteps;
	avgDevF = avgDevF / trimSteps;
    avgDevB = avgDevB / trimSteps;

    console.log(avgVal, avgDevF, avgDevB);	
	

    fwTrim = Math.round(128 * avgDevF);
 	bwTrim = Math.round(128 * avgDevB);

    console.log(fwTrim, bwTrim);		

    document.getElementById("fwTrim").innerHTML = fwTrim;
    document.getElementById("bwTrim").innerHTML = bwTrim; 
    document.getElementById("speedTable").innerHTML = Array.prototype.map.call(profileChart.data.datasets[5].data, s => s.y).toString();;

//	setButtonStatus();
    profileChart.update();
}

function getSpeedForThrottleStep(thisStep, maxSteps, throttleProfile, returnMeasuredSpeed)
{
	var tStep = Math.round((thisStep / maxSteps) * throttleProfile.ValsX[1]);
	var lineSegment = 0;
	for (var i = 0; i < throttleProfile.DataElements.length-1; i++)
		if ((tStep >= throttleProfile.DataElements[i].x) && ( tStep <= throttleProfile.DataElements[i+1].x))
		{
			lineSegment = i;
			break;
		}
	var x1 = throttleProfile.DataElements[lineSegment].x;
	var y1 = throttleProfile.DataElements[lineSegment].y;
	var dx = throttleProfile.DataElements[lineSegment+1].x - throttleProfile.DataElements[lineSegment].x;
	var dy = throttleProfile.DataElements[lineSegment+1].y - throttleProfile.DataElements[lineSegment].y;
	var scaleSpeed = y1 + ((tStep - x1) * dy/dx);
//	console.log(thisStep, tStep, lineSegment, x1, y1, dx, dy);

    // The profile data is in kn / h units  when true convers back to mm / s.
    if(returnMeasuredSpeed == true)    
    {
        if (configData.Units == 1) //imperial
		    scaleSpeed *= 1.6;
	    scaleSpeed /= configData.ScaleList[configData.ScaleIndex].Scale;
    	scaleSpeed = Math.round((scaleSpeed * 10000) / 36); //[mm/s]
    }
	return scaleSpeed;
}

var maxSpeedMotorFw = 0;
var maxSpeedMotorBw = 0;

function getMotorValueForSpeed(thisSpeed, avgDataSet, fwDataSet, bwDataSet)
{
	var motorValue = [maxSpeedMotorFw, maxSpeedMotorBw, 0, 0];
	for (var j = 1; j <= 2; j++) //techSpeedProfileGraph.LineGraphs (raw data) or set startIndex = 1 above.    
	{
        //find speedstep that has desired speed
		for (var i = 0; i < 126; i++)
        {
			if ((thisSpeed >= avgDataSet[i].y) && ( thisSpeed <= avgDataSet[i+1].y))
			{
//				console.log(thisSpeed, i);
				var y1 = avgDataSet[i].y
				var y2 = avgDataSet[i+1].y
				var dy = y2 - y1;
				var dx = 127 / 27; 
				//for 27 step mode, interpolate to 127 step
				if (avgDataSet.length < 30)
				{
					if ((i == 0) || (dy == 0))
                    {
						motorValue[j-1] = 0;
                        motorValue[j-1 + 2] = 0;
                    }
					else
                    {
						motorValue[j-1] = Math.round((i * dx) + (((thisSpeed - y1) / dy) * dx));

                        if(dy > 2.25) // slope greater than 0.5
                        {
                            motorValue[j-1 + 2] = thisSpeed; //avgDataSet[motorValue[j-1]].y;
                            motorValue[j-1 + 2] /= j == 1 ? fwDataSet[motorValue[j-1]].y : bwDataSet[motorValue[j-1]].y;
                        }
                        else
                        {
                            motorValue[j-1 + 2] = 0;
                        }
                    }
				}
				else
                {
					if ((i == 0) || (dy == 0))
                    {
						motorValue[j-1] = 0;
                        motorValue[j-1 + 2] = 0;
                    }
					else
                    {
                        motorValue[j-1] = thisSpeed > (y1 + (dy / 2)) ? i + 1 : i;

                        if(dy > 2.25) // slope greater than 0.5
                        {						
                            motorValue[j-1 + 2] = thisSpeed; //avgDataSet[motorValue[j-1]].y;
                            motorValue[j-1 + 2] /=  j == 1 ? fwDataSet[motorValue[j-1]].y : bwDataSet[motorValue[j-1]].y;
                        }
                        else
                        {
                            motorValue[j-1 + 2] = 0;
                        }
                        
                    }
                }
				//times 2 for motor value
				motorValue[j-1] *= 2; //adjust from 127 to 255 max value (raw data)               
				break;
			}
        }
	}
    if(motorValue[0] > maxSpeedMotorFw)
        maxSpeedMotorFw = motorValue[0];
    if(motorValue[1] > maxSpeedMotorBw)
        maxSpeedMotorBw = motorValue[1];
	return motorValue;
}

function loadSpeedDataDlg(sender)
{
    console.log("Click", sender);
    document.getElementById("load-speedData-btn").click();
}

function loadSpeedData(sender)
{
    console.log("Load Speed Data File", sender);
    importCsv(sender);
}

function importCsv(sender)
{
    var data;
    var fileName = document.getElementById("load-speedData-btn").files[0];

    Papa.parse(fileName, {
        header: true,
        download: true,
        dynamicTyping: true,
        complete: function(results) {
        console.log(results);
        data = results.data;        

        for(var i = 0; i < 127; i++)
        {
            profileChart.data.datasets[0].data[i] = { x: data[i].x, y: data[i]["Throttle Profile [km/h]"]};
            profileChart.data.datasets[1].data[i] = { x: data[i].x, y: data[i]["fw Speed [mm/s]"]};
            profileChart.data.datasets[2].data[i] = { x: data[i].x, y: data[i]["bw Speed [mm/s]"]};
            profileChart.data.datasets[3].data[i] = { x: data[i].x, y: data[i]["fw Speed [km/h]"]};
            profileChart.data.datasets[4].data[i] = { x: data[i].x, y: data[i]["bw Speed [km/h]"]};
            profileChart.data.datasets[5].data[i] = { x: data[i].x, y: data[i]["Speed Table [0-255]"]};
        }

        // Refit to update to latest alogorithm changes and / or
        // Create the fits for data sets that previously did not have them.
        var fwFit = regression.polynomial(profileChart.data.datasets[1].data, { order: 7, precision: 16 });
        profileChart.data.datasets[6].data = fwFit.points;
    
        var bwFit = regression.polynomial(profileChart.data.datasets[2].data, { order: 7, precision: 16 });
        profileChart.data.datasets[7].data = bwFit.points;

        profileChart.update();
        }
  
    });

    
}