var CalculateAQI = {
	getAQI: function (I_high, I_low, C_high, C_low, C) {
	  return (I_high - I_low) * (C - C_low) / (C_high - C_low) + I_low;
	},

	getPM25AQI: function (cPM25) {
	  var b = this.getPM25Breakpoints(cPM25);
	  return this.getAQI(b.iHi, b.iLo, b.cHi, b.cLo, cPM25);
	},

	getPM25Breakpoints: function (cPM25) {
	  var b = {};
	    
	  if (cPM25 <= 12) {
	    b.iHi = 50;
	    b.iLo = 0;
	    b.cHi = 12;
	    b.cLo = 0;
	  } else if (cPM25 > 12 && cPM25 <= 35.4) {
	    b.iHi = 100;
	    b.iLo = 51;
	    b.cHi = 35.4;
	    b.cLo = 12.1;
	  } else if (cPM25 > 35.4 && cPM25 <= 55.4) {
	    b.iHi = 150;
	    b.iLo = 101;
	    b.cHi = 55.4;
	    b.cLo = 35.5;
	  } else if (cPM25 > 55.4 && cPM25 <= 150.4) {
	    b.iHi = 200;
	    b.iLo = 151;
	    b.cHi = 150.4;
	    b.cLo = 55.5;
	  } else if (cPM25 > 150.4 && cPM25 <= 250.4) {
	    b.iHi = 300;
	    b.iLo = 201;
	    b.cHi = 250.4;
	    b.cLo = 150.5;
	  } else if (cPM25 > 250.4 && cPM25 <= 350.4) {
	    b.iHi = 400;
	    b.iLo = 301;
	    b.cHi = 350.4;
	    b.cLo = 250.5;
	  } else if (cPM25 > 350.4) {
	    b.iHi = 500;
	    b.iLo = 401;
	    b.cHi = 500.4;
	    b.cLo = 350.5;
	  }

	  return b;
	},

	getCategory: function (AQI) {
	  var c = {
	  	level: 'Unknown',
	  	color: 'black'
	  };
	  
	  if (AQI <= 50) {
	    c.level = "Good";
	    c.color = "green";
	  } else if (AQI > 50 && AQI <= 100) {
	    c.level = "Moderate";
	    c.color = "yellow";
	  } else if (AQI > 100 && AQI <= 150) {
	    c.level = "Unhealthy for Sensitive Groups";
	    c.color = "orange";
	  } else if (AQI > 150 && AQI <= 200) {
	    c.level = "Unhealthy";
	    c.color = "red";
	  } else if (AQI > 200 && AQI <= 300) {  
	    c.level = "Very Unhealthy";
	    c.color = "purple";
	  } else if (AQI > 300) {
	    c.level = "Hazardous";
	    c.color = "maroon";
	  }

	  return c;
	}	
};
