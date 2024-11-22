#include "controller.h"

Controller::Controller(QObject *parent)
    : QObject{parent}
{
    QObject::connect(this, &Controller::processingCompleted, this, &Controller::onProcessingComplete);
}

void Controller::readConfigFile()
{
    QString configFilePath = QApplication::applicationDirPath() + "/config/configuration.json";

    Logger::getLogger()->info("{} {}", "Read configuration file:", configFilePath.toStdString());

    QFile configFile(configFilePath);
    if (!configFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return;
    }

    // read json file to Byte array
    QByteArray data = configFile.readAll();
    configFile.close();

    // parse json file as json document with error checking
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError)
    {
        qDebug() << "Error parsing JSON: " << error.errorString();
        return;
    }

    // check if json file contains only objects
    if (!jsonDoc.isObject())
    {
        qDebug() << "Json file doesn't contain objects";
        return;
    }

    QJsonObject rootObject = jsonDoc.object();
    configParams.outputDirectory = rootObject["OutputDirectory"].toString();
    configParams.ip = rootObject["IP"].toString();
    configParams.port = rootObject["Port"].toInt();
    configParams.deviceId = rootObject["DeviceId"].toInt();
    configParams.unit = rootObject["Unit"].toString();
    configParams.luminance = rootObject["Luminance"].toBool();
    configParams.captureMode = rootObject["CaptureMode"].toString();
    configParams.periodTime = rootObject["PeriodTime"].toInt();
    configParams.intervalTime = rootObject["IntervalTime"].toInt();

    Logger::getLogger()->info("{} {}, {}, {}, {}, {}, {}, {}, {}, {}", "Configuration params: ",
                              configParams.outputDirectory.toStdString(),
                              configParams.ip.toStdString(),
                              configParams.port,
                              configParams.deviceId,
                              configParams.unit.toStdString(),
                              configParams.luminance,
                              configParams.captureMode.toStdString(),
                              configParams.periodTime,
                              configParams.intervalTime);
}

void Controller::writeCSVFile(const std::vector<std::vector<double>> &multiData)
{
    Logger::getLogger()->info("{} {}", "Write data to csv file:", configParams.outputDirectory.toStdString() + "/profile_height.csv");

    QDir saveExportDir(configParams.outputDirectory);
    if (!saveExportDir.exists())
    {
        if (!saveExportDir.mkpath(configParams.outputDirectory))
        {
            QMessageBox::warning(nullptr, "Error", "Faild to create directory.");
        }
    }


    QString profileHeightFilePath = configParams.outputDirectory + "/profile_height.csv";
    QFile fileProfileHeight(profileHeightFilePath);

    if (!fileProfileHeight.open(QIODevice::Text | QIODevice::WriteOnly))
    {
        qDebug() << "Unable to open file" << profileHeightFilePath <<  "for writing";
        return;
    }

    QTextStream stream(&fileProfileHeight);

    // write measurement content
    for (int n = 0; n < multiData.size(); n++)
    {
        auto data = multiData.at(n);
        for (int i = 0; i < data.size(); i++)
        {
            stream << data[i] << ", ";
        }

        stream << "\n";
    }

    fileProfileHeight.close();
}

void Controller::initializeDLL()
{
    LONG lRc = LJX8IF_Initialize();    // 0-ok
    Logger::getLogger()->info("LJX8IF_Initialize");
}

void Controller::finializeDLL()
{
    LONG lRc = LJX8IF_Finalize();   // 0-ok
    Logger::getLogger()->info("LJX8IF_Finalize");
}

void Controller::getVersion()
{
    LJX8IF_VERSION_INFO versionInfo = LJX8IF_GetVersion();
    Logger::getLogger()->info("{}:{}.{}.{}.{}", "getVersion", versionInfo.nMajorNumber, versionInfo.nMinorNumber, versionInfo.nRevisionNumber, versionInfo.nBuildNumber);
    qDebug() << "Version: " << versionInfo.nMajorNumber << "." << versionInfo.nMinorNumber << "." << versionInfo.nRevisionNumber << "." << versionInfo.nBuildNumber;
}

bool Controller::openEthernet()
{
    LJX8IF_ETHERNET_CONFIG ethernetConfig;
    QStringList ipFields = configParams.ip.split(".");
    ethernetConfig.abyIpAddress[0] = static_cast<quint8>(ipFields[0].toInt());
    ethernetConfig.abyIpAddress[1] = static_cast<quint8>(ipFields[1].toInt());
    ethernetConfig.abyIpAddress[2] = static_cast<quint8>(ipFields[2].toInt());
    ethernetConfig.abyIpAddress[3] = static_cast<quint8>(ipFields[3].toInt());
    ethernetConfig.wPortNo         = (WORD)configParams.port;
    ethernetConfig.reserve[0]      = (BYTE)0;
    ethernetConfig.reserve[1]      = (BYTE)0;

    LONG lRc = LJX8IF_EthernetOpen((LONG)configParams.deviceId, &ethernetConfig);
    bool connectionStatus = (lRc == LJX8IF_RC_OK) ? true : false;

    Logger::getLogger()->info("LJX8IF_EthernetOpen: Open Ethernet Communication");

    return connectionStatus;
}

