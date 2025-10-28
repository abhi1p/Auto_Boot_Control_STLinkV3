#ifndef INSTRUMENT_H
#define INSTRUMENT_H

#include <QObject>
#include <visa.h>
#include <QString>
#include <QStringList>
#include <QDebug>

typedef QPair<QString,QString> Instrument_t;
typedef QList<Instrument_t> Instruments_t;

class Instrument : public QObject
{
    Q_OBJECT
public:
    explicit Instrument(QObject *parent = nullptr);
    ~Instrument();

public slots:
    Instruments_t listInstruments(bool onlyResponsive = true);
    QString queryInstrument(const QString& resource, const QString& command = "*IDN?");
    bool writeInstrument(const QString& resource, const QString& command);
    bool isResponsive(const QString& resource, QString &idn,const QString& command = "*IDN?");
    void testMode();
    void readData(const QString& resource, QVector<double> &freq,QVector<double> &gain,QVector<double> &phase);
signals:

private:
    ViSession defaultRM;
    bool visaInitialized;
};

#endif // INSTRUMENT_H
