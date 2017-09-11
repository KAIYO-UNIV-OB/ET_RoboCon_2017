#include "CalculateRunningDirection.h"
#include "SelfPositionChecker.h"

namespace unit
{
	CalculateRunningDirection::CalculateRunningDirection(SelfPositionChecker* selfPositionChecker)
	:mSelfPositionChecker(selfPositionChecker)
	{

	}
	CalculateRunningDirection::~CalculateRunningDirection()
	{

	}

	int CalculateRunningDirection::calculate()
	{
		int direction = 0;
		// ���G�b�W���s����悤�ݒ�
		if (mSelfPositionChecker->isOnBlackLine())
		{
			direction = -1;
		}
		else
		{
			direction = 1;
		}
		return direction;
	}
}
