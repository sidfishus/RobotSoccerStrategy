/*
 * RobotSoccerStrategy_01
 * Robot Soccer strategy
 * Chris Siddall
 */

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <math.h>

#include "RobotSoccerStrategy.h"


//

const char RobotSoccerStrategy::GOALORIENTATION_LEFT = 0;

const char RobotSoccerStrategy::GOALORIENTATION_RIGHT = 1;


//

RobotSoccerStrategy::RobotSoccerStrategy()
: HOMEGOAL( NULL ), OPPGOAL( NULL )
{
}

RobotSoccerStrategy::~RobotSoccerStrategy()
{
	if ( HOMEGOAL )
		delete (GoalCoordinates *) HOMEGOAL;

	if ( OPPGOAL )
		delete (GoalCoordinates *) OPPGOAL;
}


//

void RobotSoccerStrategy::initGameVars()
{
	if ( !HOMEGOAL && !OPPGOAL )
	{
		char homeGoalOrientation;
		char oppGoalOrientation;

		if ( env->home[0].pos.x <= 50 )
		{
			homeGoalOrientation = GOALORIENTATION_LEFT;
			oppGoalOrientation = GOALORIENTATION_RIGHT;
		}
		else
		{
			homeGoalOrientation = GOALORIENTATION_RIGHT;
			oppGoalOrientation = GOALORIENTATION_LEFT;
		}

		HOMEGOAL = new GoalCoordinates( homeGoalOrientation );
		OPPGOAL = new GoalCoordinates( oppGoalOrientation );
	}
}

void RobotSoccerStrategy::makeQuadAngle( double * const angle )
{
	if ( *angle < 0.0 )
		if ( *angle < -90.0 )
			*angle = 180.0;
		else
			*angle = 0.0;
}


//

void RobotSoccerStrategy::onStrategy()
{
	initGameVars();

	defendGoal();

	defend();

	attack();
}


//

void RobotSoccerStrategy::defendGoal()
{
	// version 3.1

	// defend goal area & don't go past goal bounds

	Robot * const goalie = &env->home[0];
	const double robotY = goalie->pos.y;
	const double predictedY = predictedBall.getY();

	// if goalie is above the ball and the bottom of the goal

	if ( robotY > predictedY && robotY > HOMEGOAL->BOTTOM )

		// follow the ball & don't go past bottom of goal

		move( goalie, HOMEGOAL->X, predictedY < HOMEGOAL->BOTTOM ? HOMEGOAL->BOTTOM : predictedY );

	// if goalie is below the ball and the top of the goal

	else if ( robotY < predictedY && robotY < HOMEGOAL->TOP )

		// follow the ball & don't go past top of goal

		move( goalie, HOMEGOAL->X, predictedY > HOMEGOAL->TOP ? HOMEGOAL->TOP : predictedY );

	// if goalie is above goal y bounds

	else if ( robotY - HOMEGOAL->TOP > 2.5 )

		// move back to the top of the goal

		move( goalie, HOMEGOAL->X, HOMEGOAL->TOP );

	// if goalie is below goal y bounds

	else if ( HOMEGOAL->BOTTOM - robotY > 2.5 )

		// move back to the bottom of the goal

		move( goalie, HOMEGOAL->X, HOMEGOAL->BOTTOM );

	else
	{
		// if the previous conditions are not true, one of these MUST be triggered

		const double robotX = goalie->pos.x;

		// if goalie is out of x bounds

		if ( ( robotX > HOMEGOAL->X ? robotX - HOMEGOAL->X : HOMEGOAL->X - robotX ) > 1.5 )

			// move back to home goal x

			move( goalie, HOMEGOAL->X, goalie->pos.y );

		else

			// stay right where you are

			stopMoving( goalie );
	}
}

