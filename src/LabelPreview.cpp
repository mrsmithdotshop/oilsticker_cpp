// src/LabelPreview.cpp
#include "LabelPreview.hpp"

#include <QPainter>
#include <QFont>
#include <QFile>
#include <QCoreApplication>
#include <QDir>
#include <QDebug>
#include <QFileInfo>

LabelPreview::LabelPreview(const QString &backgroundPath, QWidget *parent)
    : QFrame(parent), nextMileage(), nextDate(), oilType(), today()
{
    setFixedSize(406, 406);

    // Load background with fallback logic
    background = loadWithFallback(backgroundPath);

    if (background.isNull()) {
        // final fallback -> white fill
        background = QPixmap(size());
        background.fill(Qt::white);
    } else {
        background = background.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    }
}

// Updated updatePreview with oilType and today's date
void LabelPreview::updatePreview(const QString &mileage,
                                 const QString &dateStr,
                                 const QString &oilTypeStr,
                                 const QString &todayStr)
{
    nextMileage = mileage;
    nextDate = dateStr;
    oilType = oilTypeStr;
    today = todayStr;
    update();
}

void LabelPreview::setBackground(const QString &backgroundPath)
{
    QPixmap pix = loadWithFallback(backgroundPath);

    if (pix.isNull()) {
        qWarning() << "LabelPreview::setBackground — could not load:" << backgroundPath;
        // keep existing background (if any) or use white placeholder
        if (background.isNull()) {
            background = QPixmap(size());
            background.fill(Qt::white);
        }
    } else {
        background = pix.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    }

    update();
}

void LabelPreview::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    // Draw background
    if (!background.isNull()) {
        QPixmap bg = background;
        if (bg.size() != size()) {
            bg = bg.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        }
        painter.drawPixmap(0, 0, bg);
    } else {
        painter.fillRect(rect(), Qt::white);
    }

    painter.setPen(Qt::black);
    QFont font("Arial", 15);  
    painter.setFont(font);

    // Draw oil type
    if (!oilType.isEmpty()) {
        painter.drawText(25, 285, oilType);
    }

    // Draw today's date 
    if (!today.isEmpty()) {
        painter.drawText(325, 285, today);
    }
    
    font.setPointSize(30);
    painter.setFont(font);

    // Draw next mileage (bottom-left)
    if (!nextMileage.isEmpty()) {
        painter.drawText(25, 365, nextMileage);
    }

    // Draw next service date (bottom-right)
    if (!nextDate.isEmpty()) {
        painter.drawText(265, 365, nextDate);
    }
}

/* Helpers (unchanged) */

QPixmap LabelPreview::tryLoadPixmap(const QString &path) const
{
    if (path.isEmpty()) return QPixmap();

    if (path.startsWith(":/")) {
        QPixmap p(path);
        if (!p.isNull()) return p;
        return QPixmap();
    }

    if (QFile::exists(path)) {
        QPixmap p(path);
        if (!p.isNull()) return p;
    }
    return QPixmap();
}

QPixmap LabelPreview::loadWithFallback(const QString &path) const
{
    // 1) exact path
    QPixmap p = tryLoadPixmap(path);
    if (!p.isNull()) return p;

    // 2) try bundle Resources/<basename>
    QFileInfo fi(path);
    QString baseName = fi.fileName();
    if (!baseName.isEmpty()) {
        QString appDir = QCoreApplication::applicationDirPath();
        QString bundleResources = QDir(appDir).filePath("../Resources/" + baseName);
        if (QFile::exists(bundleResources)) {
            QPixmap p2(bundleResources);
            if (!p2.isNull()) return p2;
        }
    }

    // 3) embedded default
    QString embedded = ":/resources/oil_label_bg.png";
    QPixmap p3(embedded);
    if (!p3.isNull()) return p3;

    return QPixmap();
}
