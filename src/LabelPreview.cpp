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
    : QFrame(parent),
      nextMileage(), nextDate(), oilType(), today(),
      customer(), car(), plate(), vin(), color(), repairOrder(),
      zebraFontFamily(), currentStyle("DEFAULT")
{
    // Widget size - white box that holds the background image (448x418)
    setFixedSize(448, 418);

    // Try to load the Zebra A0 TTF file from qrc resources (optional)
    int fontId = QFontDatabase::addApplicationFont(":/resources/tt0003m_.ttf");
    if (fontId != -1) {
        QStringList families = QFontDatabase::applicationFontFamilies(fontId);
        if (!families.isEmpty()) {
            zebraFontFamily = families.at(0);
            qDebug() << "Loaded Zebra A0 font:" << zebraFontFamily;
        }
    }

    // Default background
    setBackground(":/resources/default.png");
}

void LabelPreview::updatePreview(const QString &nm,
                                 const QString &nd,
                                 const QString &ot,
                                 const QString &td,
                                 const QString &cust,
                                 const QString &c,
                                 const QString &p,
                                 const QString &v,
                                 const QString &col,
                                 const QString &ro)
{
    nextMileage = nm;
    nextDate = nd;
    oilType = ot;
    today = td;

    customer = cust;
    car = c;
    plate = p;
    vin = v;
    color = col;
    repairOrder = ro;

    update();
}

void LabelPreview::setBackground(const QString &backgroundPath)
{
    QPixmap pix = loadWithFallback(backgroundPath);
    if (!pix.isNull()) {
        background = pix; // keep original bitmap size (expected 448x418)
    } else {
        qWarning() << "LabelPreview::setBackground — could not load:" << backgroundPath;
        // fallback white background of full widget size
        background = QPixmap(size());
        background.fill(Qt::white);
    }
    update();
}

void LabelPreview::setLabelStyle(const QString &style)
{
    if (style.isEmpty()) return;
    currentStyle = style.toUpper();
    update();
}

void LabelPreview::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // --- White base box (full widget) ---
    QRect whiteBoxRect(0, 0, width(), height());
    painter.fillRect(whiteBoxRect, Qt::white);

    // --- Draw background centered (do NOT scale - PNG should be 448x418) ---
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

    // --- Prepare fonts (fallback to Arial) ---
    QString fontFamily = zebraFontFamily.isEmpty() ? "Arial" : zebraFontFamily;

    // Style-specific variables
    int smallPoint = 15;
    int largePoint = 30;
    int smallYOffset = 285;
    int largeYOffset = 365;

    if (currentStyle == "KEYTAG") {
        smallPoint = 20;
        largePoint = 150;
        smallYOffset = 260;
        largeYOffset = 360;
    }

    // global shift up 5 px (user request)
    const int globalShiftUp = -5;
    smallYOffset += globalShiftUp;
    largeYOffset += globalShiftUp;

    // Draw clipped content inside rounded rectangle
    painter.save();
    painter.setClipPath(borderPath);

    int padding = 25;
    int smallTextY = labelRect.top() + smallYOffset * labelRect.height() / 406;
    int largeTextY = labelRect.top() + largeYOffset * labelRect.height() / 406;

    // SMALL font (oil type / date or keytag small fields)
    QFont smallFont(fontFamily, smallPoint);
    painter.setFont(smallFont);
    painter.setPen(Qt::black);

    if (currentStyle == "DEFAULT") {
        if (!oilType.isEmpty()) {
            painter.drawText(labelRect.left() + padding, smallTextY, oilType);
        }
        if (!today.isEmpty()) {
            int tw = painter.fontMetrics().horizontalAdvance(today);
            painter.drawText(labelRect.right() - padding - tw, smallTextY, today);
        }
    } else { // KEYTAG: show a compact set of keytag fields near top-left
        int y = labelRect.top() + 25;
        const int lineH = painter.fontMetrics().height() + 2;
        if (!customer.isEmpty()) { painter.drawText(labelRect.left() + padding, y, customer); y += lineH; }
        if (!car.isEmpty())      { painter.drawText(labelRect.left() + padding, y, car);      y += lineH; }
        if (!plate.isEmpty())    { painter.drawText(labelRect.left() + padding, y, plate);    y += lineH; }
        if (!vin.isEmpty())      { painter.drawText(labelRect.left() + padding, y, vin);      y += lineH; }
        if (!color.isEmpty())    { painter.drawText(labelRect.left() + padding, y, color);    y += lineH; }
        if (!repairOrder.isEmpty())    { painter.drawText(labelRect.left() + padding, y, repairOrder);    y += lineH; }
        // repairOrder will be printed larger below
    }

    // LARGE font (mileage / nextDate or repairOrder for keytag)
    QFont largeFont(fontFamily, largePoint);
    painter.setFont(largeFont);

    if (currentStyle == "DEFAULT") {
        if (!nextMileage.isEmpty()) {
            painter.drawText(labelRect.left() + padding, largeTextY, nextMileage);
        }
        if (!nextDate.isEmpty()) {
            int tw = painter.fontMetrics().horizontalAdvance(nextDate);
            painter.drawText(labelRect.right() - padding - tw, largeTextY, nextDate);
        }
    } else { // KEYTAG large field (repair order) bottom-left and duplicate on bottom-right
        QString ro = repairOrder.trimmed();   // remove leading/trailing whitespace
        bool ok = false;
        ro.toInt(&ok);                         // ok==true if the entire string is a number

        // Show repairOrder only when it's not empty AND not a pure integer
        if (!ro.isEmpty() && ok) {
            painter.drawText(labelRect.left() + padding, largeTextY, repairOrder);
        }
            //painter.drawText(labelRect.left() + padding, largeTextY, repairOrder);
        }
    

    painter.restore();
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

    // 2) try app bundle Resources/<basename>
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
    QString embedded = ":/resources/default.png";
    QPixmap p3(embedded);
    if (!p3.isNull()) return p3;

    return QPixmap();
}
