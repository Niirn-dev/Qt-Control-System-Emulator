#include "gain.h"
#include <iostream>

Gain::Gain(double gainValue) :
    gain_value(gainValue)
{
    // Do nothing
}

Gain::~Gain()
{
    // Do nothing
}

constexpr double Gain::getValue() const {
        return gain_value;
}

double Gain::update(cdbl &input) {
    return input * gain_value;
}
