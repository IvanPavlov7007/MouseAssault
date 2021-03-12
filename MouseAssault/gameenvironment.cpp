#include "gameenvironment.h"
#include <QtMath>
#include <iostream>

using namespace GameEnvironment;


Game::Game(GameSettings s)
{
	settings = s;
	
	gameCircle = new Circle(settings.rings[0].width);

	for (int i = 1; i < settings.rings.count(); i++)
	{
		gameCircle->addRing(settings.rings[i]);
	}

	resources.goodBrush = new QBrush(settings.goodColor);
	resources.fromGoodToEvilGradient = new QRadialGradient(0, 0, gameCircle->at(0).width);
	resources.fromGoodToEvilGradient->setColorAt(0, settings.goodColor);
	resources.fromGoodToEvilGradient->setColorAt(goodColorBefore, settings.goodColor);
	resources.fromGoodToEvilGradient->setColorAt(evilColorAt, Qt::transparent);

	for (int i = 0; i < settings.ringColors.count(); i++)
		resources.ringBrushes.append(new QBrush(settings.ringColors[i]));

	resources.circutPen = new QPen(settings.energyCircutColor);
	resources.energyBrush = new QBrush(settings.energyColor);
	resources.freezeBrush = new QBrush(settings.freezeColor);
}

Game::~Game()
{
	delete gameCircle;
	delete resources.goodBrush;
	delete resources.fromGoodToEvilGradient;
	delete resources.circutPen;
	delete resources.energyBrush;
	delete resources.freezeBrush;
	qDeleteAll(resources.ringBrushes);

}

void Game::setMouseRect(QRectF rect)
{
	QMutexLocker locker(&inputMutex);
	mouseRect = rect;
}

void Game::startRingDragging()
{
	QMutexLocker locker(&inputMutex);
	leftMButtonPressed = true;
}

void Game::stopRingDragging()
{
	QMutexLocker locker(&inputMutex);
	leftMButtonPressed = false;
}

void Game::freeze()
{
	QMutexLocker locker(&inputMutex);
	rightMButtonPressed = true;
}

void Game::unfreeze()
{
	QMutexLocker locker(&inputMutex);
	rightMButtonPressed = false;
}

void Game::draw(double cornerDist,QPainter & painter)
{

	circleMutex.lock();
	int radius = gameCircle->totalRadius();
	for (int i = gameCircle->count() - 1; i > 0; i--)
	{
		Ring ring = gameCircle->at(i);
		circleMutex.unlock();
		double ringRotation = ring.rotation + ring.additionalRotation;
		if (ring.selectedScore > 0)
		{
			QColor selectionColor = settings.selectedRingBackgroundColor;
			selectionColor.setAlphaF(ring.selectedScore);
			painter.setBrush(selectionColor);
			painter.drawEllipse(-radius,
				-radius,
				radius * 2,
				radius * 2);
		}
		for (int j = 0; j < ring.arcs.length(); j++)
		{
			Arc arc = ring.arcs[j];
			double nextRadius = radius - ring.width;


			painter.setBrush(*resources.ringBrushes[i - 1]);
			painter.drawPie(-radius,
				-radius,
				radius *2,
				radius *2,
				qRadiansToDegrees(ringRotation + arc.position * M_PI * 2) * 16,
				qRadiansToDegrees(2 * M_PI * arc.length) * 16
			);
			painter.setBrush(QBrush(painter.background()));
			painter.drawEllipse(-nextRadius, -nextRadius, nextRadius * 2, nextRadius * 2);
#ifdef QT_DEBUG
			painter.save();
			painter.setPen(QColor(Qt::yellow));
			painter.setBrush(Qt::BrushStyle::NoBrush);
			
			double s_ang = ringRotation + arc.position * M_PI * 2;
			double e_ang = s_ang + arc.length * 2.0 *M_PI;

			painter.drawLine(0, 0, 250.0 * cos(s_ang), 250.0 * -sin(s_ang));
			painter.drawLine(0, 0, 250.0 * cos(e_ang), 250.0 * -sin(e_ang));

			painter.restore();
#endif
		}
		radius -= ring.width;
		circleMutex.lock();
	}
	circleMutex.unlock();
	drawCore(cornerDist, painter);

#ifdef QT_DEBUG

	painter.setPen(QColor(Qt::yellow));
	painter.setBrush(Qt::BrushStyle::NoBrush);
	circleMutex.lock();
	painter.drawRect(mouseRect);
	circleMutex.unlock();

	painter.setPen(QColor(Qt::red));

	double angle;

	circleMutex.lock();

	for (int i = 0; i < arcIntersectedPoints.count(); i++)
	{
		radius = sqrt(pow(arcIntersectedPoints[i].x(), 2.0) + pow(arcIntersectedPoints[i].y(), 2.0));
		angle = atan2(arcIntersectedPoints[i].y(), arcIntersectedPoints[i].x());
		painter.drawLine(0, 0, radius*cos(angle), radius * sin(angle));
	}
	circleMutex.unlock();

#endif // QT_DEBUG
}

