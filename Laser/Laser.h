/*
 Laser.cpp -
 Created by Jildert Viet, 13-05-2015.
 Released into the public domain.
 */

#ifndef Laser_h
#define Laser_h_h

#include "Arduino.h"

class Laser
{
public:
    Laser();
    Laser(int id, int ldrPin, int laserPin);
    
    int id;
    int ldrPin;
    int laserPin;
    
    int state = 0;
    
    bool checkIfObstructed();
    
    void turnOn();
    void turnOff();
    void setState(int state);
    
    void process();
    
    int threshold = 700;
    
    bool justTurnedOn = false;
    unsigned long timeToWait;
    
    void gameOver();
    void makeWarningNoise();
    void muteWarningNoise();
    
    unsigned long warningTime = 3000; // 3s
    
    void sendValue(unsigned int value);
    void sendMsg(unsigned int mode);
    
    int getLDRval();
    
    bool obstructedDuringTimerPhase = false;
    bool noteOnPlayed = false;
    
    void test();
    
    void blink();
    bool bBlink = false;
    unsigned long blinkTime = 10000;
    unsigned long blinkEndTime;
    
    void doBlink();
};

#endif