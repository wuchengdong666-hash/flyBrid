#ifndef GAME_H
#define GAME_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QTimer>
#include "bird.h"
#include "pipe.h"

class Game : public QGraphicsView {
	Q_OBJECT
public:
	Game(QWidget* parent = nullptr);
	void keyPressEvent(QKeyEvent* event);
	void restartGame();

	// 难度模式枚举
	enum Difficulty {
		Easy,
		Normal,
		Hard,
		Expert,
		Master,
		Insane
	};

	// 设置难度的方法
	void setDifficulty(Difficulty diff);
	Difficulty getDifficulty() const;

	// 显示菜单的方法声明
	void showMenu();

	// 启动游戏（根据难度）
	void startGameWithDifficulty();

private slots:
	void gameLoop();

private:
	QGraphicsScene* scene;
	QGraphicsTextItem* scoreText;
	Bird* bird; // 只在构造函数new一次，后续只reset
	QTimer* timer; // 只在构造函数new一次，后续只stop/start
	QList<Pipe*> pipes;
	int score;
	bool isGameOver;

	// 当前难度
	Difficulty currentDifficulty;

	// 菜单是否显示
	bool isMenuVisible;
};

#endif // GAME_H
