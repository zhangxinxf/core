#include "FontCidTT.h"
#include "Document.h"
#include "Streams.h"
#include "Utils.h"
#include "FontTTWriter.h"

#include "../../DesktopEditor/fontengine/FontManager.h"
#include "../../DesktopEditor/common/File.h"

#include <string>

#include FT_TRUETYPE_TABLES_H

namespace PdfWriter
{
	static const char* c_sToUnicodeHeader = "/CIDInit /ProcSet findresource begin\n12 dict begin\nbegincmap\n";
	static const char* c_sToUnicodeInfo = "/CIDSystemInfo\n<< /Registry (Adobe)\n /Ordering (UCS)\n /Supplement 0\n >> def\n/CMapName /Adobe-Identity-UCS def\n/CMapType 2 def\n1 begincodespacerange\n<0000> <FFFF>\nendcodespacerange\n";
	static const char* c_sToUnicodeFooter = "endcmap\nCMapName currentdict /CMap defineresource pop\nend\nend\n";

	static unsigned short GetGID(FT_Face pFace, unsigned int unUnicode)
	{
		int nCharIndex = 0;

		if (!pFace)
			return nCharIndex;

		for (int nIndex = 0; nIndex < pFace->num_charmaps; nIndex++)
		{
			FT_CharMap pCharMap = pFace->charmaps[nIndex];

			if (FT_Set_Charmap(pFace, pCharMap))
				continue;

			FT_Encoding pEncoding = pCharMap->encoding;

			if (FT_ENCODING_UNICODE == pEncoding)
			{
				if (nCharIndex = FT_Get_Char_Index(pFace, unUnicode))
					return nCharIndex;
			}

			if (FT_ENCODING_NONE == pEncoding || FT_ENCODING_MS_SYMBOL == pEncoding || FT_ENCODING_APPLE_ROMAN == pEncoding)
			{
				nCharIndex = FT_Get_Char_Index(pFace, unUnicode);
			}
			/*else if ( FT_ENCODING_ADOBE_STANDARD == pEncoding )
			{
			nCharIndex = FT_Get_Char_Index( pFace, unUnicode );
			}
			else if ( FT_ENCODING_ADOBE_CUSTOM == pEncoding )
			{
			nCharIndex = FT_Get_Char_Index( pFace, unUnicode );
			}
			else if ( FT_ENCODING_ADOBE_EXPERT == pEncoding )
			{
			nCharIndex = FT_Get_Char_Index( pFace, unUnicode );
			}*/
		}

		return nCharIndex;
	}
	static int            GetSymbolicCmapIndex(FT_Face pFace)
	{
		TT_OS2 *pOs2 = (TT_OS2 *)FT_Get_Sfnt_Table(pFace, ft_sfnt_os2);
		if (NULL == pOs2 || 0xFFFF == pOs2->version)
			return -1;

		// ��������� ���������� �� 31 ���
		if (!(pOs2->ulCodePageRange1 & 0x80000000) && !(pOs2->ulCodePageRange1 == 0 && pOs2->ulCodePageRange2 == 0))
			return -1;


		for (int nIndex = 0; nIndex < pFace->num_charmaps; nIndex++)
		{
			// Symbol
			if (0 == pFace->charmaps[nIndex]->encoding_id && 3 == pFace->charmaps[nIndex]->platform_id)
				return nIndex;
		}

		return -1;
	}
	//----------------------------------------------------------------------------------------
	// CFontFileBase
	//----------------------------------------------------------------------------------------
	CFontCidTrueType::CFontCidTrueType(CXref* pXref, CDocument* pDocument, const std::wstring& wsFontPath, unsigned int unIndex) : CFontDict(pXref, pDocument)
	{
		CFontFileTrueType* pFontTT = CFontFileTrueType::LoadFromFile(wsFontPath, unIndex);
		m_pFontFile = pFontTT;

		m_wsFontPath  = wsFontPath;
		m_unFontIndex = unIndex;

		Add("Type", "Font");
		Add("Subtype", "Type0");
		Add("Encoding", "Identity-H");

		CDictObject* pFont = new CDictObject();
		m_pXref->Add(pFont);

		CArrayObject* pDescendantFonts = new CArrayObject();
		pDescendantFonts->Add(pFont);

		Add("DescendantFonts", pDescendantFonts);
		CDictObject* pToUnicodeDict = new CDictObject(m_pXref, true);
		Add("ToUnicode", pToUnicodeDict);
		pToUnicodeDict->SetFilter(STREAM_FILTER_FLATE_DECODE);
		m_pToUnicodeStream = pToUnicodeDict->GetStream();

		CreateCIDFont2(pFont);

		m_pFace         = NULL;
		m_pFaceMemory   = NULL;	
		m_nGlyphsCount  = 0;
		m_nSymbolicCmap = -1;
	}
	CFontCidTrueType::~CFontCidTrueType()
	{
		if (m_pFontFile)
			delete m_pFontFile;

		if (m_pFace)
			FT_Done_Face(m_pFace);

		if (m_pFaceMemory)
			delete[] m_pFaceMemory;
	}
	void           CFontCidTrueType::CreateCIDFont2(CDictObject* pFont)
	{
		m_pFont = pFont;
		pFont->Add("Subtype", "CIDFontType2");

		CDictObject* pSystemInfo = new CDictObject();
		pSystemInfo->Add("Registry", new CStringObject("Adobe"));
		pSystemInfo->Add("Ordering", new CStringObject("Identity"));
		pSystemInfo->Add("Supplement", 0);

		pFont->Add("CIDSystemInfo", pSystemInfo);

		CDictObject* pFontDescriptor = new CDictObject();
		// FontDescriptor ����������� ������ ���� �������
		m_pXref->Add(pFontDescriptor);
		pFontDescriptor->Add("Type", "FontDescriptor");
		m_pFontDescriptor = pFontDescriptor;

		// ���������� ��� Symbolic, � ��� NonSymbolic �������
		unsigned int nFlags = 0;
		if (!(nFlags & 4))
			UIntChangeBit(nFlags, 2);
		if (nFlags & 32)
			UIntChangeBit(nFlags, 5);

		pFontDescriptor->Add("Flags", nFlags);

		CArrayObject* pBBox = new CArrayObject();
		int* pFontBBox = m_pFontFile->GetBBox();
		pBBox->Add(pFontBBox[0]);
		pBBox->Add(pFontBBox[1]);
		pBBox->Add(pFontBBox[2]);
		pBBox->Add(pFontBBox[3]);
		pFontDescriptor->Add("FontBBox", pBBox);
		pFontDescriptor->Add("ItalicAngle", 0);
		pFontDescriptor->Add("Ascent", m_pFontFile->GetAscent());
		pFontDescriptor->Add("Descent", m_pFontFile->GetDescent());
		pFontDescriptor->Add("CapHeight", m_pFontFile->GetCapHeight());
		pFontDescriptor->Add("StemV", 0);
		pFontDescriptor->Add("FontWeight", m_pFontFile->GetWeight());

		m_pFontFileDict = new CDictObject(m_pXref, true);
		pFontDescriptor->Add("FontFile2", m_pFontFileDict);

		pFont->Add("FontDescriptor", pFontDescriptor);

		CDictObject* pCIDToGIDMapDict = new CDictObject(m_pXref, true);
		pFont->Add("CIDToGIDMap", pCIDToGIDMapDict);
		pCIDToGIDMapDict->SetFilter(STREAM_FILTER_FLATE_DECODE);
		m_pCidToGidMapStream = pCIDToGIDMapDict->GetStream();
	}
	unsigned char* CFontCidTrueType::EncodeString(unsigned int* pUnicodes, unsigned int unLen)
	{
		if (!OpenFontFace())
			return NULL;

		unsigned char* pEncodedString = new unsigned char[unLen * 2];
		if (!pEncodedString)
			return NULL;

		// ��������� �������� �� �������� � ���� ����������� ���� ��������������� �� 0x0000..0xFFFF
		// ��� ������� ���������� �������� ������� ��������������� Gid � ������.
		// ��� ������� ���� �������� ������ �������.
		for (unsigned int unIndex = 0; unIndex < unLen; unIndex++)
		{
			std::map<unsigned int, unsigned short>::const_iterator oIter = m_mUnicodeToCode.find(pUnicodes[unIndex]);
			unsigned short ushCode;
			if (oIter != m_mUnicodeToCode.end())
			{
				ushCode = oIter->second;
			}
			else
			{
				unsigned int unUnicode = pUnicodes[unIndex];
				ushCode = m_ushCodesCount;
				m_ushCodesCount++;

				m_mUnicodeToCode.insert(std::pair<unsigned int, unsigned short>(unUnicode, ushCode));				
				m_vUnicodes.push_back(unUnicode);

				unsigned short ushGID = GetGID(m_pFace, unUnicode);
				if (0 == ushGID && -1 != m_nSymbolicCmap)
					ushGID = GetGID(m_pFace, unUnicode + 0xF000);

				m_vCodeToGid.push_back(ushGID);

				// ������ ������ ������������
				m_mGlyphs.insert(std::pair<unsigned short, bool>(ushGID, true));

				// ���� ������ ������ ��������� (CompositeGlyf), ����� �� ������ ������ ��� ��� �������� ������� (subglyfs)
				if (0 == FT_Load_Glyph(m_pFace, ushGID, FT_LOAD_NO_SCALE | FT_LOAD_NO_RECURSE))
				{
					for (int nSubIndex = 0; nSubIndex < m_pFace->glyph->num_subglyphs; nSubIndex++)
					{
						FT_Int       nSubGID;
						FT_UInt      unFlags;
						FT_Int       nArg1;
						FT_Int       nArg2;
						FT_Matrix    oMatrix;
						FT_Get_SubGlyph_Info(m_pFace->glyph, nSubIndex, &nSubGID, &unFlags, &nArg1, &nArg2, &oMatrix);

						m_mGlyphs.insert(std::pair<unsigned short, bool>(nSubGID, true));
					}

					if (0 != m_pFace->units_per_EM)
						m_vWidths.push_back((unsigned int)m_pFace->glyph->metrics.horiAdvance * 1000 / m_pFace->units_per_EM);
					else
						m_vWidths.push_back((unsigned int)m_pFace->glyph->metrics.horiAdvance);
				}
			}

			pEncodedString[2 * unIndex + 0] = (ushCode >> 8) & 0xFF;
			pEncodedString[2 * unIndex + 1] = ushCode & 0xFF;
		}
		return pEncodedString;
	}
	unsigned int   CFontCidTrueType::GetWidth(unsigned short ushCode)
	{
		if (ushCode >= m_vWidths.size())
			return 0;

		return m_vWidths.at(ushCode);
	}
	void           CFontCidTrueType::BeforeWrite()
	{
		if (m_pFontFile)
		{
			unsigned short* pCodeToGid;
			unsigned int*   pWidths;
			unsigned char*  pGlyphs;
			unsigned int    unGlyphsCount;

			if (!GetWidthsAndGids(&pCodeToGid, &pWidths, &pGlyphs, unGlyphsCount))
				return;

			CStream* pStream = m_pFontFileDict->GetStream();
			m_pFontFile->WriteTTF(pStream, NULL, pCodeToGid, m_ushCodesCount, pGlyphs, unGlyphsCount);
			m_pFontFileDict->Add("Length1", pStream->Size());
			m_pFontFileDict->SetFilter(STREAM_FILTER_FLATE_DECODE);

			CArrayObject* pWArray = new CArrayObject();
			m_pFont->Add("W", pWArray);
			pWArray->Add(0);
			CArrayObject* pWidthsArray = new CArrayObject();
			pWArray->Add(pWidthsArray);
			for (unsigned short ushIndex = 0; ushIndex < m_ushCodesCount; ushIndex++)
			{
				pWidthsArray->Add(pWidths[ushIndex]);
			}

			pStream = m_pCidToGidMapStream;
			for (unsigned short ushCode = 0; ushCode < m_ushCodesCount; ushCode++)
			{
				unsigned short ushGid = pCodeToGid[ushCode];
				pStream->WriteChar(((ushGid >> 8) & 0xFF));
				pStream->WriteChar((ushGid & 0xFF));
			}

			RELEASEARRAYOBJECTS(pCodeToGid);
			RELEASEARRAYOBJECTS(pWidths);
			RELEASEARRAYOBJECTS(pGlyphs);

			WriteToUnicode();
		}
	}
	bool           CFontCidTrueType::GetWidthsAndGids(unsigned short** ppCodeToGid, unsigned int** ppWidths, unsigned char** ppGlyphs, unsigned int& unGlyphsCount)
	{
		*ppCodeToGid  = NULL;
		*ppWidths     = NULL;
		*ppGlyphs     = NULL;
		unGlyphsCount = 0;
		
		if (!m_nGlyphsCount)
			return false;

		unsigned short* pCodeToGID = new unsigned short[m_ushCodesCount];
		if (!pCodeToGID)
			return false;

		unsigned int* pWidths = new unsigned int[m_ushCodesCount];
		if (!pWidths)
		{
			delete[] pCodeToGID;
			return false;
		}
		memset((void*)pWidths, 0x00, m_ushCodesCount * sizeof(unsigned int));

		for (unsigned short ushCode = 0; ushCode < m_ushCodesCount; ushCode++)
		{
			pCodeToGID[ushCode] = m_vCodeToGid.at(ushCode);
			pWidths[ushCode]    = m_vWidths.at(ushCode);
		}

		unsigned char *pGlyphs = new unsigned char[m_nGlyphsCount];
		if (!pGlyphs)
		{
			delete[] pCodeToGID;
			delete[] pWidths;
			return false;
		}

		memset((void *)pGlyphs, 0x00, m_nGlyphsCount * sizeof(unsigned char));
		for (auto oIt : m_mGlyphs)
		{
			pGlyphs[oIt.first] = 1;
		}

		*ppCodeToGid  = pCodeToGID;
		*ppWidths     = pWidths;
		*ppGlyphs     = pGlyphs;
		unGlyphsCount = m_nGlyphsCount;
		return true;
	}
	void           CFontCidTrueType::WriteToUnicode()
	{
		CStream* pS = m_pToUnicodeStream;

		pS->WriteStr(c_sToUnicodeHeader);
		pS->WriteStr(c_sToUnicodeInfo);

		pS->WriteInt(m_ushCodesCount);
		pS->WriteStr(" beginbfchar\n");
		for (unsigned short ushCode = 0; ushCode < m_ushCodesCount; ushCode++)
		{
			unsigned int unUnicode = m_vUnicodes[ushCode];
			pS->WriteChar('<');
			pS->WriteHex(ushCode, 4);
			pS->WriteStr("> <");

			if (unUnicode < 0x10000)
			{
				pS->WriteHex(unUnicode, 4);
			}
			else
			{
				unUnicode = unUnicode - 0x10000;

				unsigned short ushLo = 0xDC00 | (unUnicode & 0x3FF);
				unsigned short ushHi = 0xD800 | (unUnicode >> 10);

				pS->WriteHex(ushHi, 4);
				pS->WriteHex(ushLo, 4);
			}

			pS->WriteStr(">\n");
		}
		pS->WriteStr("endbfchar\n");
		m_pToUnicodeStream->WriteStr(c_sToUnicodeFooter);
	}
	void           CFontCidTrueType::CloseFontFace()
	{
		if (m_pFace)
		{
			FT_Done_Face(m_pFace);
			m_pFace = NULL;
		}

		RELEASEARRAYOBJECTS(m_pFaceMemory);
	}
	bool           CFontCidTrueType::OpenFontFace()
	{
		if (m_pFace)
		{
			m_pDocument->AddFreeTypeFont(this);
			return true;
		}

		m_nGlyphsCount  = 0;
		m_nSymbolicCmap = -1;

		FT_Library pLibrary = m_pDocument->GetFreeTypeLibrary();
		if (!pLibrary)
			return false;

		DWORD dwFileSize;
		m_pFaceMemory = NULL;
		NSFile::CFileBinary::ReadAllBytes(m_wsFontPath, &m_pFaceMemory, dwFileSize);
		if (!m_pFaceMemory)
			return false;

		FT_New_Memory_Face(pLibrary, m_pFaceMemory, dwFileSize, m_unFontIndex, &m_pFace);
		if (!m_pFace)
		{
			RELEASEARRAYOBJECTS(m_pFaceMemory);
			return false;
		}

		m_pDocument->AddFreeTypeFont(this);
		m_nGlyphsCount  = m_pFace->num_glyphs;
		m_nSymbolicCmap = GetSymbolicCmapIndex(m_pFace);
		return true;
	}
}