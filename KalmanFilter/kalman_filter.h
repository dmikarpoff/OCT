#pragma once

#include "oct_utils/scan_data.h"

#include <QObject>
#include <QMessageBox>

namespace oct {

class KalmanFilter : public QObject
{
    Q_OBJECT
 public:
    struct Status {
        QMessageBox::Icon icon;
        QString caption;
        QString message;
    };

    explicit KalmanFilter(QObject* parent) : QObject(parent) {}

    void filter(QString input_csv, QString output_csv, QString output_png) const;

 private:


    ScanData applyKalmanFiltering(const ScanData& source_data) const;
};

}  // namespace oct
