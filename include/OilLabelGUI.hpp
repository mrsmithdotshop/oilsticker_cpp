#pragma once

#include <QWidget>

class QLabel;
class QLineEdit;
class QPushButton;
class LabelPreview;

class OilLabelGUI : public QWidget
{
    Q_OBJECT

public:
    explicit OilLabelGUI(QWidget *parent = nullptr);

private slots:
    void liveUpdate();
    void printLabel();
    void clearInputs();
    void selectPrinter();
    void changeBackground();
    void selectTemplate();

private:
    QLabel *mileageLabel;
    QLineEdit *mileageInput;

    QLabel *intervalLabel;
    QLineEdit *intervalInput;

    QLineEdit *oilTypeInput;   // Input field for oil type

    QPushButton *printBtn;
    QPushButton *clearBtn;

    LabelPreview *preview;

    // ---------- Add these ----------
    QString printerName;     // stores selected printer
    QString backgroundPath;  // stores selected background PNG
    int defaultMiles = 5000; // stores user’s preferred interval (PERSISTENT)
    QString templateName;  // stores selected ZPL template name
};





