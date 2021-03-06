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

#include "FilePass.h"
#include <Crypt/Decryptor.h>

namespace XLS
{


FilePass::FilePass()
{
}


FilePass::~FilePass()
{
}


BaseObjectPtr FilePass::clone()
{
	return BaseObjectPtr(new FilePass(*this));
}


void FilePass::writeFields(CFRecord& record)
{
#pragma message("####################### FilePass record is not implemented")
	Log::error("FilePass record is not implemented.");
	//record << some_value;
}


void FilePass::readFields(CFRecord& record)
{
	bool bEnabled = false;
	
	record >> wEncryptionType;
	
	if(!wEncryptionType)
	{
		record >> key;
		Log::info("FilePass: Encryption type: XOR");
		return;
	}
	else
	{
		bEnabled = true;

		majorVer = *record.getCurData<unsigned short>();
		
		if(0x0001 == majorVer) // RC4 encryption header structure
		{
			rc4HeaderPtr = CRYPTO::RC4EncryptionHeaderPtr(new CRYPTO::RC4EncryptionHeader());

			rc4HeaderPtr->load (record);

			record.getGlobalWorkbookInfo()->decryptor = 
				CRYPT::RC4DecryptorPtr(new CRYPT::RC4Decryptor(rc4HeaderPtr->RC4Data, record.getGlobalWorkbookInfo()->password, 2));
			
			Log::info("Encryption type: RC4 Standard");
		}
		else // RC4 CryptoAPI encryption header structuren
		{
			record >> rc4CryptoAPIHeader;
			Log::info("FilePass: Encryption type: RC4 Non-Standard");
		}
	}

	if (bEnabled == false && record.getGlobalWorkbookInfo()->decryptor)
		record.getGlobalWorkbookInfo()->decryptor.reset();

}

} // namespace XLS

