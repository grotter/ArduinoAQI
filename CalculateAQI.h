#ifndef CalculateAQI_h
#define CalculateAQI_h

#include <stdint.h>
#include <PMS.h>

struct Breakpoints {
  float iHi;
  float iLo;
  float cHi;
  float cLo;
};

struct Category {
  char* level;
  char* color;
};

struct SensorData {
  float PM_AE_UG_1_0;
  float PM_AE_UG_2_5;
  float PM_AE_UG_10_0;
  float AQI;
  int numReads;
};
  
class CalculateAQI {    
  public:
    static void updateSensorData(SensorData &data, PMS::DATA newData, float AQI); 
    static SensorData getAveragedData(SensorData data);    
    static Category getCategory(float AQI);
    static Breakpoints getPM25Breakpoints(float cPM25);
    static float getPM25AQI(float cPM25);
    static float getAQI(float I_high, float I_low, float C_high, float C_low, float C);
};

#endif
