var ChartAQI = function () {
    var inst = this;
    var myChart;
    var latest = document.getElementById('latest');

    var channel = {
        id: 0,
        key: '',
        purpleAirSensorIds: []
    };

    this.updateLatest = function (data) {
        var lastReadDate = new Date(data.created_at);
        latest.innerHTML = '<p><small>Latest read on ' + lastReadDate.toLocaleString('en-US') + '</small></p><h2>AQI ' + Math.round(parseFloat(data.field4)) + '</h2>';
    }

    this.initChart = function () {
        myChart = new Chart('chart', {
            type: 'line',
            options: {
                responsive: true,
                legend: {
                    position: 'right'
                },
                scales: {
                    xAxes: [{
                        type: 'time',
                        time: {
                            tooltipFormat: 'M/D/YYYY, h:mm:ss A'
                        },
                        scaleLabel: {
                            display: false,
                            fontStyle: 'bold',
                            labelString: 'Time'
                        }
                    }],
                    yAxes: [{
                        scaleLabel: {
                            display: true,
                            fontStyle: 'bold',
                            labelString: 'AQI'
                        }
                    }]
                }
            }
        });
    }

    this.getQueryVariable = function (variable) {
        var query = window.location.search.substring(1);
        var vars = query.split('&');

        for (var i = 0; i < vars.length; i++) {
            var pair = vars[i].split('=');

            if (decodeURIComponent(pair[0]) == variable) {
                return decodeURIComponent(pair[1]);
            }
        }

        return false;
    }

    this.getRandomColor = function (alpha) {
        var ran = function () {
            return Math.round(Math.random() * 255);
        }

        return 'rgba(' + ran() + ', ' + ran() + ', ' + ran() + ', ' + alpha + ')';
    }

    this.onSensorData = function (feeds, title, isCustom) {
        var data = [];

        for (var i in feeds) {
            var feed = feeds[i];
            var val = 0;

            if (isCustom) {
                // already calculated
                val = parseFloat(feed.field4).toFixed(2);
            } else {
                // raw data from a PurpleAir sensor
                val = CalculateAQI.getPM25AQI(parseFloat(feed.field8)).toFixed(2);
            }

            data.push({
                x: new Date(feed.created_at),
                y: val
            });
        }

        var border = isCustom ? 3 : 2;
        var color = isCustom ? 'black' : this.getRandomColor(.3);
        
        var myData = {
            label: title,
            fill: false,
            borderWidth: border,
            pointRadius: 7,
            backgroundColor: color,
            borderColor: color,
            pointHoverBackgroundColor: color,
            pointBackgroundColor: 'rgba(0,0,0,0)',
            pointBorderColor: 'rgba(0,0,0,0)',
            data: data
        };

        myChart.data.datasets.push(myData);
        myChart.update();
    }

    this.onPurpleAirData = function (result) {
        var numResults = this.getResults();
        var endpoint = this.getThingSpeakEndpoint(result.THINGSPEAK_PRIMARY_ID, result.THINGSPEAK_PRIMARY_ID_READ_KEY, numResults);

        var xhr = new XMLHttpRequest();
        xhr.open('GET', endpoint, true);

        xhr.onload = function () {
            var json = JSON.parse(xhr.responseText);
            
            if (json.feeds) {
                inst.onSensorData(json.feeds, 'PurpleAir / ' + result.Label);
            }
        }

        xhr.onerror = function (e) {
            console.log(e);
        }

        xhr.send();
    }

    this.getPurpleAirData = function (id) {
        var endpoint = 'https://www.purpleair.com/json?show=' + id;

        var xhr = new XMLHttpRequest();
        xhr.open('GET', endpoint, true);

        xhr.onload = function() {
            var json = JSON.parse(xhr.responseText);
            if (!json.results) return;

            inst.onPurpleAirData(json.results[0]);
        }

        xhr.onerror = function (e) {
            console.log(e);
        }

        xhr.send();
    }

    this.getThingSpeakEndpoint = function (id, key, results) {
        var endpoint = 'https://api.thingspeak.com/channels/' + id + '/feeds.json?api_key=' + key;

        if (typeof(results) == 'number') {
            endpoint += '&results=' + results;
        }

        return endpoint;
    }

    this.getResults = function () {
        var r = this.getQueryVariable('results');
        var results = parseInt(r) ? r : '100';
        return parseInt(results);
    }

    this.getData = function (single) {
        var results = 1;

        if (single) {
            // trim first value off sensorData
        } else {
            results = this.getResults();

            /*
            // start polling for new data
            setInterval(getData, 15000, true);
            */
        }

        var endpoint = this.getThingSpeakEndpoint(channel.id, channel.key, results);
        var xhr = new XMLHttpRequest();
        xhr.open('GET', endpoint, true);

        xhr.onload = function () {
            var json = JSON.parse(xhr.responseText);
            console.log(json);
            
            if (!json.feeds) {
                latest.innerHTML = '<h1>API error</h1>';
                return;
            }

            // graph sensor data
            inst.onSensorData(json.feeds, json.channel.name, true);

            // update latest
            var latestData = json.feeds[json.feeds.length - 1];
            inst.updateLatest(latestData);
        }

        xhr.onerror = function (e) {
            console.log(e);
            latest.innerHTML = '<h1>Network error</h1>';
        }

        xhr.send();
    }

    this.initialize = function () {
        for (var i in channel) {
            var myVal = this.getQueryVariable(i);
            if (myVal === false) continue;

            if (typeof(channel[i]) == 'object') {
                channel[i] = myVal.split(',');
            } else {
                channel[i] = myVal;
            }
        }

        this.initChart();
        this.getData();

        // graph any purpleair sensors
        for (var i in channel.purpleAirSensorIds) {
            this.getPurpleAirData(channel.purpleAirSensorIds[i]);
        }
    }

    this.initialize();
}
