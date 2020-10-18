#pragma once

#include "oct_utils/scan_data.h"

#include <QObject>
#include <QMessageBox>
#include <QMetaType>

#include <boost/numeric/ublas/matrix.hpp>

#include <atomic>

namespace oct {

class KalmanFilter : public QObject
{
    Q_OBJECT
 public:
    using matrix_t = boost::numeric::ublas::matrix<double>;
    struct Status
    {
        QMessageBox::Icon icon;
        QString caption;
        QString message;
    };

    explicit KalmanFilter(QObject* parent = nullptr)
        : QObject(parent)
    {
        qRegisterMetaType<Status>();
    }

    void filter(QString input_csv, QString output_csv, QString output_png,
                boost::numeric::ublas::matrix<double> R_pr,
                boost::numeric::ublas::matrix<double> R_n);
    void filterAuto(QString input_csv, QString output_csv, QString output_png,
                    boost::numeric::ublas::matrix<double> R_w,
                    boost::numeric::ublas::matrix<double> R_n);
    void interrupt()
    {
        interrupted_ = true;
    }

 signals:
    void reportProgress(QString status, double progress) const;
    void finished(Status status) const;

 private:
    template <bool kAutoFilter>
    ScanData applyKalmanFiltering(const ScanData& source_data) const;
    void stop() const;
    void filterImpl(QString input_csv, QString output_csv, QString output_png);

    std::atomic<bool> interrupted_{false};
    boost::numeric::ublas::matrix<double> R_pr_;
    boost::numeric::ublas::matrix<double> R_n_;
    boost::numeric::ublas::matrix<double> R_w_;
    bool auto_r_pr_ = false;
};

}  // namespace oct

Q_DECLARE_METATYPE(oct::KalmanFilter::Status)
