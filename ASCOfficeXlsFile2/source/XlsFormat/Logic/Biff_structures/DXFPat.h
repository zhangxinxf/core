#pragma once

#include "BiffStructure.h"
#include <Logic/Biff_structures/BitMarkedStructs.h>

namespace XLS
{;

class CFRecord;

class DXFPat : public BiffStructure
{
	BASE_OBJECT_DEFINE_CLASS_NAME(DXFPat)
public:
	BiffStructurePtr clone();

	//virtual void setXMLAttributes(MSXML2::IXMLDOMElementPtr xml_tag);
	//virtual void getXMLAttributes(MSXML2::IXMLDOMElementPtr xml_tag);
	virtual void load(CFRecord& record);
	virtual void store(CFRecord& record);

private:
	FillPattern fls;
	unsigned char icvForeground;
	unsigned char icvBackground;
};

} // namespace XLS
