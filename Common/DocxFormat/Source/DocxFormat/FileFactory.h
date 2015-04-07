#pragma once
#ifndef OOX_FILE_FACTORY_INCLUDE_H_
#define OOX_FILE_FACTORY_INCLUDE_H_

#include "../Base/SmartPtr.h"
#include "../SystemUtility/SystemUtility.h"

namespace OOX
{
	class File;
	namespace Rels
	{
		class CRelationShip;
	}
}

namespace OOX
{
	const NSCommon::smart_ptr<OOX::File> CreateFile(const OOX::CPath& oRootPath, const CPath& oPath, const OOX::Rels::CRelationShip& oRelation);
	const NSCommon::smart_ptr<OOX::File> CreateFile(const OOX::CPath& oRootPath, const CPath& oPath, OOX::Rels::CRelationShip* pRelation);
} // namespace OOX

#endif // OOX_FILE_FACTORY_INCLUDE_H_