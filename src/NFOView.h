#ifndef NFO_VIEW_H
#define NFO_VIEW_H

#include <string>

class NFOView
{
public:
    NFOView(HWND parentWindow);
    ~NFOView();

public:
    bool LoadFile(wchar_t* fileName);

    void SetFont(std::wstring fontName);
    int IncFontSize(void);
    int DecFontSize(void);

    HWND GetViewHandle(void) { return _windowHandle; }

private:
    void ChangeFont();

private:
    HWND         _windowHandle;
    std::wstring _fontName;
    int          _fontSize;
};

#endif