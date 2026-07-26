#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QSignalBlocker>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setFixedSize(size());

    configurationFilePath = QDir(QCoreApplication::applicationDirPath()).filePath("config/configuration.json");

    fillDataUIControls();
    initializeConfigurationParams();
    readConfigurationFromJSON();
    setUIControls();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initializeConfigurationParams()
{
    outputDirectory = "D:\\Sixdigma";
    ipAddress = "192.168.0.100";
    port = 24691;
    deviceId = 0;
    unit = "mm";
    luminance = true;
    captureMode = "single";
    periodTime = 5000;
    intervalTime = 1000;
    measurementRangeX = "FULL";
    thinning = "OFF";
    samplingCycle = 1000;
}

void MainWindow::fillDataUIControls()
{
    // this to prevent the signals to be emitted while adding items only,
    // this is to not call the slots that are connected to these signals like calculateProfilePoints
    const QSignalBlocker measurementBlocker(ui->measurementRangeXCombo);
    const QSignalBlocker thinningBlocker(ui->thinningCombo);
    const QSignalBlocker samplingBlocker(ui->samplingCycleCombo);
    //

    ui->unitCombo->addItem("mm");
    ui->unitCombo->addItem("inch");

    ui->captureModeCombo->addItem("One-shot acquisition", "single");
    ui->captureModeCombo->addItem("Timed acquisition", "loop");

    ui->measurementRangeXCombo->addItem("FULL", 1.0);
    ui->measurementRangeXCombo->addItem("3/4", 0.75);
    ui->measurementRangeXCombo->addItem("1/2", 0.50);
    ui->measurementRangeXCombo->addItem("1/4", 0.25);

    ui->thinningCombo->addItem("OFF", 1.0);
    ui->thinningCombo->addItem("1/2", 0.50);
    ui->thinningCombo->addItem("1/4", 0.25);

    ui->samplingCycleCombo->addItem("10", 10);
    ui->samplingCycleCombo->addItem("20", 20);
    ui->samplingCycleCombo->addItem("50", 50);
    ui->samplingCycleCombo->addItem("100", 100);
    ui->samplingCycleCombo->addItem("200", 200);
    ui->samplingCycleCombo->addItem("500", 500);
    ui->samplingCycleCombo->addItem("1kHz", 1000);
    ui->samplingCycleCombo->addItem("1.5kHz", 1500);
    ui->samplingCycleCombo->addItem("2kHz", 2000);
    ui->samplingCycleCombo->addItem("2.5kHz", 2500);
    ui->samplingCycleCombo->addItem("3kHz", 3000);
    ui->samplingCycleCombo->addItem("3.5kHz", 3500);
    ui->samplingCycleCombo->addItem("4kHz", 4000);
    ui->samplingCycleCombo->addItem("4.5kHz", 4500);
    ui->samplingCycleCombo->addItem("5kHz", 5000);
    ui->samplingCycleCombo->addItem("6kHz", 6000);
    ui->samplingCycleCombo->addItem("7kHz", 7000);
    ui->samplingCycleCombo->addItem("8kHz", 8000);
    ui->samplingCycleCombo->addItem("10kHz", 10000);
    ui->samplingCycleCombo->addItem("12kHz", 12000);
    ui->samplingCycleCombo->addItem("14kHz", 14000);
    ui->samplingCycleCombo->addItem("16kHz", 16000);
}

