#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QDir>
#include <QSaveFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QMessageBox>
#include <QTimer>
#include <QSignalBlocker>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    void initializeConfigurationParams();
    void fillDataUIControls();

    bool createDefaultConfigurationFile();
    void readConfigurationFromJSON();
    void loadConfigurationValues(const QJsonObject &object);
    void calculateProfilePoints();
    void setUIControls();
    bool saveConfigurationToJSON();

private slots:
    void on_saveBtn_clicked();

    void on_measurementRangeXCombo_currentIndexChanged(int index);

    void on_thinningCombo_currentIndexChanged(int index);

    void on_samplingCycleCombo_currentIndexChanged(int index);

    void on_luminanceOnRadio_toggled(bool checked);

    void on_restoreBtn_clicked();

    void on_selectFolderBtn_clicked();

private:
    Ui::MainWindow *ui;

    QJsonObject configurationObject;
    QString configurationFilePath;
    bool configurationLoaded = false;

    QString outputDirectory;
    QString ipAddress;
    int port;
    int deviceId;
    QString unit;
    bool luminance;
    QString captureMode;
    int periodTime;
    int intervalTime;
    QString measurementRangeX;
    QString thinning;
    int samplingCycle;

    bool writeConfigurationData(const QByteArray &jsonData);
};
#endif // MAINWINDOW_H
