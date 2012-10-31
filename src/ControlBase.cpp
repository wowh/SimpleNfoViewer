#include "stdafx.h"
#include "ControlBase.h"
      
ControlBase::WindowHandleToControlMap ControlBase::_controlMap;

ControlBase::ControlBase(void)
    :_handle(NULL),
     _oldProc(NULL)
{}

ControlBase::~ControlBase(void)
{
    UnregisterControl(_handle);
}

void ControlBase::RegisterControl(HWND hwnd)
{
    _handle = hwnd;

    WindowHandleToControlMap::iterator findIt = _controlMap.find(hwnd); 
    if (findIt != _controlMap.end())
    {
        return;
    }

    _controlMap[hwnd] = this;

    _oldProc = (WNDPROC)SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)BaseMessageProc);
}

void ControlBase::UnregisterControl(HWND hwnd)
{
    _controlMap.erase(hwnd);
}

ControlBase* ControlBase::GetControlBase(HWND hwnd)
{
    WindowHandleToControlMap::iterator findIt = _controlMap.find(hwnd); 
    if (findIt != _controlMap.end())
    {
        return findIt->second;
    }

    return NULL;
}

LRESULT ControlBase::BaseMessageProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    ControlBase* control = GetControlBase(hwnd);
    if (NULL != control)
    {
        return control->ControlMessageProc(hwnd, message, wParam, lParam);
    }

    return TRUE;
}