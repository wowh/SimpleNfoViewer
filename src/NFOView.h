#ifndef NFO_VIEW_H
#define NFO_VIEW_H

#include <string>

#include "ControlBase.h"
#include "StructDefine.h"

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
    void CheckScrollbar(void);
    void CheckSelect(void);
    void CopySelectedText(void);

    void DrawHyperlink(void);
    void DetectHyperlink(void);
    int  DetectHyperlinkEnd(const wchar_t* text, int textLength, int startOffset);

    bool IsHyperlink(POINTS& point);

    void onMouseMove(POINTS& point);
    void onSelectChanged(void);

private:
    std::wstring _fontName;
    int          _fontSize;
    HyperlinkOffsetVec _hyperlinkOffsets;
    std::wstring _nfoText;
};

#endif