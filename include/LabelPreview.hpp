#pragma once

#include <QFrame>
#include <QString>
#include <QPixmap>

class LabelPreview : public QFrame
{
    Q_OBJECT

public:
    // Constructor now only takes parent
    explicit LabelPreview(QWidget *parent = nullptr);

    // Update dynamic text fields on the preview
    void updatePreview(const QString &mileage,
                       const QString &dateStr,
                       const QString &oilType = QString(),
                       const QString &today = QString());

    // Change the background image (filesystem path or resource path)
    void setBackground(const QString &backgroundPath);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    // Try to load a pixmap from 'path'. If fails, return a null pixmap.
    QPixmap tryLoadPixmap(const QString &path) const;

    // Try multiple fallback locations for the given filename:
    // 1) exact path (if absolute or relative)
    // 2) app bundle resources folder
    // 3) embedded resource ":/resources/<basename>"
    QPixmap loadWithFallback(const QString &path) const;

    // Member state
    QPixmap background;
    QString nextMileage;
    QString nextDate;
    QString oilType;   // dynamic
    QString today;     // dynamic
    QString zebraFontFamily; // loaded Zebra A0 font family
};
