#ifndef CalculateAQI_h
#define CalculateAQI_h

class CalculateAQI {
  public:
    static char* getCategory(float AQI);
    static struct Breakpoints getPM25Breakpoints(float cPM25);
    static float getPM25AQI(float cPM25);
    static float getAQI(float I_high, float I_low, float C_high, float C_low, float C);
};

#endif
