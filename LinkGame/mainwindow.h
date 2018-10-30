#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProgressBar>
#include <QPushButton>
#include <QTimer>
#include "newitem.h"
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void createGameArea(int rows, int cols);            //游戏区域的widget
    bool findPath(NewItem* item1, NewItem* item2, QVector<QPair<int, int>> &link_route); //寻找item1和item2是否存在路径,如有保存在link_route
    void gameWin();                     //游戏胜利
    void gameLose();                    //游戏default
    void deleteGameAreaLayout();        //删除gameAreaWidget中的布局，为了重新布局
    bool deadLock();                    // 判断死锁

public:
    QPushButton* startBtn;              //开始按钮
    QPushButton* restartBtn;            //重新开始按钮
    QPushButton* clueBtn;               //提示按钮
    QWidget* gameAreaWidget;            //游戏区域的widget
    int game_area_height, game_area_width;
    QProgressBar* timerBar;             //计时器bar
protected:
    void changeEvent(QEvent* event);

private slots:
    void startGame();   // 开始游戏
    void restartGame(); // 重新开始游戏
    bool clue();        // 提示
    void itemClicked(NewItem* item);    // 如果item(图片)被点击
    void onTimeOut();                   // 计时器
private:
    Ui::MainWindow *ui;
    NewItem* items[20][20];             // 所有itme的数组，下标则为游戏的坐标
    NewItem* preClick = NULL;           // 前一个被选择的图片
    NewItem *clueClick1=NULL, *clueClick2=NULL;     // 用于保存，提示的两个item
    int gameGridRows, gameGridCols;     // 游戏区域的行和列
    QTimer *timer;                      // 计时器
    int time_length, time_step;         // 计时器bar的数据
    QVector<QLabel*> temp_resource;     // 保存临时QLabel资源，便于释放
};

#endif // MAINWINDOW_H
