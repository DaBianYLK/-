www.pudn.com > D3DScreen.rar > MultiScreen.cpp, change:2009-02-12,size:14583b

 
  Search codes 

  
 #include "stdafx.h"   
#include "MultiScreen.h"   
#include <mmsystem.h>   
   
#define D3DMAIN_CALSS "d3dmain_class"   
#define D3DCHILD_CLASS "d3dchild_class"   
#define D3DMAIN_WINDOW "d3dmain_window"   
#define D3DCHILD_WINDOW "d3dchild_window"   
   
CD3DMultiScreen * CD3DMultiScreen::m_pMultiScreen = NULL;   
   
#pragma comment(lib, "winmm.lib")   
#pragma comment(lib, "d3dx9.lib")   
#pragma comment(lib, "d3d9.lib")   
   
struct CUSTOMVERTEX   
{   
    FLOAT x, y, z;      // The untransformed, 3D position for the vertex   
    DWORD color;        // The vertex color   
};   
   
// Our custom FVF, which describes our custom vertex structure   
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE)   
   
//struct CUSTOMVERTEX   
//{   
//  FLOAT x, y, z, rhw; // The transformed position for the vertex   
//  DWORD color;        // The vertex color   
//};   
//   
//// Our custom FVF, which describes our custom vertex structure   
//#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZRHW|D3DFVF_DIFFUSE)   
   
CD3DMultiScreen::CD3DMultiScreen()   
{   
    m_pMultiScreen = this;   
   
    memset(&m_AdapterInfo, 0, sizeof(ADAPTERMONITORINFO));   
    m_hModule = LoadLibrary("USER32");   
    m_pfEnumDisplayDevices = (winEnumDisplayDevices)GetProcAddress(m_hModule, "EnumDisplayDevicesA");   
   
    m_pD3D = NULL ;   
    int i = 0;   
    for( i = 0; i < VT_MAX_MONITOR_COUNT; i++ )   
    {   
        m_pD3DDevice[i] = NULL;    
   
        m_AdapterInfo.monitorInfo[i].cbSize = sizeof(MONITORINFO);   
    }   
   
    m_nDesktopWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);   
    m_nDesktopHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);   
}   
   
CD3DMultiScreen::~CD3DMultiScreen()   
{   
    int i = 0;   
    for( i = 0; i < VT_MAX_MONITOR_COUNT ; i++ )   
    {   
        if( m_pD3DDevice[i] != NULL )   
        {   
            m_pD3DDevice[i]->Release();   
            m_pD3DDevice[i] = NULL ;   
        }   
    }   
   
    if( m_pD3D != NULL )   
    {   
        m_pD3D->Release();   
        m_pD3D = NULL ;   
    }   
}   
   
