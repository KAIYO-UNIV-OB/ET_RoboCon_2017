//*************************************************
//Stop���g
//
//
//
//*************************************************

#include "Run4mAndStop.h"
#include "Stop.h"
#include "BalanceRunning.h"
#include "ev3api.h"
#include "UsingLCD.h"


//*********************************************************
//
//stop�N���X���e
//
//
//
//*********************************************************
Stop::Stop(BalanceRunning* balanceRunning)
	: mBalanceRunning(balanceRunning)
{
}

void Stop::executeStop()
{
	
	waitThere();
	return;
}

void Stop::waitThere()
{
	ev3_lcd_draw_string("stop task:wait", 0, CALIB_FONT_HEIGHT * 1);
	mBalanceRunning->executeRun(EV3_STOP, EV3_STOP);
	return;
}
