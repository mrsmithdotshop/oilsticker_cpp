// src/OilLabelGUI.cpp
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
#include <QComboBox>
#include <QLocale>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QUrl>
#include <QByteArray>

const QSize defaultSize(500, 600);   // window size for DEFAULT style
const QSize keytagSize(500, 800);    // window size for KEYTAG style

OilLabelGUI::OilLabelGUI(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("Oil Change Label Generator");

    // -----------------------------
    // Load settings
    // -----------------------------
    QSettings settings("MyCompany", "OilStickerApp");

    printerName = settings.value("printer", "").toString();
    defaultMiles = settings.value("defaultMiles", 5000).toInt();
    labelStyle = settings.value("labelStyle", "DEFAULT").toString().toUpper();
    templateName = settings.value("template", "DEFAULT.ZPL").toString();

    // default backgrounds for styles
    QString defaultResource_default = ":/resources/default.png";
    QString defaultResource_keytag  = ":/resources/keytag.png";

    // Choose background path based on saved style
    if (labelStyle == "KEYTAG") {
        backgroundPath = settings.value("background", defaultResource_keytag).toString();
        templateName = settings.value("template", "KEYTAG.ZPL").toString();
    } else {
        backgroundPath = settings.value("background", defaultResource_default).toString();
        templateName = settings.value("template", "DEFAULT.ZPL").toString();
        labelStyle = "DEFAULT";
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
    // Style selector (combo) - above preview
    // -----------------------------
    QHBoxLayout *styleRow = new QHBoxLayout();
    QLabel *styleLabel = new QLabel("Label Style:");
    styleCombo = new QComboBox();
    styleCombo->addItem("Default", QVariant("DEFAULT"));
    styleCombo->addItem("Key Tag", QVariant("KEYTAG"));
    int idx = (labelStyle == "KEYTAG") ? 1 : 0;
    styleCombo->setCurrentIndex(idx);
    connect(styleCombo, &QComboBox::currentTextChanged, this, &OilLabelGUI::onStyleChanged);
    styleRow->addWidget(styleLabel);
    styleRow->addWidget(styleCombo);
    styleRow->addStretch();
    mainLayout->addLayout(styleRow);

    // -----------------------------
    // Preview
    // -----------------------------
    preview = new LabelPreview(this);
    preview->setBackground(backgroundPath);
    preview->setLabelStyle(labelStyle);
    //mainLayout->addWidget(preview, 0, Qt::AlignCenter);
    mainLayout->addWidget(preview, 0, Qt::AlignTop);

    // -----------------------------
    // Inputs below the preview
    // -----------------------------
    // DEFAULT section fields (template, mileage, nextService, oilType)
    templateLabel = new QLabel("Template:");
    templateInput = new QLineEdit(templateName);
    templateInput->setFixedWidth(200);

    mileageLabel = new QLabel("Current Mileage:");
    mileageInput = new QLineEdit();
    mileageInput->setFixedWidth(120);
    mileageInput->setPlaceholderText("e.g. 123456");

    intervalLabel = new QLabel("Next Service:");
    intervalInput = new QLineEdit();
    intervalInput->setFixedWidth(80);
    intervalInput->setText(QString::number(defaultMiles));

    oilTypeLabel = new QLabel("Oil Brand/Grade:");
    oilTypeInput = new QLineEdit();
    oilTypeInput->setFixedWidth(200);
    oilTypeInput->setPlaceholderText("e.g. MOBIL1 0W40");

    // KEYTAG section fields
    kt_templateLabel = new QLabel("Template:");
    kt_templateInput = new QLineEdit("KEYTAG.ZPL");
    customerLabel = new QLabel("Customer:");
    customerInput = new QLineEdit();
    carLabel = new QLabel("Car:");
    carInput = new QLineEdit();
    plateLabel = new QLabel("Plate:");
    plateInput = new QLineEdit();
    vinLabel = new QLabel("VIN:");
    vinInput = new QLineEdit();
    colorLabel = new QLabel("Color:");
    colorInput = new QLineEdit();
    repairOrderLabel = new QLabel("Repair Order:");
    repairOrderInput = new QLineEdit();

    quantityLabel = new QLabel("Quantity:");
    quantityInput = new QLineEdit();
    quantityInput->setFixedWidth(80);
    quantityInput->setPlaceholderText("1");
    quantityInput->setText("1");
    quantityInput->setValidator(new QIntValidator(1, 99, this));

    // Arrange default fields
    QHBoxLayout *tmplRow = new QHBoxLayout();
    tmplRow->addWidget(templateLabel);
    tmplRow->addWidget(templateInput);
    tmplRow->addStretch();
    mainLayout->addLayout(tmplRow);

    QHBoxLayout *mRow = new QHBoxLayout();
    mRow->addWidget(mileageLabel);
    mRow->addWidget(mileageInput);
    mRow->addSpacing(20);
    mRow->addWidget(intervalLabel);
    mRow->addWidget(intervalInput);
    mRow->addStretch();
    mainLayout->addLayout(mRow);

    QHBoxLayout *oilRow = new QHBoxLayout();
    oilRow->addWidget(oilTypeLabel);
    oilRow->addWidget(oilTypeInput);
    oilRow->addStretch();
    mainLayout->addLayout(oilRow);

    // Arrange keytag fields (stacked)
    QVBoxLayout *ktBox = new QVBoxLayout();
    QHBoxLayout *ktTmplRow = new QHBoxLayout();
    ktTmplRow->addWidget(kt_templateLabel);
    ktTmplRow->addWidget(kt_templateInput);
    ktTmplRow->addStretch();
    ktBox->addLayout(ktTmplRow);

    auto addRowTo = [&](QVBoxLayout *parent, QLabel *lab, QLineEdit *edit){
        QHBoxLayout *r = new QHBoxLayout();
        r->addWidget(lab);
        r->addWidget(edit);
        r->addStretch();
        parent->addLayout(r);
    };
    addRowTo(ktBox, customerLabel, customerInput);
    addRowTo(ktBox, carLabel, carInput);
    addRowTo(ktBox, plateLabel, plateInput);
    addRowTo(ktBox, vinLabel, vinInput);
    addRowTo(ktBox, colorLabel, colorInput);
    addRowTo(ktBox, repairOrderLabel, repairOrderInput);
    addRowTo(ktBox, quantityLabel, quantityInput);

    mainLayout->addLayout(ktBox);

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
    // Visibility initial state per style
    // -----------------------------
    bool isKeyTag = (labelStyle == "KEYTAG");
    templateLabel->setVisible(0);
    templateInput->setVisible(0);
    mileageLabel->setVisible(!isKeyTag);
    mileageInput->setVisible(!isKeyTag);
    intervalLabel->setVisible(!isKeyTag);
    intervalInput->setVisible(!isKeyTag);
    oilTypeLabel->setVisible(!isKeyTag);
    oilTypeInput->setVisible(!isKeyTag);

    kt_templateLabel->setVisible(0);
    kt_templateInput->setVisible(0);
    customerLabel->setVisible(isKeyTag);
    customerInput->setVisible(isKeyTag);
    carLabel->setVisible(isKeyTag);
    carInput->setVisible(isKeyTag);
    plateLabel->setVisible(isKeyTag);
    plateInput->setVisible(isKeyTag);
    vinLabel->setVisible(isKeyTag);
    vinInput->setVisible(isKeyTag);
    colorLabel->setVisible(isKeyTag);
    colorInput->setVisible(isKeyTag);
    repairOrderLabel->setVisible(isKeyTag);
    repairOrderInput->setVisible(isKeyTag);
    quantityLabel->setVisible(isKeyTag);
    quantityInput->setVisible(isKeyTag);

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
    connect(templateInput, &QLineEdit::editingFinished, this, [this]() {
        templateName = templateInput->text().toUpper();
        QSettings settings("MyCompany", "OilStickerApp");
        settings.setValue("template", templateName);
    });

    connect(kt_templateInput, &QLineEdit::editingFinished, this, [this]() {
        // if editing keytag template store uppercase
        templateName = kt_templateInput->text().toUpper();
        QSettings settings("MyCompany", "OilStickerApp");
        settings.setValue("template", templateName);
    });

    connect(oilTypeInput, &QLineEdit::textChanged, this, [=](const QString &text) {
        QString upper = text.toUpper();
        if (text != upper) {
            oilTypeInput->blockSignals(true);
            oilTypeInput->setText(upper);
            oilTypeInput->blockSignals(false);
        }
        liveUpdate();
    });

    connect(quantityInput, &QLineEdit::textChanged, this, [this](const QString &text) {
        bool ok;
        int q = text.toInt(&ok);
        if (!ok || q < 1) q = 1;
            preview->setQuantity(q);
    });

auto ktConnect = [&](QLineEdit *le){
    connect(le, &QLineEdit::textChanged, this, [=](const QString &text) {
        QString upper = text.toUpper();
        if (text != upper) {
            le->blockSignals(true);
            le->setText(upper);
            le->blockSignals(false);
        }
        liveUpdate();
    });
};

    ktConnect(customerInput);
    ktConnect(carInput);
    ktConnect(plateInput);
    ktConnect(vinInput);
    ktConnect(colorInput);
    ktConnect(repairOrderInput);

    // initial preview blank
    preview->updatePreview(QString(), QString(), QString(), QString());
}

//
// Live Update
//
void OilLabelGUI::liveUpdate()
{
    bool okMileage, okInterval;
    int mileage = mileageInput->text().toInt(&okMileage);
    int interval = intervalInput->text().toInt(&okInterval);

    if (!okInterval) interval = defaultMiles;

    if (labelStyle == "DEFAULT") {
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
    } else { // KEYTAG preview uses keytag fields
        bool okQty;
        int qty = quantityInput->text().toInt(&okQty);
        if (!okQty || qty < 1) qty = 1;
        preview->updatePreview(QString(), QString(),
                               QString(), QString(),
                               customerInput->text().trimmed(),
                               carInput->text().trimmed(),
                               plateInput->text().trimmed(),
                               vinInput->text().trimmed(),
                               colorInput->text().trimmed(),
                               repairOrderInput->text().trimmed());
        preview->setQuantity(quantityInput->text().toInt());
    }
}

//
// Print Label
//
void OilLabelGUI::printLabel()
{
    if (labelStyle == "DEFAULT") {
        bool okMileage, okInterval;
        int mileage = mileageInput->text().toInt(&okMileage);
        int interval = intervalInput->text().toInt(&okInterval);

        if (!okInterval) interval = defaultMiles;
        if (!okMileage || !okInterval) {
            QMessageBox::warning(this, "Invalid Input", "Inputs must be numbers.");
            return;
        }

        int nextMileage = mileage + interval;
        QString formattedMileage = QLocale(QLocale::English).toString(nextMileage);
        QString nextDate = QDate::currentDate().addMonths(6).toString("MM/dd/yy");
        QString oilType = oilTypeInput->text().trimmed();
        QString today = QDate::currentDate().toString("MM/dd/yy");

       bool okQty;
        int qty = quantityInput->text().toInt(&okQty);
        if (!okQty || qty < 1) qty = 1;

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
        // Print via sendZplToPrinter function
        sendZplToPrinter(zpl);

    } else { // KEYTAG print
        int qty = 1;
        bool okQty = false;

        qty = quantityInput->text().toInt(&okQty);
        if (!okQty || qty < 1) qty = 1;

        // divide by 2, round UP
        qty = (qty + 1) / 2;

        if (qty > 1) {
            templateName = "LABEL.ZPL";
        }

        QString zpl = QString(
        "^XA\n"
        "^PQ%8\n"
        "^XF%1^FS\n"
        "^FN2^FD%2^FS\n"
        "^FN3^FD%3^FS\n"
        "^FN4^FD%4^FS\n"
        "^FN5^FD%5^FS\n"
        "^FN6^FD%6^FS\n"
        "^FN7^FD%7^FS\n"
        "^XZ"
        )
        .arg(templateName)
        .arg(customerInput->text().trimmed())
        .arg(carInput->text().trimmed())
        .arg(plateInput->text().trimmed())
        .arg(vinInput->text().trimmed())
        .arg(colorInput->text().trimmed())
        .arg(repairOrderInput->text().trimmed())
        .arg(qty);


        if (printerName.isEmpty()) {
            QMessageBox::warning(this, "No Printer Selected", "Please select a printer in Settings.");
            return;
        }
    // Print via sendZplToPrinter function
    sendZplToPrinter(zpl);

    }

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

//
// Clear Inputs
//
void OilLabelGUI::clearInputs()
{
    // Clear fields except keep nextService (intervalInput) as requested
    mileageInput->clear();
    oilTypeInput->clear();
    customerInput->clear();
    carInput->clear();
    plateInput->clear();
    vinInput->clear();
    colorInput->clear();
    repairOrderInput->clear();
    quantityInput->setText("1");

    // Reset template inputs to stored templateName
    templateInput->setText(templateName);
    kt_templateInput->setText(templateName);

    preview->updatePreview(QString(), QString(), QString(), QString());
}

//
// Select Printer
//
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

    useIppPrinting = lpstatFailed;

    if (!lpstatFailed) {
        for (const QString &line : output.split('\n', Qt::SkipEmptyParts)) {
            printers << line.split(' ').first();
        }
    }

    // If none found prompt for IP (Windows-style)
    if (printers.isEmpty()) {
        QSettings settings("MyCompany", "OilStickerApp");
        QString storedIP = settings.value("printer", "").toString();

        bool ok = false;
        QString ip = QInputDialog::getText(
            this,
            "Enter Printer IP",
            "No printers detected. Enter printer IP or hostname:",
            QLineEdit::Normal,
            storedIP,
            &ok
        );

        if (ok && !ip.trimmed().isEmpty()) {
            printerName = ip.trimmed();
            settings.setValue("printer", printerName);
        }
        return;
    }

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

//
// Change Background
//
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
    if (dialog.exec() == QDialog::Accepted) fileName = dialog.selectedFiles().first();

    if (!fileName.isEmpty()) {
        backgroundPath = fileName;
        settings.setValue("background", backgroundPath);
        QFileInfo fi(fileName);
        settings.setValue("backgroundFolder", fi.absolutePath());
    } else {
        QString savedBg = settings.value("background", "").toString();
        if (!savedBg.isEmpty() && !QFile::exists(savedBg)) {
            if (labelStyle == "KEYTAG")
                backgroundPath = ":/resources/keytag.png";
            else
                backgroundPath = ":/resources/default.png";
            settings.setValue("background", backgroundPath);
        }
    }

    preview->setBackground(backgroundPath);
}

//
// Select Template
//
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
        templateName = input.toUpper();
        QSettings settings("MyCompany", "OilStickerApp");
        settings.setValue("template", templateName);
        // update displayed template fields
        templateInput->setText(templateName);
        kt_templateInput->setText(templateName);
    }
}

