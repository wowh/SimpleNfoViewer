#ifndef NFO_VIEW_H
#define NFO_VIEW_H

#include <string>

#include "ControlBase.h"

class NFOView : public ControlBase
{
public:
    NFOView(HWND parentWindow);
    ~NFOView();

public:
    bool LoadFile(wchar_t* fileName);

    void SetFont(std::wstring fontName);
    int IncFontSize(void);
    int DecFontSize(void);

protected:
    LRESULT ControlMessageProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
    void AfterLoadFile(void);
    void AfterChangeFont(void);
    void ChangeFont(void);
    static void CheckScrollbar(HWND hwnd);
    static LRESULT ViewMessageProc(HWND hwnd,UINT message, WPARAM wParam, LPARAM lParam);
    static void CopySelectedText(HWND hwnd);
    static void DrawHyperlink(HWND hwnd);
    static void DetectHyperlink(wchar_t* text, int textLength, int start, int end);
    static int  DetectHyperlinkEnd(wchar_t* text, int textLength, int startOffset);

private:
    std::wstring _fontName;
    int          _fontSize;
};

#endif