void Game::drawUI(double width, double height, QPainter & painter)
{
	painter.setPen(*resources.circutPen);

	circleMutex.lock();
	double freezeVol = currentFreezeVolume;
	double energyVol = currentEnergyVolume;
	circleMutex.unlock();

	painter.setBrush(*resources.energyBrush);

	if (energyVol < settings.energyVolume)
		painter.drawPie(width - 3 * indicatorsMargin - 2 * indicatorsDiameter, indicatorsMargin,
			indicatorsDiameter, indicatorsDiameter,
			qRadiansToDegrees(M_PI / 2 * indicatorsStartQuarter) * 16,
			qRadiansToDegrees(2 * M_PI * energyVol / settings.energyVolume) * 16);
	else
		painter.drawEllipse(width - 3 * indicatorsMargin - 2 * indicatorsDiameter, indicatorsMargin,
			indicatorsDiameter, indicatorsDiameter);
	
	painter.setBrush(*resources.freezeBrush);
	
	if (freezeVol < settings.freezeVolume)
		painter.drawPie(width - indicatorsMargin - indicatorsDiameter, indicatorsMargin,
			indicatorsDiameter, indicatorsDiameter,
			qRadiansToDegrees(M_PI / 2 * indicatorsStartQuarter) * 16,
			qRadiansToDegrees(2 * M_PI * freezeVol / settings.freezeVolume) * 16);
	else
		painter.drawEllipse(width - indicatorsMargin - indicatorsDiameter, indicatorsMargin,
			indicatorsDiameter, indicatorsDiameter);
}

void GameEnvironment::Game::stopExecution()
{
	QMutexLocker locker(&inputMutex);
	executing = false;
}

double Game::atan4(double y, double x)
{
	double ang = atan2(y, x);
	return ang > 0 ? ang : ang + 2 * M_PI;
}

