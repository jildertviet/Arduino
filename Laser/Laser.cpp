/*
 Laser.cpp -
 Created by Jildert Viet, 13-05-2015.
 Released into the public domain.
 */

#include "Arduino.h"
#include "Laser.h"

Laser::Laser(){
    
}

Laser::Laser(int id, int ldrPin, int laserPin){
    this->id = id;
    this->ldrPin = ldrPin;
    this->laserPin = laserPin;
    pinMode(laserPin, OUTPUT);
}

bool Laser::checkIfObstructed(){
    if(state){ // If state == 1 (On)
        int value = analogRead(ldrPin);

        if(millis() > timeToWait){ // na x sec wordt de laser een gewone KILLING laser
            justTurnedOn = false;
        }
        //-------------------------------------------------------------------------------------------------------------------------------
        if(justTurnedOn){
            if(value > threshold){
                if(!obstructedDuringTimerPhase){
                    obstructedDuringTimerPhase = true;
                    makeWarningNoise();
                }
            } else{
                if(!noteOnPlayed){
                    sendMsg(1);
                    noteOnPlayed = true;
                }
                if(obstructedDuringTimerPhase){
                    obstructedDuringTimerPhase = false;
                    muteWarningNoise();
//                    sendMsg(1);
                }
                
            }
        //-------------------------------------------------------------------------------------------------------------------------------
        } else{ // If laser is running normally
            if(value > threshold){
                // JE BENT AF!
                gameOver();
                return true;
            }
        }
    // Read pin, return true is obstructed, false if not
    } else{
        // laser is off, dus geen check nodig
    }
    return false;
}

void Laser::turnOn(){
    noteOnPlayed = false;
    state = 1;
    digitalWrite(laserPin, state);
    justTurnedOn = true;
    timeToWait = millis() + warningTime;
    obstructedDuringTimerPhase = false;
}

void Laser::turnOff(){
    state = 0;
    digitalWrite(laserPin, state);
    justTurnedOn = false;
    sendMsg(2);
}

int Laser::getLDRval(){
    return analogRead(ldrPin);
}

void Laser::setState(int stateTemp){
    if(stateTemp==1){
        turnOn();
        return;
    }
    if(stateTemp==0){
        turnOff();
        return;
    }
}

void Laser::gameOver(){
    turnOff();
    sendMsg(3);
    doBlink();
//    sendMsg(millis());
}

void Laser::makeWarningNoise(){
    sendMsg(4);
//    sendMsg(millis());
//    sendMsg(timeToWait);
}

void Laser::muteWarningNoise(){
    sendMsg(5);
}

void Laser::sendValue(unsigned int value){
    Serial.print(value);
    Serial.write(32);
}

void Laser::sendMsg(unsigned int mode){
    sendValue(id);
    sendValue(mode);
    Serial.println();
}

void Laser::test(){
    sendValue(id);
    sendValue(6);
    int ldrVal = analogRead(ldrPin);
    sendValue(ldrVal);
    if(ldrVal < threshold){
        sendValue(0);
    } else{
        sendValue(1);
    }
    Serial.print("  ");
}

void Laser::blink(){
    if(bBlink){
        if(blinkEndTime < millis()){
            bBlink = false;
            digitalWrite(laserPin, LOW);
            return;
        }
        // The blinkyness
        if((millis() % 1000) < 500){
            digitalWrite(laserPin, HIGH);
        } else{
            digitalWrite(laserPin, LOW);
        }
    }
}

void Laser::doBlink(){
    bBlink = true;
    blinkEndTime = millis() + blinkTime;
}