//
// Reset Qsettings
//
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
                                 "All settings have been cleared.\nPlease restart the application.");
        qApp->quit();
    }
}

void OilLabelGUI::showAboutDialog()
{
    // The font you expect to be using
    QFont requestedFont("Swis721 BT");
    requestedFont.setPointSize(12);

    QFontInfo info(requestedFont);

    QString versionString = QString(
        "Oil Sticker App\n"
        "Version %1.%2.%3 (Build %4)\n\n"
        "Font Info:\n"
        "Requested: %5\n"
        "Resolved:  %6\n"
        "Point Size: %7\n"
        "Exact Match: %8\n"
        "Platform: %9"
    )
    .arg(PROJECT_VERSION_MAJOR)
    .arg(PROJECT_VERSION_MINOR)
    .arg(PROJECT_VERSION_PATCH)
    .arg(PROJECT_VERSION_BUILD)
    .arg(requestedFont.family())
    .arg(info.family())
    .arg(info.pointSize())
    .arg(info.exactMatch() ? "Yes" : "NO (fallback)")
    .arg(QSysInfo::prettyProductName());

    QMessageBox::about(this, "About Oil Sticker App", versionString);
}


//
// Style changed handler
//
void OilLabelGUI::onStyleChanged(const QString &style)
{
    QString s = style.toUpper();
    if (s == "KEY TAG") s = "KEYTAG";

    if (s == "KEYTAG") {
        labelStyle = "KEYTAG";
        templateName = "KEYTAG.ZPL";
        backgroundPath = ":/resources/keytag.png";
    } else {
        labelStyle = "DEFAULT";
        templateName = "DEFAULT.ZPL";
        backgroundPath = ":/resources/default.png";
    }

    // persist
    QSettings settings("MyCompany", "OilStickerApp");
    settings.setValue("labelStyle", labelStyle);
    settings.setValue("template", templateName);
    settings.setValue("background", backgroundPath);

    // update UI visibility
    bool isKeyTag = (labelStyle == "KEYTAG");
    templateLabel->setVisible(0);
    templateInput->setVisible(0);
    mileageLabel->setVisible(!isKeyTag);
    mileageInput->setVisible(!isKeyTag);
    intervalLabel->setVisible(!isKeyTag);
    intervalInput->setVisible(!isKeyTag);
    oilTypeLabel->setVisible(!isKeyTag);
    oilTypeInput->setVisible(!isKeyTag);

    kt_templateLabel->setVisible(0);
    kt_templateInput->setVisible(0);
    customerLabel->setVisible(isKeyTag);
    customerInput->setVisible(isKeyTag);
    carLabel->setVisible(isKeyTag);
    carInput->setVisible(isKeyTag);
    plateLabel->setVisible(isKeyTag);
    plateInput->setVisible(isKeyTag);
    vinLabel->setVisible(isKeyTag);
    vinInput->setVisible(isKeyTag);
    colorLabel->setVisible(isKeyTag);
    colorInput->setVisible(isKeyTag);
    repairOrderLabel->setVisible(isKeyTag);
    repairOrderInput->setVisible(isKeyTag);
    quantityLabel->setVisible(isKeyTag);
    quantityInput->setVisible(isKeyTag);

    // update preview style / background
    preview->setLabelStyle(labelStyle);
    preview->setBackground(backgroundPath);

    // update template inputs
    templateInput->setText(templateName);
    kt_templateInput->setText(templateName);

    // Adjust window size
    if (labelStyle == "KEYTAG")
    resize(keytagSize);
        else
    resize(defaultSize);
    adjustSize();
}
//
// Print ZPL
//
void OilLabelGUI::sendZplToPrinter(const QString &zpl)
{
    if (printerName.isEmpty()) {
        QMessageBox::warning(this, "No Printer Selected",
                             "Please select a printer in Settings.");
        return;
    }

    if (!useIppPrinting) {
        // -----------------------------
        // CUPS / lpr path (macOS, Linux)
        // -----------------------------
        QProcess lp;
        QStringList args;
        args << "-P" << printerName << "-o" << "raw";

        lp.start("lpr", args);
        lp.write(zpl.toUtf8());
        lp.closeWriteChannel();

        if (!lp.waitForFinished(3000)) {
            QMessageBox::warning(this, "Print Error",
                                 "lpr did not finish sending the job.");
        }

    } else {
        // -----------------------------
        // IPP / HTTP path (Windows)
        // -----------------------------
        QNetworkAccessManager *networkManager =
            new QNetworkAccessManager(this);

        // NOTE:
        // Port 9100 is *RAW socket*, not IPP.
        // Zebra IPP is usually :631/ipp/print
        QUrl printerUrl(
            QString("http://%1:631/ipp/print").arg(printerName)
        );

        QNetworkRequest request(printerUrl);
        request.setHeader(
            QNetworkRequest::ContentTypeHeader,
            "application/ipp"
        );

        QNetworkReply *reply =
            networkManager->post(request, zpl.toUtf8());

        connect(reply, &QNetworkReply::finished, this,
            [this, reply]() {
                if (reply->error() == QNetworkReply::NoError) {
                    QMessageBox::information(
                        this, "Printed",
                        "Label sent to printer successfully."
                    );
                } else {
                    QMessageBox::warning(
                        this, "Print Error",
                        QString("Failed to send label:\n%1")
                            .arg(reply->errorString())
                    );
                }
                reply->deleteLater();
            });
    }
}
