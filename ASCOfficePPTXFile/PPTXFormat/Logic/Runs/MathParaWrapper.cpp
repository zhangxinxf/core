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
#include "MathParaWrapper.h"
#include "../../../Common/DocxFormat/Source/DocxFormat/Math/oMathPara.h"
#include "../../../ASCOfficeDocxFile2/BinWriter/BinWriters.h"
#include "../../../ASCOfficeDocxFile2/BinReader/FileWriter.h"

namespace PPTX
{
	namespace Logic
	{
		MathParaWrapper& MathParaWrapper::operator=(const MathParaWrapper& oSrc)
		{
			//todo
			parentFile		= oSrc.parentFile;
			parentElement	= oSrc.parentElement;

			m_oMathPara = oSrc.m_oMathPara;
			m_oMath = oSrc.m_oMath;
			return *this;
		}
		void MathParaWrapper::fromXML(XmlUtils::CXmlNode& node)
		{
			CString sBegin(_T("<root xmlns:wpc=\"http://schemas.microsoft.com/office/word/2010/wordprocessingCanvas\" xmlns:mc=\"http://schemas.openxmlformats.org/markup-compatibility/2006\" xmlns:p=\"urn:schemas-microsoft-com:office:powerpoint\" xmlns:v=\"urn:schemas-microsoft-com:vml\" xmlns:x=\"urn:schemas-microsoft-com:office:excel\" xmlns:o=\"urn:schemas-microsoft-com:office:office\" xmlns:w10=\"urn:schemas-microsoft-com:office:word\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\" xmlns:ve=\"http://schemas.openxmlformats.org/markup-compatibility/2006\" xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:wp=\"http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing\" xmlns:wp14=\"http://schemas.microsoft.com/office/word/2010/wordprocessingDrawing\" xmlns:w14=\"http://schemas.microsoft.com/office/word/2010/wordml\" xmlns:w15=\"http://schemas.microsoft.com/office/word/2012/wordml\" xmlns:wpg=\"http://schemas.microsoft.com/office/word/2010/wordprocessingGroup\" xmlns:wpi=\"http://schemas.microsoft.com/office/word/2010/wordprocessingInk\" xmlns:wne=\"http://schemas.microsoft.com/office/word/2006/wordml\" xmlns:wps=\"http://schemas.microsoft.com/office/word/2010/wordprocessingShape\" xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" xmlns:a14=\"http://schemas.microsoft.com/office/drawing/2010/main\" xmlns:pic=\"http://schemas.openxmlformats.org/drawingml/2006/picture\" xmlns:xdr=\"http://schemas.openxmlformats.org/drawingml/2006/spreadsheetDrawing\">"));
			CString sEnd(_T("</root>"));
			CString sXml = sBegin + node.GetXml() + sEnd;
			XmlUtils::CXmlLiteReader oReader;
			oReader.FromString(sXml.GetBuffer());
			sXml.ReleaseBuffer();
			oReader.ReadNextNode();//root
			oReader.ReadNextNode();//a14:m
			oReader.ReadNextNode();//oMath oMathPara
			
			std::wstring strNameP = XmlUtils::GetNameNoNS(oReader.GetName());
			if(L"oMathPara" == strNameP)
			{
				m_oMathPara = oReader;
			}
			else if(L"oMath" == strNameP)
			{
				m_oMath = oReader;
			}

			FillParentPointersForChilds();
		}

		CString MathParaWrapper::toXML() const
		{
			if(m_oMathPara.IsInit() || m_oMath.IsInit() || m_oXml.IsInit())
			{
				CString sXml = L"<mc:AlternateContent xmlns:mc=\"http://schemas.openxmlformats.org/markup-compatibility/2006\"><mc:Choice xmlns:a14=\"http://schemas.microsoft.com/office/drawing/2010/main\" Requires=\"a14\"><a14:m>";
				if(m_oMathPara.IsInit())
				{
					sXml += m_oMathPara->toXML();
				}
				else if(m_oMath.IsInit())
				{
					sXml += m_oMath->toXML();
				}
				else
				{
					sXml += m_oXml.get();
				}
				sXml += L"</a14:m></mc:Choice><mc:Fallback></mc:Fallback></mc:AlternateContent>";
				return sXml;
			}
			else
			{
				return L"";
			}
		}

