#include "game.h"
#include <QKeyEvent>
#include <QGraphicsTextItem>
#include <QIcon>
#include <QPushButton>
#include <QGraphicsProxyWidget>

Game::Game(QWidget* parent) : QGraphicsView(parent), score(0),isGameOver(false), currentDifficulty(Normal), isMenuVisible(true) {
	scene = new QGraphicsScene(this);
	setScene(scene);

    setWindowTitle("Ikun牌小鸟");

    QIcon icon(":/assets/images/bluebird-upflap.png");
    setWindowIcon(icon);

	setFixedSize(400, 600);
	scene->setSceneRect(0, 0, 400, 600);

	scene->setBackgroundBrush(QBrush(QImage(":/assets/images/background-day.png").scaled(400, 600)));

	// 取消滚动条
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	scoreText = nullptr;
    bird = nullptr;

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Game::gameLoop);
	showMenu();
}

void Game::showMenu() {
    // 清空场景
    scene->clear();
    isMenuVisible = true;
    // 标题
    QGraphicsTextItem* title = new QGraphicsTextItem("选择游戏难度");
    title->setDefaultTextColor(Qt::black);
    title->setFont(QFont("Arial", 24, QFont::Bold));
    title->setPos(width() / 2 - title->boundingRect().width() / 2, 60);
    scene->addItem(title);
    // 难度按钮
    const char* modeNames[6] = {"刘思哲", "乐志财", "景志丰", "吴程东", "周琼日", "阮熙玉"};
    for (int i = 0; i < 6; ++i) {
        QPushButton* btn = new QPushButton(modeNames[i]);
        btn->setMinimumWidth(120);
        btn->setFont(QFont("Arial", 16));
        QGraphicsProxyWidget* proxy = scene->addWidget(btn);
        proxy->setPos(width() / 2 - 60, 140 + i * 60);
        connect(btn, &QPushButton::clicked, this, [this, i]() {
            setDifficulty(static_cast<Difficulty>(i));
            QTimer::singleShot(0, this, [this]() { startGameWithDifficulty(); });
        });
    }
}

void Game::startGameWithDifficulty() {
    // 清空场景，初始化游戏元素
    scene->clear();
    isMenuVisible = false;
    score = 0;
    isGameOver = false;
    pipes.clear();
    // 先delete旧的bird和scoreText
    if (bird) { delete bird; bird = nullptr; }
    if (scoreText) { delete scoreText; scoreText = nullptr; }
    // 重新添加分数文本
    scoreText = new QGraphicsTextItem(QString("Score: %1").arg(score));
    scoreText->setZValue(1);
    scoreText->setDefaultTextColor(Qt::white);
    scoreText->setFont(QFont("Arial", 20));
    scoreText->setPos(10, 10);
    scene->addItem(scoreText);
    bird = new Bird();
    scene->addItem(bird);
    // 设置难度参数
    switch (currentDifficulty) {
    case Easy:
        bird->gravity = 0.5;
        break;
    case Normal:
        bird->gravity = 0.98;
        break;
    case Hard:
        bird->gravity = 1.3;
        break;
    case Expert:
        bird->gravity = 1.6;
        break;
    case Master:
        bird->gravity = 2.0;
        break;
    case Insane:
        bird->gravity = 2.5;
        break;
    }
    // 重新设置背景
    scene->setBackgroundBrush(QBrush(QImage(":/assets/images/background-day.png").scaled(400, 600)));
    timer->stop();
    timer->start(20);
}

void Game::setDifficulty(Difficulty diff) {
    currentDifficulty = diff;
}

Game::Difficulty Game::getDifficulty() const {
    return currentDifficulty;
}

void Game::keyPressEvent(QKeyEvent* event) {
    if (isMenuVisible) return; // 菜单显示时不响应游戏按键
    if (event->key() == Qt::Key_Space) {
        if (isGameOver) {
            restartGame();
        } else {
            bird->flap();
        }
    }
}

void Game::restartGame()
{
	// 清除场景中的管道和文本
	for (Pipe* pipe : pipes) {
		scene->removeItem(pipe);
		delete pipe;
	}
	pipes.clear();

	// bird只new一次，重置状态和位置
    bird->setPos(100, 300);
    bird->reset();

	// 重置分数
	score = 0;
	scoreText->setPlainText(QString("Score: %1").arg(score));

	// 移除 Game Over 画面
	QList<QGraphicsItem*> items = scene->items();
	for (QGraphicsItem* item : items) {
		if (QGraphicsPixmapItem* pixmapItem = dynamic_cast<QGraphicsPixmapItem*>(item)) 
		{
			if (pixmapItem->pixmap().cacheKey() == QPixmap(":/assets/images/gameover.png").cacheKey()) 
			{
				scene->removeItem(pixmapItem);
				delete pixmapItem;
			}
		}
		if (QGraphicsTextItem* textItem = dynamic_cast<QGraphicsTextItem*>(item)) {
			if (textItem->toPlainText() == "按空格键重新开始") {
				scene->removeItem(textItem);
				delete textItem;
			}
		}	
	}

	// 重置游戏状态
	isGameOver = false;
	timer->stop();
    timer->start(20);
}

void Game::gameLoop() {
	bird->updatePosition();

	// 生成新的管道
	if (pipes.isEmpty() || pipes.last()->x() < 200) {
		Pipe* pipe = new Pipe();
		pipes.append(pipe);
		scene->addItem(pipe);
	}

	// 管道移动与检测碰撞
	auto it = pipes.begin();
	while (it != pipes.end()) {
		Pipe* pipe = *it;
		pipe->movePipe();

		// 检测与小鸟的碰撞
		if (bird->collidesWithItem(pipe)) {
			timer->stop();
			QGraphicsPixmapItem* gameOverItem = scene->addPixmap(QPixmap(":/assets/images/gameover.png"));
			// 将 Game Over 画面放在中间位置
			gameOverItem->setPos(this->width() / 2 - gameOverItem->pixmap().width() / 2, this->height() / 2 - gameOverItem->pixmap().height() / 2);
			isGameOver = true;
            //提示按空格重新游戏，用QGraphicsTextItem
            QGraphicsTextItem* restartText = new QGraphicsTextItem("按空格键重新开始");
			restartText->setDefaultTextColor(Qt::black);
			restartText->setFont(QFont("Arial", 12, QFont::Bold));
			//放在中间
			restartText->setPos(this->width() / 2 - restartText->boundingRect().width() / 2, this->height() / 2 + gameOverItem->pixmap().height() / 2 + 10);
           
            scene->addItem(restartText);
			return;
		}

		// 如果小鸟通过了管道（即小鸟的x坐标刚好超过管道的x坐标）
		if (pipe->x() + pipe->boundingRect().width() < bird->x() && !pipe->isPassed) {
			// 增加分数
			score++;
			pipe->isPassed = true;  // 确保不会重复加分

			// 更新分数显示
			scoreText->setPlainText(QString("Score: %1").arg(score));
		}

		// 如果管道移出了屏幕，将其从场景和列表中删除
		if (pipe->x() < -60) {
			scene->removeItem(pipe);
			delete pipe;
			it = pipes.erase(it);  // 从列表中安全移除管道
		}
		else {
			++it;  // 继续遍历
		}
	}
}
