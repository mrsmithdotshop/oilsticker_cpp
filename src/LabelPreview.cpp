// src/LabelPreview.cpp
#include "LabelPreview.hpp"

#include <QPainter>
#include <QFont>
#include <QFontDatabase>
#include <QFile>
#include <QCoreApplication>
#include <QDir>
#include <QDebug>
#include <QFileInfo>
#include <QPainterPath>

LabelPreview::LabelPreview(QWidget *parent)
    : QFrame(parent), nextMileage(), nextDate(), oilType(), today()
{
    // Set widget size to full label white box
    setFixedSize(448, 418);

    // Load Zebra A0 TTF font from resources
    int fontId = QFontDatabase::addApplicationFont(":/resources/tt0003m_.ttf");
    if (fontId != -1) {
        QStringList families = QFontDatabase::applicationFontFamilies(fontId);
        if (!families.isEmpty()) {
            zebraFontFamily = families.at(0);
            //qDebug() << "Loaded Zebra A0 font:" << zebraFontFamily;
        } else {
            qWarning() << "Loaded font but no families found!";
            zebraFontFamily.clear();
        }
    } else {
        qWarning() << "Failed to load font tt0003m_.ttf";
        zebraFontFamily.clear();
    }

    // Set initial background using the same method as loading new images
    setBackground(":/resources/max.png");
}

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
    if (!pix.isNull()) {
        background = pix; // keep original size (448x418)
    } else {
        qWarning() << "LabelPreview::setBackground — could not load:" << backgroundPath;
        // fallback white background of full widget size
        background = QPixmap(size());
        background.fill(Qt::white);
    }
    update();
}

void LabelPreview::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // --- Draw white box background (full widget) ---
    QRect whiteBoxRect(0, 0, width(), height());
    painter.fillRect(whiteBoxRect, Qt::white);

    // --- Draw PNG background centered on white box ---
    if (!background.isNull()) {
        int bgX = (width()  - background.width())  / 2;
        int bgY = (height() - background.height()) / 2;
        painter.drawPixmap(bgX, bgY, background);
    }

    // --- Draw 406x406 rounded rectangle (black outline) centered ---
    const int labelWidth  = 406;
    const int labelHeight = 406;
    int labelX = (width()  - labelWidth)  / 2;
    int labelY = (height() - labelHeight) / 2;
    QRect labelRect(labelX, labelY, labelWidth, labelHeight);

    QPainterPath borderPath;
    borderPath.addRoundedRect(labelRect, 20, 20);

    QPen borderPen(Qt::black);
    borderPen.setWidth(2);
    painter.setPen(borderPen);
    painter.setBrush(Qt::NoBrush);
    painter.drawPath(borderPath);

    // --- Prepare fonts ---
    QString fontFamily = zebraFontFamily.isEmpty() ? "Arial" : zebraFontFamily;
    QFont smallFont(fontFamily, 15);
    QFont largeFont(fontFamily, 30);
    painter.setPen(Qt::black);

    // --- Draw text inside label using temporary clip ---
    painter.save();
    painter.setClipPath(borderPath); // only clip inside rounded rectangle

    int padding = 25;
    int smallTextY = labelRect.top() + 280 * labelRect.height() / 406;
    int largeTextY = labelRect.top() + 360 * labelRect.height() / 406;

    painter.setFont(smallFont);
    if (!oilType.isEmpty()) {
        painter.drawText(labelRect.left() + padding, smallTextY, oilType);
    }
    if (!today.isEmpty()) {
        painter.drawText(labelRect.right() - padding - painter.fontMetrics().horizontalAdvance(today),
                         smallTextY, today);
    }

    painter.setFont(largeFont);
    if (!nextMileage.isEmpty()) {
        painter.drawText(labelRect.left() + padding, largeTextY, nextMileage);
    }
    if (!nextDate.isEmpty()) {
        painter.drawText(labelRect.right() - padding - painter.fontMetrics().horizontalAdvance(nextDate),
                         largeTextY, nextDate);
    }

    painter.restore(); // remove clipping
}

/* --- Helpers --- */

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

    // 2) try app bundle resources folder
    QFileInfo fi(path);
    QString baseName = fi.fileName();
    if (!baseName.isEmpty()) {
        QString appDir = QCoreApplication::applicationDirPath();
        QString bundleResources = QDir(appDir).filePath("resources/" + baseName);
        if (QFile::exists(bundleResources)) {
            QPixmap p2(bundleResources);
            if (!p2.isNull()) return p2;
        }
    }

    // 3) embedded default resource
    QString embedded = ":/resources/oil_label_bg.png";
    QPixmap p3(embedded);
    if (!p3.isNull()) return p3;

    return QPixmap();
}
