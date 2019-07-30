#include "widget.h"
#include "ui_widget.h"
#include <iostream>
#include <math.h>
#include <limits>

// --------------------------
// Set stop time here
// --------------------------
#define ENDOFTIME 150 // Seconds
#define ENDOFTIME_DSCR 150
// #define SAMPLING 10 // Period in msec; e.g. T = 10 ms = 0.01 s
// --------------------------
// Set stop time here
// --------------------------

template <typename T>
inline constexpr int sgn(T val) noexcept {
    return (T(0) < val) - (val < T(0));
}

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    bool thinkRealTime = false;

    ui->setupUi(this);

    // Set window size
    this->setFixedSize(1400,700);

    // Add main layout with two plots
    mainlayout = new QGridLayout(this);
    inputPlot = new QCustomPlot(this);
    outputPlot = new QCustomPlot(this);
    m_dscrInputPlot = new QCustomPlot(this);
    m_dscrOutputPlot = new QCustomPlot(this);

    mainlayout->addWidget(inputPlot,0,0);
    mainlayout->addWidget(outputPlot,0,1);
    mainlayout->addWidget(m_dscrInputPlot, 1, 0);
    mainlayout->addWidget(m_dscrOutputPlot, 1, 1);
    inputPlot->setFixedSize(this->width()/2 - 30,this->height()/2 - 15);
    outputPlot->setFixedSize(this->width()/2 - 30,this->height()/2 - 15);
    m_dscrInputPlot->setFixedSize(this->width()/2 - 30, this->height()/2 - 15);
    m_dscrOutputPlot->setFixedSize(this->width()/2 - 30, this->height()/2 - 15);

    // Give the axes some labels:
    inputPlot->xAxis->setLabel("t, c");
    inputPlot->yAxis->setLabel("u");
    outputPlot->xAxis->setLabel("t, c");
    outputPlot->yAxis->setLabel("y");
    m_dscrInputPlot->xAxis->setLabel("t, c");
    m_dscrInputPlot->yAxis->setLabel("u_discrete");
    m_dscrOutputPlot->xAxis->setLabel("t, c");
    m_dscrOutputPlot->yAxis->setLabel("y_discrete");

    // --------------------------
    // Change ranges if you need
    // --------------------------
    // Set axes ranges so see all data:
    inputPlot->xAxis->setRange(0, ENDOFTIME);
    inputPlot->yAxis->setRange(0, 30);
    m_dscrInputPlot->xAxis->setRange(0, ENDOFTIME_DSCR);
    m_dscrInputPlot->yAxis->setRange(0, 30);

    if (thinkRealTime) {
        outputPlot->xAxis->setRange(0, ENDOFTIME);
        outputPlot->yAxis->setRange(-20, 20);
        m_dscrOutputPlot->xAxis->setRange(0, ENDOFTIME_DSCR);
        m_dscrOutputPlot->yAxis->setRange(-20, 20);
    }

    // --------------------------
    // Setup serial port
    // --------------------------
    m_serialPort = new QSerialPort(this);
    m_serialPort->setPortName("OUT1");
    if (!m_serialPort->setBaudRate(QSerialPort::Baud115200)) {
        std::cout << "Failed to set up the baud rate." << std::endl;
    }
    m_serialPort->setParity(QSerialPort::Parity::NoParity);
    m_serialPort->setStopBits(QSerialPort::StopBits::OneStop);

    m_pack = std::make_unique<PckNS::Package>();

    // --------------------------
    // Create the object here
    // --------------------------
    m_inputModel = std::make_unique<Plant>(
                1u,
                tfInitType{ 0.0 },
                tfInitType{ 1.0, 0.2 },
                integrInitStates{ 10.0 * M_E });
    m_mainModel = std::make_unique<Plant>(
                3u,
                tfInitType{ 1.0, 3.0, 1.0 },
                tfInitType{ 1.0, 5.0, 2.5, 10.0 });
