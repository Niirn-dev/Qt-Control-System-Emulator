#ifndef GAIN_H
#define GAIN_H

#include <inttypes.h>

using cdbl = const double;

class Gain
{
public:
    Gain(double gain_value);

    Gain(const Gain&) = delete;
    Gain& operator=(const Gain&) = delete;

    ~Gain();

    constexpr double getValue() const;
    double update(cdbl &input);

private:
    double gain_value;
};

#endif // GAIN_H