void Game::run()
{
	QTime timer;
	double deltaTime = 0;
	double mouseAngleDifference;
	QRectF currentMouseRect;

	mouseRect.setRect(-INFINITY, -INFINITY, 0, 0);
	lastMouseRect = mouseRect;

	leftMButtonPressed = false;
	rightMButtonPressed = false;

	executing = true;
	gameFinished = false;

	for (int i = 0; i < gameCircle->count(); i++)
	{
		gameCircle->addRingRotation(i, gameCircle->at(i).additionalRotation);
		gameCircle->moveRing(i, 0);
		gameCircle->setIsRingSelected(i, false);
		gameCircle->setRingSelectionScore(i, 0);
		gameCircle->setIsRingRotating(i, false);
	}

	currentCoreWidthScore = 0;
	currentEnergyVolume = settings.energyVolume;
	currentFreezeVolume = settings.freezeVolume;

	bool rotating = false;
	bool frozen = false;
	double fullRotation;

	double xMin;
	double yMin;
	double xMax;
	double yMax;

	double xMinSqr;
	double xMaxSqr;
	double yMinSqr;
	double yMaxSqr;

	double sqrExternalRadius;
	double sqrInternalRadius;

	double startAngle;
	double endAngle;

	double pointAngle;

	bool emitted = false;

	emit Start();
	timer.start();

	forever
	{




		deltaTime = getDeltaTime(timer);
		inputMutex.lock();
		if (executing == false)
		{
			inputMutex.unlock();
			break;
		}

		if (lastMouseRect != mouseRect)
		{
			mouseAngleDifference = atan4(lastMouseRect.y(), lastMouseRect.x()) - atan4(mouseRect.y(), mouseRect.x());
			lastMouseRect = mouseRect;
			currentMouseRect = mouseRect;
		}
		else
			mouseAngleDifference = 0;

		if (rotating != leftMButtonPressed)
		{
			energyStartTime = deltaTime;
			lastEnergyVolume = currentEnergyVolume;
		}
		rotating = leftMButtonPressed;

		if (frozen != rightMButtonPressed)
		{
			freezeStartTime = deltaTime;
			lastFreezeVolume = currentFreezeVolume;
		}
		frozen = rightMButtonPressed;
		inputMutex.unlock();

		if (!gameFinished)
		{
			circleMutex.lock();
			if (rotating)
			{
				if (currentEnergyVolume >= 0)
					currentEnergyVolume = lastEnergyVolume - (deltaTime - energyStartTime) / double(1000);
				else
				{
					leftMButtonPressed = false;
					currentEnergyVolume = 0;
				}
			}
			else
			{
				if (currentEnergyVolume < settings.energyVolume)
				{
					currentEnergyVolume = lastEnergyVolume + settings.energyRegenirationSpeed *(deltaTime - energyStartTime) / double(1000);
				}
				else if (currentEnergyVolume != settings.energyVolume)
				{
					currentEnergyVolume = settings.energyVolume;
				}
			}

			if (frozen)
			{
				if (currentFreezeVolume >= 0)
					currentFreezeVolume = lastFreezeVolume - (deltaTime - freezeStartTime) / double(1000);
				else
				{
					rightMButtonPressed = false;
					currentFreezeVolume = 0;
				}
			}
			else
			{
				if (currentFreezeVolume < settings.freezeVolume)
				{
					currentFreezeVolume = lastFreezeVolume + settings.freezeRegenirationSpeed *(deltaTime - freezeStartTime) / double(1000);
				}
				else if (currentFreezeVolume != settings.freezeVolume)
				{
					currentFreezeVolume = settings.freezeVolume;
				}
			}
			circleMutex.unlock();
		}

		if (gameFinished)
		{
			QMutexLocker locker(&circleMutex);
			if ((currentCoreWidthScore > -1 & !gameWon) ||
				(currentCoreWidthScore < 1 & gameWon))
			{
				currentCoreWidthScore = (gameWon ? settings.goodSpreadingSpeed : settings.goodClearingSpeed) * (deltaTime - gameFinishedTime) / 1000.0;
			}
			else if(!emitted)
			{
				emit gameWon ? GameWon() : GameOver();
				emitted = true;
				//break;
			}
		}

		for (int i = 0; i < gameCircle->count(); i++)
		{
			circleMutex.lock();
			deltaTime = getDeltaTime(timer);
			Ring ring = gameCircle->at(i);
			circleMutex.unlock();
			if (i != 0)
			{
				circleMutex.lock();
				if (ring.isRotating || (frozen && !gameFinished))
					gameCircle->addRingRotation(i, ring.rotation - ring.angleSpeed * deltaTime / double(1000));
				gameCircle->moveRing(i, ring.angleSpeed * deltaTime / double(1000));
				ring = gameCircle->at(i);
				circleMutex.unlock();
				fullRotation = ring.additionalRotation + ring.rotation;

				if (ring.isRotating && !rotating)
				{
					circleMutex.lock();
					gameCircle->setIsRingRotating(i, false);
					circleMutex.unlock();
					ring.isRotating = false;
				}
			}

			xMin = currentMouseRect.x();
			yMin = currentMouseRect.y();
			xMax = xMin + currentMouseRect.width();
			yMax = yMin + currentMouseRect.height();

			sqrExternalRadius = pow(ring.internalRadius + ring.width,2.0);
			sqrInternalRadius = pow(ring.internalRadius,2.0);

			xMinSqr = xMin < 0 && xMax > 0 ? 0 : pow(qMin(abs(xMin), abs(xMax)), 2.0);
			xMaxSqr = pow(xMinSqr == 0? xMax : qMax(abs(xMin), abs(xMax)),2.0);
			yMinSqr = yMin < 0 && yMax > 0 ? 0 : pow(qMin(abs(yMin), abs(yMax)), 2.0);
			yMaxSqr = pow(yMinSqr == 0? yMax : qMax(abs(yMin), abs(yMax)), 2.0);

			
			if (xMinSqr + yMinSqr <= sqrExternalRadius && xMaxSqr + yMaxSqr >= sqrInternalRadius && !gameFinished)
			{
				if (i == 0)
				{
					gameFinished = true;
					gameWon = true;
					gameFinishedTime = deltaTime;
					continue;
				}


				circleMutex.lock();
				arcIntersectedPoints.clear();
				addPointIfInsideIntersectedArea(xMin, yMin, sqrInternalRadius, sqrExternalRadius, arcIntersectedPoints);
				addPointIfInsideIntersectedArea(xMin, yMax, sqrInternalRadius, sqrExternalRadius, arcIntersectedPoints);
				addPointIfInsideIntersectedArea(xMax, yMin, sqrInternalRadius, sqrExternalRadius, arcIntersectedPoints);
				addPointIfInsideIntersectedArea(xMax, yMax, sqrInternalRadius, sqrExternalRadius, arcIntersectedPoints);

				find_and_add_pointsThatIntersectsRadiusInArea_X(xMin, yMin, yMax, sqrExternalRadius, arcIntersectedPoints);
				find_and_add_pointsThatIntersectsRadiusInArea_X(xMax, yMin, yMax, sqrExternalRadius, arcIntersectedPoints);
				find_and_add_pointsThatIntersectsRadiusInArea_X(xMin, yMin, yMax, sqrInternalRadius, arcIntersectedPoints);
				find_and_add_pointsThatIntersectsRadiusInArea_X(xMax, yMin, yMax, sqrInternalRadius, arcIntersectedPoints);

				find_and_add_pointsThatIntersectsRadiusInArea_Y(yMin, xMin, xMax, sqrExternalRadius, arcIntersectedPoints);
				find_and_add_pointsThatIntersectsRadiusInArea_Y(yMax, xMin, xMax, sqrExternalRadius, arcIntersectedPoints);
				find_and_add_pointsThatIntersectsRadiusInArea_Y(yMin, xMin, xMax, sqrInternalRadius, arcIntersectedPoints);
				find_and_add_pointsThatIntersectsRadiusInArea_Y(yMax, xMin, xMax, sqrInternalRadius, arcIntersectedPoints);
				circleMutex.unlock();

				for (int j = 0; j < ring.arcs.count(); j++)
				{
					Arc arc = ring.arcs[j];
					startAngle = Circle::adjustAngle(fullRotation + arc.position * 2.0 * M_PI);
					if (startAngle < 0)
						startAngle += 2.0 * M_PI;
					endAngle = Circle::adjustAngle(fullRotation + (arc.position + arc.length)* 2.0 * M_PI);
					if (endAngle < 0)
						endAngle += 2.0 * M_PI;

					circleMutex.lock();
					for (int k = 0; k < arcIntersectedPoints.count(); k++)
					{
						pointAngle = atan4(-arcIntersectedPoints[k].y(), arcIntersectedPoints[k].x());
						if (startAngle < endAngle)
						{
							if (pointAngle > startAngle & pointAngle < endAngle)
							{
								gameFinished = true;
								gameWon = false;
								gameFinishedTime = deltaTime;
								continue;
							}
						}
						else
						{
							if ((pointAngle > startAngle) || (pointAngle > 0 & pointAngle < endAngle))
							{
								gameFinished = true;
								gameWon = false;
								gameFinishedTime = deltaTime;
								continue;
							}
						}
					}
					circleMutex.unlock();

				}

				if (!ring.isRotating && rotating)
				{
					circleMutex.lock();
					gameCircle->setIsRingRotating(i, true);
					circleMutex.unlock();
					ring.isRotating = true;
				}

				if (ring.isRotating)
				{
					circleMutex.lock();
					gameCircle->addRingRotation(i, mouseAngleDifference);
					circleMutex.unlock();
				}

				if (!ring.isSelected)
				{
					circleMutex.lock();
					gameCircle->setIsRingSelected(i, true);
					gameCircle->setRingSelectionStartDeltaTime(i, deltaTime);
					ring = gameCircle->at(i);
					circleMutex.unlock();
				}

				if (ring.selectedScore < 1)
				{
					QMutexLocker locker(&circleMutex);
					gameCircle->setRingSelectionScore(i, qMin(ring.lastSelectionScore + settings.ringSelectingSpeed * (deltaTime - ring.selectionStartTime) / double(1000), 1.0));
				}
			}
			else
			{
				if (ring.isRotating)
				{
					circleMutex.lock();
					gameCircle->setIsRingRotating(i, false);
					circleMutex.unlock();
					ring.isRotating = false;
				}
				if (ring.isSelected)
				{
					circleMutex.lock();
					gameCircle->setIsRingSelected(i, false);
					gameCircle->setRingSelectionStartDeltaTime(i, deltaTime);
					ring = gameCircle->at(i);
					circleMutex.unlock();
				}

				if (ring.selectedScore > 0)
				{
					QMutexLocker locker(&circleMutex);
					gameCircle->setRingSelectionScore(i, qMax(ring.lastSelectionScore - settings.ringSelectingSpeed * (deltaTime - ring.selectionStartTime) / double(1000), 0.0));
				}
			}
		}
		if(!gameFinished)
		std::cerr << currentEnergyVolume << std::endl;
	}
}



