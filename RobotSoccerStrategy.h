/*
 * RobotSoccerStrategy_01
 * Robot Soccer strategy
 * Chris Siddall
 */

#ifndef ROBOTSOCCERSTRATEGY_H
#define ROBOTSOCCERSTRATEGY_H


//

#include "..\StrategyShell\RobotSoccer.h"
#include "GoalCoordinates.h"


//

class RobotSoccerStrategy : public RobotSoccer
{

public:

	static const char GOALORIENTATION_LEFT;
	static const char GOALORIENTATION_RIGHT;
	
	RobotSoccerStrategy();
	~RobotSoccerStrategy();

private:

	const GoalCoordinates *HOMEGOAL;
	const GoalCoordinates *OPPGOAL;

	void initGameVars();

	static void makeQuadAngle( double * const angle );

	virtual void onStrategy();

	void defendGoal();
	void defend();
	void attack();
};


//

#endif // ROBOTSOCCERSTRATEGY_H