bool Controller::closeEthernet()
{
    LONG lRc = LJX8IF_CommunicationClose((LONG)configParams.deviceId);
    bool connectionStatus = (lRc == LJX8IF_RC_OK) ? true : false;

    Logger::getLogger()->info("LJX8IF_CommunicationClose: Close Ethernet Communication");

    return connectionStatus;
}

bool Controller::triggerMeasurement()
{
    LONG lRc = LJX8IF_Trigger((LONG)configParams.deviceId);
    Logger::getLogger()->info("LJX8IF_Trigger: Trigger Measurement is sent");

    bool triggerStatus = (lRc == LJX8IF_RC_OK) ? true : false;

    return triggerStatus;
}

bool Controller::startMeasurement()
{
    LONG lRc = LJX8IF_StartMeasure((LONG)configParams.deviceId);
    Logger::getLogger()->info("LJX8IF_StartMeasure: Start Measurement");

    bool startStatus = (lRc == LJX8IF_RC_OK) ? true : false;

    return startStatus;
}

bool Controller::stopMeasurement()
{
    LONG lRc = LJX8IF_StopMeasure((LONG)configParams.deviceId);
    Logger::getLogger()->info("LJX8IF_StopMeasure: Stop Measurement");

    bool stopStatus = (lRc == LJX8IF_RC_OK) ? true : false;

    return stopStatus;
}

int Controller::getOneProfileDataSize()
{
    int nMultipleValueForLuminanceOutput = configParams.luminance ? 2 : 1;
    int GetXDirectionDataCount =  3200 * nMultipleValueForLuminanceOutput;

    int nProfileCount = GetXDirectionDataCount;
    // Buffer size (in units of bytes)
    UINT oneProfileBufferSize = 0;

    // Number of headers
    oneProfileBufferSize += (UINT)nProfileCount;

    //in units of bytes
    oneProfileBufferSize *= sizeof(UINT);

    oneProfileBufferSize += sizeof(LJX8IF_PROFILE_HEADER);					// Sizes of the header and footer structures
    oneProfileBufferSize += sizeof(LJX8IF_PROFILE_FOOTER);

    return oneProfileBufferSize;
}

void Controller::logResponse(LJX8IF_GET_PROFILE_RESPONSE response)
{
    Logger::getLogger()->info("{} --> {}:{}, {}:{}", "Response",
                              "Current Profile No", response.dwCurrentProfileNo,
                              "Profile Count", response.byGetProfileCount);

}

void Controller::logProfileInfo(LJX8IF_PROFILE_INFO profileInfo)
{
    Logger::getLogger()->info("{} --> {}:{}, {}:{}", "Profile Info",
                              "Profile Count", profileInfo.byProfileCount,
                              "Luminance Output", (profileInfo.byLuminanceOutput == 1) ? "ON" : "OFF",
                              "Profile Data Count", profileInfo.wProfileDataCount);
}

std::vector<PROFILE_DATA> Controller::analyzeProfileData(BYTE byGetProfileCount, LJX8IF_PROFILE_INFO profileInfo, std::vector<int> profileData)
{
    std::vector<PROFILE_DATA> vecProfileData;

    int nMultipleValue = (profileInfo.byLuminanceOutput == 1) ? 2 : 1;
    int nDataUnitSize = (sizeof(LJX8IF_PROFILE_HEADER) + sizeof(int) * profileInfo.wProfileDataCount * profileInfo.byProfileCount * nMultipleValue + sizeof(LJX8IF_PROFILE_FOOTER)) / sizeof(int);
    for (int i = 0; i < byGetProfileCount; i++)
    {
        int *pnBlock = &profileData.at(nDataUnitSize * i);

        LJX8IF_PROFILE_HEADER* pHeader = (LJX8IF_PROFILE_HEADER*)pnBlock;
        int* pnProfileData = pnBlock + (sizeof(LJX8IF_PROFILE_HEADER) / sizeof(DWORD));
        LJX8IF_PROFILE_FOOTER* pFooter = (LJX8IF_PROFILE_FOOTER*)(pnProfileData + profileInfo.wProfileDataCount * profileInfo.byProfileCount * nMultipleValue);

        // Store the profile data
        vecProfileData.push_back(PROFILE_DATA(profileInfo, pHeader, pnProfileData, pFooter));
    }

    return vecProfileData;
}

