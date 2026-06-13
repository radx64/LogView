#include "AboutDialog.hpp"

#include <algorithm>

#include <QDialogButtonBox>
#include <QImage>
#include <QPainter>
#include <QPushButton>
#include <QRandomGenerator>
#include <QResizeEvent>
#include <QTimer>
#include <QVBoxLayout>

namespace
{
// Classic 37-step fire gradient: black -> red -> orange -> yellow -> white.
const QRgb kFirePalette[] = {
    0x070707, 0x1F0707, 0x2F0F07, 0x470F07, 0x571707, 0x671F07, 0x771F07, 0x8F2707,
    0x9F2F07, 0xAF3F07, 0xBF4707, 0xC74707, 0xDF4F07, 0xDF5707, 0xDF5707, 0xD75F07,
    0xD75F07, 0xD7670F, 0xCF6F0F, 0xCF770F, 0xCF7F0F, 0xCF8717, 0xC78717, 0xC78F17,
    0xC7971F, 0xBF9F1F, 0xBF9F1F, 0xBFA727, 0xBFA727, 0xBFAF2F, 0xB7AF2F, 0xB7B72F,
    0xB7B737, 0xCFCF6F, 0xDFDF9F, 0xEFEFC7, 0xFFFFFF,
};
constexpr int kPaletteMax = static_cast<int>(sizeof(kFirePalette) / sizeof(kFirePalette[0])) - 1;
}

AboutDialog::AboutDialog(const QString& text, QWidget *parent)
    : QDialog(parent), text_(text)
{
    setWindowTitle(tr("About application"));
    setFixedSize(440, 320);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);

    auto* layout = new QVBoxLayout(this);
    layout->addStretch();
    layout->addWidget(buttons);

    timer_ = new QTimer(this);
    timer_->setInterval(33);
    connect(timer_, &QTimer::timeout, this, &AboutDialog::step);
    timer_->start();

    initFire();
}

void AboutDialog::initFire()
{
    fire_width_ = std::max(1, width() / scale_);
    fire_height_ = std::max(1, height() / scale_);
    fire_.assign(static_cast<int>(fire_width_) * fire_height_, 0);

    // Seed the bottom row with maximum intensity (the fire source).
    for (int x = 0; x < fire_width_; ++x)
        fire_[(fire_height_ - 1) * fire_width_ + x] = kPaletteMax;
}

void AboutDialog::spreadFire(int src)
{
    const int pixel = fire_[src];
    if (pixel == 0)
    {
        fire_[src - fire_width_] = 0;
        return;
    }

    const int rnd = QRandomGenerator::global()->bounded(4);
    const int dst = src - rnd + 1;
    const int target = dst - fire_width_;
    if (target >= 0 && target < fire_.size())
        fire_[target] = static_cast<quint8>(pixel - (rnd & 1));
}

void AboutDialog::step()
{
    for (int x = 0; x < fire_width_; ++x)
        for (int y = 1; y < fire_height_; ++y)
            spreadFire(y * fire_width_ + x);

    update();
}

void AboutDialog::resizeEvent(QResizeEvent* event)
{
    QDialog::resizeEvent(event);
    initFire();
}

void AboutDialog::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter painter(this);

    QImage frame(fire_width_, fire_height_, QImage::Format_RGB32);
    for (int y = 0; y < fire_height_; ++y)
    {
        QRgb* scanline = reinterpret_cast<QRgb*>(frame.scanLine(y));
        for (int x = 0; x < fire_width_; ++x)
        {
            const int intensity = std::clamp<int>(fire_[y * fire_width_ + x], 0, kPaletteMax);
            scanline[x] = 0xFF000000u | kFirePalette[intensity];
        }
    }

    painter.fillRect(rect(), Qt::black);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.drawImage(rect(), frame);

    // Dim panel behind the text so it stays readable over the flames.
    const QRect panel(20, 24, width() - 40, 150);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 150));
    painter.drawRoundedRect(panel, 10, 10);

    QFont title_font = font();
    title_font.setPointSizeF(title_font.pointSizeF() + 6.0);
    title_font.setBold(true);
    painter.setFont(title_font);
    painter.setPen(QColor(255, 230, 150));
    painter.drawText(panel.adjusted(16, 12, -16, 0), Qt::AlignTop | Qt::AlignHCenter, "LogView");

    painter.setFont(font());
    painter.setPen(Qt::white);
    painter.drawText(panel.adjusted(16, 44, -16, -12),
                     Qt::AlignHCenter | Qt::AlignTop | Qt::TextWordWrap, text_);
}
