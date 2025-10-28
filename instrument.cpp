#include "instrument.h"

Instrument::Instrument(QObject *parent)
    : QObject{parent},visaInitialized(false)
{
    ViStatus status = viOpenDefaultRM(&defaultRM);
    visaInitialized = (status >= VI_SUCCESS);
    if (!visaInitialized)
        qWarning() << "Failed to open VISA resource manager.";
}


Instrument::~Instrument()
{
    if (visaInitialized)
    {
        viClose(defaultRM);
    }
}

Instruments_t Instrument::listInstruments(bool onlyResponsive)
{
    Instruments_t foundInstruments;

    if (!visaInitialized)
    {

        return foundInstruments;
    }

    ViFindList findList;
    ViUInt32 numInstruments;
    ViChar instrDesc[VI_FIND_BUFLEN];
    ViStatus status;

    status = viFindRsrc(defaultRM, (ViString)"USB?*", &findList, &numInstruments, instrDesc);
    if (status < VI_SUCCESS)
    {
        qWarning() << "No VISA instruments found.";
        return foundInstruments;
    }

    QString idn;
    // First instrument
    QString resource = QString::fromLatin1(instrDesc);
    if (!onlyResponsive || isResponsive(resource,idn))
    {
        Instrument_t instr;
        instr.first=resource;
        instr.second=idn;
        foundInstruments.append(instr);
    }

    // Next instruments
    for (ViUInt32 i = 1; i < numInstruments; ++i)
    {
        status = viFindNext(findList, instrDesc);
        if (status >= VI_SUCCESS)
        {
            resource = QString::fromLatin1(instrDesc);
            if (!onlyResponsive || isResponsive(resource,idn))
            {
                Instrument_t instr;
                instr.first=resource;
                instr.second=idn;
                foundInstruments.append(instr);
            }
        }
    }

    viClose(findList);
    return foundInstruments;
}

QString Instrument::queryInstrument(const QString& resource, const QString& command)
{
    if (!visaInitialized)
        return QString();

    ViSession session;
    ViUInt32 retCount;
    ViChar buffer[256] = {0};

    ViStatus status = viOpen(defaultRM, (ViRsrc)resource.toLatin1().constData(), VI_NULL, VI_NULL, &session);
    if (status < VI_SUCCESS)
        return QString();

    // Set timeout (e.g., 2 seconds)
    viSetAttribute(session, VI_ATTR_TMO_VALUE, 2000);

    // Write command
    status = viWrite(session, (ViBuf)command.toLatin1().constData(), command.length(), &retCount);
    if (status < VI_SUCCESS) {
        viClose(session);
        return QString();
    }

    // Read response
    status = viRead(session, (ViBuf)buffer, sizeof(buffer) - 1, &retCount);
    viClose(session);

    if (status < VI_SUCCESS)
        return QString();

    buffer[retCount] = '\0';
    return QString::fromLatin1(buffer);
}

bool Instrument::writeInstrument(const QString &resource, const QString &command)
{
    if (!visaInitialized)
        return false;

    ViSession session;
    ViUInt32 retCount;

    ViStatus status = viOpen(defaultRM, (ViRsrc)resource.toLatin1().constData(), VI_NULL, VI_NULL, &session);
    if (status < VI_SUCCESS)
        return false;

    // Set timeout (optional)
    viSetAttribute(session, VI_ATTR_TMO_VALUE, 2000);

    // Write the command
    status = viWrite(session, (ViBuf)command.toLatin1().constData(), command.length(), &retCount);
    viClose(session);

    return (status >= VI_SUCCESS);
}

bool Instrument::isResponsive(const QString& resource, QString &idn, const QString& command)
{
    QString response = queryInstrument(resource, command);
    idn=response.trimmed();
    // qInfo()<<"Response:" <<response;
    return !idn.isEmpty();
}

void Instrument::testMode()
{

}

void Instrument::readData(const QString& resource, QVector<double> &freq,QVector<double> &gain,QVector<double> &phase)
{
    if (!visaInitialized)
        return;

    // QVector<float> result;
    ViSession session;
    ViUInt32 retCount;

    ViStatus status = viOpen(defaultRM, (ViRsrc)resource.toLatin1().constData(), VI_NULL, VI_NULL, &session);
    if (status < VI_SUCCESS)
        return;

    // Set timeout (optional)
    viSetAttribute(session, VI_ATTR_TMO_VALUE, 2000);

    // Write the command
    QString command="FRANalysis:DATA?\n";
    status = viWrite(session, (ViBuf)command.toLatin1().constData(), command.length(), &retCount);

    // Read header first (IEEE block)
    char header[11] = {0}; // Max # header is 2 + 9 digits + 1 (safe side)
    viRead(session, (ViBuf)header, 2, &retCount); // e.g. "#4"
    if (header[0] != '#' || !isdigit(header[1])) {
        qWarning() << "Invalid block header.";
        viClose(session);
        return;
    }


    int numDigits = header[1] - '0';
    viRead(session, (ViBuf)(header + 2), numDigits, &retCount); // Read size part
    header[2 + numDigits] = '\0';
    int byteCount = QString(header + 2).toInt();
    qInfo()<<"Data size: "<<byteCount;

    // Read the full binary data block
    QByteArray data(byteCount, 0);
    status = viRead(session, (ViBuf)data.data(), byteCount, &retCount);
    if (status < VI_SUCCESS) {
        qWarning() << "Failed to read FRA data.";
        viClose(session);
        return;
    }

    // QByteArray buffer;
    // Split into lines and parse
    QList<QByteArray> lines = data.split('\n');
    for (int i = 1; i < lines.size(); ++i)// Skip header
    {
        const QByteArray& line = lines.at(i).trimmed();
        if (line.isEmpty()) continue;

        QList<QByteArray> fields = line.split(',');
        if (fields.size() < 4) continue;
        bool ok;
        double f = fields.at(1).trimmed().toDouble(&ok);
        double g = fields.at(3).trimmed().toDouble(&ok);
        double p = fields.at(4).trimmed().toDouble(&ok);
        freq.append(f);
        gain.append(g);
        phase.append(p);
        // QVector<double> row;
        // for (const QByteArray& field : std::as_const(fields))
        // {
        //     bool ok;
        //     double val = field.trimmed().toDouble(&ok);
        //     row.append(ok ? val : NAN);
        // }
        //result.append(row);
    }

    viClose(session);


}
