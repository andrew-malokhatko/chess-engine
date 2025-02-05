#include <windows.h>
#include "Clipboard.h"

namespace Clipboard
{
	std::string getBuffer()
	{
		if (!OpenClipboard(nullptr))
			return "";

		HANDLE hData = GetClipboardData(CF_TEXT);
		if (hData == nullptr)
			return "";

		char* pszText = static_cast<char*>(GlobalLock(hData));
		if (pszText == nullptr)
			return "";

		std::string text(pszText);

		GlobalUnlock(hData);
		CloseClipboard();

		return text;
	}
}