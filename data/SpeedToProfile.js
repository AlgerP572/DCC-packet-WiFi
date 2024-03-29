var DataElementTemplate = {"x":0,"y":0, "m":0};


function SpeedDataToProfile(chart, fileName, buttonCsv) {
    
    var buttonCsv = document.getElementById(buttonCsv);
    buttonCsv.addEventListener("click", function () {
        parseProfile({
            filename: (fileName || "chart-data") + ".json",
            chart: chart
        })
    });
}

function convertChartDataToProfile(args) {
    var data;
   
    data = args.data.datasets || null;
    if (data == null || !data.length) {
        return null;
    }
    
    var headers = data.map((i) => i.label);
    headers.splice(0, 0, "x");
    console.log(headers);

    throttleDef.GraphData.DataElements = [];
    throttleDef.GraphData.DimX = "Speed Step";
    throttleDef.GraphData.DimY = "km/h";
    throttleDef.GraphData.ValsX = [];
    throttleDef.GraphData.ValsY = [];

    throttleDef.GraphData.ValsX.push(args.data.datasets[3].data[0].x);
    throttleDef.GraphData.ValsX.push(args.data.datasets[3].data[data[3].data.length - 1].x);
    throttleDef.GraphData.ValsX.push(10);

    throttleDef.GraphData.ValsY.push(args.data.datasets[3].data[0].y);
    throttleDef.GraphData.ValsY.push(args.data.datasets[3].data[data[3].data.length - 1].y);
    throttleDef.GraphData.ValsY.push(10);

    var throttleFit = regression.polynomial(args.data.datasets[3].data, { order: 7, precision: 16 });
    console.log(throttleFit);
   
    // all datasets should have the same number of points.  
    var step = Math.floor(throttleFit.points.length / 10);
    var vMax = 0;
    var atStep = 0;

    for(var i = 0; i < throttleFit.points.length; i+= step)
    {
        var newElement = JSON.parse(JSON.stringify(DataElementTemplate)); // = {"x":0,"y":0};

        newElement.x = throttleFit.points[i].x;
	    newElement.y = Math.floor(throttleFit.points[i].y);
	    throttleDef.GraphData.DataElements.push(newElement);

        if(newElement.y > vMax)
        {
            vMax = newElement.y;
            atStep = i;
        }
    }

    throttleDef.SpeedSteps = 128;
    throttleDef.VMax = vMax;
    throttleDef.AtStep = atStep;

    var newElement = JSON.parse(JSON.stringify(DataElementTemplate)); // = {"x":0,"y":0};
    newElement.x = throttleFit.points[126].x;
	newElement.y = throttleFit.points[126].y;
	throttleDef.GraphData.DataElements.push(newElement);

    console.log(throttleDef.GraphData.DataElements);

    var json = JSON.stringify(throttleDef);
    console.log(json);
    return json;
}

function parseProfile(args) {
    var json = "";
    
    json += convertChartDataToProfile({
        data: args.chart.config.data
    });

    if (json == null) return;
    var filename = args.filename || 'peofile.json';
    if (!json.match(/^data:text\/json/i)) {
        json = 'data:text/json;charset=utf-8,' + json;
    }
    downloadFile(json, filename);
}

function downloadFile(extData, filename) {
    var data = encodeURI(extData);
    var link = document.createElement('a');
    link.setAttribute('href', data);
    link.setAttribute('download', filename);
    document.body.appendChild(link); // Required for FF
    link.click();
    document.body.removeChild(link);
}