#ifndef Stretch_h
#define Stretch_h

#include <arduino.h>

class StretchSensor {
  private:
    int m_Low, m_High;
    int m_Pin;
    
  public:
    StretchSensor(int);
    void calibrate();
  
  private:
    void setLow();
    void setHigh();
  
};

#endif//Stretch_h
