#ifndef TIMESERIES_DATA_H
#define TIMESERIES_DATA_H

#include <QtCore/QDateTime>

#define TIME_SERIES_CAPACITY (60*60*24)

struct TimeSeriesData
{
    TimeSeriesData() { dt = QDateTime::currentDateTime(); value = 0.0; }
    TimeSeriesData(double val) : value(val) { dt = QDateTime::currentDateTime(); }
    QDateTime dt;
    double value;
};

#endif // TIMESERIES_DATA_H
