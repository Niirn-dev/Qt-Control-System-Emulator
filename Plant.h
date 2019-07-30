#ifndef MODEL_H
#define MODEL_H

#include <iostream>
#include <vector>
#include <memory>
#include <inttypes.h>

#include "blocks/gain.h"
#include "blocks/integrator.h"

using cuint = const uint32_t;
using cdbl = const double;
using tfInitType = const std::vector<double> &;
using integrInitStates = const std::vector<double> &;
using gVec = std::vector<std::vector<Gain *>>;
using iVec = std::vector<Integrator *>;


enum class GainSubLayer : std::uint8_t {
    InputGain = 0,
    OutputGain = 1
};

enum class DataType : bool {
    Continuous = false,
    Discrete = true
};

class Plant
{
public:
    // Rank is the number of integrators
    Plant(cuint &rank, DataType isDiscrete = DataType::Continuous);
    Plant(cuint &rank, tfInitType numerator, tfInitType denominator, DataType isDiscrete = DataType::Continuous);
    Plant(cuint &rank, tfInitType numerator, tfInitType denominator, integrInitStates initStates, DataType isDiscrete = DataType::Continuous);
    Plant(cuint &rank, const gVec &gains, const iVec &integrators, DataType isDiscrete = DataType::Continuous);

    Plant(const Plant&) = delete;
    Plant& operator=(const Plant&) = delete;

    ~Plant();

    double update(cdbl &input, cdbl &deltaTime) const;
    double update(cdbl &input) const;

private:
    int addGain(cuint &layer, const GainSubLayer &subLayer, cdbl &gainValue);
    int addIntegrator(cdbl &initState);

    double getLayerInput(cuint &layer, cdbl &input, cdbl &pastLayerInput) const;

    cuint m_rank;

    gVec m_gains;
    iVec m_integrators;

    const bool m_isDiscrete;
};

#endif // MODEL_H