void RobotSoccerStrategy::defend()
{
	// version 2.0

	Robot * const defender = &env->home[1];

	const double goalY = OPPGOAL->BOTTOM + ( OPPGOAL->TOP - OPPGOAL->BOTTOM ) / 2;

	const double bottomMax = OPPGOAL->BOTTOM - 18.0;
	const double topMax = OPPGOAL->TOP + 18.0;

	// if home goal is on the left

	if ( HOMEGOAL->ORIENTATION == GOALORIENTATION_LEFT )
	{ 
		const double xDiff = predictedBall.getX() - HOMEGOAL->X;
		const double yDiff = goalY - predictedBall.getY();

		double moveX = HOMEGOAL->X + xDiff / 180 * 100;
		double moveY = goalY - yDiff / 180 * 100;

		if ( moveX < HOMEGOAL->X + 8.0 )
		{
			const double bottomMin = OPPGOAL->BOTTOM - 8.0;
			
			if ( moveY > bottomMin )
			{
				const double topMin = OPPGOAL->TOP + 8.0;

				if ( moveY < topMin )
					moveY = moveY > goalY ? topMin : bottomMin;
			}
			else if ( moveY < bottomMax )
				moveY = bottomMax;
			else if ( moveY > topMax )
				moveY = topMax;
		}
		else
		{
			const double xMax = HOMEGOAL->X + 20.0;

			if ( moveX > xMax )
				moveX = xMax;
			
			if ( moveY < bottomMax )
				moveY = bottomMax;
			else if ( moveY > topMax )
				moveY = topMax;
		}

		move( defender, moveX, moveY );
	}
	else
	{
		// home goal is on the right

		const double xDiff = HOMEGOAL->X - predictedBall.getX();
		const double yDiff = goalY - predictedBall.getY();

		double moveX = HOMEGOAL->X - xDiff / 180 * 100;
		double moveY = goalY - yDiff / 180 * 100;

		if ( moveX > HOMEGOAL->X - 8.0 )
		{
			const double bottomMin = OPPGOAL->BOTTOM - 8.0;
			
			if ( moveY > bottomMin )
			{
				const double topMin = OPPGOAL->TOP + 8.0;

				if ( moveY < topMin )
					moveY = moveY > goalY ? topMin : bottomMin;
			}
			else if ( moveY < bottomMax )
				moveY = bottomMax;
			else if ( moveY > topMax )
				moveY = topMax;
		}
		else
		{
			const double xMax = HOMEGOAL->X - 20.0;

			if ( moveX < xMax )
				moveX = xMax;
			
			if ( moveY < bottomMax )
				moveY = bottomMax;
			else if ( moveY > topMax )
				moveY = topMax;
		}

		move( defender, moveX, moveY );
	}
}

void RobotSoccerStrategy::attack()
{
	// version 3.2

	// previous version was getting too close (make sure it never gets nearer than 5)

	Robot * const attacker = &env->home[2];

	// if opposition goal is on the left

	if ( OPPGOAL->ORIENTATION == GOALORIENTATION_LEFT )
	{
		// calc angles of goal in relation to ball and angle of ball in relation to robot

		double topGoalAngle = arcTanInDegrees( predictedBall.getX() - OPPGOAL->X, predictedBall.getY() - HOMEGOAL->TOP );
		double bottomGoalAngle =  arcTanInDegrees( predictedBall.getX() - OPPGOAL->X, predictedBall.getY() - HOMEGOAL->BOTTOM );
		const double ballAngle = arcTanInDegrees( attacker->pos.x - predictedBall.getX(), attacker->pos.y - predictedBall.getY() );

		// make sure angles are between 0 and 180 degrees

		makeQuadAngle( &topGoalAngle );
		makeQuadAngle( &bottomGoalAngle );

		// if robot is behind the ball and is in line with the goal to ball angle 

		if ( attacker->pos.x > predictedBall.getX() && ballAngle > bottomGoalAngle && ballAngle < topGoalAngle )
			// move to ball and hopefully score!

			move( attacker, predictedBall.getX(), predictedBall.getY() );
		else
		{
			// move to intercept ball and get in correct line with the goal

			double nearY = ( bottomGoalAngle + ( ( topGoalAngle - bottomGoalAngle ) / 2 ) ) / 180.0 * -25.0 + 12.5;
			const double nearX = predictedBall.getX() + ( nearY < 0.0 ? nearY + 12.5 : 12.5 - nearY );
			double distance;

			nearY += predictedBall.getY();

			{
				const double yDiff = attacker->pos.y > nearY ? attacker->pos.y - nearY : nearY - attacker->pos.y;
				const double xDiff = attacker->pos.x > nearX ? attacker->pos.x - nearX : nearX - attacker->pos.x;

				distance = sqrt( xDiff * xDiff + yDiff * yDiff );
			}

			move( attacker, nearX + ( nearX - OPPGOAL->X ) / 150 * distance, nearY + ( nearY - ( OPPGOAL->BOTTOM + ( OPPGOAL->TOP - OPPGOAL->BOTTOM ) / 2 ) ) / 150 * distance );
		}
	}
	else
	{
		// opposition goal is on the right

		// calc angles of goal in relation to ball and angle of ball in relation to robot

		double topGoalAngle = arcTanInDegrees( OPPGOAL->X - predictedBall.getX(), predictedBall.getY() - HOMEGOAL->TOP );
		double bottomGoalAngle = arcTanInDegrees( OPPGOAL->X - predictedBall.getX(), predictedBall.getY() - HOMEGOAL->BOTTOM );
		const double ballAngle = arcTanInDegrees( predictedBall.getX() - attacker->pos.x, attacker->pos.y - predictedBall.getY() );

		// make sure angles are between 0 and 180 degrees

		makeQuadAngle( &topGoalAngle );
		makeQuadAngle( &bottomGoalAngle );

		// if robot is behind the ball and is in line with the goal to ball angle

		if ( attacker->pos.x < predictedBall.getX() && ballAngle > bottomGoalAngle && ballAngle < topGoalAngle )
			// move to ball and hopefully score!

			move( attacker, predictedBall.getX(), predictedBall.getY() );
		else
		{
			// move to intercept ball

			double nearY = ( bottomGoalAngle + ( ( topGoalAngle - bottomGoalAngle ) / 2 ) ) / 180.0 * -25.0 + 12.5;
			const double nearX = predictedBall.getX() - ( nearY < 0.0 ? nearY + 12.5 : 12.5 - nearY );
			double distance;

			nearY += predictedBall.getY();

			{
				const double yDiff = attacker->pos.y > nearY ? attacker->pos.y - nearY : nearY - attacker->pos.y;
				const double xDiff = attacker->pos.x > nearX ? attacker->pos.x - nearX : nearX - attacker->pos.x;

				distance = sqrt( xDiff * xDiff + yDiff * yDiff );
			}

			move( attacker, nearX - ( OPPGOAL->X - nearX ) / 150 * distance, nearY + ( nearY - ( OPPGOAL->BOTTOM + ( OPPGOAL->TOP - OPPGOAL->BOTTOM ) / 2 ) ) / 150 * distance );
		}
	}
}

