#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include <QFile>
#include <QDir>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QApplication>
#include <QTimer>
#include <algorithm>
#include <fstream>
#include <iostream>
#include "logger.h"
#include <Windows.h>
#include "LJX8_IF.h"
#include "LJX8_ErrorCode.h"

struct PROFILE_DATA
{
    LJX8IF_PROFILE_INFO m_profileInfo;

    LJX8IF_PROFILE_HEADER m_profileHeader;
    int* m_pnProfileData;
    LJX8IF_PROFILE_FOOTER m_profileFooter;

    PROFILE_DATA() : m_pnProfileData(NULL)
    {

    }

    PROFILE_DATA(const LJX8IF_PROFILE_INFO &profileInfo, const LJX8IF_PROFILE_HEADER *header, const int *data, const LJX8IF_PROFILE_FOOTER *footer)
    {
        m_profileInfo = profileInfo;

        m_profileHeader = *header;

        int nMultipleValue = GetIsLuminanceOutput(profileInfo) ? 2 : 1;
        int nReceiveDataSize = profileInfo.wProfileDataCount * nMultipleValue * profileInfo.byProfileCount;
        m_pnProfileData = new int[nReceiveDataSize];
        memcpy_s(m_pnProfileData, sizeof(int) * nReceiveDataSize, data, sizeof(int) * nReceiveDataSize);

        m_profileFooter = *footer;
    }

    PROFILE_DATA(const PROFILE_DATA& obj)
    {
        m_profileInfo = obj.m_profileInfo;
        m_profileHeader = obj.m_profileHeader;
        m_profileFooter = obj.m_profileFooter;

        int nMultipleValue = GetIsLuminanceOutput(obj.m_profileInfo) ? 2 : 1;
        int nReceiveDataSize = obj.m_profileInfo.wProfileDataCount * obj.m_profileInfo.byProfileCount * nMultipleValue;
        m_pnProfileData = new int[nReceiveDataSize];
        for (int i = 0; i < nReceiveDataSize; i++)
        {
            m_pnProfileData[i] = obj.m_pnProfileData[i];
        }
    }

    BOOL GetIsLuminanceOutput(LJX8IF_PROFILE_INFO profileInfo)
    {
        return profileInfo.byLuminanceOutput == 1;
    }

    ~PROFILE_DATA()
    {
        delete[] m_pnProfileData;
    }
};

class Controller : public QObject
{
    Q_OBJECT
public:
    explicit Controller(QObject *parent = nullptr);
    void runProcessing();

    struct configuration {
        QString outputDirectory;
        QString ip;
        int port;
        int deviceId;
        QString unit;
        QString captureMode;
        bool luminance;
        int periodTime;
        int intervalTime;
        QString measurementRangeX;
        QString thinning;
        int samplingCycle;
    };

    QString getCaptureMode() const;

private:
    bool readConfigFile();
    bool initializeDLL();
    bool finializeDLL();
    void getVersion();
    bool openEthernet();
    bool closeEthernet();
    bool triggerMeasurement();
    void getProfile(std::vector<PROFILE_DATA> &vecProfileDataResult);

    void scanProfile(std::vector<PROFILE_DATA> &vecProfileDataResult);
    void onPeriodTime();
    void onScanTime();

    // profile related processing
    bool getIsXBinningOn();
    int getProfileCountByMeasureRange();
    int getDivideValueByThinning();
    int getProfileCount();
    int getOneProfileDataSize();
    void logResponse(LJX8IF_GET_PROFILE_RESPONSE response);
    void logProfileInfo(LJX8IF_PROFILE_INFO profileInfo);
    std::vector<PROFILE_DATA> analyzeProfileData(BYTE byGetProfileCount, LJX8IF_PROFILE_INFO profileInfo, std::vector<int> profileData);
    bool exportData(const PROFILE_DATA *profileData, QString strFileName, int nProfileCount, int nDataStartIndex);
    bool exportMultiData(const PROFILE_DATA *profileData, QTextStream &out, int nProfileCount, int nDataStartIndex);

public slots:   
    void onProcessingComplete();

signals:
    void notification(const QString &msg);
    void processingCompleted();
    void closingApp();
private:
    configuration configParams;
    QTimer *periodTimer;
    QTimer *scanTimer;

    std::vector<std::vector<PROFILE_DATA>> multiData;
};

#endif // CONTROLLER_H
