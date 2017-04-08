#if defined(UNICODE) && !defined(_UNICODE)
    #define _UNICODE
#elif defined(_UNICODE) && !defined(UNICODE)
    #define UNICODE
#endif

#include <tchar.h>
#include <windows.h>
#include<shlobj.h>

enum
{
    ID_WINRUN=0,
    ID_ENABLE=1,
    ID_EXIT=2
};


bool bEnable=false;
bool bWinRun=false;
HHOOK KeyBoardHook;
HINSTANCE hInstance;

char *PathIco,*PathExe;

void setPaths()
{
    PathIco = (char*) malloc(MAX_PATH);
    memset(PathIco,0,MAX_PATH);

    PathExe = (char*) malloc(MAX_PATH);
    memset(PathExe,0,MAX_PATH);

    strcat(PathIco,getenv("programfiles"));
    strcat(PathIco,"\\EasyKey\\key.ico");

    strcat(PathExe,getenv("programfiles"));
    strcat(PathExe,"\\EasyKey\\EasyKey.exe");
}

bool findWinRun()
{
    HKEY hKey;
    if(RegOpenKeyEx(HKEY_CURRENT_USER, "SOFTWARE\\MICROSOFT\\WINDOWS\\CurrentVersion\\Run", 0, KEY_ALL_ACCESS, &hKey)== ERROR_SUCCESS)
    {
        char buffer[255] = {0};
        DWORD dwBufferSize = sizeof(buffer);
        DWORD dwType = REG_SZ;
        if( RegQueryValueEx(hKey,"EasyKey",0,&dwType,(BYTE*)buffer,&dwBufferSize) == ERROR_SUCCESS )
        {
            RegCloseKey(hKey);
            return true;
        }
    }
    RegCloseKey(hKey);
    return false;
}

void enableWinRun()
{
    if(!findWinRun())
    {
        HKEY hKey;

        if(RegOpenKeyEx(HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_ALL_ACCESS, &hKey)== ERROR_SUCCESS)
        {
            LPCTSTR data = PathExe;
            RegSetValueEx (hKey, "EasyKey", 0, REG_SZ, (LPBYTE)data, strlen(data)+1);
            RegCloseKey(hKey);
        }
    }
}

void disableWinRun()
{
    HKEY hkey;
    RegOpenKey(HKEY_CURRENT_USER, "SOFTWARE\\MICROSOFT\\WINDOWS\\CurrentVersion\\Run", &hkey);
    RegDeleteValue(hkey, "EasyKey");
    RegCloseKey(hkey);
}



LRESULT CALLBACK HookKey(int nCode, WPARAM wParam, LPARAM lParam)
{
    PKBDLLHOOKSTRUCT Key = (PKBDLLHOOKSTRUCT) (lParam);
    if (wParam == WM_KEYDOWN)
    {
        if(Key->vkCode==VK_RCONTROL)
        {
            keybd_event(VK_SHIFT,0xa3,0, 0);
            return 3;
        }
    }
    if (wParam == WM_KEYUP)
    {
        if(Key->vkCode==VK_RCONTROL)
        {
            keybd_event(VK_SHIFT,0xA3, KEYEVENTF_KEYUP,0);
            return 3;
        }
    }
    return CallNextHookEx(NULL,nCode, wParam, lParam);
}

void EnableDisable()
{
    if(bEnable)
    {
        bEnable=false;
        UnhookWindowsHookEx(KeyBoardHook);
    }
    else
    {
        bEnable=true;
        KeyBoardHook =SetWindowsHookEx(WH_KEYBOARD_LL,HookKey,hInstance,0);
    }
}

