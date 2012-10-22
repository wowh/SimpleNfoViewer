// listplug.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "cunicode.h"
#include "ListPlugin.h"
#include "NFOView.h"

#define SupportedExtension1 L".nfo"
#define SupportedExtension2 L".diz"
#define parsefunction "ext=\"NFO\" | ext=\"DIZ\""

HINSTANCE hinst;
char ConfigFileName[MAX_PATH]="plugin.ini";  // Unused in this plugin, may be used to save data
NFOView* viewWindow = NULL;

BOOL APIENTRY DllMain(HANDLE hModule, 
                      DWORD  ul_reason_for_call, 
                      LPVOID lpReserved)
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			hinst=(HINSTANCE)hModule;
			break;
		case DLL_PROCESS_DETACH:
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
    }
    return TRUE;
}

char* strlcpy(char* p,char*p2,int maxlen)
{
	if ((int)strlen(p2)>=maxlen) {
		strncpy(p,p2,maxlen);
		p[maxlen]=0;
	} else
		strcpy(p,p2);
	return p;
}

BOOL __stdcall LoadFile(WCHAR* FileName, HWND NfoViewHandle);

int __stdcall ListNotificationReceived(HWND ListWin,int Message,WPARAM wParam,LPARAM lParam)
{
	return 0;
}

HWND __stdcall ListLoadW(HWND ParentWin, WCHAR* FileToLoad, int ShowFlags)
{
	WCHAR *p;
    HWND viewHandle = NULL;

	if (ShowFlags & lcp_forceshow == 0) {  // don't check extension in this case!
		p = wcsrchr(FileToLoad, '\\');
		if (!p)
			return NULL;
		p = wcsrchr(p, '.');
		if (!p || (wcsicmp(p, SupportedExtension1) != 0 && wcsicmp(p, SupportedExtension2) != 0))
			return NULL;
	}

    viewWindow = new NFOView(ParentWin);

	if (viewWindow->GetViewHandle()) {
        viewHandle = viewWindow->GetViewHandle();

		PostMessage(ParentWin, WM_COMMAND, MAKELONG(lcp_ansi,itm_fontstyle), (LPARAM)viewHandle);

    	viewWindow->SetFont(L"Lucida Console");
			
		bool loadRet = viewWindow->LoadFile(FileToLoad);

		if (!loadRet) {
			delete viewWindow;
			viewHandle=NULL;
		}
        else
        {
    		ShowWindow(viewHandle, SW_SHOW);
        }
	}

	return viewHandle;
}

HWND __stdcall ListLoad(HWND ParentWin,char* FileToLoad,int ShowFlags)
{
	WCHAR FileToLoadW[wdirtypemax];
	return ListLoadW(ParentWin,awfilenamecopy(FileToLoadW,FileToLoad),ShowFlags);
}

int __stdcall ListLoadNextW(HWND ParentWin,HWND ListWin,WCHAR* FileToLoad,int ShowFlags)
{
	return LISTPLUGIN_OK;
}

int __stdcall ListLoadNext(HWND ParentWin,HWND ListWin,char* FileToLoad,int ShowFlags)
{
	WCHAR FileToLoadW[wdirtypemax];
	return ListLoadNextW(ParentWin,ListWin,awfilenamecopy(FileToLoadW,FileToLoad),ShowFlags);
}

int __stdcall ListSendCommand(HWND ListWin,int Command,int Parameter)
{
	switch (Command) {
	case lc_copy:
		SendMessage(ListWin, WM_COPY, 0, 0);
		return LISTPLUGIN_OK;
	case lc_newparams:
		PostMessage(GetParent(ListWin), WM_COMMAND, MAKELONG(0, itm_next), (LPARAM)ListWin);
		return LISTPLUGIN_ERROR;
	case lc_selectall:
		SendMessage(ListWin, EM_SETSEL, 0, -1);
		return LISTPLUGIN_OK;
	}
	return LISTPLUGIN_ERROR;
}

int _stdcall ListSearchText(HWND ListWin,char* SearchString,int SearchParameter)
{
	return LISTPLUGIN_OK;
}

void __stdcall ListCloseWindow(HWND ListWin)
{
	DestroyWindow(ListWin);
    delete viewWindow;
	return;
}

int __stdcall ListPrint(HWND ListWin,char* FileToPrint,char* DefPrinter,int PrintFlags,RECT* Margins)
{
	return 0;
}

void __stdcall ListGetDetectString(char* DetectString,int maxlen)
{
	strlcpy(DetectString,parsefunction,maxlen);
}

void __stdcall ListSetDefaultParams(ListDefaultParamStruct* dps)
{
	strlcpy(ConfigFileName,dps->DefaultIniName,MAX_PATH-1);
}

HBITMAP __stdcall ListGetPreviewBitmapW(WCHAR* FileToLoad,int width,int height,
    char* contentbuf,int contentbuflen)
{
	return NULL;
}

HBITMAP __stdcall ListGetPreviewBitmap(char* FileToLoad,int width,int height,
    char* contentbuf,int contentbuflen)
{
	WCHAR FileToLoadW[wdirtypemax];
	return ListGetPreviewBitmapW(FileToLoadW,width,height,
		contentbuf,contentbuflen);
}

int _stdcall ListSearchDialog(HWND ListWin,int FindNext)
{
	return LISTPLUGIN_ERROR; 
}