void Controller::getProfile(std::vector<PROFILE_DATA> &vecProfileDataResult)
{
    Logger::getLogger()->info("LJX8IF_GetProfile: Get Profile");

    // define and set the request for getting the profile data
    LJX8IF_GET_PROFILE_REQUEST request;
    request.byTargetBank = 0;
    request.byPositionMode = 0;
    request.dwGetProfileNo = 0;    // first profile
    request.byGetProfileCount = 1;
    request.byErase = 0;
    request.reserve[0]   = 0;
    request.reserve[1]   = 0;
    request.reserve2[0]  = 0;
    request.reserve2[1]  = 0;

    DWORD dwOneProfileDataBufferSize = getOneProfileDataSize();
    DWORD dwDataSize = dwOneProfileDataBufferSize * request.byGetProfileCount;
    std::vector<int> vecProfileData(dwDataSize/sizeof(DWORD));

    LJX8IF_GET_PROFILE_RESPONSE response;
    LJX8IF_PROFILE_INFO profileInfo;
    LONG lRc = LJX8IF_GetProfile((LONG)configParams.deviceId, &request, &response, &profileInfo, (DWORD*)&vecProfileData.at(0), dwDataSize);

    if (lRc != LJX8IF_RC_OK)
        return;

    // log response and profileInfo result
    logResponse(response);
    logProfileInfo(profileInfo);

    vecProfileDataResult = analyzeProfileData(response.byGetProfileCount, profileInfo, vecProfileData);
}

void Controller::runProcessing()
{
    readConfigFile();  // json
    initializeDLL();   // LJX DLL
    getVersion();
    bool connectionStatus = openEthernet();

    connectionStatus = true;  // pass for debug only  --> remove

    if (connectionStatus)    //connectionStatus
    {
        Logger::getLogger()->info("Successful to open the device");
        qDebug() << "Successful to open the device";

        if (QString(configParams.captureMode).toLower() == "single")    // only get one profile
        {
            qDebug() << "run processing single profile...";

            multiData.clear();
            std::vector<PROFILE_DATA> vecProfileDataResult;
            scanProfile(vecProfileDataResult);
            multiData.push_back(vecProfileDataResult);

            emit processingCompleted();
        }
        else                                                        // multi profiles for x period at y intervals
        {
            qDebug() << "run processing multi profile...";

            multiData.clear();

            // main period timer
            periodTimer.setSingleShot(true);    // single shot timer to cover the requested speriod
            periodTimer.setInterval(configParams.periodTime);
            connect(&periodTimer, &QTimer::timeout, this, &Controller::onPeriodTime);
            periodTimer.start();

            scanTimer.setInterval(configParams.intervalTime);
            connect(&scanTimer, &QTimer::timeout, this, &Controller::onScanTime);
            scanTimer.start();
        }
    }
    else
    {
        Logger::getLogger()->info("Failed to open the device");
        qDebug() << "Failed to open the device";
    }
}

