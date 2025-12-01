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
    templateName = settings.value("template", "DEFAULT.ZPL").toString();

    QString savedBg = settings.value("background", "").toString();
    QString defaultResource = ":/resources/oil_label_bg.png";

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
    //preview = new LabelPreview(backgroundPath, this);
    preview = new LabelPreview(this);
    // mainLayout->addWidget(preview);
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
    if (oilType.isEmpty()) oilType = "N/A";

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

    QProcess lp;
    QStringList args;
    args << "-P" << printerName << "-o" << "raw";
    lp.start("lpr", args);
    lp.write(zpl.toUtf8());
    lp.closeWriteChannel();
    lp.waitForFinished();

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
void OilLabelGUI::clearInputs()
{
    mileageInput->clear();
    oilTypeInput->clear();
    intervalInput->setText(QString::number(defaultMiles));
    preview->updatePreview(QString(), QString(), QString(), QString());
}

// -----------------------------
// Select Printer
// -----------------------------
void OilLabelGUI::selectPrinter()
{
    QProcess process;
    process.start("lpstat", QStringList() << "-a");
    process.waitForFinished();
    QStringList printers;
    QString output = process.readAllStandardOutput();
    for (const QString &line : output.split('\n', Qt::SkipEmptyParts)) {
        printers << line.split(' ').first();
    }

    if (printers.isEmpty()) printers << "Autoshop_Label_Printer";

    int currentIndex = printers.indexOf(printerName);
    if (currentIndex == -1) currentIndex = 0;

    bool ok;
    QString printer = QInputDialog::getItem(
        this,
        "Select Printer",
        "Printers:",
        printers,
        currentIndex,
        false,
        &ok
    );

    if (ok && !printer.isEmpty()) {
        printerName = printer;
        QSettings settings("MyCompany", "OilStickerApp");
        settings.setValue("printer", printerName);
    }
}

// -----------------------------
// Change Background
// -----------------------------
void OilLabelGUI::changeBackground()
{
    QSettings settings("MyCompany", "OilStickerApp");

    QString lastFolder = settings.value(
        "backgroundFolder",
        QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)
    ).toString();

    QFileDialog dialog(this, "Select Background PNG", lastFolder);
    dialog.setNameFilter("PNG Images (*.png)");
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setOption(QFileDialog::DontUseNativeDialog, true);

    QString fileName;
    if (dialog.exec() == QDialog::Accepted) {
        fileName = dialog.selectedFiles().first();
    }

    if (!fileName.isEmpty()) {
        backgroundPath = fileName;
        settings.setValue("background", backgroundPath);

        QFileInfo fi(fileName);
        settings.setValue("backgroundFolder", fi.absolutePath());
    } else {
        QString savedBg = settings.value("background", "").toString();
        if (!savedBg.isEmpty() && !QFile::exists(savedBg)) {
            backgroundPath = ":/resources/oil_label_bg.png";
            settings.setValue("background", backgroundPath);
        }
    }

    preview->setBackground(backgroundPath);
}

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
// Reset Qsettings
// -----------------------------
void OilLabelGUI::resetSettings()
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Reset Settings",
        "Are you sure you want to clear all saved settings?",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        QSettings settings("MyCompany", "OilStickerApp");
        settings.clear();
        settings.sync();

        QMessageBox::information(this, "Settings Reset",
                                 "All settings have been cleared.\n"
                                 "Please restart the application.");

        // Optional: immediately reload defaults in the running app:
        //  - reset printerName, backgroundPath, templateName, etc.
        //  - OR simply exit and let user restart

        // Example forced exit:
        qApp->quit();
    }
}

// -----------------------------
// Version Display
// -----------------------------
void OilLabelGUI::showAboutDialog()
{
    QString versionString = QString("Oil Sticker App\n"
                                    "Version %1.%2.%3 (Build %4)")
                                .arg(PROJECT_VERSION_MAJOR)
                                .arg(PROJECT_VERSION_MINOR)
                                .arg(PROJECT_VERSION_PATCH)
                                .arg(PROJECT_VERSION_BUILD);

    QMessageBox::about(this, "About Oil Sticker App", versionString);
}

