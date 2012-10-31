#ifndef ICONTROL_BASE_H
#define ICONTROL_BASE_H

#include <map>

class ControlBase
{
public:
    ControlBase(void);
    ~ControlBase(void);

    HWND GetHandle(void) { return _handle; }

protected:
    virtual LRESULT ControlMessageProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) = 0;
    static LRESULT BaseMessageProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void RegisterControl(HWND hwnd);

protected:
    WNDPROC _oldProc;
    HWND _handle;

private:
    void UnregisterControl(HWND hwnd);

    static ControlBase* GetControlBase(HWND hwnd);
    typedef std::map<HWND, ControlBase*> WindowHandleToControlMap;
    static WindowHandleToControlMap _controlMap;
};


#endif