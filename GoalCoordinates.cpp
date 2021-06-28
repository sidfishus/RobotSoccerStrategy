/*
 * RobotSoccerStrategy_01
 * Robot Soccer strategy
 * Chris Siddall
 */

#include "GoalCoordinates.h"
#include "RobotSoccerStrategy.h"


//

const double GoalCoordinates::TOP = 47.5;

const double GoalCoordinates::BOTTOM = 35.0;


//

GoalCoordinates::GoalCoordinates( const char orientation )
: X( orientation == RobotSoccerStrategy::GOALORIENTATION_LEFT ? 8.5 : 91.5 ), ORIENTATION( orientation )
{
}
