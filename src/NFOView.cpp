#include "stdafx.h"
#include "NFOView.h"

#include <vector>
#include "nfo2txt.h"

#define DEFAULT_FONT_SIZE 11
#define HYPERLINK_START_MAX_LENGTH 16

WNDPROC EditProc;

struct HyperlinkOffset
{
    int start;
    int end;
};

static wchar_t HyperlinkStart[][HYPERLINK_START_MAX_LENGTH] =
{
    L"http://",
    L"mailto:",
    L"www."
};

typedef std::vector<HyperlinkOffset> HyperlinkOffsetVec;
static HyperlinkOffsetVec hyperlinkOffsets; 

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

void NFOView::AfterChangeFont(void)
{
    CheckScrollbar(_windowHandle);
}

void NFOView::ChangeFont(void)
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

    AfterChangeFont();
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
    DrawHyperlink(_windowHandle);
}

void NFOView::CheckScrollbar(HWND windowHandle)
{
    RECT viewWindowRect;
    GetClientRect(windowHandle, &viewWindowRect);

    HDC viewWindowDC = GetDC(windowHandle);
    HFONT hFont = (HFONT)SendMessage(windowHandle, WM_GETFONT, NULL, NULL);
    SelectObject(viewWindowDC, hFont);
    TEXTMETRICW tm;
    GetTextMetricsW(viewWindowDC, &tm); 
    ReleaseDC(windowHandle, viewWindowDC);

    // check vertical scrollbar
    int lineCount = SendMessage(windowHandle, EM_GETLINECOUNT, NULL, NULL);
    int lineCountOfViewWindow = viewWindowRect.bottom / tm.tmHeight;
    ShowScrollBar(windowHandle, SB_VERT, lineCount > lineCountOfViewWindow);

    // check horizontal scrollbar
    int maxLineLength = 0;
    for (int lineNum = 0; lineNum < lineCount; lineNum++)
    {
        int charIndex = SendMessage(windowHandle, EM_LINEINDEX, (WPARAM)lineNum, NULL);
        if (-1 != charIndex)
        {
            int lineLength = SendMessage(windowHandle, EM_LINELENGTH, (WPARAM)charIndex, NULL);
            if (lineLength > maxLineLength)
            {
                maxLineLength = lineLength;
            }
        }
    }
    int lineLengthOfViewWindow = viewWindowRect.right / tm.tmAveCharWidth;
    ShowScrollBar(windowHandle, SB_HORZ, maxLineLength > lineLengthOfViewWindow);
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
        case WM_HSCROLL:
        case WM_VSCROLL:
            DrawHyperlink(hwnd);
            break;
        case WM_PAINT:
            LRESULT result = CallWindowProc(EditProc, hwnd, message, wParam, lParam);
            DrawHyperlink(hwnd);
            return result;
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

void NFOView::DrawHyperlink(HWND hwnd)
{
    RECT viewWindowRect;
    GetClientRect(hwnd, &viewWindowRect);
    POINT leftTop = { viewWindowRect.left, viewWindowRect.top };
    POINT rightBottom = { viewWindowRect.right, viewWindowRect.bottom };

    int startCharIndex = LOWORD(SendMessage(hwnd, 
                                            EM_CHARFROMPOS,
                                            NULL,
                                            MAKELPARAM(viewWindowRect.left, viewWindowRect.top)));
    int endCharIndex = LOWORD(SendMessage(hwnd, 
                                          EM_CHARFROMPOS,
                                          NULL,
                                          MAKELPARAM(viewWindowRect.right, viewWindowRect.bottom)));

    int textLength = GetWindowTextLength(hwnd);
    if (textLength == 0)
    {
        return;
    }

    wchar_t* text = new wchar_t[textLength+1]();
    GetWindowTextW(hwnd, text, textLength);

    DetectHyperlink(text, textLength, startCharIndex, endCharIndex);

    HDC viewWindowDC = GetDC(hwnd);
    HFONT hFont = (HFONT)SendMessage(hwnd, WM_GETFONT, NULL, NULL);
    SelectObject(viewWindowDC, hFont);
    SetBkMode(viewWindowDC, TRANSPARENT);
    IntersectClipRect(viewWindowDC, viewWindowRect.left, viewWindowRect.top, viewWindowRect.right, viewWindowRect.bottom);
    SetTextColor(viewWindowDC, RGB(0, 102, 204));
    for (int i = 0; i < hyperlinkOffsets.size(); i++)
    {
        for (int charIndex  = hyperlinkOffsets[i].start; charIndex <= hyperlinkOffsets[i].end; charIndex++)
        {
            DWORD pos = SendMessage(hwnd, EM_POSFROMCHAR, (WPARAM)charIndex, NULL);
            POINT charPt = {LOWORD(pos), HIWORD(pos)};    
            TextOutW(viewWindowDC, charPt.x, charPt.y, text+charIndex, 1);
        }
    }

    ReleaseDC(hwnd, viewWindowDC);

    delete [] text;
}

bool IsHyperlinkStart(wchar_t* text, int textLength)
{
    for (int i = 0; i < sizeof(HyperlinkStart)/sizeof(HyperlinkStart[0]); i++)
    {
        int linkStartLen = wcslen(HyperlinkStart[i]);
        if (textLength < linkStartLen)
        {   continue; }    
        if (0 == _wcsnicmp(HyperlinkStart[i], text, linkStartLen))
        {   return true; }
    }

    return false;
}

void NFOView::DetectHyperlink(wchar_t* text, int textLength, int start, int end)
{
    hyperlinkOffsets.clear();

    for (int i = 0; i < textLength + 1; i++)
    {
        int remainLen = textLength - i;

        if (IsHyperlinkStart(text+i, remainLen))            
        { 
            HyperlinkOffset offset = {};
            offset.start = i;
            offset.end = DetectHyperlinkEnd(text, textLength, i+1);
            i = offset.end;
            hyperlinkOffsets.push_back(offset);
        }
    }
}

bool IsCharCanInHyperlink(wchar_t ch)
{
    if (ch >= 0x21 && ch <= 0x7e)
    {
        return true;
    }

    return false;
}

int NFOView::DetectHyperlinkEnd(wchar_t* text, int textLength, int startOffset)
{
    for (int i = startOffset; i < textLength + 1; i++)
    {
        if (!IsCharCanInHyperlink(text[i]))
        {
            return i - 1;
        }

        if (IsHyperlinkStart(text+i, textLength-i))
        {
            if ((startOffset+1 >= wcslen(L"http://")) && 
                (0 == _wcsnicmp(text+i-wcslen(L"http://"), 
                                L"http://",
                                wcslen(L"http://")))
               )
            { continue ;}
            return i - 1;
        }
    }
    
    return textLength;
}