void CD3DMultiScreen::EnumMonitors()   
{   
    DISPLAY_DEVICE dispDev;   
    DISPLAY_DEVICE dispDev2;   
    memset(&dispDev, 0, sizeof(DISPLAY_DEVICE));   
    memset(&dispDev2, 0, sizeof(DISPLAY_DEVICE));   
    dispDev.cb = sizeof(DISPLAY_DEVICE);   
    dispDev2.cb= sizeof(DISPLAY_DEVICE);   
   
    DWORD iDevNum = 0;   
    int   nMonitors = 0;   
    DEVMODE devMode;   
    memset(&devMode, 0, sizeof(DEVMODE));   
   
    devMode.dmSize = sizeof(DEVMODE);   
   
    while(m_pfEnumDisplayDevices(NULL, iDevNum, &dispDev, 0 ) )   
    {   
        if( (dispDev.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER) == 0 )   
        {   
            strcpy(m_AdapterInfo.szAdapterName[nMonitors], dispDev.DeviceString);   
            m_pfEnumDisplayDevices(dispDev.DeviceName, 0, &dispDev2, 0);   
            strcpy(m_AdapterInfo.szMonitorName[nMonitors], dispDev2.DeviceString);   
            EnumDisplaySettings(dispDev.DeviceName, ENUM_CURRENT_SETTINGS, &devMode);   
   
            if( dispDev.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE )   
            {   
                m_AdapterInfo.monitorInfo[nMonitors].rcMonitor.left = 0;   
                m_AdapterInfo.monitorInfo[nMonitors].rcMonitor.top = 0;   
                m_AdapterInfo.monitorInfo[nMonitors].rcWork.left = 0;   
                m_AdapterInfo.monitorInfo[nMonitors].rcWork.top = 0;   
            }   
            else   
            {   
                m_AdapterInfo.monitorInfo[nMonitors].rcMonitor.left = devMode.dmPosition.x;   
                m_AdapterInfo.monitorInfo[nMonitors].rcMonitor.top = devMode.dmPosition.y;   
                m_AdapterInfo.monitorInfo[nMonitors].rcWork.left = devMode.dmPosition.x;   
                m_AdapterInfo.monitorInfo[nMonitors].rcWork.top = devMode.dmPosition.y;   
            }   
   
            m_AdapterInfo.monitorInfo[nMonitors].rcMonitor.right = m_AdapterInfo.monitorInfo[nMonitors].rcMonitor.left + devMode.dmPelsWidth;   
            m_AdapterInfo.monitorInfo[nMonitors].rcMonitor.bottom = m_AdapterInfo.monitorInfo[nMonitors].rcMonitor.top + devMode.dmPelsHeight;   
            m_AdapterInfo.monitorInfo[nMonitors].rcWork.right = m_AdapterInfo.monitorInfo[nMonitors].rcWork.left + devMode.dmPelsWidth;   
            m_AdapterInfo.monitorInfo[nMonitors].rcWork.bottom = m_AdapterInfo.monitorInfo[nMonitors].rcWork.top + devMode.dmPelsHeight;   
   
            m_AdapterInfo.hMonitor[nMonitors] = MonitorFromRect(&m_AdapterInfo.monitorInfo[nMonitors].rcMonitor, MONITOR_DEFAULTTONULL);   
            nMonitors++;   
        }   
        iDevNum++;   
    }   
   
    m_AdapterInfo.nMonitors = nMonitors;   
   
}   
   
void CD3DMultiScreen::AdjustWindowPos()   
{   
    RECT rect;   
    GetClientRect(m_hWindow, &rect);   
   
    ClientToScreen(m_hWindow, (LPPOINT)&rect.left);   
    int norgX = rect.left;   
    int norgY = rect.top;   
   
    int i = 0;   
   
    for( i = 0; i < m_AdapterInfo.nMonitors; i++ )   
    {   
        if( IsWindow(m_AdapterInfo.hWnd[i]))   
        {   
            MoveWindow(m_AdapterInfo.hWnd[i],   
                m_AdapterInfo.monitorInfo[i].rcMonitor.left - norgX, m_AdapterInfo.monitorInfo[i].rcMonitor.top - norgY,   
                m_AdapterInfo.monitorInfo[i].rcMonitor.right - m_AdapterInfo.monitorInfo[i].rcMonitor.left,   
                m_AdapterInfo.monitorInfo[i].rcMonitor.bottom - m_AdapterInfo.monitorInfo[i].rcMonitor.top, FALSE);   
        }   
    }   
}   
   
LRESULT CALLBACK CD3DMultiScreen::s_MainProc(HWND h, UINT uMsg, WPARAM wParam, LPARAM lParam)   
{   
    if( m_pMultiScreen )   
    {   
        return m_pMultiScreen->MainProc(h, uMsg, wParam, lParam);   
    }   
    return ::DefWindowProc(h, uMsg, wParam, lParam);   
}   
   
LRESULT CALLBACK CD3DMultiScreen::s_ChildProc(HWND h, UINT uMsg, WPARAM wParam, LPARAM lParam)   
{   
    if( m_pMultiScreen )   
    {   
        return m_pMultiScreen->ChildProc(h, uMsg, wParam, lParam);   
    }   
   
    return ::DefWindowProc(h, uMsg, wParam, lParam);   
}   
   
void CD3DMultiScreen::SetHinstance(HINSTANCE h)   
{   
    m_hInstance = h;   
}   
   
LRESULT CD3DMultiScreen::MainProc(HWND h, UINT uMsg, WPARAM wParam, LPARAM lParam)   
{   
    if( uMsg == WM_MOVE )   
    {   
        AdjustWindowPos();   
    }   
    else if( uMsg == WM_CLOSE )   
    {   
        DestroyWindow(h);   
        PostQuitMessage(0);   
    }   
    return ::DefWindowProc(h, uMsg, wParam, lParam);   
}   
   
LRESULT CD3DMultiScreen::ChildProc(HWND h, UINT uMsg, WPARAM wParam, LPARAM lParam)   
{   
    return ::DefWindowProc(h, uMsg, wParam, lParam);   
}   
   
