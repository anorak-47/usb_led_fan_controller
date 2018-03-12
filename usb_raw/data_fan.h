#pragma once

#include "data.h"
#include "fan_type.h"
#include "types.h"
#include <QtCore/QObject>

class QMutex;

class DataFan : public DataWithAChannel
{
	Q_OBJECT

public:
    DataFan(int channel);
    virtual ~DataFan();

    virtual QString name() const override;
    virtual QString description() const override;

    virtual void update() override;
    virtual void updateValues() override;

    void updateType(FANTYPE type);
    void updateMode(FANMODE mode);
    void updateSensorIndex(int index);
    void updateMinRpmStalled(double rpm);
    void updateMinDuty(int duty);
    void updateMaxDuty(int duty);

    void updateFixedDuty(int duty);

    void updatePiControllerParameters(double kp, double ki, double kt);
    void updatePiControllerSetpoint(int snsIdx, double offset);

    void updateLinearGain(double gain);
    void updateLinearOffset(double offset);

    void updateTripPoints();

    void setRPM(unsigned int rpm);
    unsigned int getRPM() const;

    void setScaledDuty(double duty); // scaled to 0%..100%
    double getScaledDuty() const;

    void setScaledFixedDuty(double duty); // scaled to 0%..100%
    double getScaledFixedDuty() const;

    void setMinRpmStalled(double rpm);
    double getMinRpmStalled() const;

    double getLinearGain() const;
    void setLinearGain(double linearGain);

    double getLinearOffset() const;
    void setLinearOffset(double linearOffset);

    void setPiControllerParameters(double kp, double ki, double kt);

    double getPiControllerKp() const;
    double getPiControllerKi() const;
    double getPiControllerKt() const;

    double getPiSetpointOffset() const;
    void setPiSetpointOffset(double piSetpointOffset);

    double getSetpointValue() const;
    void setSetpointValue(double setpointValue);

    double getPiControllerDebugE() const;
    double getPiControllerDebugP() const;
    double getPiControllerDebugI() const;

    Fan &data();

signals:
    void signalSensorIndexChanged();

protected:
    virtual bool handleEvent(CommandEvent *event) override;

private:
    Fan _fan;

    double _scaledDuty = 0.0;
    double _scaledFixedDuty = 0.0;
    double _minRpmStalled = 300.0;
    double _linearGain = 1.0;
    double _linearOffset = 0.0;
    double _piControllerKp = 1.0;
    double _piControllerKi = 2.0;
    double _piControllerKt = 2.0;
    double _piControllerDebugE = 0.0;
    double _piControllerDebugP = 0.0;
    double _piControllerDebugI = 0.0;
    double _piSetpointOffset = 0.0;
    double _setpointValue = 0.0;
};