int Game::getDeltaTime(QTime & timer)
{
	return timer.elapsed();
}

void Game::addPointIfInsideIntersectedArea(double x, double y, double sqrInternalRadius, double sqrExternalRadius, QVector<QPointF> &arcIntersectedPoints)
{
	double sumOfSqr = pow(x, 2.0) + pow(y, 2.0);
	if (sumOfSqr <= sqrExternalRadius && sumOfSqr >= sqrInternalRadius)
		arcIntersectedPoints.append(QPointF(x, y));
}

void Game::find_and_add_pointsThatIntersectsRadiusInArea_X(double x, double yMin, double yMax, double sqrRadius, QVector<QPointF>& arcIntersectedPoints)
{
	double absY = sqrRadius - pow(x, 2.0);

	if (absY >= 0)
	{
		absY = sqrt(absY);
		if (yMax >= absY && yMin <= absY)
			arcIntersectedPoints.append(QPointF(x, absY));
		absY = -absY;
		if (yMax >= absY && yMin <= absY)
			arcIntersectedPoints.append(QPointF(x, absY));
	}
}

void Game::find_and_add_pointsThatIntersectsRadiusInArea_Y(double y, double xMin, double xMax, double sqrRadius, QVector<QPointF>& arcIntersectedPoints)
{

	double absX = sqrRadius - pow(y, 2.0);

	if (absX >= 0)
	{
		absX = sqrt(absX);
		if (xMax >= absX && xMin <= absX)
			arcIntersectedPoints.append(QPointF(absX, y));
		absX = -absX;
		if (xMax >= absX && xMin <= absX)
			arcIntersectedPoints.append(QPointF(absX, y));
	}
}