BOOL CD3DMultiScreen::CreateMultiWindow()   
{   
    int nDesktopWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);   
    int nDesktopHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);   
   
    WNDCLASSEX wndclass;   
    wndclass.cbClsExtra = 0;   
    wndclass.cbSize = sizeof(WNDCLASSEX);   
    wndclass.cbWndExtra = 0;   
    wndclass.hbrBackground = (HBRUSH)WHITE_BRUSH;   
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);   
    wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);   
    wndclass.hIconSm = NULL;   
    wndclass.hInstance = m_hInstance;   
    wndclass.lpfnWndProc = s_MainProc;   
    wndclass.lpszClassName = D3DMAIN_CALSS;   
    wndclass.lpszMenuName = NULL;   
    wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;   
   
    RegisterClassEx(&wndclass);   
    wndclass.hIcon = NULL;   
    wndclass.lpfnWndProc = s_ChildProc;   
    wndclass.lpszClassName = D3DCHILD_CLASS;   
    RegisterClassEx(&wndclass);   
   
    int x = (nDesktopWidth - 800) / 2;   
    int y = (nDesktopHeight - 600) / 2;   
   
    m_hWindow = CreateWindow(D3DMAIN_CALSS, D3DMAIN_WINDOW, WS_OVERLAPPEDWINDOW, x, y, 800,   
        600, NULL, NULL, m_hInstance, NULL);   
   
    int i = 0;   
    for( i = 0; i < m_AdapterInfo.nMonitors; i++ )   
    {   
        m_AdapterInfo.hWnd[i] = CreateWindow(D3DCHILD_CLASS, D3DCHILD_WINDOW, WS_CHILD, 0, 0,CW_USEDEFAULT, CW_USEDEFAULT,    
            m_hWindow, NULL, m_hInstance, NULL);   
    }   
   
    AdjustWindowPos();   
   
    for( i = 0; i < m_AdapterInfo.nMonitors; i++ )   
    {   
        ShowWindow(m_AdapterInfo.hWnd[i], SW_HIDE);   
        //UpdateWindow(m_AdapterInfo.hWnd[i]);   
    }   
   
    ShowWindow(m_hWindow, SW_SHOW);   
    UpdateWindow(m_hWindow);   
   
    return TRUE;   
}   
   