/*void RobotSoccerStrategy::defendGoal()
{
	// version 2.0

	Robot * const goalie = &env->home[0];
	const double robotY = goalie->pos.y;
	const double predictedY = predictedBall.getY();

	if ( !homeGoalX ) // set x coord of goal depending on which team
		homeGoalX = new double( env->home[0].pos.x >= 50 ? 91.0 : 9.0 );

	if ( robotY > predictedY )
	{
		if ( robotY > HOMEGOAL->BOTTOM )
			move( goalie, HOMEGOAL->X, predictedY < HOMEGOAL->BOTTOM ? HOMEGOAL->BOTTOM : predictedY );
		
		else
			stopMoving( goalie );
	}

	else if ( robotY < HOMEGOAL->TOP )
		move( goalie, HOMEGOAL->X, predictedY > HOMEGOAL->TOP ? HOMEGOAL->TOP : predictedY );

	else
		stopMoving( goalie );
}

void RobotSoccerStrategy::defendGoal()
{
	// version 3.0

	Robot * const goalie = &env->home[0];
	const double robotY = goalie->pos.y;
	const double predictedY = predictedBall.getY();

	if ( !homeGoalX ) // set x coord of goal depending on which team
		homeGoalX = new double( env->home[0].pos.x >= 50 ? 91.5 : 8.5 );

	// defend goal area

	if ( robotY > predictedY && robotY > HOMEGOAL->BOTTOM )
		move( goalie, HOMEGOAL->X, predictedY < HOMEGOAL->BOTTOM ? HOMEGOAL->BOTTOM : predictedY );

	else if ( robotY < predictedY && robotY < HOMEGOAL->TOP )
		move( goalie, HOMEGOAL->X, predictedY > HOMEGOAL->TOP ? HOMEGOAL->TOP : predictedY );

	else
	{
		double robotX = goalie->pos.x;

		if ( ( robotX > HOMEGOAL->X ? robotX - HOMEGOAL->X : HOMEGOAL->X - robotX ) > 0.8 )
			move( goalie, HOMEGOAL->X, goalie->pos.y );

		else
			stopMoving( goalie );
	}
}

void RobotSoccerStrategy::defend()
{
	// version 1.0

	Robot * const defender = &env->home[1];

	// if home goal is on the left

	if ( HOMEGOAL->ORIENTATION == GOALORIENTATION_LEFT )
	{
		// calc angle of ball in relation to goal

		/*double ballAngle = arcTanInDegrees( predictedBall.getX() - HOMEGOAL->X, OPPGOAL->BOTTOM + ( OPPGOAL->TOP - OPPGOAL->BOTTOM ) / 2 - predictedBall.getY() );

		makeQuadAngle( &ballAngle );

		char blah[124] = "";
		sprintf( blah, "%g", ballAngle );
		debug( blah );*/

		/*const double goalY = OPPGOAL->BOTTOM + ( OPPGOAL->TOP - OPPGOAL->BOTTOM ) / 2;
		const double xDiff = predictedBall.getX() - HOMEGOAL->X;
		const double yDiff = goalY - predictedBall.getY();

		double moveX = HOMEGOAL->X + xDiff / 180 * 100;
		double moveY = goalY - yDiff / 180 * 100;

		const double minX = HOMEGOAL->X + 2.0;
		const double maxX = HOMEGOAL->X + 15.0;
		const double minY = OPPGOAL->BOTTOM - 8.0;
		const double maxY = OPPGOAL->TOP + 8.0;

		if ( moveX < minX )
			moveX = minX;
		else if ( moveX > maxX )
			moveX = maxX;

		if ( moveY < minY )
			moveY = minY;
		else if ( moveY > maxY )
			moveY = maxY;

		/*char blah[124] = "";
		sprintf( blah, "move x: %g\nmove y: %g", moveX, moveY );
		debug( blah );

		move( defender, moveX, moveY );
	}
}

void RobotSoccerStrategy::attack()
{
	// version 1.0

	Robot * const attacker = &env->home[3];

	// if attacker behind ball

	if ( OPPGOAL->ORIENTATION == GOALORIENTATION_RIGHT ? predictedBall.getX() - attacker->pos.x > 0.0 : attacker->pos.x - predictedBall.getX() > 0.0 )
		move( attacker, predictedBall.getX(), predictedBall.getY() );

	else
		move( attacker, OPPGOAL->ORIENTATION == GOALORIENTATION_RIGHT ? predictedBall.getX() - 10.0 : predictedBall.getX() + 10.0, predictedBall.getY() - env->currentBall.pos.y > 0.0 ? predictedBall.getY() - 5.0 : predictedBall.getY() + 5.0 );
}

void RobotSoccerStrategy::attack()
{
	// version 2.0

	Robot * const attacker = &env->home[2];

	if ( OPPGOAL->ORIENTATION == GOALORIENTATION_LEFT )
		// goal is on the left

		if ( predictedBall.getX() < 75.0 )
		{
			// ball is in midfield or upfront

			double ang = 180.0 / 3.14159265 * atan2( OPPGOAL->X - predictedBall.getX(), HOMEGOAL->BOTTOM + ( ( HOMEGOAL->TOP - HOMEGOAL->BOTTOM ) / 2 ) - predictedBall.getY() );
			double x = 0.0;
			double y = 0.0;

			if ( ang > 0.0 )
				if ( ang < 90.0 )
					ang = 0.0;
				else
					ang = 180.0;

			else
				ang = ang + ( - ang * 2 );

			if ( ang < 90.0 )
			{
				// ball is in bottom half

				// smaller the angle, higher y

				x = ang / 90.0 * 1.0;
				y = 1.0 - x;

				move( attacker, predictedBall.getX() + x, predictedBall.getY() - y );
			}
			else
			{
				// ball is in top half

				// higher the angle, higher y

				y = ( ang - 90.0 ) / 90 * 1.0;
				x = 1.0 - y;
			
				move( attacker, predictedBall.getX() + x, predictedBall.getY() + y );
			}

		}

		else
			stopMoving( attacker );

	else
		// goal is on the right

		if ( predictedBall.getX() > 25.0 )
		{
			// ball is in midfield or upfront
		}
}

void RobotSoccerStrategy::attack()
{
	// version 3.0

	Robot * const attacker = &env->home[2];

	if ( OPPGOAL->ORIENTATION == GOALORIENTATION_LEFT )
	{
		// goal is on the left

		// calc angles of goal in relation to ball and angle of ball in relation to robot

		double topGoalAngle = 180.0 / 3.14159265 * atan2( predictedBall.getX() - OPPGOAL->X, predictedBall.getY() - HOMEGOAL->TOP );
		double bottomGoalAngle =  180.0 / 3.14159265 * atan2( predictedBall.getX() - OPPGOAL->X, predictedBall.getY() - HOMEGOAL->BOTTOM );
		double ballAngle = 180.0 / 3.14159265 * atan2( attacker->pos.x - predictedBall.getX(), attacker->pos.y - predictedBall.getY() );

		if ( topGoalAngle < 0.0 )
			if ( topGoalAngle < -90.0 )
				topGoalAngle = 180.0;
			else
				topGoalAngle = 0.0;
		
		if ( bottomGoalAngle < 0.0 )
			if ( bottomGoalAngle < -90.0 )
				bottomGoalAngle = 180.0;
			else
				bottomGoalAngle = 0.0;

		if ( attacker->pos.x > predictedBall.getX() && ballAngle > bottomGoalAngle && ballAngle < topGoalAngle )
		{
			//debug( "in position" );
			move( attacker, predictedBall.getX(), predictedBall.getY() );
		}
		else
		{
			//const double y = bottomGoalAngle / 180.0 * -12.0 + 6.0;
			const double y = ( bottomGoalAngle + ( ( topGoalAngle - bottomGoalAngle ) / 2 ) ) / 180.0 * -25.0 + 12.5 ;
			const double x = y < 0.0 ? y + 12.5 : 12.5 - y;

			move( attacker, predictedBall.getX() + x, predictedBall.getY() + y );
			char blah[124] = "";
			sprintf( blah, "bottom angle:\t%g\npredicted x:\t%g\ngoto x:\t%g\npredicted y:\t%g\ngoto y:\t%g", bottomGoalAngle, predictedBall.getY(), predictedBall.getY() + x, predictedBall.getY(), predictedBall.getY() + y );
			//debug( blah );
		}
	}
}

void RobotSoccerStrategy::attack()
{
	// version 3.1
	// todo: improve distance calculation (for robot behind ball)

	// sort out move, if goto x is out of boudns check

	Robot * const attacker = &env->home[2];

	if ( OPPGOAL->ORIENTATION == GOALORIENTATION_LEFT )
	{
		// goal is on the left

		// calc angles of goal in relation to ball and angle of ball in relation to robot

		double topGoalAngle = 180.0 / 3.14159265 * atan2( predictedBall.getX() - OPPGOAL->X, predictedBall.getY() - HOMEGOAL->TOP );
		double bottomGoalAngle =  180.0 / 3.14159265 * atan2( predictedBall.getX() - OPPGOAL->X, predictedBall.getY() - HOMEGOAL->BOTTOM );
		double ballAngle = 180.0 / 3.14159265 * atan2( attacker->pos.x - predictedBall.getX(), attacker->pos.y - predictedBall.getY() );

		if ( topGoalAngle < 0.0 )
			if ( topGoalAngle < -90.0 )
				topGoalAngle = 180.0;
			else
				topGoalAngle = 0.0;
		
		if ( bottomGoalAngle < 0.0 )
			if ( bottomGoalAngle < -90.0 )
				bottomGoalAngle = 180.0;
			else
				bottomGoalAngle = 0.0;

		if ( attacker->pos.x > predictedBall.getX() && ballAngle > bottomGoalAngle && ballAngle < topGoalAngle )
		{
			//debug( "in position" );
			move( attacker, predictedBall.getX(), predictedBall.getY() );
		}
		else
		{
			double distance;
			
			{
				const double yDiff = attacker->pos.y > predictedBall.getY() ? attacker->pos.y - predictedBall.getY() : predictedBall.getY() - attacker->pos.y;
				const double xDiff = attacker->pos.x > predictedBall.getX() ? attacker->pos.x - predictedBall.getX() : predictedBall.getX() - attacker->pos.x;

				distance = sqrt( xDiff * xDiff + yDiff * yDiff );
			}

			double gotoX = predictedBall.getX() + ( predictedBall.getX() - OPPGOAL->X ) / 75 * distance;
			double gotoY = predictedBall.getY() + ( predictedBall.getY() - ( OPPGOAL->BOTTOM + ( OPPGOAL->TOP - OPPGOAL->BOTTOM ) / 2 ) ) / 75 * distance;
					
			move( attacker, gotoX, gotoY );

			char blah[1024] = "";
			sprintf( blah, "predicted X:\t%g\npredicted Y:\t%g\ngoto X:\t%g\ngoto Y:\t%g\ndistance:\t%g\n", predictedBall.getX(), predictedBall.getY(), gotoX, gotoY, distance );
			debug( blah );
		}
	}
}*/