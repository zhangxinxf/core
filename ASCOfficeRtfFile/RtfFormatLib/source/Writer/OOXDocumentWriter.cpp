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
#include "OOXDocumentWriter.h"
//#include "OOXContentTypesWriter.h"
#include "OOXRelsWriter.h"
#include "OOXDocumentWriter.h"
#include "OOXNumberingWriter.h"
#include "OOXFontTableWriter.h"
#include "OOXStylesWriter.h"
#include "OOXSettingsWriter.h"
#include "OOXThemeWriter.h"
#include "OOXFootnoteWriter.h"
#include "OOXStylesWriter.h"

//#include "../../../../ASCOfficeDocxFile2/BinReader/ContentTypesWriter.h"

CString OOXDocumentWriter::CreateXmlStart()
{
	//пишем Footnotes
	RenderParameter oNewParam;
	oNewParam.poDocument = &m_oDocument;
	oNewParam.poWriter = &m_oWriter;
	oNewParam.poRels = &m_oWriter.m_oDocRels;
	oNewParam.nType = RENDER_TO_OOX_PARAM_UNKNOWN;



	//пишем document.xml
	CString sResult = _T("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\" ?>\n");
	sResult += _T("<w:document");
	sResult += _T(" xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\"");
	sResult += _T(" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\"");
	sResult += _T(" xmlns:v=\"urn:schemas-microsoft-com:vml\"");
	sResult += _T(" xmlns:o=\"urn:schemas-microsoft-com:office:office\"");
	sResult += _T(" xmlns:wp=\"http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing\"");
	sResult += _T(" xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\"");
	sResult += _T(" xmlns:w10=\"urn:schemas-microsoft-com:office:word\"");
	sResult += _T(" xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\"");
	sResult += _T(">");

	sResult += _T("<w:body>");
	return sResult;
}
CString OOXDocumentWriter::CreateXmlEnd( )
{
	CString sResult ;

	//пишем все кроме document.xml
	RenderParameter oNewParam;
	oNewParam.poDocument	= &m_oDocument;
	oNewParam.poWriter		= &m_oWriter;
	oNewParam.poRels		= &m_oWriter.m_oDocRels;
	oNewParam.nType			= RENDER_TO_OOX_PARAM_UNKNOWN;

	OOXEndnoteWriter*	poEndnoteWriter		= static_cast<OOXEndnoteWriter*>( m_oWriter.m_poEndnoteWriter );
	OOXFootnoteWriter*	poFootnoteWriter	= static_cast<OOXFootnoteWriter*>( m_oWriter.m_poFootnoteWriter );
	
	if( NULL != m_oDocument.m_oFootnoteCon )
	{
		oNewParam.poRels = poFootnoteWriter->m_oRelsWriter.get();
		oNewParam.nType = RENDER_TO_OOX_PARAM_UNKNOWN;
		poFootnoteWriter->AddFootnoteBegin( _T("continuationSeparator"), 1, m_oDocument.m_oFootnoteCon->RenderToOOX( oNewParam ) );
	}
	if( NULL != m_oDocument.m_oFootnoteSep )
	{
		oNewParam.poRels = poFootnoteWriter->m_oRelsWriter.get();
		oNewParam.nType = RENDER_TO_OOX_PARAM_UNKNOWN;
		poFootnoteWriter->AddFootnoteBegin( _T("separator"), 0, m_oDocument.m_oFootnoteSep->RenderToOOX(oNewParam) );
	}
	if( NULL != m_oDocument.m_oEndnoteCon )
	{
		oNewParam.poRels = poEndnoteWriter->m_oRelsWriter.get();
		oNewParam.nType = RENDER_TO_OOX_PARAM_UNKNOWN;
		poEndnoteWriter->AddEndnoteBegin( _T("continuationSeparator"), 1, m_oDocument.m_oEndnoteSep->RenderToOOX(oNewParam) );
	}
	if( NULL != m_oDocument.m_oEndnoteSep )
	{
		oNewParam.poRels = poEndnoteWriter->m_oRelsWriter.get();
		oNewParam.nType = RENDER_TO_OOX_PARAM_UNKNOWN;
		poEndnoteWriter->AddEndnoteBegin( _T("separator"), 0, m_oDocument.m_oEndnoteSep->RenderToOOX(oNewParam) );
	}

	//fontTable.xml
	OOXFontTableWriter* poFontTableWriter = static_cast<OOXFontTableWriter*>( m_oWriter.m_poFontTableWriter );
	oNewParam.poRels = poFontTableWriter->m_oRelsWriter.get();
	oNewParam.nType = RENDER_TO_OOX_PARAM_FONTTABLE;
	poFontTableWriter->AddContent( m_oDocument.m_oFontTable.RenderToOOX( oNewParam ) );

	//settings.xml
	OOXSettingsWriter* poSettingsWriter = static_cast<OOXSettingsWriter*>( m_oWriter.m_poSettingsWriter );
	oNewParam.poRels = poSettingsWriter->m_oRelsWriter.get();
	oNewParam.nType = RENDER_TO_OOX_PARAM_UNKNOWN;
	poSettingsWriter->AddContent( m_oDocument.m_oProperty.RenderToOOX(oNewParam) );
	//poSettingsWriter->AddContent( m_oDocument.m_oColorSchemeMapping.RenderToOOX(oNewParam) );
	poSettingsWriter->AddContent( m_oDocument.m_oMathProp.RenderToOOX(oNewParam) );

	//numbering.xml
	OOXNumberingWriter* poNumberingWriter = static_cast<OOXNumberingWriter*>( m_oWriter.m_poNumberingWriter );
	oNewParam.poRels	= poNumberingWriter->m_oRelsWriter.get();
	oNewParam.nType		= RENDER_TO_OOX_PARAM_NUMBERING;
	
	poNumberingWriter->AddNumbering( m_oDocument.m_oListTable.RenderToOOX(oNewParam) );
	poNumberingWriter->AddNumbering( m_oDocument.m_oListOverrideTable.RenderToOOX(oNewParam) );

	//style.xml
	OOXStylesWriter* poStylesWriter = static_cast<OOXStylesWriter*>( m_oWriter.m_poStylesWriter );
	oNewParam.poRels	= poNumberingWriter->m_oRelsWriter.get();
	oNewParam.nType		= RENDER_TO_OOX_PARAM_STYLES;

	CString sStyles;
	CString sTempParaDef = m_oDocument.m_oDefaultParagraphProp.RenderToOOX(oNewParam);
	CString sTempCharDef = m_oDocument.m_oDefaultCharProp.RenderToOOX(oNewParam);
	
	if( false == sTempParaDef.IsEmpty() || false == sTempCharDef.IsEmpty() )
	{
		sStyles += _T("<w:docDefaults>");
		if( false == sTempCharDef.IsEmpty() )
		{
			sStyles += _T("<w:rPrDefault><w:rPr>");
			sStyles += sTempCharDef ;
			sStyles += _T("</w:rPr></w:rPrDefault>");
		}	
		if( false == sTempParaDef.IsEmpty() )
		{
			sStyles += _T("<w:pPrDefault><w:pPr>");
			sStyles +=  sTempParaDef;
			sStyles += _T("</w:pPr></w:pPrDefault>");
		}
		sStyles += _T("</w:docDefaults>");
	}

	sStyles += m_oDocument.m_oStyleTable.RenderToOOX(oNewParam);
	poStylesWriter->AddContent(sStyles);

	//core.xml
	oNewParam.poRels	= NULL;
	oNewParam.nType		= RENDER_TO_OOX_PARAM_CORE;
	oNewParam.poWriter	= m_oWriter.m_poDocPropsCore;
	m_oDocument.m_oInformation.RenderToOOX(oNewParam);

	//app.xml
	oNewParam.poRels	= NULL;
	oNewParam.nType		= RENDER_TO_OOX_PARAM_APP;
	oNewParam.poWriter	= m_oWriter.m_poDocPropsApp;
	m_oDocument.m_oInformation.RenderToOOX(oNewParam);


	//пишем финальные свойства секции
	oNewParam.poDocument = &m_oDocument;
	oNewParam.poWriter = &m_oWriter;
	oNewParam.poRels = &m_oWriter.m_oDocRels;
	oNewParam.nType = RENDER_TO_OOX_PARAM_UNKNOWN;
	
	sResult += m_oDocument[0].props->m_oProperty.RenderToOOX(oNewParam);

	sResult += _T("</w:body>");
	sResult += _T("</w:document>");

	return sResult;
}

