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

    this.drawChart = function (title, feeds) {
        var sensorData = [];

        for (var i in feeds) {
            var obj = feeds[i];

            sensorData.push({
                x: new Date(obj.created_at),
                y: parseFloat(obj.field4)
            });
        }

        var randomColor = this.getRandomColor();

        myChart = new Chart('chart', {
            type: 'line',
            data: {
                datasets: [
                    {
                        label: title,
                        fill: false,
                        borderWidth: 1,
                        pointRadius: 2,
                        backgroundColor: randomColor,
                        borderColor: randomColor,
                        data: sensorData
                    }
                ]
            },
            options: {
                responsive: true,
                scales: {
                    xAxes: [{
                        type: 'time',
                        time: {
                            tooltipFormat: 'll HH:mm'
                        },
                        scaleLabel: {
                            display: true,
                            labelString: 'Time'
                        }
                    }],
                    yAxes: [{
                        display: true,
                        scaleLabel: {
                            display: true,
                            labelString: 'AQI'
                        }
                    }]
                }
            }
        });
    }

    this.getRandomColor = function () {
        var letters = '0123456789ABCDEF'.split('');
        var color = '#';
        
        for (var i = 0; i < 6; i++ ) {
            color += letters[Math.floor(Math.random() * 16)];
        }

        return color;
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

    this.onPurpleAirData = function (result) {
        var numResults = this.getResults();
        var endpoint = this.getThingSpeakEndpoint(result.THINGSPEAK_PRIMARY_ID, result.THINGSPEAK_PRIMARY_ID_READ_KEY, numResults);

        var xhr = new XMLHttpRequest();
        xhr.open('GET', endpoint, true);

        xhr.onload = function () {
            var json = JSON.parse(xhr.responseText);
            var data = [];

            for (var i in json.feeds) {
                var feed = json.feeds[i];
                var val = parseFloat(feed.field8);

                data.push({
                    x: new Date(feed.created_at),
                    y: CalculateAQI.getPM25AQI(val)
                });
            }

            var randomColor = inst.getRandomColor();

            myChart.data.datasets.push({
                label: 'PurpleAir / ' + result.Label,
                id: result.ID,
                fill: false,
                borderWidth: 1,
                pointRadius: 2,
                backgroundColor: randomColor,
                borderColor: randomColor,
                data: data
            });

            myChart.update();
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
            sensorData.removeRow(0);
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
            // graph any purpleair sensors
            for (var i in channel.purpleAirSensorIds) {
                inst.getPurpleAirData(channel.purpleAirSensorIds[i]);
            }

            var json = JSON.parse(xhr.responseText);

            if (json.feeds) {
                inst.drawChart(json.channel.name, json.feeds);

                var latestData = json.feeds[json.feeds.length - 1];
                inst.updateLatest(latestData);
            } else {
                console.log(json);
                latest.innerHTML = '<h1>API error</h1>';
            }
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

        this.getData();
    }

    this.initialize();
}