bool MainWindow::createDefaultConfigurationFile()
{
    const QJsonObject defaultObject
        {
            {"OutputDirectory", outputDirectory},
            {"IP",              ipAddress},
            {"Port",            port},
            {"DeviceId",        deviceId},
            {"Unit",            unit},
            {"Luminance",       luminance},
            {"CaptureMode",     captureMode},
            {"PeriodTime",      periodTime},
            {"IntervalTime",    intervalTime},
            {"MeasuringRangeX", measurementRangeX},
            {"Thinning",        thinning},
            {"SamplingCycle",   samplingCycle}
        };

    // Create the config directory if it does not exist.
    const QString configDirectory = QFileInfo(configurationFilePath).absolutePath();

    if (!QDir().mkpath(configDirectory))
    {
        QMessageBox::warning(
            this,
            "Configuration",
            "Could not create the configuration directory:\n" +
                QDir::toNativeSeparators(configDirectory));

        return false;
    }

    const QJsonDocument jsonDocument(defaultObject);
    const QByteArray jsonData = jsonDocument.toJson(QJsonDocument::Indented);

    QSaveFile configFile(configurationFilePath);
    if (!configFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(
            this,
            "Configuration",
            "Could not create the configuration file:\n" +
                QDir::toNativeSeparators(configurationFilePath));

        return false;
    }

    if (configFile.write(jsonData) != jsonData.size())
    {
        configFile.cancelWriting();

        QMessageBox::warning(
            this,
            "Configuration",
            "Could not write the default configuration.");

        return false;
    }

    if (!configFile.commit())
    {
        QMessageBox::warning(
            this,
            "Configuration",
            "Could not save the default configuration file:\n" +
                QDir::toNativeSeparators(configurationFilePath));

        return false;
    }

    return true;
}

void MainWindow::readConfigurationFromJSON()
{
    // if configuration.json file somehow is deleted or not created before then create a default one
    if (!QFileInfo::exists(configurationFilePath))
    {
        if (!createDefaultConfigurationFile())
        {
            return;
        }

        QTimer::singleShot(0, this, [this]()
                           {
                               QMessageBox::information(
                                   this,
                                   "Configuration",
                                   "The configuration file was not found.\n\n"
                                   "A new configuration file was created with default values:\n" +
                                       QDir::toNativeSeparators(configurationFilePath));
                           });
    }

    // read the configuration.json as normal
    QFile configFile(configurationFilePath);
    if (!configFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::warning(this,
                             "Configuration",
                             "Could not open configuration file:\n" + QDir::toNativeSeparators(configurationFilePath));
        return;
    }

    const QByteArray data = configFile.readAll();
    configFile.close();

    QJsonParseError parseError;
    const QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError)
    {
        QMessageBox::warning(this,
                             "Configuration",
                             "Could not parse configuration file:\n"
                                 + parseError.errorString());
        return;
    }

    if (!jsonDoc.isObject())
    {
        QMessageBox::warning(this,
                             "Configuration",
                             "Configuration file must contain a JSON object.");
        return;
    }

    configurationObject = jsonDoc.object();
    configurationLoaded = true;

    loadConfigurationValues(configurationObject);
}

void MainWindow::setUIControls()
{
    // this to prevent the signals to be emitted while adding items only,
    // this is to not call the slots that are connected to these signals like calculateProfilePoints
    const QSignalBlocker luminanceOnBlocker(ui->luminanceOnRadio);
    const QSignalBlocker luminanceOffBlocker(ui->luminanceOffRadio);
    const QSignalBlocker measurementBlocker(ui->measurementRangeXCombo);
    const QSignalBlocker thinningBlocker(ui->thinningCombo);
    const QSignalBlocker samplingBlocker(ui->samplingCycleCombo);
    //

    ui->outputDirectoryEdit->setText(QDir::toNativeSeparators(outputDirectory));
    ui->ipEdit->setText(ipAddress);
    ui->portEdit->setText(QString::number(port));
    ui->deviceIdEdit->setText(QString::number(deviceId));

    const int unitIndex = ui->unitCombo->findText(unit);
    if (unitIndex >= 0)
    {
        ui->unitCombo->setCurrentIndex(unitIndex);
    }

    ui->luminanceOnRadio->setChecked(luminance);
    ui->luminanceOffRadio->setChecked(!luminance);

    int captureModeIndex = ui->captureModeCombo->findData(captureMode);
    if (captureModeIndex < 0)
    {
        captureModeIndex = ui->captureModeCombo->findText(captureMode);
    }
    if (captureModeIndex >= 0)
    {
        ui->captureModeCombo->setCurrentIndex(captureModeIndex);
    }

    ui->periodTimeSpinBox->setValue(periodTime);
    ui->intervalTimeSpinBox->setValue(intervalTime);

    int measurementRangeIndex = ui->measurementRangeXCombo->findText(measurementRangeX);
    if (measurementRangeIndex >= 0)
    {
        ui->measurementRangeXCombo->setCurrentIndex(measurementRangeIndex);
    }

    int thinningIndex = ui->thinningCombo->findText(thinning);
    if (thinningIndex >= 0)
    {
        ui->thinningCombo->setCurrentIndex(thinningIndex);
    }

    int samplingCycleIndex = ui->samplingCycleCombo->findData(samplingCycle);
    if (samplingCycleIndex >= 0)
    {
        ui->samplingCycleCombo->setCurrentIndex(samplingCycleIndex);
    }

    calculateProfilePoints();
}

