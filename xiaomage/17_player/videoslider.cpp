#include "videoslider.h"
#include <QMouseEvent>
#include <QStyle>

VideoSlider::VideoSlider(QWidget *parent) : QSlider(parent)
{

}

void VideoSlider::mousePressEvent(QMouseEvent *ev) {

//    int value = minimum() + (ev->pos().x() / (maximum() - minimum()));
//    setValue(value);

    int value = QStyle::sliderValueFromPosition(minimum(), maximum(), ev->pos().x(), width());
    setValue(value);

    QSlider::mousePressEvent(ev);

    emit clicked(this);
}
