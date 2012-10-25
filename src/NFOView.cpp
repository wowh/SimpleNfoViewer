#include "stdafx.h"
#include "NFOView.h"

#include "nfo2txt.h"

#define DEFAULT_FONT_SIZE 11

WNDPROC EditProc;

NFOView::NFOView(HWND parentWindow)
    :_fontSize(DEFAULT_FONT_SIZE), _windowHandle(NULL)
{
    RECT parentRect;
    GetClientRect(parentWindow, &parentRect);
    HINSTANCE inst = GetModuleHandle(0);

    _windowHandle = CreateWindowExW(WS_EX_CLIENTEDGE,
                                    L"EDIT",
                                    L"",
                                    WS_CHILD | WS_VISIBLE | WS_HSCROLL | ES_MULTILINE | ES_NOHIDESEL | ES_READONLY,
                                    parentRect.left, 
                                    parentRect.top,
                                    parentRect.right-parentRect.left,
                                    parentRect.bottom-parentRect.top,
                                    parentWindow, NULL, inst, NULL);

    if (NULL != _windowHandle)
    {
        EditProc = (WNDPROC)SetWindowLongPtrW(_windowHandle, GWLP_WNDPROC, (LONG_PTR)ViewMessageProc);
    }
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

    AfterLoadFile();

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

    SendMessage(_windowHandle, WM_SETFONT, (WPARAM)font, MAKELPARAM(TRUE, 0));
}

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

void NFOView::AfterLoadFile(void)
{
    CheckScrollbar(_windowHandle);
}

void NFOView::CheckScrollbar(HWND windowHandle)
{
    int lineCount = SendMessage(windowHandle, EM_GETLINECOUNT, NULL, NULL);

    RECT viewWindowRect;
    GetClientRect(windowHandle, &viewWindowRect);

    HDC viewWindowDC = GetDC(windowHandle);
    HFONT hFont = (HFONT)SendMessage(windowHandle, WM_GETFONT, NULL, NULL);
    SelectObject(viewWindowDC, hFont);
    TEXTMETRICW tm;
    GetTextMetricsW(viewWindowDC, &tm); 
    ReleaseDC(windowHandle, viewWindowDC);

    int lineCountOfViewWindow = viewWindowRect.bottom / tm.tmHeight;
    ShowScrollBar(windowHandle, SB_VERT, lineCount > lineCountOfViewWindow);
}

LRESULT NFOView::ViewMessageProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        case WM_LBUTTONDOWN:
            break;
        case WM_LBUTTONUP:
            CopySelectedText(hwnd);
            break;
        case WM_RBUTTONDOWN:
            break;
        case WM_SIZE:
            CheckScrollbar(hwnd);
            break;
    }

    return CallWindowProc(EditProc, hwnd, message, wParam, lParam);
}

void NFOView::CopySelectedText(HWND hwnd)
{
    DWORD selStart;
    DWORD selEnd;
    SendMessage(hwnd, EM_GETSEL, (WPARAM)&selStart, (LPARAM)&selEnd);

    if (selEnd > selStart)
    {
        int textLength = GetWindowTextLengthW(hwnd);
        if (textLength > 0)
        {
            HGLOBAL selTextHandle;
            selTextHandle = GlobalAlloc(GHND, sizeof(wchar_t)*(selEnd - selStart + 1));

            // copy selected text to memory
            wchar_t* selText = (wchar_t*)GlobalLock(selTextHandle);
            wchar_t* text = new wchar_t[textLength+1]();
            GetWindowTextW(hwnd, text, textLength);
            memcpy(selText, text+selStart, (selEnd-selStart)*sizeof(wchar_t));
            delete [] text;
            GlobalUnlock(selTextHandle);

            // copy to clipboard
            if (!OpenClipboard(hwnd))
            { 
                return;
            }
            EmptyClipboard();
            SetClipboardData(CF_UNICODETEXT, selTextHandle);
            CloseClipboard();
        }
    }
    
    // deselect text after copy
    SendMessage(hwnd, EM_SETSEL, (WPARAM)-1, (LPARAM)0);
}