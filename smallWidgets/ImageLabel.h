#ifndef IMAGELABEL_H
#define IMAGELABEL_H

#include <QWidget>

namespace Ui {
class ImageLabel;
}

class ImageLabel : public QWidget
{
    Q_OBJECT

public:
    explicit ImageLabel(QWidget *parent = 0);
    ~ImageLabel();
    void setImage(QPixmap pixmap);
    void setResolution(QString resolution);
    void setResolution(QSize resolution);

private:
    Ui::ImageLabel *ui;
};

#endif // IMAGELABEL_H