void Controller::onProcessingComplete()
{
    bool isSaveHeightSuccess = false;
    bool isSaveLuminaceSuccess = false;

    if (configParams.captureMode == "single")
    {
        // write profile height (and/or luminance) to csv file
        if (multiData.size() == 1)
        {
            std::vector<PROFILE_DATA> vecProfileDataResult = multiData[0];
            if (vecProfileDataResult.size() > 0)
            {
                QString fileNameHeight = configParams.outputDirectory + "/Height.csv";
                isSaveHeightSuccess = exportData(&(vecProfileDataResult.at(0)),
                                                 fileNameHeight.toStdString().c_str(),
                                                 (int)vecProfileDataResult.size(),
                                                 0);


                if (vecProfileDataResult[0].m_profileInfo.byLuminanceOutput == 1)   // Luminance ON
                {
                    QString fileNameLuminance = configParams.outputDirectory + "/Luminance.csv";
                    isSaveLuminaceSuccess = exportData(&(vecProfileDataResult.at(0)),
                                                       fileNameLuminance.toStdString().c_str(),
                                                       (int)vecProfileDataResult.size(),
                                                       vecProfileDataResult[0].m_profileInfo.wProfileDataCount);


                }
            }
        }
    }
    else
    {
        if (multiData.size() > 0)
        {
            QString fileNameHeight = configParams.outputDirectory + "/Height.csv";
            QFile fileHeight(fileNameHeight);
            if (fileHeight.open(QIODevice::WriteOnly | QIODevice::Text))
            {
                QTextStream outHeightStream(&fileHeight);
                outHeightStream.setEncoding(QStringConverter::Utf16LE); // Set UTF-16 Little Endian encoding
                outHeightStream.setGenerateByteOrderMark(true);     // Write BOM

                bool openLuminanceFile = false;
                QFile fileLuminance;
                QTextStream outLuminanceStream;
                std::vector<PROFILE_DATA> vecProfileDataResult = multiData[0];     // get profile to check for --> Luminance ON
                if (vecProfileDataResult.size() > 0)
                {
                    if (vecProfileDataResult[0].m_profileInfo.byLuminanceOutput == 1)     // Luminance ON
                    {
                        QString fileNameLuminance = configParams.outputDirectory + "/Luminance.csv";
                        fileLuminance.setFileName(fileNameLuminance);
                        openLuminanceFile = fileLuminance.open(QIODevice::WriteOnly | QIODevice::Text);
                        if (openLuminanceFile)
                        {
                            outLuminanceStream.setDevice(&fileLuminance);
                            outLuminanceStream.setEncoding(QStringConverter::Utf16LE); // Set UTF-16 Little Endian encoding
                            outLuminanceStream.setGenerateByteOrderMark(true);     // Write BOM
                        }
                    }
                }


                for (int i = 0; i < multiData.size(); i++)
                {
                    std::vector<PROFILE_DATA> vecProfileDataResult = multiData[i];
                    if (vecProfileDataResult.size() > 0)
                    {
                        isSaveHeightSuccess = exportMultiData(&(vecProfileDataResult.at(0)), outHeightStream, (int)vecProfileDataResult.size(), 0);

                        if (vecProfileDataResult[0].m_profileInfo.byLuminanceOutput == 1)   // Luminance ON
                        {
                            isSaveLuminaceSuccess = exportMultiData(&(vecProfileDataResult.at(0)),
                                                                    outLuminanceStream,
                                                                    (int)vecProfileDataResult.size(),
                                                                    vecProfileDataResult[0].m_profileInfo.wProfileDataCount);
                        }
                    }
                }

                fileHeight.close();
                if (openLuminanceFile)
                {
                    fileLuminance.close();
                }
            }
        }
    }

    Logger::getLogger()->info("{} {}", "Save Profile Height to csv is done", isSaveHeightSuccess);
    Logger::getLogger()->info("{} {}", "Save Lumiinance to csv is done", isSaveLuminaceSuccess);

    closeEthernet();
    finializeDLL();

    multiData.clear();

    qDebug() << "Finish.";
}



void Controller::scanProfile(std::vector<PROFILE_DATA> &vecProfileDataResult)
{
    bool triggerStatus = triggerMeasurement();
    if (triggerStatus)   // get profile of height
    {

    }

    getProfile(vecProfileDataResult);
}

void Controller::onPeriodTime()
{
    qDebug() << "performMeasurements is finished";
    // when finish stop the scan timer and emit signal to close the ethernet and finialize the DLL
    scanTimer.stop();
    emit processingCompleted();
}

void Controller::onScanTime()
{
    std::vector<PROFILE_DATA> vecProfileDataResult;
    scanProfile(vecProfileDataResult);
    multiData.push_back(vecProfileDataResult);

    qDebug() << "performMeasurements...";
}

bool Controller::exportData(const PROFILE_DATA *profileData, QString strFileName, int nProfileCount, int nDataStartIndex)
{
    QFile file(strFileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return false;  // Failed to open the file
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf16LE); // Set UTF-16 Little Endian encoding
    out.setGenerateByteOrderMark(true);     // Write BOM
    //out << QChar(0xFEFF);      // Write BOM

    // Write ProfileData
    for (int i = 0; i < nProfileCount; i++)
    {
        for (int k = 0; k < profileData[i].m_profileInfo.wProfileDataCount; k++) // Profile Data
        {
            int dataValue = profileData[i].m_pnProfileData[nDataStartIndex + k];
            double unitValue = double(dataValue) / 100000.0;    // convert  to um
            if (configParams.unit == "inch")
            {
                unitValue = unitValue / 25.4;
            }

            out << unitValue << "\t";  // Write data and a tab
        }
        out << "\r\n";  // Add a new line (CRLF)
    }

    file.close();
    return true;
}

bool Controller::exportMultiData(const PROFILE_DATA *profileData, QTextStream &out, int nProfileCount, int nDataStartIndex)
{
    // Write ProfileData
    for (int i = 0; i < nProfileCount; i++)
    {
        for (int k = 0; k < profileData[i].m_profileInfo.wProfileDataCount; k++) // Profile Data
        {
            int dataValue = profileData[i].m_pnProfileData[nDataStartIndex + k];
            double unitValue = double(dataValue) / 100000.0;   // convert  to um
            if (configParams.unit == "inch")
            {
                unitValue = unitValue / 25.4;
            }
            out << unitValue << "\t";  // Write data and a tab
        }
        out << "\r\n";  // Add a new line (CRLF)
    }

    return true;
}
