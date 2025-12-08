#include "OilLabelGUI.hpp"
#include "LabelPreview.hpp"
#include "version.hpp"

#include <QApplication>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QInputDialog>
#include <QFileDialog>
#include <QSettings>
#include <QDate>
#include <QProcess>
#include <QStringList>
#include <QFont>
#include <QFile>
#include <QTimer>
#include <QStandardPaths>
#include <QFileInfo>
#include <QtNetwork/QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QByteArray>

OilLabelGUI::OilLabelGUI(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("Oil Change Label Generator");
    setFixedSize(500, 600);

    // -----------------------------
    // Load settings
    // -----------------------------
    QSettings settings("MyCompany", "OilStickerApp");

    printerName = settings.value("printer", "").toString();
    defaultMiles = settings.value("defaultMiles", 5000).toInt();
    templateName = settings.value("template", "MAX.ZPL").toString();

    QString savedBg = settings.value("background", "").toString();
    QString defaultResource = ":/resources/max.png";

    if (savedBg.isEmpty()) {
        backgroundPath = defaultResource;
        settings.setValue("background", backgroundPath);
    } else if (savedBg.startsWith(":/")) {
        backgroundPath = savedBg;
    } else if (QFile::exists(savedBg)) {
        backgroundPath = savedBg;
    } else {
        backgroundPath = defaultResource;
        settings.setValue("background", backgroundPath);
    }

    // -----------------------------
    // Menu Bar
    // -----------------------------
    QMenuBar *menuBar = new QMenuBar(this);

    // Settings menu
    QMenu *settingsMenu = menuBar->addMenu("Settings");
    QAction *changePrinter = new QAction("Select Printer", this);
    connect(changePrinter, &QAction::triggered, this, &OilLabelGUI::selectPrinter);
    settingsMenu->addAction(changePrinter);

    QAction *changeBackgroundAct = new QAction("Select Background", this);
    connect(changeBackgroundAct, &QAction::triggered, this, &OilLabelGUI::changeBackground);
    settingsMenu->addAction(changeBackgroundAct);

    QAction *changeTemplateAct = new QAction("Select Template", this);
    connect(changeTemplateAct, &QAction::triggered, this, &OilLabelGUI::selectTemplate);
    settingsMenu->addAction(changeTemplateAct);

    QAction *resetSettingsAct = new QAction("Reset All Settings", this);
    connect(resetSettingsAct, &QAction::triggered, this, &OilLabelGUI::resetSettings);
    settingsMenu->addAction(resetSettingsAct);

    // Help menu
    QMenu *helpMenu = menuBar->addMenu("Help");
    QAction *aboutAction = new QAction("About", this);
    connect(aboutAction, &QAction::triggered, this, &OilLabelGUI::showAboutDialog);
    helpMenu->addAction(aboutAction);

    // -----------------------------
    // Layouts
    // -----------------------------
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setMenuBar(menuBar);

    // -----------------------------
    // Inputs
    // -----------------------------
    // Current Mileage
    mileageLabel = new QLabel("Current Mileage:");
    mileageInput = new QLineEdit();
    mileageInput->setPlaceholderText("e.g. 123456");
    mileageInput->setFixedWidth(80);

    // Next Service
    intervalLabel = new QLabel("Next Service:");
    intervalInput = new QLineEdit();
    intervalInput->setFixedWidth(50);
    intervalInput->setText(QString::number(defaultMiles));

    // Create a horizontal layout for both fields
    QHBoxLayout* mileageRow = new QHBoxLayout();
    mileageRow->addWidget(mileageLabel);
    mileageRow->addWidget(mileageInput);
    mileageRow->addSpacing(20);   // Optional spacing between the two groups
    mileageRow->addWidget(intervalLabel);
    mileageRow->addWidget(intervalInput);
    mileageRow->addStretch();     // optional to push widgets left
    mainLayout->addLayout(mileageRow);

    // Oil type
    QLabel *oilTypeLabel = new QLabel("Oil Brand and Grade:");
    oilTypeInput = new QLineEdit();
    oilTypeInput->setPlaceholderText("e.g. MOBIL1 0W40");
    oilTypeInput->setFixedWidth(270);

    // Oil type row
    QHBoxLayout *oilTypeRow = new QHBoxLayout();
    oilTypeRow->addWidget(oilTypeLabel);
    oilTypeRow->addWidget(oilTypeInput);
    oilTypeRow->addStretch();
    mainLayout->addLayout(oilTypeRow);

    // -----------------------------
    // Preview
    // -----------------------------
    preview = new LabelPreview(this);
    preview->setBackground(backgroundPath);
    mainLayout->addWidget(preview, 0, Qt::AlignCenter);

    // -----------------------------
    // Buttons
    // -----------------------------
    printBtn = new QPushButton("Print Label");
    printBtn->setFixedWidth(150);
    connect(printBtn, &QPushButton::clicked, this, &OilLabelGUI::printLabel);

    clearBtn = new QPushButton("Clear");
    clearBtn->setFixedWidth(100);
    connect(clearBtn, &QPushButton::clicked, this, &OilLabelGUI::clearInputs);

    QHBoxLayout *buttonRow = new QHBoxLayout();
    buttonRow->addWidget(printBtn);
    buttonRow->addWidget(clearBtn);
    buttonRow->addStretch();
    mainLayout->addLayout(buttonRow);

    setLayout(mainLayout);

    // -----------------------------
    // Persist default miles when editing
    // -----------------------------
    connect(intervalInput, &QLineEdit::editingFinished, this, [this]() {
        bool ok;
        int val = intervalInput->text().toInt(&ok);
        if (ok && val > 0) {
            defaultMiles = val;
            QSettings settings("MyCompany", "OilStickerApp");
            settings.setValue("defaultMiles", defaultMiles);
        }
    });

    // -----------------------------
    // Live Update signals
    // -----------------------------
    connect(mileageInput, &QLineEdit::textChanged, this, &OilLabelGUI::liveUpdate);
    connect(intervalInput, &QLineEdit::textChanged, this, &OilLabelGUI::liveUpdate);

    connect(oilTypeInput, &QLineEdit::textChanged, this, [=](const QString &text) {
        QString upper = text.toUpper();
        if (text != upper) {
            oilTypeInput->blockSignals(true);
            oilTypeInput->setText(upper);
            oilTypeInput->blockSignals(false);
        }
        liveUpdate();
    });
}

