// @see
// https://en.wikipedia.org/wiki/Air_quality_index#United_States

#include "CalculateAQI.h"

struct Breakpoints {
    float iHi;
    float iLo;
    float cHi;
    float cLo;
};

float CalculateAQI::getAQI(float I_high, float I_low, float C_high, float C_low, float C) {
  return (I_high - I_low) * (C - C_low) / (C_high - C_low) + I_low;
}

float CalculateAQI::getPM25AQI(float cPM25) {
  Breakpoints b = CalculateAQI::getPM25Breakpoints(cPM25);
  return CalculateAQI::getAQI(b.iHi, b.iLo, b.cHi, b.cLo, cPM25);
}

struct Breakpoints CalculateAQI::getPM25Breakpoints(float cPM25) {
  Breakpoints b;
    
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
}

char* CalculateAQI::getCategory(float AQI) {
  if (AQI <= 50) {
    return "Good";
  } else if (AQI > 50 && AQI <= 100) {
    return "Moderate";
  } else if (AQI > 100 && AQI <= 150) {
    return "Unhealthy for Sensitive Groups";
  } else if (AQI > 150 && AQI <= 200) {
    return "Unhealthy";
  } else if (AQI > 200 && AQI <= 300) {  
    return "Very Unhealthy";
  } else if (AQI > 300) {
    return "Hazardous";
  }

  return "Unknown";
}
