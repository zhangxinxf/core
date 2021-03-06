﻿/*
 * (c) Copyright Ascensio System SIA 2010-2016
 *
 * This program is a free software product. You can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License (AGPL)
 * version 3 as published by the Free Software Foundation. In accordance with
 * Section 7(a) of the GNU AGPL its Section 15 shall be amended to the effect
 * that Ascensio System SIA expressly excludes the warranty of non-infringement
 * of any third-party rights.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR  PURPOSE. For
 * details, see the GNU AGPL at: http://www.gnu.org/licenses/agpl-3.0.html
 *
 * You can contact Ascensio System SIA at Lubanas st. 125a-25, Riga, Latvia,
 * EU, LV-1021.
 *
 * The  interactive user interfaces in modified source and object code versions
 * of the Program must display Appropriate Legal Notices, as required under
 * Section 5 of the GNU AGPL version 3.
 *
 * Pursuant to Section 7(b) of the License you must retain the original Product
 * logo when distributing the program. Pursuant to Section 7(e) we decline to
 * grant you any rights under trademark law for use of our trademarks.
 *
 * All the Product's GUI elements, including illustrations and icon sets, as
 * well as technical writing content are licensed under the terms of the
 * Creative Commons Attribution-ShareAlike 4.0 International. See the License
 * terms at http://creativecommons.org/licenses/by-sa/4.0/legalcode
 *
 */


#include <boost/foreach.hpp>
#include <vector>
#include <cpdoccore/xml/simple_xml_writer.h>
#include "mediaitems_utils.h"
#include "oox_rels.h"

#include "oox_chart_context.h"

namespace cpdoccore {
namespace oox {

class oox_chart_context::Impl
{
public:
    Impl(std::wstring name){}
    std::wstring name_;

    std::wstringstream  chartData_;

    std::wstring drawingName_;
    std::wstring drawingId_;
};

oox_chart_context::oox_chart_context(mediaitems & m, std::wstring name) :
		impl_(new oox_chart_context::Impl( name)), mediaitems_(m)
{
	
}

void oox_chart_context::reset_fill(oox::_oox_fill &f)
{
	if (f.bitmap)
	{
		bool isInternal = true;
		std::wstring ref;
		f.bitmap->rId = mediaitems_.add_or_find(f.bitmap->xlink_href_, typeImage, isInternal, ref); 

		rels_.push_back(_rel(isInternal, f.bitmap->rId, ref, typeImage));
	}
}

std::wostream & oox_chart_context::chartData()
{
    return impl_->chartData_;
}

void oox_chart_context::dump_rels(rels & Rels)
{
	for (int i = 0; i < rels_.size(); i++)
    {
		_rel & r = rels_[i];

		if (r.type == typeImage)
		{
			Rels.add(relationship(
						r.rid,
						utils::media::get_rel_type(r.type),
						r.is_internal ? std::wstring(L"../") + r.ref : r.ref,
						(r.is_internal ? L"" : L"External")
						) 
				);
		}
		else if (r.type == typeHyperlink)
		{
			Rels.add(relationship(
						r.rid,
						L"http://schemas.openxmlformats.org/officeDocument/2006/relationships/hyperlink",
						r.ref,
						L"External")
			);
		}
	}
}

void oox_chart_context::serialize(std::wostream & strm)
{
	CP_XML_WRITER(strm)
	{
		CP_XML_NODE(L"c:chartSpace")
		{          
            CP_XML_ATTR(L"xmlns:r", L"http://schemas.openxmlformats.org/officeDocument/2006/relationships");
			CP_XML_ATTR(L"xmlns:a", L"http://schemas.openxmlformats.org/drawingml/2006/main");
            CP_XML_ATTR(L"xmlns:c", L"http://schemas.openxmlformats.org/drawingml/2006/chart");
		
			CP_XML_NODE(L"c:lang")
			{
				CP_XML_ATTR(L"val",L"en-US");
			}
			CP_XML_NODE(L"c:chart")
			{
				if (plot_area_.current_chart_->is3D_)
				{
					//CP_XML_NODE(L"c:view3D");
					CP_XML_NODE(L"c:floor");
					CP_XML_NODE(L"c:backWall");
				}
				title_.oox_serialize	(CP_XML_STREAM());
				plot_area_.oox_serialize(CP_XML_STREAM());
				legend_.oox_serialize	(CP_XML_STREAM());
				
				CP_XML_NODE(L"c:plotVisOnly")
				{
					CP_XML_ATTR(L"val",1);
				}	
				CP_XML_NODE(L"c:dispBlanksAs")
				{
					CP_XML_ATTR(L"val", L"zero");
				}
				CP_XML_NODE(L"c:showDLblsOverMax")
				{
					CP_XML_ATTR(L"val",1);
				}
			}
			oox_chart_shape shape;

			shape.set(graphic_properties_, fill_);
			shape.oox_serialize(CP_XML_STREAM());
	

		}
	}

}

oox_chart_context::~oox_chart_context()
{
}

void oox_chart_context::set_cache_only	(bool val)
{
	for (int i = 0 ; i < plot_area_.charts_.size(); i++)
	{
		plot_area_.charts_[i]->set_cache_only(val);
	}
}



}
}

