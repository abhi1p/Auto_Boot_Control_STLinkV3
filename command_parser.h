#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H

#include <QObject>
#include <QFile>
#include <QDir>
#include <QRegularExpression>
#include "parameters.h"

/*
 *
 * CMD:<Command to send> -> (command) Send command
 * CMDR:<Command to send> <Response to verify> -> (Command with Response) Send commands and verify response
 * CMDGV:<Command to send> <value1> <value2> -> (Command Get Value) command to get value1 and value2
 * CMDBLEV:<Command to send> <Response to verify> <Name of BLE device to scan> -> (Command BLE Verify) command to start BLE advertising and scan for BLE deivce name
 *
 * */

class Command_Parser : public QObject
{
    Q_OBJECT
public:
    explicit Command_Parser(QObject *parent = nullptr);

public slots:
    bool parseCommands(QString filePath, QString &invalidCommand);
    bool parse_old(QString filePath, Test_Header_t &headerInfo, QList<Test_Info_t> &testsInfo, QString &invalidCommand);
    bool parse(QString filePath, Test_Header_t &headerInfo, QList<Test_Info_t> &testsInfo, QString &invalidCommand, int &invalidLineCount);

signals:

private slots:
    void print(const QList<Test_Info_t> &tests);
private:
    QString m_commentSeperator = "#";
    QString m_keyValueSeparator = ":";
    QString m_valueSeparator = ",";
    // QList<Test_Info_t> m_testsInfo;
    // QMap<QString,QString> m_headerInfo;
    // QMap
    // QRegularExpression m_valueSeparator=QRegularExpression("\\s+");
    // QStringList m_supportedCommands={"TEST","DELAY","TESTBLE",};
    QMap<QString,int> m_supportedCommands={{"WRITE",1},{"TEST",3},{"DELAY",1},{"TESTBLE",3},{"ICNT",2},
                                              {"IDCNT",1},{"IQUERYL",4},{"IQUERYS",3},{"IREADS",2},
                                              {"IWRITE",2},{"IQPLOTBD",3},{"ITEST",3},{"IQUERY",2}};
    QList<QStringList> m_parsedCommands;
};

#endif // COMMAND_PARSER_H
