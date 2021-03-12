#pragma once
#include <QWidget>
#include <QTimerEvent>
#include <QMouseEvent>
#include "gameenvironment.h"

class GameWindow : public QWidget
{
	Q_OBJECT
public:
	GameWindow();
	~GameWindow();

	QSize minimumSizeHint() const;
protected:
	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void keyPressEvent(QKeyEvent *event);
	void paintEvent(QPaintEvent* event);
	void timerEvent(QTimerEvent* event);
private slots:
	void startGame();
	void restartGame();
private:
	GameEnvironment::Game* game;
	bool gameStarted = false;
	int timerId;

	int logicalSquareSide;
	int side;

	const double cursorWidth = 10;
	const double cursorHeight = 18;
};