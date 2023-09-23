// @see
// https://en.wikipedia.org/wiki/Air_quality_index#United_States

#include "CalculateAQI.h"

SensorData CalculateAQI::getAveragedData(SensorData data) {
  return {
    PM_AE_UG_1_0: (data.PM_AE_UG_1_0 / data.numReads),
    PM_AE_UG_2_5: (data.PM_AE_UG_2_5 / data.numReads),
    PM_AE_UG_10_0: (data.PM_AE_UG_10_0 / data.numReads),
    AQI: (data.AQI / data.numReads),
    numReads: data.numReads
  };
}

void CalculateAQI::updateSensorData(SensorData &data, PMS::DATA newData, float AQI) {
  data.PM_AE_UG_1_0 += newData.PM_AE_UG_1_0;
  data.PM_AE_UG_2_5 += newData.PM_AE_UG_2_5;
  data.PM_AE_UG_10_0 += newData.PM_AE_UG_10_0;
  data.AQI += AQI;
  data.numReads++;
}

float CalculateAQI::getAQI(float I_high, float I_low, float C_high, float C_low, float C) {
  return (I_high - I_low) * (C - C_low) / (C_high - C_low) + I_low;
}

float CalculateAQI::getPM25AQI(float cPM25) {
  Breakpoints b = CalculateAQI::getPM25Breakpoints(cPM25);
  return CalculateAQI::getAQI(b.iHi, b.iLo, b.cHi, b.cLo, cPM25);
}

Breakpoints CalculateAQI::getPM25Breakpoints(float cPM25) {
  if (cPM25 <= 12) {
    return {
      iHi: 50,
      iLo: 0,
      cHi: 12,
      cLo: 0  
    };
  }

  if (cPM25 <= 35.4) {
    return {
      iHi: 100,
      iLo: 51,
      cHi: 35.4,
      cLo: 12.1
    };
  }

  if (cPM25 <= 55.4) {
    return {
      iHi: 150,
      iLo: 101,
      cHi: 55.4,
      cLo: 35.5
    };
  }

  if (cPM25 <= 150.4) {
    return {
      iHi: 200,
      iLo: 151,
      cHi: 150.4,
      cLo: 55.5
    };
  }

  if (cPM25 <= 250.4) {
    return {
      iHi: 300,
      iLo: 201,
      cHi: 250.4,
      cLo: 150.5
    };
  }

  if (cPM25 <= 350.4) {
    return {
      iHi: 400,
      iLo: 301,
      cHi: 350.4,
      cLo: 250.5
    };
  }

  return {
    iHi: 500,
    iLo: 401,
    cHi: 500.4,
    cLo: 350.5
  };
}

Category CalculateAQI::getCategory(float AQI) {
  if (AQI <= 50) {
    return {
      level: "Good",
      color: "green"
    };
  }

  if (AQI <= 100) {
    return {
      level: "Moderate",
      color: "yellow"
    };
  }

  if (AQI <= 150) {
    return {
      level: "Unhealthy for Sensitive Groups",
      color: "orange"
    };
  }

  if (AQI <= 200) {
    return {
      level: "Unhealthy",
      color: "red"
    };
  }

  if (AQI <= 300) {  
    return {
      level: "Very Unhealthy",
      color: "purple"
    };
  }

  return {
    level: "Hazardous",
    color: "maroon"
  };
}
