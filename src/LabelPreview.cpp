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

    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    updatePreviewSize();

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

    QString newStyle = style.toUpper();
    if (newStyle == currentStyle)
        return;

    currentStyle = newStyle;

    updatePreviewSize();
    update();
}


void LabelPreview::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    QFont font("Swis721 BT");
    qreal scale = painter.device()->logicalDpiY() / 72.0;  // 72 DPI baseline
    font.setPointSizeF(12 / scale);                        // adjust 12pt
    painter.setFont(font);
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
    int labelWidth  = 406;
    int labelHeight = (currentStyle == "KEYTAG") ? 203 : 406;

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

    // --- Defaults ---
    int smallPoint;
    int largePoint;
    int smallYOffset = 285;
    int largeYOffset = 365;

    // --- Platform-specific font sizes ---
    #if defined(Q_OS_MACOS)
        smallPoint = 15;
        largePoint = 30;
    #elif defined(Q_OS_WIN)
        smallPoint = 11;
        largePoint = 22;
    #else
        smallPoint = 15;
        largePoint = 30;
    #endif

    // --- Style overrides ---
    if (currentStyle == "KEYTAG") {
    #if defined(Q_OS_MACOS)
        smallPoint = 20;
        largePoint = 150;
    #elif defined(Q_OS_WIN)
        smallPoint = 15;
        largePoint = 112;
    #endif
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

/*
if (quantity > 1) {
    painter.save();

    // Move origin to start a new block below the top section
    int offsetY = labelRect.top() + 25 + (painter.fontMetrics().height() + 2) * 6 + 10; 
    // 6 lines of text + 10px spacing between top and bottom copy

    int y2 = offsetY;
    const int lineH2 = painter.fontMetrics().height() + 2;

    if (!customer.isEmpty()) {
        painter.drawText(labelRect.left() + padding, y2, customer);
        y2 += lineH2;
    }
    if (!car.isEmpty()) {
        painter.drawText(labelRect.left() + padding, y2, car);
        y2 += lineH2;
    }
    if (!plate.isEmpty()) {
        painter.drawText(labelRect.left() + padding, y2, plate);
        y2 += lineH2;
    }
    if (!vin.isEmpty()) {
        painter.drawText(labelRect.left() + padding, y2, vin);
        y2 += lineH2;
    }
    if (!color.isEmpty()) {
        painter.drawText(labelRect.left() + padding, y2, color);
        y2 += lineH2;
    }
    if (!repairOrder.isEmpty()) {
        painter.drawText(labelRect.left() + padding, y2, repairOrder);
        y2 += lineH2;
    }

    painter.restore();
}
*/

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
    } 

    painter.restore();
}

// Set Quantitiy
void LabelPreview::setQuantity(int q)
{
    quantity = q > 0 ? q : 1;
    update();
}

void LabelPreview::updatePreviewSize()
{
    int w = 448;

    // --- Platform-specific padding ---
    #if defined(Q_OS_MACOS)
        int pad = 108;
    #elif defined(Q_OS_WIN)
        int pad = 138;
    #else
        int pad = 0;
    #endif

    int h = (currentStyle == "KEYTAG") ? ((418 + pad) / 2) : 418;

    setMinimumSize(w, h);
    setMaximumSize(w, h);
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
