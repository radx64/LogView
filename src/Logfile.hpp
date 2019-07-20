#ifndef LOGFILE_HPP
#define LOGFILE_HPP

#include <QFile>
#include <QMessageBox>
#include <QVector>

struct Line
{
    uint32_t number;
    QString text;
};

using Lines = QVector<Line>;

class Logfile
{
public:
    Logfile(const QString& filename)
    {
        filename_ = filename;
        lines_.clear();

        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly))
        {
            QMessageBox msg;
            msg.setText("Unable to open file" + file.errorString());
            msg.setStandardButtons(QMessageBox::Ok);
            msg.setIcon(QMessageBox::Critical);
            msg.exec();
            return;
        }

        uint32_t index{};
        while(!file.atEnd())
        {
            ++index;
            lines_.append({index, file.readLine()});
        }
    }

    const Lines& getLines() const
    {
        return lines_;
    }
    const QString& getFileName() const
    {
        return filename_;
    }

protected:
    Lines lines_;
    QString filename_;

};

#endif // LOGFILE_HPP
