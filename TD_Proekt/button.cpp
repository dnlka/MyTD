#include "button.h"
#include <QImage>

Button::Button(QString filePath, QString h_filePath, qreal scale): Image(filePath, scale), active(false)
{
    QImage* raw = new QImage(h_filePath);
    activeImage = new QImage(raw->scaled(raw->width()/scale, raw->height()/scale, Qt::KeepAspectRatio));
}

Button::Button(Image* passive, Image* active) : Image(*passive), activeImage(active->getImage()), active(false){}

Button::~Button(){
    delete activeImage;
}
