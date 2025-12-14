#pragma once

#include <QFrame>
#include <QString>
#include <QPixmap>

class LabelPreview : public QFrame
{
    Q_OBJECT

public:
    explicit LabelPreview(QWidget *parent = nullptr);

    void setQuantity(int q);

    // Unified updatePreview() covering both styles.
    // For DEFAULT style provide nextMileage/nextDate/oilType/today.
    // For KEYTAG style provide customer/car/plate/vin/color/repairOrder in the last params.
    void updatePreview(const QString &nextMileage = QString(),
                       const QString &nextDate = QString(),
                       const QString &oilType = QString(),
                       const QString &today = QString(),
                       const QString &customer = QString(),
                       const QString &car = QString(),
                       const QString &plate = QString(),
                       const QString &vin = QString(),
                       const QString &color = QString(),
                       const QString &repairOrder = QString());

    // Change the background image (filesystem path or resource path)
    void setBackground(const QString &backgroundPath);

    // Select which label style to render. "DEFAULT" or "KEYTAG"
    void setLabelStyle(const QString &style);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    // Try to load a pixmap from 'path'. If fails, return a null pixmap.
    QPixmap tryLoadPixmap(const QString &path) const;

    // Try multiple fallback locations for the given filename:
    // 1) exact path (if absolute or relative)
    // 2) app bundle Resources/<basename> (useful inside .app)
    // 3) embedded resource ":/resources/<basename>" (your default)
    QPixmap loadWithFallback(const QString &path) const;

    // Quantity
    //int m_quantity = 1;
    int quantity = 1;

    // Member state
    QPixmap background;
    QString nextMileage;
    QString nextDate;
    QString oilType;
    QString today;

    // Keytag fields
    QString customer;
    QString car;
    QString plate;
    QString vin;
    QString color;
    QString repairOrder;

    QString zebraFontFamily;
    QString currentStyle; // "DEFAULT" or "KEYTAG"
};
