#include "stdafx.h"
#include "NFOView.h"

#include "nfo2txt.h"

#define DEFAULT_FONT_SIZE 11

NFOView::NFOView(HWND parentWindow)
    :_fontSize(DEFAULT_FONT_SIZE), _windowHandle(NULL)
{
    RECT parentRect;
    GetClientRect(parentWindow, &parentRect);
    HINSTANCE inst = GetModuleHandle(0);

    _windowHandle = CreateWindowExW(WS_EX_CLIENTEDGE,
                                    L"EDIT",
                                    L"",
                                    WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_NOHIDESEL | ES_READONLY,
                                    parentRect.left, 
                                    parentRect.top,
                                    parentRect.right-parentRect.left,
                                    parentRect.bottom-parentRect.top,
                                    parentWindow, NULL, inst, NULL);
}

NFOView::~NFOView()
{
    DestroyWindow(_windowHandle);
}

bool NFOView::LoadFile(wchar_t *fileName)
{
    HANDLE fileHandle;
    fileHandle = CreateFileW(fileName,
                             GENERIC_READ,
                             FILE_SHARE_READ,
                             NULL,
                             OPEN_EXISTING,
                             0,
                             NULL);
    if (INVALID_HANDLE_VALUE == fileHandle)
    {
        return false;
    }

    DWORD fileSize = GetFileSize(fileHandle, NULL);
    if (0xFFFFFFFF == fileSize)
    {
        CloseHandle(fileHandle);
        return false;
    }

    HANDLE heapHandle = GetProcessHeap();
    void* fileContents = HeapAlloc(heapHandle,
                                   HEAP_ZERO_MEMORY,
                                   fileSize + sizeof(wchar_t));
    if (NULL == fileContents)
    {
        CloseHandle(fileHandle);
        return false;
    }

    DWORD bytesReaded;
    if (!ReadFile(fileHandle, fileContents, fileSize, &bytesReaded, NULL))
    {
        CloseHandle(fileHandle);
        return false;
    }

    if (IsTextUnicode(fileContents, fileSize, NULL))
    {
        SetWindowTextW(_windowHandle, (wchar_t*)fileContents);
    }
    else
    {
        std::wstring nfoText = nfo2txt((char*)fileContents, fileSize);
        SetWindowTextW(_windowHandle, nfoText.c_str());
    }

    HeapFree(heapHandle, 0, fileContents);
    CloseHandle(fileHandle);

    return true;
}

void NFOView::ChangeFont()
{
	HFONT font = CreateFontW(_fontSize,
				       0,
					   0,
					   0,
					   FW_DONTCARE,
			           FALSE, FALSE, FALSE,
					   DEFAULT_CHARSET,
					   OUT_DEFAULT_PRECIS,
					   CLIP_DEFAULT_PRECIS,
				       DEFAULT_QUALITY,
				       DEFAULT_PITCH,
                       _fontName.c_str());

    SendMessage(_windowHandle, WM_SETFONT, (WPARAM)font, MAKELPARAM(TRUE, 0));}

void NFOView::SetFont(std::wstring fontName)
{
    _fontName = fontName;

    ChangeFont();
}

int NFOView::IncFontSize()
{
    _fontSize++;

    ChangeFont();

    return _fontSize;
}

int NFOView::DecFontSize()
{
    if (_fontSize > 1)
    {
        _fontSize--;
    }

    ChangeFont();
    return _fontSize;
}