#include "precompiled_xls.h"
#include "Scatter.h"

namespace XLS
{;

Scatter::Scatter()
{
}


Scatter::~Scatter()
{
}


BaseObjectPtr Scatter::clone()
{
	return BaseObjectPtr(new Scatter(*this));
}


void Scatter::writeFields(CFRecord& record)
{
	unsigned __int16 flags = 0;
	SETBIT(flags, 0, fBubbles);
	SETBIT(flags, 1, fShowNegBubbles);
	SETBIT(flags, 2, fHasShadow);
	record >> pcBubbleSizeRatio >> wBubbleSize >> flags;
}


void Scatter::readFields(CFRecord& record)
{
	unsigned __int16 flags;
	record >> pcBubbleSizeRatio >> wBubbleSize >> flags;
	fBubbles = GETBIT(flags, 0);
	fShowNegBubbles = GETBIT(flags, 1);
	fHasShadow = GETBIT(flags, 2);
}

} // namespace XLS
