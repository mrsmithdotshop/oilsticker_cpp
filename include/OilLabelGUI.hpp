#pragma once

#include <QWidget>

class QLabel;
class QLineEdit;
class QPushButton;
class LabelPreview;
class QComboBox;

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
    void resetSettings();
    void showAboutDialog();
    void onStyleChanged(const QString &style);

private:
    // Common
    QLabel *mileageLabel;
    QLineEdit *mileageInput;

    QLabel *intervalLabel;
    QLineEdit *intervalInput;

    QPushButton *printBtn;
    QPushButton *clearBtn;

    // Default-style fields
    QLabel *templateLabel;
    QLineEdit *templateInput; // displays template name for editing (also stored in templateName)
    QLabel *oilTypeLabel;
    QLineEdit *oilTypeInput;

    // Keytag fields (hidden when default)
    QLabel *kt_templateLabel;
    QLineEdit *kt_templateInput;
    QLabel *customerLabel;
    QLineEdit *customerInput;
    QLabel *carLabel;
    QLineEdit *carInput;
    QLabel *plateLabel;
    QLineEdit *plateInput;
    QLabel *vinLabel;
    QLineEdit *vinInput;
    QLabel *colorLabel;
    QLineEdit *colorInput;
    QLabel *repairOrderLabel;
    QLineEdit *repairOrderInput;
    QLabel *quantityLabel;
    QLineEdit *quantityInput;

    LabelPreview *preview;

    QComboBox *styleCombo;           // dropdown to pick style
    QString labelStyle;              // "DEFAULT" or "KEYTAG"
    QString printerName;             // stores selected printer (or IP)
    QString backgroundPath;          // stores selected background PNG
    QString templateName;            // stores template name (DEFAULT.ZPL / KEYTAG.ZPL)
    int defaultMiles;
};
