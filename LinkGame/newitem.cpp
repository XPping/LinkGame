#include "newitem.h"

#include <QDebug>

NewItem::NewItem(int imageIndex, int posX, int posY)
{
    index = imageIndex;
    x = posX;
    y = posY;
}

int NewItem::imageIndex()
{
    return index;
}
int NewItem::posX()
{
    return x;
}
int NewItem::posY()
{
    return y;
}

void NewItem::mousePressEvent(QMouseEvent *e)
{
    if(e->button() == Qt::LeftButton){
        emit this->clicked(this);           // 发出该控件被点击的信号
    }
}
