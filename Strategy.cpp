/*
 * RobotSoccerStrategy_01
 * Robot Soccer strategy
 * Chris Siddall
 */

#include "RobotSoccerStrategy.h"
#include "..\StrategyShell\Debug.h"


//

extern "C"
{

	void __stdcall Create( Environment * const env )
	{
		env->userData = new RobotSoccerStrategy();

		ifdebug( "Strategy loaded." );
	}

	void __stdcall Destroy( Environment * const env )
	{
		if ( env->userData )
			delete (RobotSoccerStrategy *) env->userData;

		ifdebug( "Strategy unloaded." );
	}

	void __stdcall Strategy( Environment * const env )
	{
		( (RobotSoccerStrategy *) env->userData )->strategy( env );
	}

}


//
