#pragma once

#include "OperandPtg.h"

namespace XLS
{;

class CFRecord;

class PtgElfColS: public OperandPtg
{
	BASE_OBJECT_DEFINE_CLASS_NAME(PtgElfColS)
public:
	BiffStructurePtr clone();

	virtual void loadFields(CFRecord& record);
	virtual void storeFields(CFRecord& record);

	virtual void assemble(AssemblerStack& ptg_stack, PtgQueue& extra_data, BiffStructurePtr & parent);

};

} // namespace XLS