		void MathParaWrapper::toXmlWriter(NSBinPptxRW::CXmlWriter* pWriter) const
		{
			pWriter->WriteString(toXML());
		}

		void MathParaWrapper::toPPTY(NSBinPptxRW::CBinaryFileWriter* pWriter) const
		{
			int nRecordType;
			void* pElem = NULL;
			OOX::EElementType eElemType;
			if(m_oMathPara.IsInit())
			{
				nRecordType = PARRUN_TYPE_MATHPARA;
				pElem = m_oMathPara.GetPointer();
				eElemType = m_oMathPara->getType();
			}
			else if(m_oMath.IsInit())
			{
				nRecordType = PARRUN_TYPE_MATH;
				pElem = m_oMath.GetPointer();
				eElemType = m_oMath->getType();
			}
			if(NULL != pElem)
			{
				pWriter->StartRecord(nRecordType);
				long lDataSize = 0;
				if(NULL != pWriter->m_pMainDocument)
				{
					pWriter->m_pMainDocument->getBinaryContentElem(eElemType, pElem, *pWriter, lDataSize);
				}
				else
				{
					BinDocxRW::CDocxSerializer oDocxSerializer;
					NSBinPptxRW::CDrawingConverter oDrawingConverter;
					NSBinPptxRW::CBinaryFileWriter* pOldWriter = oDrawingConverter.m_pBinaryWriter;
					//NSCommon::smart_ptr<PPTX::CCommonRels> pOldRels = *oDrawingConverter.m_pBinaryWriter->m_pCommonRels;
					oDrawingConverter.m_pBinaryWriter = pWriter;

					DocWrapper::FontProcessor fp;
					BinDocxRW::ParamsWriter oParamsWriter(pWriter, &fp, &oDrawingConverter, NULL);
					oDocxSerializer.m_pParamsWriter = &oParamsWriter;
					oDocxSerializer.getBinaryContentElem(eElemType, pElem, *pWriter, lDataSize);
					//*oDrawingConverter.m_pBinaryWriter->m_pCommonRels = pOldRels;
					oDrawingConverter.m_pBinaryWriter = pOldWriter;
				}

				pWriter->EndRecord();
			}
		}

		void MathParaWrapper::fromPPTY(BYTE type, NSBinPptxRW::CBinaryFileReader* pReader)
		{
			OOX::EElementType eType;
			if(PARRUN_TYPE_MATHPARA == type)
			{
				eType = OOX::et_m_oMathPara;
			}
			else
			{
				eType = OOX::et_m_oMath;
			}
			LONG _end = pReader->GetPos() + pReader->GetLong() + 4;
			CString sXml;
			if(NULL != pReader->m_pMainDocument)
			{
				pReader->m_pMainDocument->getXmlContentElem(eType, *pReader, m_oXml.get());
			}
			else
			{
				BinDocxRW::CDocxSerializer oDocxSerializer;
				NSBinPptxRW::CDrawingConverter oDrawingConverter;
				NSBinPptxRW::CImageManager2* pOldImageManager = oDrawingConverter.m_pImageManager;
				NSBinPptxRW::CBinaryFileReader* pOldReader = oDrawingConverter.m_pReader;
				oDrawingConverter.m_pImageManager = pReader->m_pRels->m_pManager;
				oDrawingConverter.m_pReader = pReader;

				oDocxSerializer.m_pCurFileWriter = new Writers::FileWriter(L"", L"", true, BinDocxRW::g_nFormatVersion, false, &oDrawingConverter, L"");
				oDocxSerializer.getXmlContentElem(eType, *pReader, sXml);

				oDrawingConverter.m_pReader = pOldReader;
				oDrawingConverter.m_pImageManager = pOldImageManager;
				RELEASEOBJECT(oDocxSerializer.m_pCurFileWriter);
			}
			m_oXml = sXml;
			pReader->Seek(_end);
		}

		CString MathParaWrapper::GetText() const
		{
			//todo
			return _T("");
		}
	} // namespace Logic
} // namespace PPTX