BOOL CD3DMultiScreen::InitializeEnvironment()   
{   
    CUSTOMVERTEX vertices[] =   
    {   
        { -1.0f,-1.0f, 0.0f, 0xffff0000, },   
        {  1.0f,-1.0f, 0.0f, 0xff0000ff, },   
        {  0.0f, 1.0f, 0.0f, 0xffffffff, },   
    };   
   
    //CUSTOMVERTEX vertices[] =   
    //{   
    //  { 150.0f,  50.0f, 0.5f, 1.0f, 0xffff0000, }, // x, y, z, rhw, color   
    //  { 250.0f, 250.0f, 0.5f, 1.0f, 0xff00ff00, },   
    //  {  50.0f, 250.0f, 0.5f, 1.0f, 0xff00ffff, },   
    //};   
   
    int nAdapter = 0;   
    int nAdapterCount = 0;   
   
    D3DPRESENT_PARAMETERS d3dParameter;   
    memset(&d3dParameter, 0, sizeof(D3DPRESENT_PARAMETERS));   
   
    d3dParameter.Windowed = TRUE;   
    d3dParameter.SwapEffect = D3DSWAPEFFECT_DISCARD;   
    d3dParameter.BackBufferFormat = D3DFMT_UNKNOWN ;   
   
    m_pD3D = Direct3DCreate9(D3D_SDK_VERSION);   
   
    nAdapterCount = m_pD3D->GetAdapterCount();   
   
    int i = 0;   
    for( nAdapter = 0; nAdapter < nAdapterCount; nAdapter++ )   
    {   
        HMONITOR h = m_pD3D->GetAdapterMonitor(nAdapter);   
           
        for( i = 0;i < m_AdapterInfo.nMonitors; i++ )   
        {   
            if( m_AdapterInfo.hMonitor[i] == h )   
            {   
                m_AdapterInfo.vAdapter[i] = nAdapter;   
                break;   
            }   
        }   
    }   
   
    D3DCAPS9 d3dCaps;   
    memset(&d3dCaps, 0, sizeof(D3DCAPS9));   
   
    D3DDISPLAYMODE d3dMode;   
   
    D3DDEVTYPE d3dType;   
    DWORD dwBehavior;   
   
    for( i = 0; i < m_AdapterInfo.nMonitors; i++ )   
    {   
        m_pD3D->GetAdapterDisplayMode(m_AdapterInfo.vAdapter[i], &d3dMode);   
        if( m_pD3D->CheckDeviceType(m_AdapterInfo.vAdapter[i], D3DDEVTYPE_HAL, d3dMode.Format, d3dMode.Format, TRUE) == D3D_OK)   
        {   
            d3dType = D3DDEVTYPE_HAL;   
        }   
        else   
        {   
            d3dType = D3DDEVTYPE_REF ;   
        }   
   
        m_pD3D->GetDeviceCaps(m_AdapterInfo.vAdapter[i], d3dType, &d3dCaps);   
        if( d3dCaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT )   
        {   
            dwBehavior = D3DCREATE_HARDWARE_VERTEXPROCESSING ;   
        }   
        else   
        {   
            dwBehavior = D3DCREATE_SOFTWARE_VERTEXPROCESSING ;   
        }   
   
        m_pD3D->CreateDevice(m_AdapterInfo.vAdapter[i], d3dType, m_hWindow,/*m_AdapterInfo.hWnd[i],*/   
            dwBehavior, &d3dParameter, &m_pD3DDevice[i]);   
   
        m_pD3DDevice[i]->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );   
   
        // Turn off D3D lighting, since we are providing our own vertex colors   
        m_pD3DDevice[i]->SetRenderState( D3DRS_LIGHTING, FALSE );   
   
        m_pD3DDevice[i]->CreateVertexBuffer( 3*sizeof(CUSTOMVERTEX),   
            0, D3DFVF_CUSTOMVERTEX,   
            D3DPOOL_DEFAULT, &m_pVertex[i], NULL ) ;    
   
        VOID* pVertices;   
        if( FAILED( m_pVertex[i]->Lock( 0, sizeof(vertices), (void**)&pVertices, 0 ) ) )   
            return E_FAIL;   
        memcpy( pVertices, vertices, sizeof(vertices) );   
        m_pVertex[i]->Unlock();   
    }   
   
    return TRUE;   
}   
   
VOID CD3DMultiScreen::SetupMatrices()   
{   
    // For our world matrix, we will just rotate the object about the y-axis.   
    D3DXMATRIXA16 matWorld;   
   
    // Set up the rotation matrix to generate 1 full rotation (2*PI radians)    
    // every 1000 ms. To avoid the loss of precision inherent in very high    
    // floating point numbers, the system time is modulated by the rotation    
    // period before conversion to a radian angle.   
    UINT  iTime  = timeGetTime() % 1000;   
    FLOAT fAngle = iTime * (2.0f * D3DX_PI) / 1000.0f;   
    D3DXMatrixRotationY( &matWorld, fAngle );   
    m_pCurDevice->SetTransform( D3DTS_WORLD, &matWorld );   
   
    // Set up our view matrix. A view matrix can be defined given an eye point,   
    // a point to lookat, and a direction for which way is up. Here, we set the   
    // eye five units back along the z-axis and up three units, look at the   
    // origin, and define "up" to be in the y-direction.   
    D3DXVECTOR3 vEyePt( 0.0f, 3.0f,-5.0f );   
    D3DXVECTOR3 vLookatPt( 0.0f, 0.0f, 0.0f );   
    D3DXVECTOR3 vUpVec( 0.0f, 1.0f, 0.0f );   
    D3DXMATRIXA16 matView;   
    D3DXMatrixLookAtLH( &matView, &vEyePt, &vLookatPt, &vUpVec );   
    m_pCurDevice->SetTransform( D3DTS_VIEW, &matView );   
   
    // For the projection matrix, we set up a perspective transform (which   
    // transforms geometry from 3D view space to 2D viewport space, with   
    // a perspective divide making objects smaller in the distance). To build   
    // a perpsective transform, we need the field of view (1/4 pi is common),   
    // the aspect ratio, and the near and far clipping planes (which define at   
    // what distances geometry should be no longer be rendered).   
    D3DXMATRIXA16 matProj;   
    D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI/4, 1.0f, 1.0f, 100.0f );   
    m_pCurDevice->SetTransform( D3DTS_PROJECTION, &matProj );   
}   
   