void Game::drawCore(double cornerDist, QPainter & painter)
{
	circleMutex.lock();
	int radius = gameCircle->at(0).width;
	int exRadius = radius + currentCoreWidthScore * (gameWon? cornerDist / 0.3 : radius);
	circleMutex.unlock();

	if (exRadius == radius)
	{
		painter.setBrush(*resources.fromGoodToEvilGradient);
		painter.drawEllipse(-radius, -radius, radius * 2, radius * 2);
	}
	else
	{
		exRadius *= goodColorBefore;
		
		painter.setBrush(*resources.goodBrush);
		painter.drawEllipse(-exRadius, -exRadius, exRadius * 2, exRadius * 2);
	}
}

Circle::Circle(double coreWidth)
{
	addRing(Ring(coreWidth));
}

void Circle::addRing(Ring ring)
{
	ring.internalRadius = radius;
	rings.append(ring);
	radius += ring.width;
}

void Circle::moveRing(int index, double ringRotation)
{
	rings[index].rotation = adjustAngle(ringRotation);
}

void Circle::setIsRingSelected(int index, bool isSelected)
{
	rings[index].isSelected = isSelected;
}

void Circle::setRingSelectionStartDeltaTime(int index, double lastDeltaTime)
{
	rings[index].lastSelectionScore = rings[index].selectedScore;
	rings[index].selectionStartTime = lastDeltaTime;
}

void Circle::setRingSelectionScore(int index, double score)
{
	rings[index].selectedScore = score;
}

void Circle::addRingRotation(int index, double additionalRotation)
{
	rings[index].additionalRotation += additionalRotation;
	rings[index].additionalRotation = adjustAngle(rings[index].additionalRotation);
}

void Circle::setIsRingRotating(int index, bool isRotating)
{
	rings[index].isRotating = isRotating;
}

Ring Circle::operator[](int i) const
{
	return rings[i];
}

Ring Circle::at(int i) const
{
	return rings.at(i);
}

int Circle::count()
{
	return rings.count();
}

int Circle::totalRadius() const
{
	return radius;
}

double Circle::adjustAngle(double angle)
{
	int k = angle / (2 * M_PI);
	if (k > 0 || k < 0)
	{
		angle -= k * 2 * M_PI;
	}
	return angle;
}