bool MainWindow::saveConfigurationToJSON()
{
    if (!configurationLoaded)
    {
        QMessageBox::warning(
            this,
            "Configuration",
            "No valid configuration file has been loaded.");

        return false;
    }

    // Get the latest values from the UI.
    outputDirectory = ui->outputDirectoryEdit->text();
    ipAddress = ui->ipEdit->text();
    port = ui->portEdit->text().toInt();
    deviceId = ui->deviceIdEdit->text().toInt();

    unit = ui->unitCombo->currentText();
    luminance = ui->luminanceOnRadio->isChecked();
    captureMode = ui->captureModeCombo->currentData().toString();

    periodTime = ui->periodTimeSpinBox->value();
    intervalTime = ui->intervalTimeSpinBox->value();

    measurementRangeX = ui->measurementRangeXCombo->currentText();

    thinning = ui->thinningCombo->currentText();

    samplingCycle = ui->samplingCycleCombo->currentData().toInt();

    /*
     * Create a temporary updated object.
     *
     * configurationObject continues to represent the last
     * successfully saved configuration until commit() succeeds.
     */
    QJsonObject updatedObject = configurationObject;

    updatedObject["OutputDirectory"] = outputDirectory;
    updatedObject["IP"] = ipAddress;
    updatedObject["Port"] = port;
    updatedObject["DeviceId"] = deviceId;
    updatedObject["Unit"] = unit;
    updatedObject["Luminance"] = luminance;
    updatedObject["CaptureMode"] = captureMode;
    updatedObject["PeriodTime"] = periodTime;
    updatedObject["IntervalTime"] = intervalTime;
    updatedObject["MeasuringRangeX"] = measurementRangeX;
    updatedObject["Thinning"] = thinning;
    updatedObject["SamplingCycle"] = samplingCycle;

    const QJsonDocument jsonDocument(updatedObject);
    const QByteArray jsonData = jsonDocument.toJson(QJsonDocument::Indented);

    QSaveFile configFile(configurationFilePath);

    if (!configFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(
            this,
            "Configuration",
            "Could not open configuration file for writing:\n" +
                QDir::toNativeSeparators(
                    configurationFilePath));

        return false;
    }

    if (configFile.write(jsonData) != jsonData.size())
    {
        configFile.cancelWriting();

        QMessageBox::warning(
            this,
            "Configuration",
            "Could not write the configuration data.");

        return false;
    }

    if (!configFile.commit())
    {
        QMessageBox::warning(
            this,
            "Configuration",
            "Could not save configuration file:\n" +
            QDir::toNativeSeparators(configurationFilePath));

        return false;
    }

    /*
     * The disk write succeeded. The temporary object can now
     * become the new last-saved configuration.
     */
    configurationObject = updatedObject;

    return true;
}

