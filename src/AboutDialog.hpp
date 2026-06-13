#pragma once

#include <QDialog>
#include <QString>
#include <QVector>

class QTimer;

// About box with a hidden easter egg: a procedural "Doom fire" animation
// rendered behind the application info.
class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(const QString& text, QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void step();

private:
    void initFire();
    void spreadFire(int src);

    QString text_;
    QTimer* timer_;
    QVector<quint8> fire_;
    int fire_width_ = 0;
    int fire_height_ = 0;
    int scale_ = 4;
};
