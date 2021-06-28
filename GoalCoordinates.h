/*
 * RobotSoccerStrategy_01
 * Robot Soccer strategy
 * Chris Siddall
 */

#ifndef GOALCOORDINATES_H
#define GOALCOORDINATES_H


//

class GoalCoordinates
{

public:

	static const double TOP;
	static const double BOTTOM;

	const double X;
	const char ORIENTATION;

	GoalCoordinates( const char orientation );
};


//

#endif // GOALCOORDINATES_H