BOOL CD3DMultiScreen::isRectInterSet(const RECT & rect1, const RECT &rect2)   
{   
    BOOL bVertical = FALSE;   
    BOOL bHorition = FALSE;   
    BOOL bResult = FALSE;   
    if( rect1.left > rect2.left )   
    {   
        if( rect2.right > rect1.left )   
        {   
            bHorition = TRUE;   
        }   
    }   
    else if( rect1.left < rect2.left )   
    {   
        if( rect1.right > rect2.left )   
        {   
            bHorition = TRUE;   
        }   
    }   
    else   
    {   
        bHorition = TRUE;   
    }   
   
    if( bHorition )   
    {   
        if( rect1.top > rect2.top )   
        {   
            if( rect2.bottom > rect1.top )   
                bVertical = TRUE;   
        }   
        else if( rect1.top < rect2.top )   
        {   
            if( rect1.bottom > rect2.top )   
                bVertical = TRUE;   
        }   
        else   
        {   
            bVertical = TRUE;   
        }   
    }   
    if( bHorition && bVertical )   
        bResult = TRUE;   
   
    return bResult;   
}   
   
void CD3DMultiScreen::MultiTransform(const RECT &screenRect, const int &nIndex)   
{   
    RECT tRect = screenRect;   
   
    D3DXMATRIX tempMatrix;   
    D3DXMATRIX pamMatrix;   
    D3DXMATRIX nowMatrix;   
   
    RECT winRect = m_AdapterInfo.monitorInfo[nIndex].rcMonitor ;   
   
    D3DVIEWPORT9 d3dViewPort;   
   
    RECT clRect;   
   
    ScreenToClient(m_AdapterInfo.hWnd[nIndex], (LPPOINT)&tRect.left);   
   
    int nWidth = screenRect.right - screenRect.left;   
    int nHeight = screenRect.bottom - screenRect.top ;   
   
    float fx = (float)nWidth / (float)(winRect.right - winRect.left)  ;   
    float fy = (float)nHeight /(float)(winRect.bottom - winRect.top)  ;   
   
    //d3dViewPort.X = tRect.left;   
    //d3dViewPort.Y = tRect.top ;   
    //d3dViewPort.Width = nWidth;   
    //d3dViewPort.Height = nHeight;   
    //d3dViewPort.MinZ = 0.0f;   
    //d3dViewPort.MaxZ = 100.0f;   
   
    //HRESULT hr = m_pCurDevice->SetViewport(&d3dViewPort);   
   
    m_pCurDevice->GetTransform(D3DTS_WORLD, &nowMatrix);   
    //  D3DXMatrixScaling(&tempMatrix, )   
}   
   
void CD3DMultiScreen::TestRender()   
{   
    RECT rect;   
    GetClientRect(m_hWindow, &rect);   
    RECT screenRect;   
    screenRect = rect;   
   
    ClientToScreen(m_hWindow, (LPPOINT)&screenRect.left);   
    ClientToScreen(m_hWindow, (LPPOINT)&screenRect.right);   
   
    int i = 0;   
   
    BOOL bInterSet = FALSE;   
   
   
    for( i = 0;i < m_AdapterInfo.nMonitors; i++ )   
    {   
        m_pCurDevice = m_pD3DDevice[i] ;   
        if( isRectInterSet(m_AdapterInfo.monitorInfo[i].rcMonitor, screenRect) )   
        {   
            m_pCurDevice->Clear( 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0,0,0), 1.0f, 0 );   
            if( SUCCEEDED( m_pCurDevice->BeginScene() ) )   
            {   
                // Setup the world, view, and projection matrices   
                SetupMatrices();   
   
                //MultiTransform(screenRect, i);   
   
                // Render the vertex buffer contents   
                m_pCurDevice->SetStreamSource( 0, m_pVertex[i], 0, sizeof(CUSTOMVERTEX) );   
                m_pCurDevice->SetFVF( D3DFVF_CUSTOMVERTEX );   
                m_pCurDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, 1 );   
   
                // End the scene   
                m_pCurDevice->EndScene();   
            }   
   
            // Present the backbuffer contents to the display   
        }   
    }   
   
    for( i = 0; i < m_AdapterInfo.nMonitors; i++ )   
    {   
        if( isRectInterSet(m_AdapterInfo.monitorInfo[i].rcMonitor, screenRect) )   
        m_pD3DDevice[i]->Present(NULL, NULL, NULL, NULL);   
    }   
}  

 
