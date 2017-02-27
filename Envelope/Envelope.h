/*
 Envelop.cpp - Library for using envelopes.
 Created by Jildert Viet, October 24, 2015.
 Released into the public domain.
 */

#ifndef Envelope_h
#define Envelope_h

#include "Arduino.h"

class Envelope
{
public:
    Envelope();
    Envelope(int attackTime, int sustainTime, int releaseTime);
    float process();
    bool gate, sustainBool;
    float attackTime, releaseTime, sustainTime;
    void attack(), release(), sustain();
    bool loop;
    float value;
};

#endif