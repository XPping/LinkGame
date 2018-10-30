#ifndef NEWITEM_H
#define NEWITEM_H

#include <QLabel>
#include <QMouseEvent>

class NewItem : public QLabel
{
    Q_OBJECT
public:
    NewItem(int imageIndex, int posX, int posY);
    int imageIndex();   //  该控件对应的图片id
    int posX();         //  该控件所在游戏区域的坐标
    int posY();
protected:
    void mousePressEvent(QMouseEvent *e);
private:
    int index;
    int x;
    int y;
signals:
    void clicked(NewItem* item);    // 发出被点击的信号
};

#endif // NEWITEM_H