BOOL ShowPopupMenu( HWND hWnd, POINT *curpos, int wDefaultItem )
{
    HMENU hPop = CreatePopupMenu();
    if(bWinRun){InsertMenu( hPop, 0, MF_BYPOSITION | MF_CHECKED , ID_WINRUN, "Ejecutar al inicio" );}
    else{InsertMenu( hPop, 0, MF_BYPOSITION | MF_UNCHECKED , ID_WINRUN, "Ejecutar al inicio" );}
    if(bEnable){InsertMenu( hPop, 1, MF_BYPOSITION | MF_CHECKED, ID_ENABLE, "Activado" );}
    else{InsertMenu( hPop, 1, MF_BYPOSITION | MF_UNCHECKED, ID_ENABLE, "Activado" );}
    InsertMenu( hPop, 2, MF_BYPOSITION | MF_STRING, ID_EXIT , "Cerrar" );


    SetMenuDefaultItem( hPop, ID_ENABLE, FALSE );
    SetFocus          ( hWnd );
    SendMessage       ( hWnd, WM_INITMENUPOPUP, (WPARAM)hPop, 0 );

    {
        POINT pt;
        if (!curpos)
        {
          GetCursorPos( &pt );
          curpos = &pt;
        }

        {
          WORD cmd = TrackPopupMenu( hPop, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY, curpos->x, curpos->y, 0, hWnd, NULL );
          SendMessage( hWnd, WM_COMMAND, cmd, 0 );
        }
      }
      DestroyMenu(hPop);
      return 0;
}


void AddTrayIcon( HWND hWnd, UINT uID, UINT uCallbackMsg, UINT uIcon )
{
    NOTIFYICONDATA  nid;
    nid.hWnd             = hWnd;
    nid.uID              = uID;
    nid.uFlags           = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = uCallbackMsg;

    ExtractIconEx( PathIco, 0, NULL, &(nid.hIcon), 1 );
    strcpy       (nid.szTip, "EasyKey © Lion - para mi hermosa, Te amo Sharon! <3");
    Shell_NotifyIcon( NIM_ADD, &nid );
}

LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);


TCHAR szClassName[ ] = _T("EasyKey_Lion");

int WINAPI WinMain (HINSTANCE hThisInstance,HINSTANCE hPrevInstance,LPSTR lpszArgument,int nCmdShow)
{
    HWND hwnd;
    MSG messages;
    WNDCLASSEX wincl;

    hInstance = hThisInstance;
    wincl.hInstance = hThisInstance;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WindowProcedure;
    wincl.style = CS_DBLCLKS;
    wincl.cbSize = sizeof (WNDCLASSEX);


    wincl.hIcon = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;
    wincl.cbClsExtra = 0;
    wincl.cbWndExtra = 0;

    wincl.hbrBackground = (HBRUSH) COLOR_BACKGROUND;

    if (!RegisterClassEx (&wincl))
        return 0;

    hwnd = CreateWindowEx(0,szClassName,_T("EasyKey - Lion"),WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,544,375,HWND_DESKTOP,NULL,hThisInstance,NULL);

    while (GetMessage (&messages, NULL, 0, 0))
    {
        TranslateMessage(&messages);
        DispatchMessage(&messages);
    }

    return messages.wParam;
}


LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_CREATE:
            EnableDisable();
            setPaths();
            if(findWinRun()){bWinRun=true;}else{bWinRun=false;}
            AddTrayIcon( hwnd, 1, WM_APP, 0 );
            break;

        case WM_COMMAND:
          switch (LOWORD(wParam))
          {

            case ID_ENABLE:
                EnableDisable();
                return 0;

            case ID_WINRUN:
                if(bWinRun){disableWinRun();}else{enableWinRun();}
                if(findWinRun()){bWinRun=true;}else{bWinRun=false;}
                return 0;

            case ID_EXIT:
              PostMessage( hwnd, WM_CLOSE, 0, 0 );
              return 0;
          }
          return 0;

        case WM_APP:
          switch (lParam)
          {
            case WM_RBUTTONUP:
              SetForegroundWindow( hwnd );
              ShowPopupMenu(hwnd, NULL, -1 );
              PostMessage( hwnd, WM_APP + 1, 0, 0 );
              return 0;
          }
          return 0;


        case WM_DESTROY:
            PostQuitMessage (0);
        default:
            return DefWindowProc (hwnd, message, wParam, lParam);
    }

    return 0;
}



