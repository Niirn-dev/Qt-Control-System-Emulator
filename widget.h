#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTimer>
#include <QVector>
#include <QtSerialPort/QSerialPort>

#include <memory>

#ifdef __linux__
#include <sys/time.h>
#endif

#include "qcustomplot.h"
#include "blocks/gain.h"
#include "blocks/integrator.h"
#include "Plant.h"
#include "package.h"

namespace Ui
{

class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();

public slots:
    void makePlot();

private:
    Ui::Widget *ui{ nullptr };
    QGridLayout *mainlayout{ nullptr };
    QCustomPlot *inputPlot{ nullptr };
    QCustomPlot *outputPlot{ nullptr };
    QCustomPlot *m_dscrInputPlot{ nullptr };
    QCustomPlot *m_dscrOutputPlot{ nullptr };

    double startTime;
    double dt;

    QTimer *timer;
    QVector<double> time;
    QVector<double> input;
    QVector<double> output;

    QSerialPort* m_serialPort;
    std::unique_ptr<PckNS::Package> m_pack;

    // --------------------------
    // Add pointer to the object here
    // --------------------------
    std::unique_ptr<Plant> m_inputModel;
    std::unique_ptr<Plant> m_mainModel;

    std::unique_ptr<Plant> m_dscrSignal;
    std::unique_ptr<Plant> m_dscrModel;

    double m_timeStep;
    QVector<double> m_dscrTime;
    QVector<double> m_dscrInput;
    QVector<double> m_dscrOutput;
    // --------------------------
    // Add pointer to the object here
    // --------------------------
};

#endif // WIDGET_H
