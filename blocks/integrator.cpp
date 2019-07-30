#include "integrator.h"

Integrator::Integrator(double initState /*= 0.0*/, bool isDiscrete /*= false*/) :
    m_area(initState),
    m_state(initState),
    m_isDiscrete(isDiscrete)
{
    // Do nothing
}

Integrator::~Integrator()
{
    // Do nothing
}

double Integrator::update(double input, double deltaTime /*= 0.0*/)
{
    if (m_isDiscrete) {
        auto tempState{ m_state };
        m_state = input;
        return tempState;
    }

    m_area += (input + m_state) / 2 * deltaTime;
    m_state = input;

    return m_area;
}

double Integrator::getCurrentOutput() const
{
    return m_isDiscrete ? m_state : m_area;
}
