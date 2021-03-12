#pragma once
#include <QPixmap>
#include <QPointF>
#include <QColor>
#include <QThread>
#include <QMutex>
#include <QTime>
#include <QVector>
#include <QReadWriteLock>
#include <QPainter>
#include <QWaitCondition>

namespace GameEnvironment
{


	class Circle;
	struct Ring;
	struct Arc;

	struct GameSettings
	{
		QVector<Ring> rings;

		QColor goodColor;
		QVector<QColor> ringColors;
		QColor selectedRingBackgroundColor;
		QColor energyCircutColor;
		QColor energyColor;
		QColor freezeColor;

		double ringSelectingSpeed;//from 0 to 1 per sec
		double goodSpreadingSpeed;//+part of window corner distance
		double goodClearingSpeed;//-part of core radius
		double energyRegenirationSpeed;//per sec
		double freezeRegenirationSpeed;
		double energyVolume;//in secs
		double freezeVolume;

	};

	struct GameResources
	{
		QRadialGradient* fromGoodToEvilGradient;
		double currentEvilColorAt;//from 0 to 1
		QVector<QBrush*> ringBrushes;
		QBrush* goodBrush;
		QPen* circutPen;
		QBrush* energyBrush;
		QBrush* freezeBrush;
	};

	class Game : public QThread
	{
		Q_OBJECT
	public:
		Game(GameSettings s);
		~Game();
		void setMouseRect(QRectF rect);
		void startRingDragging();
		void stopRingDragging();
		void freeze();
		void unfreeze();
		void draw(double cornerDist, QPainter &painter);
		void drawUI(double width, double height, QPainter &painter);
		void stopExecution();

		static double atan4(double y, double x);
	signals:
		void Start();
		void GameWon();
		void GameOver();
	protected:
		void run();
	private:
		int getDeltaTime(QTime &timer);
		void addPointIfInsideIntersectedArea(double x, double y, double sqrInternalRadius, double sqrExternalRadius, QVector<QPointF> &arcIntersectedPoints);
		void find_and_add_pointsThatIntersectsRadiusInArea_X(double x, double yMin, double yMax, double sqrRadius, QVector<QPointF> &arcIntersectedPoints);
		void find_and_add_pointsThatIntersectsRadiusInArea_Y(double y, double xMin, double xMax, double sqrRadius, QVector<QPointF> &arcIntersectedPoints);
		void drawCore(double cornerDist, QPainter &painter);

		GameSettings settings;
		GameResources resources;

		Circle *gameCircle;
		QRectF lastMouseRect;
		QRectF mouseRect;
		QMutex circleMutex;
		QMutex inputMutex;
		QVector<QPointF> arcIntersectedPoints;

		double currentCoreWidthScore = 0;

		const double goodColorBefore = 0.3;
		const double evilColorAt = 0.6;

		const int indicatorsMargin = 5;
		const int indicatorsDiameter = 45;
		const double indicatorsStartQuarter = 1;

		double currentEnergyVolume;
		double energyStartTime;
		double lastEnergyVolume;

		double currentFreezeVolume;
		double freezeStartTime;
		double lastFreezeVolume;

		bool leftMButtonPressed = false;
		bool rightMButtonPressed = false;

		double gameFinishedTime;
		bool gameFinished = false;
		bool gameWon = false;

		bool executing = true;
	};

	class Circle
	{
	public:
		Circle(double coreWidth);
		void addRing(Ring ring);
		void moveRing(int index, double rotation);
		void setIsRingSelected(int index, bool isSelected);
		void setRingSelectionStartDeltaTime(int index, double deltaTime);
		void setRingSelectionScore(int index, double score);
		void addRingRotation(int index, double additionalRotation);
		void setIsRingRotating(int index, bool isRotating);
		Ring operator[](int i) const;
		Ring at(int i) const;
		int count();
		int totalRadius() const;

		static double adjustAngle(double angle);

	private:
		QVector<Ring> rings;
		int radius = 0;
	};

	struct  Ring
	{
		Ring(int width = 25, double angleSpeed = 0, double rotation = 0, double additionalRotation = 0)
			: width(width), angleSpeed(angleSpeed), rotation(rotation), additionalRotation(additionalRotation) {};

		double angleSpeed;
		double rotation;
		double selectionStartTime = 0;
		QVector<Arc> arcs;
		bool isRotating = false;
		bool isSelected = false;
		int width;
		int internalRadius = 0;
		double selectedScore = 0;//from 0 to 1
		double lastSelectionScore = 0;
		double additionalRotation = 0;

	};

	struct Arc
	{
		double position;//angle of the top-left end of the arc from center
		double length; //part of rings's lenght from 0 to 1
	};
}