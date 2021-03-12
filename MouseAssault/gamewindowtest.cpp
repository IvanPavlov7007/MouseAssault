#include "gamewindowtest.h"
#include <QPainter>
#include <QPalette>
#include <QtMath>
#include <QApplication>
#include <QScreen>

using namespace GameEnvironment;

GameWindow::GameWindow() : QWidget()
{
	setMinimumSize(minimumSizeHint());
	//test
	GameSettings testSettings;
	testSettings.rings.append(Ring(40));

	Ring ring(30, M_PI / 2);
	ring.arcs.append({ 0, 0.3 });
	ring.arcs.append({ 0.6, 0.2 });
	testSettings.rings.append(ring);

	ring.arcs.clear();
	ring.width = 35;
	ring.angleSpeed = -M_PI / 4.0;
	ring.additionalRotation = M_PI / 4;
	ring.arcs.append({ 0, 0.8 });
	testSettings.rings.append(ring);

	ring.arcs.clear();
	ring.width = 40;
	ring.angleSpeed = -M_PI / 2;
	ring.additionalRotation = 0;
	ring.arcs.append({ 0.4,0.1 });
	ring.arcs.append({ 0.6,0.3 });
	testSettings.rings.append(ring);

	ring.arcs.clear();
	ring.width = 35;
	ring.angleSpeed = 0;
	ring.arcs.append({ 0.1,0.4 });
	ring.arcs.append({ 0.6,0.4 });
	testSettings.rings.append(ring);

	ring.arcs.clear();
	ring.width = 40;
	ring.angleSpeed = M_PI / 6;
	ring.additionalRotation = 0.1 * M_PI;
	ring.arcs.append({ 0,0.1 });
	ring.arcs.append({ 0.3,0.05 });
	ring.arcs.append({ 0.39,0.1 });
	ring.arcs.append({ 0.6,0.05 });
	ring.arcs.append({ 0.7,0.01 });
	ring.arcs.append({ 0.78,0.2 });
	testSettings.rings.append(ring);

	ring.arcs.clear();
	ring.width = 50;
	ring.angleSpeed = -M_PI /2;
	ring.additionalRotation = M_PI;
	ring.arcs.append({ 0,0.3 });
	ring.arcs.append({ 0.5,0.2 });
	ring.arcs.append({ 0.8,0.1 });
	testSettings.rings.append(ring);

	testSettings.ringColors.append(Qt::blue);
	testSettings.ringColors.append(Qt::yellow);
	testSettings.ringColors.append(Qt::red);
	testSettings.ringColors.append(Qt::GlobalColor::red);
	testSettings.ringColors.append(Qt::green);
	testSettings.ringColors.append(Qt::blue);

	testSettings.energyCircutColor = Qt::transparent;
	testSettings.energyColor = Qt::blue;
	testSettings.freezeColor = Qt::red;
	testSettings.energyRegenirationSpeed = 0.3;
	testSettings.freezeRegenirationSpeed = 0.3;
	testSettings.energyVolume = 4;
	testSettings.freezeVolume = 5;
	testSettings.goodClearingSpeed = -3;
	testSettings.goodSpreadingSpeed = 3;
	testSettings.goodColor = Qt::white;
	testSettings.ringSelectingSpeed = 1.5;
	testSettings.selectedRingBackgroundColor = Qt::gray;


	//setting backGround
	QPalette myPalette = palette();
	myPalette.setColor(QPalette::ColorRole::Background, Qt::black);
	setPalette(myPalette);


	game = new Game(testSettings);
	connect(game, &Game::Start, this, &GameWindow::startGame);
	connect(game, &QThread::finished, this, &GameWindow::restartGame);
	setMouseTracking(true);
	game->start();
}

GameWindow::~GameWindow()
{
	delete game;
}

QSize GameWindow::minimumSizeHint() const
{
	return QSize(800, 600);
}

void GameWindow::mouseMoveEvent(QMouseEvent * event)
{
	if (gameStarted)
	{
		double dSide = static_cast<double>(side);
		double k = dSide / static_cast<double>(logicalSquareSide);
		double a = event->x();
		double b = event->y();
		double curX = (static_cast<double>(event->x()) - (width() - side) / 2 - side / 2) / k;
		double curY = (static_cast<double>(event->y()) - (height() - side) / 2 - side / 2) / k;
		double curWidth = cursorWidth / k;
		double curHeight = cursorHeight / k;

		game->setMouseRect(QRectF(curX, curY, curWidth, curHeight));
	}
}

void GameWindow::mousePressEvent(QMouseEvent * event)
{
	if (gameStarted)
	{
		int button = event->button();
		if (button == Qt::LeftButton)
			game->startRingDragging();
		else if (button == Qt::RightButton)
			game->freeze();

	}
}

void GameWindow::mouseReleaseEvent(QMouseEvent * event)
{
	if (gameStarted)
	{
		int button = event->button();
		if (button == Qt::LeftButton)
			game->stopRingDragging();
		else if (button == Qt::RightButton)
			game->unfreeze();
	}
}
void GameWindow::keyPressEvent(QKeyEvent * event)
{
	if (gameStarted)
	{
		switch (event->key())
		{
		case Qt::Key_E:
		{
			qApp->quit();
			break;
		}
		case Qt::Key_R:
			game->stopExecution();
		}
	}
	QWidget::keyPressEvent(event);
}
void GameWindow::paintEvent(QPaintEvent * event)
{
	if (gameStarted)
	{
		QPainter painter(this);
		painter.setRenderHint(QPainter::Antialiasing, true);
		painter.setPen(Qt::PenStyle::NoPen);

		game->drawUI(width(), height(), painter);

		side = qMin(width(), height());
		logicalSquareSide = side;
		painter.setViewport((width() - side) / 2, (height() - side) / 2, side, side);
		painter.setWindow(-logicalSquareSide / 2, -logicalSquareSide / 2, logicalSquareSide, logicalSquareSide);
		painter.setPen(Qt::PenStyle::NoPen);
		game->draw(sqrt(pow(width(), 2.0) + pow(height(), 2.0)) / 2, painter);
	}
	else
		QWidget::paintEvent(event);
}

void GameWindow::timerEvent(QTimerEvent * event)
{
	if (event->timerId() == timerId)
		update();
}

void GameWindow::restartGame()
{
	killTimer(timerId);
	gameStarted = false;
	game->start();
}

void GameWindow::startGame()
{
	timerId = startTimer(15);
	gameStarted = true;
	QCursor::setPos(mapToGlobal(QPoint(0,0)));
}