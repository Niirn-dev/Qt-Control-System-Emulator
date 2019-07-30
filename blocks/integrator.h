#ifndef INTEGRATOR_H
#define INTEGRATOR_H


class Integrator
{
public:
    Integrator(double initState = 0.0, bool isDescrete = false);

    Integrator(const Integrator&) = delete;
    Integrator& operator=(const Integrator&) = delete;

    ~Integrator();

    double update(double input, double deltaTime = 0.0);
    double getCurrentOutput() const;

private:
    double m_area;
    double m_state{ 0.0 };

    const bool m_isDiscrete;
};

#endif // INTEGRATOR_H