/*
    // 5 Hz
    m_timeStep = 0.2;
    m_dscrSignal = std::make_unique<Plant>(
                1u,
                tfInitType{ 0.0 },
                tfInitType{ 1.0, -0.960789439152323 },
                integrInitStates{ 10.0 * M_E },
                DataType::Discrete);
    m_dscrModel = std::make_unique<Plant>(
                3u,
                tfInitType{ 0.168707364208113, -0.257592843702264, 0.093900579116729 },
                tfInitType{ 1.0, -2.275999946991332, 1.694030385628198, -0.367879441171442 },
                DataType::Discrete);
*/
/*
    // 50 Hz
    m_timeStep = 0.02;
    m_dscrSignal = std::make_unique<Plant>(
                1u,
                tfInitType{ 0.0 },
                tfInitType{ 1.0, -0.996007989343992 },
                integrInitStates{ 10.0 * M_E },
                DataType::Discrete);
    m_dscrModel = std::make_unique<Plant>(
                3u,
                tfInitType{ 0.019611022866077, -0.038072640971826, 0.018469230477931 },
                tfInitType{ 1.0, -2.903847175387286, 2.808760717145068, -0.904837418035960 },
                DataType::Discrete);
*/

    // 100 Hz
    m_timeStep = 0.01;
    m_dscrSignal = std::make_unique<Plant>(
                1u,
                tfInitType{ 0.0 },
                tfInitType{ 1.0, -0.998001998667333 },
                integrInitStates{ 10.0 * M_E },
                DataType::Discrete);
    m_dscrModel = std::make_unique<Plant>(
                3u,
                tfInitType{ 0.009901397070292, -0.019509205456794, 0.009608783777691 },
                tfInitType{ 1.0, -2.950980659107747, 2.902219837520355, -0.951229424500715 },
                DataType::Discrete);


    if (!thinkRealTime) {
        // --------------------------------------------------
        // Draw discrete plots outside of timer interrupts
        // --------------------------------------------------
        {
            m_dscrTime.clear();
            m_dscrInput.clear();
            m_dscrOutput.clear();

            auto maxOutput{ std::numeric_limits<double>::min() };
            auto minOutput{ std::numeric_limits<double>::max() };

            for (double i = 0.0; i < ENDOFTIME_DSCR; i += m_timeStep) {
                if (m_dscrTime.empty()) {
                    m_dscrTime.append(0.0);
                } else {
                    m_dscrTime.append(m_dscrTime.back() + m_timeStep);
                }

                double dscrSignal = m_dscrSignal->update(0.0);

                m_dscrInput.append(dscrSignal);
                m_dscrOutput.append(m_dscrModel->update(dscrSignal));

                std::cout << m_dscrOutput.back() << std::endl;

                if (m_dscrOutput.back() > maxOutput) {
                    maxOutput = m_dscrOutput.back();
                } else if (m_dscrOutput.back() < minOutput) {
                    minOutput = m_dscrOutput.back();
                }
            }

            auto minBound { std::floor(minOutput) * ( 1 - 0.2 * sgn(minOutput)) };
            auto maxBound { std::ceil(maxOutput) * ( 1 + 0.2 * sgn(maxOutput)) };

            m_dscrOutputPlot->xAxis->setRange(0, ENDOFTIME_DSCR);
            m_dscrOutputPlot->yAxis->setRange(minBound, maxBound);

            m_dscrInputPlot->addGraph();
            m_dscrInputPlot->graph(0)->setData(m_dscrTime, m_dscrInput);
            m_dscrOutputPlot->addGraph();
            m_dscrOutputPlot->graph(0)->setData(m_dscrTime, m_dscrOutput);

            m_dscrInputPlot->replot();
            m_dscrOutputPlot->replot();
        }

        // --------------------------------------------------
        // Draw continuous plots outside of timer interrupts
        // --------------------------------------------------
        {
            time.clear();
            input.clear();
            output.clear();

            auto maxOutput{ std::numeric_limits<double>::min() };
            auto minOutput{ std::numeric_limits<double>::max() };

            for (double i = 0.0; i < ENDOFTIME; i += m_timeStep) {
                if (time.empty()) {
                    time.append(0.0);
                } else {
                    time.append(time.back() + m_timeStep);
                }

                double signal = m_inputModel->update(0.0, m_timeStep);

                input.append(signal);
                output.append(m_mainModel->update(signal, m_timeStep));

                if (output.back() > maxOutput) {
                    maxOutput = output.back();
                } else if (output.back() < minOutput) {
                    minOutput = output.back();
                }

                float outputFloat = static_cast<float>(output.back());
                PckNS::byte_t const * outputBytes = reinterpret_cast<std::uint8_t*>(&outputFloat);
                for (std::size_t i = 0; i < sizeof(float); ++i) {
                    m_pack->setByte(i, outputBytes[sizeof(float) - 1 - i]);
                }
                /*
                auto stockPack{ m_pack->makePackage() };
                auto encodedPack{ m_pack->makeEncodedPackage() };

                std::cout << "stockPack: [ ";
                for (auto it = stockPack.begin(); it != stockPack.end(); ++it) {
                    std::cout << std::hex << static_cast<int>(*it) << ' ';
                }
                std::cout << "]\t";

                std::cout << "encodedPack: [ ";
                for (auto it = encodedPack.begin(); it != encodedPack.end(); ++it) {
                    std::cout << std::hex << static_cast<int>(*it) << ' ';
                }
                std::cout << "]\n";
                */
            }

            auto minBound { std::floor(minOutput) * ( 1 - 0.2 * sgn(minOutput)) };
            auto maxBound { std::ceil(maxOutput) * ( 1 + 0.2 * sgn(maxOutput)) };

            outputPlot->xAxis->setRange(0, ENDOFTIME_DSCR);
            outputPlot->yAxis->setRange(minBound, maxBound);

            inputPlot->addGraph();
            inputPlot->graph(0)->setData(time, input);
            outputPlot->addGraph();
            outputPlot->graph(0)->setData(time, output);

            inputPlot->replot();
            outputPlot->replot();
        }
    }

    // Get time in msec
    // --------------------------
    // Google for MacOS timings
    // --------------------------
