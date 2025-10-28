#include "command_parser.h"

Command_Parser::Command_Parser(QObject *parent)
    : QObject{parent}
{

}

bool Command_Parser::parseCommands(QString filePath, QString &invalidCommand)
{

    QFile commandsFile(filePath);
    if(commandsFile.exists())
    {
        if(commandsFile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            // QByteArray line=commandsFile.readAll();

            QTextStream in(&commandsFile);
            // QStringList commands;

            qInfo()<<"Commands:-";

            while (!in.atEnd())
            {
                QString line = in.readLine().trimmed();
                // if (line.startsWith("CMD:"))
                // {
                //     QString command = line.section("CMD:", 1, 1).trimmed();
                //     if (!command.isEmpty())
                //     {
                //         commands.append(command);
                //     }
                // }

                qInfo()<<line;
                // Skip empty lines
                if (line.isEmpty()) continue;

                // Split only on the first colon
                int colonIndex = line.indexOf(m_keyValueSeparator);
                // if (colonIndex == -1)
                // {
                //     continue;
                // }

                QString key = line.left(colonIndex).trimmed();
                qInfo()<<key<<": "<<m_supportedCommands.contains(key);
                if(!m_supportedCommands.contains(key))
                {
                    invalidCommand=key;
                    qInfo()<<"Invalid commands";
                    if(m_parsedCommands.size()>0)
                    {
                        m_parsedCommands.clear();
                    }
                    return false;

                }
                QString value = line.mid(colonIndex + 1).trimmed();

                // Now split the value by space
                QStringList values = value.split(m_valueSeparator, Qt::SkipEmptyParts);

                // Prepend the key to the value list
                values.prepend(key);
                m_parsedCommands.append(values);

            }
            commandsFile.close();
            // Print the parsed output
            for (QStringList &entry : m_parsedCommands)
            {
                qInfo() << entry;
            }
            return true;
        }
        else
        {
            qInfo()<<commandsFile.errorString();
            return false;
        }
    }
    else
    {
        qInfo()<<"File doesnot exist";
        return false;
    }
}
bool Command_Parser::parse(QString filePath, Test_Header_t &headerInfo, QList<Test_Info_t> &testsInfo, QString &invalidCommand, int &invalidLineCount)
{

    QFile commandsFile(filePath);
    if(commandsFile.exists())
    {
        if(commandsFile.open(QIODevice::ReadOnly | QIODevice::Text))
        {

            QTextStream in(&commandsFile);
            qInfo()<<"Commands:-";
            testsInfo.clear();
            headerInfo.clear();
            invalidLineCount=-1;
            int lineCounter=0;
            while (!in.atEnd())
            {
                QString line = in.readLine().trimmed();
                lineCounter++;
                // Skip empty lines
                if (line.isEmpty()) continue;

                // Split only on the first colon
                int colonIndex = line.indexOf(m_commentSeperator);

                QString command = line.left(colonIndex).trimmed();
                QString comment,testStr,reqStr;
                if(colonIndex!=-1)
                {
                    comment = line.mid(colonIndex+1).trimmed();
                    int cmtindex = comment.indexOf(m_keyValueSeparator);
                    testStr=comment.left(cmtindex).trimmed();
                    reqStr=comment.mid(cmtindex+1).trimmed();
                }

                if(!command.isEmpty())
                {
                    int cmdindex = command.indexOf(m_keyValueSeparator);

                    QString key=command.left(cmdindex).trimmed();
                    if(m_supportedCommands.contains(key))
                    {
                        QString valuesStr=command.mid(cmdindex+1).trimmed();
                        QStringList values=valuesStr.split(m_valueSeparator,Qt::SkipEmptyParts);
                        int size=values.size();
                        if(m_supportedCommands.value(key)==size)
                        {
                            bool toPrint= ((key=="TEST")|| (key=="ITEST") || (key=="IQPLOTD"));
                            Test_Info_t testInfo;
                            testInfo.toPrint=toPrint;
                            testInfo.command=key;
                            testInfo.firstParam=values.at(0).trimmed();
                            testInfo.secondParam= (size>1) ? values.at(1).trimmed():"";
                            testInfo.thirdParam=(size>2) ? values.at(2).trimmed():"";
                            testInfo.fourthParam=(size>3) ? values.at(3).trimmed():"";
                            testInfo.testDescription=testStr;
                            testInfo.testReqirement=reqStr;

                            testsInfo.append(testInfo);
                        }
                        else
                        {
                            invalidCommand=key;
                            invalidLineCount=lineCounter;
                            qInfo()<<"At line "<<lineCounter;
                            qInfo()<<"Required parameter count for "<<key<< " is "<<m_supportedCommands.value(key);
                            qInfo()<<"Given parameter count is "<<values.size();
                            return false;
                        }

                    }

                    else
                    {

                        invalidCommand=key;
                        invalidLineCount=lineCounter;
                        qInfo()<<"Invalid command at line no "<<invalidLineCount;
                        return false;
                    }

                }
                else
                {
                    if(!headerInfo.contains(testStr))
                    {
                        headerInfo[testStr]=reqStr;
                    }


                }
            }
            qInfo()<<"Total line count: "<<lineCounter;
            commandsFile.close();
            //return true;
            qInfo()<<"Header: "<<headerInfo;
            print(testsInfo);

        }
        else
        {
            return false;
        }

    }
    else
    {
        return false;
    }
    return true;
}
bool Command_Parser::parse_old(QString filePath, Test_Header_t &headerInfo, QList<Test_Info_t> &testsInfo, QString &invalidCommand)
{

    QFile commandsFile(filePath);
    if(commandsFile.exists())
    {
        if(commandsFile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            // QByteArray line=commandsFile.readAll();

            QTextStream in(&commandsFile);
            // QStringList commands;

            qInfo()<<"Commands:-";
            testsInfo.clear();
            headerInfo.clear();
            while (!in.atEnd())
            {
                QString line = in.readLine().trimmed();
                // if (line.startsWith("CMD:"))
                // {
                //     QString command = line.section("CMD:", 1, 1).trimmed();
                //     if (!command.isEmpty())
                //     {
                //         commands.append(command);
                //     }
                // }

                // qInfo()<<"Line: "<<line;
                // Skip empty lines
                if (line.isEmpty()) continue;

                // Split only on the first colon
                int colonIndex = line.indexOf(m_commentSeperator);
                if (colonIndex == -1)
                {
                    qInfo()<<"Without comment: "<<line;
                    int index = line.indexOf(m_keyValueSeparator);
                    if(index!=-1)
                    {
                        QString key=line.left(index).trimmed();
                        if(m_supportedCommands.contains(key))
                        {
                            QString value=line.mid(index+1).trimmed();

                            Test_Info_t testInfo={false,key,value,"","","",""};
                            testsInfo.append(testInfo);
                        }
                        else
                        {
                            invalidCommand=key;
                            return false;
                        }

                    }

                }
                else
                {
                    QString command = line.left(colonIndex).trimmed();
                    QString comment = line.mid(colonIndex+1).trimmed();
                    int cmdindex = command.indexOf(m_keyValueSeparator);
                    int cmtindex = comment.indexOf(m_keyValueSeparator);
                    QString testStr=comment.left(cmtindex).trimmed();
                    QString reqStr=comment.mid(cmtindex+1).trimmed();
                    if(!command.isEmpty())
                    {

                        QString key=command.left(cmdindex).trimmed();
                        if(m_supportedCommands.contains(key))
                        {
                            QString valuesStr=command.mid(cmdindex+1).trimmed();
                            QStringList values=valuesStr.split(m_valueSeparator,Qt::SkipEmptyParts);

                            Test_Info_t testInfo={true,key,values.at(0),values.at(1),values.at(2),testStr,reqStr};
                            testsInfo.append(testInfo);
                        }
                        else
                        {
                            invalidCommand=key;
                            return false;
                        }

                    }
                    else
                    {
                        //headers
                        //commands

                        // QString test=comment.left(cmtindex).trimmed();
                        // QString req=comment.mid(cmtindex+1).trimmed();
                        headerInfo[testStr]=reqStr;

                    }
                    qInfo()<<"Comment: "<<comment;
                    qInfo()<<"Command: "<<command;
                }

                // qInfo()<<key<<": "<<m_supportedCommands.contains(key);
            //     if(!m_supportedCommands.contains(key))
            //     {
            //         invalidCommand=key;
            //         qInfo()<<"Invalid commands";
            //         if(m_parsedCommands.size()>0)
            //         {
            //             m_parsedCommands.clear();
            //         }
            //         return false;

            //     }
            //     QString value = line.mid(colonIndex + 1).trimmed();

            //     // Now split the value by space
            //     QStringList values = value.split(m_valueSeparator, Qt::SkipEmptyParts);

            //     // Prepend the key to the value list
            //     values.prepend(key);
            //     m_parsedCommands.append(values);

            // }

            // Print the parsed output
            // for (QStringList &entry : m_parsedCommands)
            // {
            //     qInfo() << entry;
            // }

            }
            commandsFile.close();
            //return true;
            qInfo()<<"Header: "<<headerInfo;
            print(testsInfo);

        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
    return true;
}

// void Command_Parser::createPDF(QString pdfFilePath)
// {

// }

void Command_Parser::print(const QList<Test_Info_t> &tests)
{
    qInfo("");
    for(const Test_Info_t &val:tests)
    {

        qInfo()<<"Print to PDF: "<<val.toPrint;
        qInfo()<<"Command: "<<val.command;
        qInfo()<<"First parameter: "<<val.firstParam;
        qInfo()<<"Second parameter: "<<val.secondParam;
        qInfo()<<"Third parameter: "<<val.thirdParam;
        qInfo()<<"Fourth parameter: "<<val.fourthParam;
        qInfo()<<"Test description: "<<val.testDescription;
        qInfo()<<"Test requirements: "<<val.testReqirement<<"\n";
    }
}
