#include "Plant.h"

Plant::Plant(cuint &rank, DataType isDiscrete /*= DataType::Continuous*/) :
    m_rank(rank),
    m_isDiscrete(static_cast<bool>(isDiscrete))
{
    m_gains.resize(m_rank); //< Reserve number of gain layers
    for (auto it = m_gains.begin(); it != m_gains.end(); ++it) {
        it->resize(2);
    }
}

Plant::Plant(cuint &rank, tfInitType numerator, tfInitType denominator, DataType isDiscrete /*= DataType::Continuous*/) :
    m_rank(rank),
    m_isDiscrete(static_cast<bool>(isDiscrete))
{
    m_gains.resize(m_rank);
    for (auto it = m_gains.begin(); it != m_gains.end(); ++it) {
        it->resize(2);
    }

    for (auto layer = 0u; layer < m_rank; ++layer) {
        addGain(layer, GainSubLayer::InputGain, numerator[layer]);
        addGain(layer, GainSubLayer::OutputGain, -denominator[m_rank - layer]);

        addIntegrator(0.0);
    }
}

Plant::Plant(cuint &rank, tfInitType numerator, tfInitType denominator, integrInitStates initStates, DataType isDiscrete /*= DataType::Continuous*/) :
    m_rank(rank),
    m_isDiscrete(static_cast<bool>(isDiscrete))
{
    m_gains.resize(m_rank);
    for (auto it = m_gains.begin(); it != m_gains.end(); ++it) {
        it->resize(2);
    }

    for (auto layer = 0u; layer < m_rank; ++layer) {
        addGain(layer, GainSubLayer::InputGain, numerator[layer]);
        addGain(layer, GainSubLayer::OutputGain, -denominator[m_rank - layer]);

        addIntegrator(initStates[layer]);
    }
}

Plant::Plant(cuint &rank, const gVec &gains, const iVec &integrators, DataType isDiscrete /*= DataType::Continuous*/) :
    m_rank(rank),
    m_gains(std::move(gains)),
    m_integrators(std::move(integrators)),
    m_isDiscrete(static_cast<bool>(isDiscrete))
{
    // Do nothing
}

Plant::~Plant()
{
    for (auto it = m_gains.begin(); it != m_gains.end(); ++it) {
        while (!it->empty()) {
            delete it->back();
            it->pop_back();
        }
    }
    m_gains.clear();

    while (!m_integrators.empty()) {
        delete m_integrators.back();
        m_integrators.pop_back();
    }
}

double Plant::update(cdbl &input, cdbl &deltaTime) const
{
    if ((m_integrators.size() != m_gains.size() || (m_integrators.size() != m_rank))) {
        std::cout << "Dimentions of integrator and gain vectors do not match!" << std::endl;
        return 1.0;
    }

    double output{ 0.0 };

    for (auto layer = 0u; layer < m_rank; ++layer) {
        if (m_isDiscrete) {
            output = m_integrators[layer]->update(getLayerInput(layer, input, output));
        } else {
            output = m_integrators[layer]->update(getLayerInput(layer, input, output), deltaTime);
        }
    }

    return output;
}

double Plant::update(cdbl &input) const {
    if ((m_integrators.size() != m_gains.size() || (m_integrators.size() != m_rank))) {
        std::cout << "Dimentions of integrator and gain vectors do not match!" << std::endl;
        return 1.0;
    }

    double output{ 0.0 };

    for (auto layer = 0u; layer < m_rank; ++layer) {
        output = m_integrators[layer]->update(getLayerInput(layer, input, output));
    }

    return output;
}

int Plant::addGain(cuint &layer, const GainSubLayer &subLayer, cdbl &gainValue)
{
    if (layer >= m_rank) {
        std::cout << "Wrong layer number!" << std::endl;
    }

    m_gains[layer][static_cast<std::size_t>(subLayer)] = new Gain(gainValue);

    return 0;
}

int Plant::addIntegrator(cdbl &initState)
{
    if (m_integrators.size() == m_rank) {
        std::cout << "Can't add more integrators for the system of this rank(" << m_rank << ')' << std::endl;
        return 1; //< Can't add more integrators for the system of this rank
    }

    m_integrators.emplace_back(new Integrator(initState, m_isDiscrete));

    return 0;
}

double Plant::getLayerInput(cuint &layer, cdbl &input, cdbl &pastLayerOutput) const
{
    return m_gains[layer][static_cast<std::size_t>(GainSubLayer::InputGain)]->update(input)
         + m_gains[layer][static_cast<std::size_t>(GainSubLayer::OutputGain)]->update(m_integrators[m_rank - 1]->getCurrentOutput())
        + pastLayerOutput;
}