#ifdef __linux__
    struct timeval tmpStruct;
    gettimeofday(&tmpStruct, NULL);
    startTime = tmpStruct.tv_sec * 1000 + tmpStruct.tv_usec / 1000 + 0.5;
#endif
#ifdef _WIN32
    SYSTEMTIME tmpStruct;
    GetSystemTime(&tmpStruct);
    startTime = tmpStruct.wSecond * 1000 + tmpStruct.wMilliseconds + 0.5;
#endif

    makePlot();
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(makePlot()));

    // --------------------------
    // Set sampling time here
    // --------------------------
    if (thinkRealTime) {
        timer->start(static_cast<int>(m_timeStep * 1000));
    }
    // --------------------------
    // Set sampling time here
    // --------------------------
}

Widget::~Widget()
{
    delete ui;
    delete inputPlot;
    delete outputPlot;
    delete timer;
    delete mainlayout;

    delete m_dscrInputPlot;
    delete m_dscrOutputPlot;

    // --------------------------
    // Delete the object here
    // --------------------------
    // delete m_gain;
    // delete m_integrator;
    // --------------------------
    // Delete the object here
    // --------------------------
}

void Widget::makePlot() {

    // generate some data:


#ifdef __linux__
    struct timeval tmpTime;
    gettimeofday(&tmpTime, NULL);
    double tmp = (tmpTime.tv_sec * 1000 + tmpTime.tv_usec / 1000 + 0.5)-startTime;
#endif
#ifdef _WIN32
    SYSTEMTIME tmpTime;
    GetSystemTime(&tmpTime);
    double tmp = tmpTime.wSecond * 1000 + tmpTime.wMilliseconds + 0.5 - startTime;
#endif

    // Get elapsed time
    if (time.empty()) {
        dt = 0;
    } else {
        dt = tmp / 1000.0 - time.last(); //< in SECONDS!
    }
    // Update time array to plot
    time.append(tmp/1000);

    if (m_dscrTime.empty()) {
        m_dscrTime.append(0.0);
    } else {
        m_dscrTime.append(m_dscrTime.back() + m_timeStep);
    }

    // --------------------------
    // Replace input signal with ours
    // --------------------------
    double signal = m_inputModel->update(0.0, dt);
    double dscrSignal = m_dscrSignal->update(0.0);

    // --------------------------
    // Replace input signal with ours
    // --------------------------

    // Update input array to plot
    input.append(signal);
    m_dscrInput.append(dscrSignal);

    // --------------------------
    // Update the object here
    // --------------------------
    output.append(m_mainModel->update(signal, dt));
    m_dscrOutput.append(m_dscrModel->update(dscrSignal));

    // --------------------------
    // Update the object here
    // --------------------------

    inputPlot->addGraph();
    inputPlot->graph(0)->setData(time, input);

    outputPlot->addGraph();
    outputPlot->graph(0)->setData(time, output);

    m_dscrInputPlot->addGraph();
    m_dscrInputPlot->graph(0)->setData(m_dscrTime, m_dscrInput);

    m_dscrOutputPlot->addGraph();
    m_dscrOutputPlot->graph(0)->setData(m_dscrTime, m_dscrOutput);

    inputPlot->replot();
    outputPlot->replot();

    m_dscrInputPlot->replot();
    m_dscrOutputPlot->replot();

    float outputFloat = static_cast<float>(output.back());
    PckNS::byte_t const * outputBytes = reinterpret_cast<std::uint8_t*>(&outputFloat);
    for (std::size_t i = 0; i < sizeof(float); ++i) {
        m_pack->setByte(i, outputBytes[sizeof(float) - 1 - i]);
    }

    auto stockPack{ m_pack->makePackage() };
    auto encodedPack{ m_pack->makeEncodedPackage() };


/*
    // --------------------
    // Output packages
    // --------------------

    std::cout << "stockPack: [ ";
    for (auto it = stockPack.begin(); it != stockPack.end(); ++it) {
        std::cout << std::hex << static_cast<int>(*it) << ' ';
    }
    std::cout << "]\t";

    std::cout << "encodedPack: [ ";
    for (auto it = encodedPack.begin(); it != encodedPack.end(); ++it) {
        std::cout << std::hex << static_cast<int>(*it) << ' ';
    }
    std::cout << "]\n";
*/
    if (tmp/1000 > ENDOFTIME) {
        timer->stop();
    }
}