// -----------------------------
// Live Update
// -----------------------------
void OilLabelGUI::liveUpdate()
{
    bool okMileage, okInterval;
    int mileage = mileageInput->text().toInt(&okMileage);
    int interval = intervalInput->text().toInt(&okInterval);

    if (!okInterval) interval = defaultMiles;

    if (okMileage) {
        int nextMileage = mileage + interval;
        QDate nextDate = QDate::currentDate().addMonths(6);

        QString oilTypeStr = oilTypeInput->text().trimmed();
        QString todayStr = QDate::currentDate().toString("MM/dd/yy");

        preview->updatePreview(
            QString("%L1").arg(nextMileage),
            nextDate.toString("MM/dd/yy"),
            oilTypeStr,
            todayStr
        );
    } else {
        preview->updatePreview(QString(), QString(), QString(), QString());
    }
}

// -----------------------------
// Print Label
// -----------------------------
void OilLabelGUI::printLabel()
{
    bool okMileage, okInterval;
    int mileage = mileageInput->text().toInt(&okMileage);
    int interval = intervalInput->text().toInt(&okInterval);

    if (!okInterval) interval = defaultMiles;

    if (!okMileage || !okInterval) {
        QMessageBox::warning(this, "Invalid Input", "Inputs must be numbers.");
        clearInputs();
        return;
    }

    int nextMileage = mileage + interval;
    QString formattedMileage = QLocale(QLocale::English).toString(nextMileage);

    QString nextDate = QDate::currentDate().addMonths(6).toString("MM/dd/yy");

    QString oilType = oilTypeInput->text().trimmed();
    if (oilType.isEmpty()) oilType = "";

    QString today = QDate::currentDate().toString("MM/dd/yy");

    QString zpl = QString(
        "^XA\n"
        "^XF%1^FS\n"
        "^FN2^FD%2^FS\n" // oil type
        "^FN3^FD%3^FS\n" // today's date
        "^FN4^FD%4^FS\n" // next mileage
        "^FN5^FD%5^FS\n" // next date
        "^XZ"
    ).arg(templateName)
     .arg(oilType)
     .arg(today)
     .arg(formattedMileage)
     .arg(nextDate);

    if (printerName.isEmpty()) {
        QMessageBox::warning(this, "No Printer Selected", "Please select a printer in Settings.");
        return;
    }

    // Create a QNetworkAccessManager instance
    QNetworkAccessManager *networkManager = new QNetworkAccessManager(this);

    // Construct the printer URL with port and IPP path
    QUrl printerUrl(QString("http://%1:9100/ipp/print").arg(printerName)); // Assuming printerName is the IP or hostname
    QNetworkRequest request(printerUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/ipp"); // Set content type for IPP

    // Send the ZPL data as the HTTP POST body
    QNetworkReply *reply = networkManager->post(request, zpl.toUtf8());

    // Handle the reply (optional)
    connect(reply, &QNetworkReply::finished, this, [reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QMessageBox::information(nullptr, "Success", "Label sent to printer successfully.");
        } else {
            QMessageBox::warning(nullptr, "Error", QString("Failed to send label: %1").arg(reply->errorString()));
        }
        reply->deleteLater();
    });

    // Auto-closing message box with printer name
    QMessageBox *msgBox = new QMessageBox(this);
    msgBox->setWindowTitle("Printed");
    msgBox->setText(QString("Label sent to printer: %1").arg(printerName));
    msgBox->setIcon(QMessageBox::Information);
    msgBox->setStandardButtons(QMessageBox::NoButton);
    msgBox->show();

    QTimer::singleShot(3000, msgBox, &QMessageBox::accept);

    clearInputs();
}