bool OOXDocumentWriter::SaveByItemStart( CString sFolder )
{
	CString pathWord = sFolder + FILE_SEPARATOR_STR + _T("word");
    FileSystem::Directory::CreateDirectory(pathWord) ;

	try
	{
		CString sFilename = pathWord + FILE_SEPARATOR_STR + _T("document.xml");
		m_oFileWriter = new NFileWriter::CBufferedFileWriter( sFilename );
	}
	catch(...)
	{
		return false;
	}
	m_oWriter.m_oRels.AddRelationship( _T("http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument"), _T("word/document.xml") );
	m_oWriter.m_oContentTypes.AddContent( _T("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"), _T("/word/document.xml") );

	CString sXml = CreateXmlStart( );
    std::string sXmlUTF = NSFile::CUtf8Converter::GetUtf8StringFromUnicode(sXml.GetBuffer());

    m_oFileWriter->Write((BYTE*)sXmlUTF.c_str(), sXmlUTF.length());
    return true;
}
bool OOXDocumentWriter::SaveByItem()
{
	if( true == m_bFirst )
		m_bFirst = false;
	else
	{
		RenderParameter oNewParam;
		oNewParam.poDocument	= &m_oDocument;
		oNewParam.poWriter		= &m_oWriter;
		oNewParam.poRels		= &m_oWriter.m_oDocRels;
		oNewParam.nType			= RENDER_TO_OOX_PARAM_UNKNOWN;

		if( m_oDocument.GetCount() > 1)//если что-то есть в следующей секции значит предыдущая закончилась
		{
			std::wstring sXml, sectPr;

			_section section;
			if (m_oDocument.GetItem(section, 0))
			{
				sectPr = section.props->RenderToOOX(oNewParam).GetBuffer();
			}

			if( m_oDocument[0].props->GetCount() > 0 )
			{
				sXml = m_oDocument[0].props->operator[](0)->RenderToOOX(oNewParam).GetBuffer();
				
				int nFind = -1, nFindPict = -1, pos = sXml.size();

				do
				{
					nFindPict	= sXml.rfind(L"<w:pict>", pos);
					nFind		= sXml.rfind(L"</w:pPr>", pos);
					pos = nFindPict - 1;
				}while(nFindPict > 0 && nFind > nFindPict);


				if( -1 != nFind)
				{
					sXml.insert( nFind, sectPr );
				}
				else
				{
					int Find = sXml.rfind( L"<w:p>" );
					if( -1 != nFind )
						sXml.insert( nFind + 5, L"<w:pPr>" + sectPr + L"</w:pPr>" );
				}
			}
			else
			{
				//генерация ???
				sXml = _T("<w:p><w:pPr>") + sectPr + _T("</w:pPr></w:p>");
			}
	
			std::string sXmlUTF = NSFile::CUtf8Converter::GetUtf8StringFromUnicode(sXml);
			m_oFileWriter->Write((BYTE*)sXmlUTF.c_str(), sXmlUTF.length());
			
			m_oDocument.RemoveItem( 0 ); //удаляем секцию
		}
		else if( m_oDocument.GetCount() > 0 && m_oDocument[0].props->GetCount() > 1 )//пишем параграф - один всегда  "прозапас для секций"
		{
			CString sXml = m_oDocument[0].props->operator[](0)->RenderToOOX(oNewParam);
            std::string sXmlUTF = NSFile::CUtf8Converter::GetUtf8StringFromUnicode(sXml.GetBuffer());

			if (m_oFileWriter)
			{
				m_oFileWriter->Write((BYTE*)sXmlUTF.c_str(), sXmlUTF.length());
			}
			else
			{
				//!!!!
				m_oFileWriter = NULL;
			}
			
			m_oDocument[0].props->RemoveItem( 0 );//удаляем первый параграф
        }
	}
	return true;
}
bool OOXDocumentWriter::SaveByItemEnd()
{
	RenderParameter oNewParam;
	oNewParam.poDocument	= &m_oDocument;
	oNewParam.poWriter		= &m_oWriter;
	oNewParam.poRels		= &m_oWriter.m_oDocRels;
	oNewParam.nType			= RENDER_TO_OOX_PARAM_UNKNOWN;

	if( m_oDocument.GetCount() > 0 && m_oDocument[0].props->GetCount() > 0 )//дописываем последний параграф
	{
		CString sXml = m_oDocument[0].props->operator[](0)->RenderToOOX(oNewParam);
		//удаляем первый параграф
		m_oDocument[0].props->RemoveItem( 0 );
        std::string sXmlUTF = NSFile::CUtf8Converter::GetUtf8StringFromUnicode(sXml.GetBuffer());

        m_oFileWriter->Write((BYTE*)sXmlUTF.c_str(), sXmlUTF.length());
    }
	CString sXml = CreateXmlEnd( );
    std::string sXmlUTF = NSFile::CUtf8Converter::GetUtf8StringFromUnicode(sXml.GetBuffer());

	if (m_oFileWriter)
	{
		m_oFileWriter->Write((BYTE*)sXmlUTF.c_str(), sXmlUTF.length());
	}
    RELEASEOBJECT( m_oFileWriter );
	return true;
}