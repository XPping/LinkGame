#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->restart_btn->setEnabled(false); // 刚开始重新开始游戏不能点击
    this->ui->clue_btn->setEnabled(false); // 提示按钮不能点击
    gameAreaWidget = ui->game_area_qwidget; // 动态生成游戏区域
    timerBar = ui->timer_bar;               // 时间进度条
    gameGridRows = 8;                       // 游戏区域的大小
    gameGridCols = 8;
    game_area_height = gameAreaWidget->size().height();     //游戏区域QWidget的大小，为了支持窗口放大缩小
    game_area_width = gameAreaWidget->size().width();

    for(int i=0; i<gameGridRows+2; i++)                     // 初始化游戏区域的坐标
        for(int j=0; j<gameGridCols+2; j++)
            items[i][j] = NULL;
    connect(this->ui->start_btn, SIGNAL(clicked()), this, SLOT(startGame()));       // 开始游戏
    connect(this->ui->restart_btn, SIGNAL(clicked()), this, SLOT(restartGame()));   // 玩游戏时，重新开始游戏
    connect(this->ui->clue_btn, SIGNAL(clicked()), this, SLOT(clue()));             // 给出提示
    // 计时器
    time_step = time_length = 300;
    timerBar->setRange(0, time_length);
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(onTimeOut()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::startGame()
{
    deleteGameAreaLayout();     // 删除游戏区域的所有控件
    gameAreaWidget->setAutoFillBackground(true);    // 设置背景
    QString str;
    str = ":/images/images/backgroud.png";
    QPixmap pixmp(str);
    QPalette palette = gameAreaWidget->palette();
    palette.setBrush(QPalette::Window,
                     QBrush(pixmp.scaled(gameAreaWidget->size(),
                     Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
    gameAreaWidget->setPalette(palette);

    this->ui->start_btn->setEnabled(false); //设置开始按钮不能点击
    this->ui->restart_btn->setEnabled(true); // 重新开始按钮可以点击
    this->ui->clue_btn->setEnabled(true); // 提示按钮可以点击
    createGameArea(gameGridRows, gameGridCols); // 初始化游戏Item
    if (timer->isActive())
        timer->stop();
    time_step = time_length;
    timer->start(time_length);  // 开始计时
}

void MainWindow::restartGame()
{
    // 删除游戏区域余下的Item
    for(int i=1; i<=gameGridCols; i++)
        for(int j=1; j<=gameGridRows; j++){
            if(items[i][j] != NULL)
                delete items[i][j];
            items[i][j] = NULL;
        }
    deleteGameAreaLayout();
    createGameArea(gameGridRows, gameGridCols); // 初始化游戏Item
    if (timer->isActive())
        timer->stop();
    time_step = time_length;
    timer->start(time_length);  // 开始计时
}

bool MainWindow::clue()
{
    for(int i=1; i<=gameGridCols; i++){
        for(int j=1; j<=gameGridRows; j++){
            for(int k=1; k<=gameGridCols; k++){
                for(int l=1; l<=gameGridRows; l++){
                    // items[i][j]是否与items[k][l]有连通,有则保存到clueClick1和clueClik2中
                    QVector<QPair<int, int> > tmp;
                    if(items[i][j] != NULL && items[k][l] != NULL
                            && items[i][j]->imageIndex()== items[k][l]->imageIndex()
                            && findPath(items[i][j], items[k][l], tmp)){
                        items[i][j]->setStyleSheet("border:2px solid red;");
                        items[k][l]->setStyleSheet("border:2px solid red;");
                        clueClick1 = items[i][j];
                        clueClick2 = items[k][l];
                        if(preClick != NULL)
                            preClick->setStyleSheet("border:0px solid red;");
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

void MainWindow::gameWin()
{
    // 游戏胜利
    deleteGameAreaLayout();
    gameAreaWidget->setAutoFillBackground(true);
    QString str;
    str = ":/images/images/vectory.png";    // 设置victory图片
    QPixmap pixmp(str);
    QPalette palette = gameAreaWidget->palette();
    palette.setBrush(QPalette::Window,
                     QBrush(pixmp.scaled(gameAreaWidget->size(),
                     Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
    gameAreaWidget->setPalette(palette);
    this->ui->start_btn->setEnabled(true); //设置开始按钮可以点击
    this->ui->restart_btn->setEnabled(false);
    this->ui->clue_btn->setEnabled(false);
}

void MainWindow::gameLose()
{
     // 游戏失败
    for(int i=1; i<=gameGridCols; i++)
        for(int j=1; j<=gameGridRows; j++){
            if(items[i][j] != NULL)
                delete items[i][j];
            items[i][j] = NULL;
        }
    deleteGameAreaLayout();
    gameAreaWidget->setAutoFillBackground(true);
    QString str;
    str = ":/images/images/defeat.png"; //设置defeat图片
    QPixmap pixmp(str);
    QPalette palette = gameAreaWidget->palette();
    palette.setBrush(QPalette::Window,
                     QBrush(pixmp.scaled(gameAreaWidget->size(),
                     Qt::IgnoreAspectRatio,Qt::SmoothTransformation)));
    gameAreaWidget->setPalette(palette);
    this->ui->start_btn->setEnabled(true); //设置开始按钮可以点击
    this->ui->restart_btn->setEnabled(false);
    this->ui->clue_btn->setEnabled(false);
}

void MainWindow::createGameArea(int rows, int cols)
{
    preClick = clueClick1 = clueClick2 = NULL;
    // 每一个item对应一个随机的图片id, 图片id成对出现
    QVector<int> vec;
    for(int i=0; i<rows * cols / 2; i++){
        int image_index = rand() % 10 +1; //10组图片
        vec.push_back(image_index);
        vec.push_back(image_index);
    }
    // 随机打乱item的排序，即打乱vec的索引值
    for(int i=0; i<vec.size(); i++){
        int rand_index = rand() % vec.size();
        int tmp = vec[i];
        vec[i] = vec[rand_index];
        vec[rand_index] = tmp;
    }
    for(int i=1; i<=rows; i++)
        for(int j=1; j<=cols; j++){
            int image_index = vec[(i-1)*rows+j-1];
            items[i][j] = new NewItem(image_index, i, j);
            QString str;
            str = ":/images/images/"+QString("%1").arg(image_index)+".png";
            QPixmap pixmp(str);
            items[i][j]->setPixmap(pixmp);
            items[i][j]->resize(game_area_width / gameGridCols, game_area_height / gameGridCols);
        }
    // 动态添加游戏区域
    QGridLayout* layout = new QGridLayout();
    for(int i=1; i<=rows; i++)
        for(int j=1; j<=cols; j++){
            layout->addWidget(items[i][j], i, j, 1, 1);
            connect(items[i][j], SIGNAL(clicked(NewItem*)), this, SLOT(itemClicked(NewItem*))); // 给每一个item建立信号与槽
        }
    deleteGameAreaLayout();
    gameAreaWidget->setLayout(layout);
}

bool MainWindow::findPath(NewItem *item1, NewItem *item2, QVector<QPair<int, int>> &link_route) // link_route保存最短连通路径
{
    if(item1 == NULL || item2 == NULL) return false;
    int x1 = item1->posX();
    int y1 = item1->posY();
    int x2 = item2->posX();
    int y2 = item2->posY();
    if(x1==x2 && y1==y2)
        return false;
    QMap<QPair<int, int>, QPair<int, int> > route; // 保存路径,即route.second可以到达route.first;
    //qDebug()<<x1<<" "<<y1<<" "<<x2<<" "<<y2<<endl;
    QSet<QPair<int , int > > one_line; // (x1, y1)可以直线到达的点(x2, y2)
    for(int i=x1+1; i<gameGridCols+2; i++){
        one_line.insert(QPair<int, int>(i, y1));
        route[QPair<int, int>(i, y1)] = QPair<int, int>(x1, y1);
        if(items[i][y1] != NULL) break;
    }
    for(int i=x1-1; i>=0; i--){
        one_line.insert(QPair<int, int>(i, y1));
        route[QPair<int, int>(i, y1)] = QPair<int, int>(x1, y1);
        if(items[i][y1] != NULL) break;
    }
    for(int i=y1+1; i<gameGridRows+2; i++){
        one_line.insert(QPair<int, int>(x1, i));
        route[QPair<int, int>(x1, i)] = QPair<int, int>(x1, y1);
        if(items[x1][i] != NULL) break;
    }
    for(int i=y1-1; i>=0; i--){
        one_line.insert(QPair<int, int>(x1, i));
        route[QPair<int, int>(x1, i)] = QPair<int, int>(x1, y1);
        if(items[x1][i] != NULL) break;
    }
    if(one_line.find(QPair<int, int>(x2, y2)) != one_line.end()){
        link_route.push_back(QPair<int, int>(x1, y1));
        link_route.push_back(QPair<int, int>(x2, y2));
        return true;
    }

    QSet<QPair<int, int> > two_line; // (x1, y1)可以通过一个拐点到达(x2, y2)
    QSetIterator<QPair<int, int> > one_line_iter(one_line);
    //qDebug()<<"one_Line: ";
    while(one_line_iter.hasNext()){
        QPair<int, int> tmp = one_line_iter.next();
        int tmp_x = tmp.first, tmp_y = tmp.second;
        //qDebug()<<tmp_x<<","<<tmp_y<<" ";
        if(items[tmp_x][tmp_y] != NULL) continue;  // 拐点不能有图片
        for(int i=tmp_x+1; i<gameGridCols+2; i++){
            if(one_line.find(QPair<int, int>(i, tmp_y)) == one_line.end()){
                two_line.insert(QPair<int, int>(i, tmp_y));
                route[QPair<int, int>(i, tmp_y)] = QPair<int, int>(tmp_x, tmp_y);
            }
            if(items[i][tmp_y] != NULL) break;
        }
        for(int i=tmp_x-1; i>=0; i--){
            if(one_line.find(QPair<int, int>(i, tmp_y)) == one_line.end()){
                two_line.insert(QPair<int, int>(i, tmp_y));
                route[QPair<int, int>(i, tmp_y)] = QPair<int, int>(tmp_x, tmp_y);
            }
            if(items[i][tmp_y] != NULL) break;
        }
        for(int i=tmp_y+1; i<gameGridRows+2; i++){
            if(one_line.find(QPair<int, int>(tmp_x, i)) == one_line.end()){
                two_line.insert(QPair<int, int>(tmp_x, i));
                route[QPair<int, int>(tmp_x, i)] = QPair<int, int>(tmp_x, tmp_y);
            }
            if(items[tmp_x][i] != NULL) break;
        }
        for(int i=tmp_y-1; i>=0; i--){
            if(one_line.find(QPair<int, int>(tmp_x, i)) == one_line.end()){
                two_line.insert(QPair<int, int>(tmp_x, i));
                route[QPair<int, int>(tmp_x, i)] = QPair<int, int>(tmp_x, tmp_y);
            }
            if(items[tmp_x][i] != NULL) break;
        }
    }
    if(two_line.find(QPair<int, int>(x2, y2)) != two_line.end()){
        link_route.push_back(QPair<int, int>(x1, y1));
        link_route.push_back(route[QPair<int, int>(x2, y2)]);
        link_route.push_back(QPair<int, int>(x2, y2));
        return true;
    }
    QSet<QPair<int, int> > three_line; // (x1, y1)可以通过两个拐点到达(x2, y2)
    QSetIterator<QPair<int, int> > two_line_iter(two_line);
    //qDebug()<<endl;
    //qDebug()<<"tow line: ";
    while(two_line_iter.hasNext()){
        QPair<int, int> tmp = two_line_iter.next();
        int tmp_x = tmp.first, tmp_y = tmp.second;
        //qDebug()<<tmp_x<<","<<tmp_y<<" ";
        if(items[tmp_x][tmp_y] != NULL) continue;  // 拐点不能有图片
        for(int i=tmp_x+1; i<gameGridCols+2; i++){
            if(two_line.find(QPair<int, int>(i, tmp_y)) == two_line.end()
                    && one_line.find(QPair<int, int>(i, tmp_y)) == one_line.end()){
                three_line.insert(QPair<int, int>(i, tmp_y));
                route[QPair<int, int>(i, tmp_y)] = QPair<int, int>(tmp_x, tmp_y);
            }
            if(items[i][tmp_y] != NULL) break;
        }
        for(int i=tmp_x-1; i>=0; i--){
            if(two_line.find(QPair<int, int>(i, tmp_y)) == two_line.end()
                    && one_line.find(QPair<int, int>(i, tmp_y)) == one_line.end()){
                three_line.insert(QPair<int, int>(i, tmp_y));
                route[QPair<int, int>(i, tmp_y)] = QPair<int, int>(tmp_x, tmp_y);
            }
            if(items[i][tmp_y] != NULL) break;
        }
        for(int i=tmp_y+1; i<gameGridRows+2; i++){
            if(two_line.find(QPair<int, int>(tmp_x, i)) == two_line.end()
                    && one_line.find(QPair<int, int>(tmp_x, i)) == one_line.end()){
                three_line.insert(QPair<int, int>(tmp_x, i));
                route[QPair<int, int>(tmp_x, i)] = QPair<int, int>(tmp_x, tmp_y);
            }
            if(items[tmp_x][i] != NULL) break;
        }
        for(int i=tmp_y-1; i>=0; i--){
            if(two_line.find(QPair<int, int>(tmp_x, i)) == two_line.end()
                    && one_line.find(QPair<int, int>(tmp_x, i)) == one_line.end()){
                three_line.insert(QPair<int, int>(tmp_x, i));
                route[QPair<int, int>(tmp_x, i)] = QPair<int, int>(tmp_x, tmp_y);
            }
            if(items[tmp_x][i] != NULL) break;
        }
    }

    if(three_line.find(QPair<int, int>(x2, y2)) != three_line.end()){
        link_route.push_back(QPair<int, int>(x1, y1));
        link_route.push_back(route[route[QPair<int, int>(x2, y2)]]);
        link_route.push_back(route[QPair<int, int>(x2, y2)]);
        link_route.push_back(QPair<int, int>(x2, y2));
        return true;
    }
    return false;
}


void MainWindow::deleteGameAreaLayout()
{
    // 删除游戏区域资源
    while(temp_resource.size()){
        (*temp_resource.begin())->close();
        temp_resource.remove(0);
    }
    if(gameAreaWidget->layout() == NULL) return;
    QLayoutItem *child = gameAreaWidget->layout()->takeAt(0);
    while(child){
        child = gameAreaWidget->layout()->takeAt(0);
        delete child;
    }
    delete gameAreaWidget->layout();
}

bool MainWindow::deadLock()
{
    // 判断游戏死锁，一直随机打乱游戏区域item的下标，知道没有死锁
    while(clue()){
        QVector<NewItem*> tmp_vec;
        for(int i=1; i<=gameGridCols; i++)
            for(int j=1; j<=gameGridRows; j++){
                tmp_vec.push_back(items[i][j]);
            }
        QVector<int> random_index(tmp_vec.size());
        for(int i=0; i<random_index.size(); i++){
            int rand_index = rand() % random_index.size();
            int tmp = random_index[i];
            random_index[i] = random_index[rand_index];
            random_index[rand_index] = tmp;
        }
        for(int i=1; i<=gameGridCols; i++)
            for(int j=1; j<=gameGridRows; j++){
                int index = random_index[(i-1)*gameGridCols+j-1];
                items[i][j] = tmp_vec[index];
            }
    }
}

void MainWindow::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::WindowStateChange){ // 适应窗口变大变小
        game_area_height = gameAreaWidget->size().height();
        game_area_width = gameAreaWidget->size().width();
        qDebug()<<game_area_height<<' '<<game_area_width;
        // 删除原布局，重新布局游戏区域，因为QGridLayout一出某一行或一列的组件，剩余的会重组，导致布局缩小
        bool not_start = true;
        for(int i=1; i<=gameGridRows; i++)
            for(int j=1; j<=gameGridCols; j++)
                if(items[i][j]!=NULL){not_start=false; break;}
        if(not_start)
            return;
        deleteGameAreaLayout();
        QGridLayout* layout = new QGridLayout();
        for(int i=1; i<=gameGridRows; i++)
            for(int j=1; j<=gameGridCols; j++){
                if(items[i][j]!=NULL){
                    items[i][j]->resize(game_area_width / gameGridCols, game_area_height / gameGridCols);
                    layout->addWidget(items[i][j], i, j, 1, 1);
                }
                else{
                    QString str;
                    str = ":/images/images/backgroud.png";
                    QPixmap pixmp(str);
                    QLabel *label = new QLabel();
                    label->setPixmap(pixmp);
                    label->resize(game_area_width / gameGridCols, game_area_height / gameGridCols);
                    temp_resource.push_back(label);
                    layout->addWidget(label, i, j, 1, 1);
                }
            }
        gameAreaWidget->setLayout(layout);
    }
}

void MainWindow::itemClicked(NewItem* item)
{
    // 当使用clue时，要额外处理，因为有可能不选择提示的内容，此时要把提示的内容还原
    if(clueClick1 != NULL && clueClick2 != NULL){
        if(item != clueClick1 && item != clueClick2){
            clueClick1->setStyleSheet("border:0px solid red;"); // 取消被点击图片的红色外框
            clueClick2->setStyleSheet("border:0px solid red;");// 取消被点击图片的红色外框
            clueClick1 = clueClick2 = NULL;
        }else if(preClick == clueClick1 && item != clueClick2){
            clueClick2->setStyleSheet("border:0px solid red;");// 取消被点击图片的红色外框
            clueClick1 = clueClick2 = NULL;
        }else if (preClick == clueClick2 && item != clueClick1){
            clueClick1->setStyleSheet("border:0px solid red;");// 取消被点击图片的红色外框
            clueClick1 = clueClick2 = NULL;
        }
    }
    QVector<QPair<int, int> > link_route; // 保存路径
    if(preClick != NULL && item != NULL
            && preClick->imageIndex() == item->imageIndex()
            && findPath(preClick, item, link_route)){
        QVectorIterator<QPair<int, int> > iter(link_route);
        qDebug()<<"link route: ";
         while(iter.hasNext()){ // 如果存在路径则打印出来
             QPair<int, int> tmp = iter.next();
             int tmp_x = tmp.first, tmp_y = tmp.second;
             qDebug()<<tmp_x<<","<<tmp_y<<" ";
         }
        items[preClick->posX()][preClick->posY()]->close();
        items[item->posX()][item->posY()]->close();
        items[preClick->posX()][preClick->posY()] = NULL;
        items[item->posX()][item->posY()] = NULL;
        // 删除原布局，重新布局游戏区域，因为QGridLayout一出某一行或一列的组件，剩余的会重组，导致布局缩小
        deleteGameAreaLayout();
        QGridLayout* layout = new QGridLayout();
        for(int i=1; i<=gameGridRows; i++)
            for(int j=1; j<=gameGridCols; j++){
                if(items[i][j]!=NULL){
                    items[i][j]->resize(game_area_width / gameGridCols, game_area_height / gameGridCols);
                    layout->addWidget(items[i][j], i, j, 1, 1);
                }
                else{
                    QString str;
                    str = ":/images/images/backgroud.png";
                    QPixmap pixmp(str);
                    QLabel *label = new QLabel();
                    label->setPixmap(pixmp);
                    label->resize(game_area_width / gameGridCols, game_area_height / gameGridCols);
                    temp_resource.push_back(label);
                    layout->addWidget(label, i, j, 1, 1);
                }
            }
        gameAreaWidget->setLayout(layout);
        preClick = NULL;
        return;
    }


    if(preClick != NULL){
        preClick->setStyleSheet("border:0px solid red;");   // 取消被点击图片的红色外框
    }
    item->setStyleSheet("border:1px solid red;");   // 被点击的图片则有红色外框标记出来
    preClick = item;
}

void MainWindow::onTimeOut()
{
    --time_step;
    timerBar->setValue(time_step);
    bool is_win = true;
    for(int i=1; i<=gameGridCols; i++)      // 游戏区域所有item都为NULL，则胜利
        for(int j=1; j<=gameGridRows; j++)
            if(items[i][j] != NULL){is_win = false; break;}
    if(is_win) {
        gameWin();
        timerBar->setValue(0);
        timer->stop();
        time_step = time_length;
    }
    if(time_step == 0){     // 时间到，失败
        qDebug()<<"ha";
        if(timer->isActive())
            timer->stop();
        time_step = time_length;
        gameLose();
        qDebug()<<"hee";
    }
}