void MainWindow::loadConfigurationValues(const QJsonObject &object)
{
    outputDirectory = object.value("OutputDirectory").toString();
    ipAddress = object.value("IP").toString();
    port = object.value("Port").toInt();
    deviceId = object.value("DeviceId").toInt();
    unit = object.value("Unit").toString();
    luminance = object.value("Luminance").toBool();
    captureMode = object.value("CaptureMode").toString();
    periodTime = object.value("PeriodTime").toInt();
    intervalTime = object.value("IntervalTime").toInt();
    measurementRangeX = object.value("MeasuringRangeX").toString();
    thinning = object.value("Thinning").toString();
    samplingCycle = object.value("SamplingCycle").toInt();
}

void MainWindow::calculateProfilePoints()
{
    const bool currentLuminance = ui->luminanceOnRadio->isChecked();
    const double measurementRangeFactor = ui->measurementRangeXCombo->currentData().toDouble();
    const double thinningFactor = ui->thinningCombo->currentData().toDouble();
    const int samplingCycle = ui->samplingCycleCombo->currentData().toInt();

    double currentSamplingCycleFactor = 1.0;
    if (currentLuminance &&
               (samplingCycle == 4500 ||
                samplingCycle == 5000 ||
                samplingCycle == 6000 ||
                samplingCycle == 7000 ||
                samplingCycle == 8000))
    {
        currentSamplingCycleFactor = 0.50;
    }
    else  if (!currentLuminance &&
               (samplingCycle == 12000 ||
                samplingCycle == 16000 ||
                samplingCycle == 10000 ||
                samplingCycle == 14000))
    {
        currentSamplingCycleFactor = 0.50;
    }

    const int profilePoints = static_cast<int>(3200.0 * measurementRangeFactor * thinningFactor * currentSamplingCycleFactor);

    ui->profilePointsLabel->setText(QString::number(profilePoints));
}

void MainWindow::on_saveBtn_clicked()
{
    const QMessageBox::StandardButton result = QMessageBox::question(
            this,
            "Confirm",
            "Do you want to save the current configuration?",
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);

    if (result == QMessageBox::Yes)
    {
        if (saveConfigurationToJSON())
        {
            QMessageBox::information(
                this,
                "Configuration",
                "The configuration was saved successfully.");
        }
    }
}


void MainWindow::on_measurementRangeXCombo_currentIndexChanged(int index)
{
    calculateProfilePoints();
}


void MainWindow::on_thinningCombo_currentIndexChanged(int index)
{
    calculateProfilePoints();
}


void MainWindow::on_samplingCycleCombo_currentIndexChanged(int index)
{
    calculateProfilePoints();
}


void MainWindow::on_luminanceOnRadio_toggled(bool checked)
{
    calculateProfilePoints();
}


void MainWindow::on_restoreBtn_clicked()
{
    if (!configurationLoaded)
    {
        QMessageBox::warning(
            this,
            "Configuration",
            "No valid configuration is available to restore.");

        return;
    }

    const QMessageBox::StandardButton result = QMessageBox::question(
            this,
            "Restore Configuration",
            "Do you want to discard all unsaved changes and restore "
            "the last saved configuration?",
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);

    if (result == QMessageBox::Yes)
    {
        loadConfigurationValues(configurationObject);
        setUIControls();
    }
}


void MainWindow::on_selectFolderBtn_clicked()
{
    QString initialDirectory = ui->outputDirectoryEdit->text().trimmed();

    if (initialDirectory.isEmpty() || !QDir(initialDirectory).exists())
    {
        initialDirectory = QDir::homePath();
    }

    const QString selectedDirectory = QFileDialog::getExistingDirectory(
            this,
            "Select Output Directory",
            initialDirectory,
            QFileDialog::ShowDirsOnly);

    // An empty result means the user cancelled the dialog.
    if (selectedDirectory.isEmpty())
    {
        return;
    }

    ui->outputDirectoryEdit->setText(QDir::toNativeSeparators(selectedDirectory));
}