// -----------------------------
// Clear Inputs
// -----------------------------
void OilLabelGUI::clearInputs() {
    mileageInput->clear();
    intervalInput->setText(QString::number(defaultMiles));
    oilTypeInput->clear();
    preview->updatePreview(QString(), QString(), QString(), QString());
}

// -----------------------------
// Change Background
// -----------------------------
void OilLabelGUI::changeBackground() {
    QString filePath = QFileDialog::getOpenFileName(this, "Select Background Image", "", "Images (*.png *.jpg *.bmp)");
    if (!filePath.isEmpty()) {
        backgroundPath = filePath;
        QSettings settings("MyCompany", "OilStickerApp");
        settings.setValue("background", backgroundPath);
        preview->setBackground(backgroundPath);
    }
}

// -----------------------------
// Select Template
// -----------------------------
// -----------------------------
// Select Template
// -----------------------------
void OilLabelGUI::selectTemplate()
{
    bool ok;
    QString input = QInputDialog::getText(
        this,
        "Select ZPL Template",
        "Template Name (include extension, e.g., TEMPLATE.ZPL):",
        QLineEdit::Normal,
        templateName,
        &ok
    );

    if (ok && !input.isEmpty()) {
        templateName = input.toUpper();   // Capitalize before storing
        QSettings settings("MyCompany", "OilStickerApp");
        settings.setValue("template", templateName);
    }
}

// -----------------------------
// Reset Settings
// -----------------------------
void OilLabelGUI::resetSettings() {
    QSettings settings("MyCompany", "OilStickerApp");
    settings.clear();
    QMessageBox::information(this, "Settings Reset", "All settings have been reset to default.");
    QApplication::quit(); // Restart the application to apply changes
}

// -----------------------------
// Show About Dialog
// -----------------------------
void OilLabelGUI::showAboutDialog() {
    QMessageBox::about(this, "About Oil Sticker App",
        "Oil Sticker App\n"
        "Version 1.0.0\n\n"
        "Developed by MyCompany.\n"
        "This application generates oil change labels.");
}

// -----------------------------
// Select Printer
// -----------------------------
void OilLabelGUI::selectPrinter()
{
    QProcess process;
    process.start("lpstat", QStringList() << "-a");
    process.waitForFinished(1500);

    QString output = process.readAllStandardOutput().trimmed();
    QString error  = process.readAllStandardError().trimmed();

    QStringList printers;

    bool lpstatFailed =
        output.isEmpty() ||
        output.contains("not recognized", Qt::CaseInsensitive) ||
        error.contains("not recognized", Qt::CaseInsensitive) ||
        error.contains("lpstat", Qt::CaseInsensitive);

    if (!lpstatFailed) {
        // -----------------------------------------
        // Mac/Linux → Parse printer list normally
        // -----------------------------------------
        for (const QString &line : output.split('\n', Qt::SkipEmptyParts)) {
            printers << line.split(' ').first();
        }
    }

    // If lpstat found nothing, behave like Windows
    if (printers.isEmpty()) {
        // -----------------------------------------
        // Windows → Ask for printer IP address
        // -----------------------------------------
        QSettings settings("MyCompany", "OilStickerApp");
        QString storedIP = settings.value("printer", "").toString(); // Retrieve stored IP

        bool ok = false;
        QString ip = QInputDialog::getText(
            this,
            "Enter Printer IP",
            "No printers were auto-detected.\n"
            "Please enter the printer’s IP address:",
            QLineEdit::Normal,
            storedIP, // Show stored IP as default value
            &ok
        );

        if (ok && !ip.trimmed().isEmpty()) {
            printerName = ip.trimmed();
            settings.setValue("printer", printerName); // Save the new IP
        }

        return;
    }
}