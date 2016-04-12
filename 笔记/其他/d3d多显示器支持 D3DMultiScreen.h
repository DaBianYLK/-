#ifndef __d_multiScreen_h__ 
#define __d_multiScreen_h__ 
 
#include <d3d9.h> 
#include <d3dx9.h> 
 
#define VT_MAX_MONITOR_COUNT  9 
 
typedef struct tagADAPTERMONITORINFO 
{ 
	MONITORINFO monitorInfo[VT_MAX_MONITOR_COUNT]; 
	HMONITOR    hMonitor[VT_MAX_MONITOR_COUNT]; 
	int         vAdapter[VT_MAX_MONITOR_COUNT]; 
	int         nMonitors; 
	char        szAdapterName[VT_MAX_MONITOR_COUNT][128]; 
	char        szMonitorName[VT_MAX_MONITOR_COUNT][128]; 
	HWND        hWnd[VT_MAX_MONITOR_COUNT]; 
}ADAPTERMONITORINFO, *LPADAPTERMONITORINFO; 
 
class CD3DMultiScreen 
{ 
public: 
	CD3DMultiScreen(); 
	~CD3DMultiScreen(); 
 
public: 
	void SetHinstance(HINSTANCE h); 
	void EnumMonitors(); 
	BOOL InitializeEnvironment(); 
 
	BOOL CreateMultiWindow(); 
	void AdjustWindowPos(); 
 
	static LRESULT CALLBACK s_MainProc(HWND h, UINT uMsg, WPARAM wParam, LPARAM lParam); 
	static LRESULT CALLBACK s_ChildProc(HWND h, UINT uMsg, WPARAM wParam, LPARAM lParam); 
 
	static CD3DMultiScreen * m_pMultiScreen; 
 
	LRESULT MainProc(HWND h, UINT uMsg, WPARAM wParam, LPARAM lParam); 
	LRESULT ChildProc(HWND h, UINT uMsg, WPARAM wParam, LPARAM lParam); 
 
	void TestRender(); 
	VOID SetupMatrices(); 
 
protected: 
	BOOL isRectInterSet(const RECT & rect1, const RECT &rect2); 
	void MultiTransform(const RECT &screenRect,const int &nIndex); 
 
protected: 
 
	ADAPTERMONITORINFO  m_AdapterInfo; 
	winEnumDisplayDevices m_pfEnumDisplayDevices; 
	HMODULE  m_hModule; 
 
	int   m_nDesktopWidth; 
	int   m_nDesktopHeight; 
 
	IDirect3D9 * m_pD3D; 
	IDirect3DDevice9 * m_pD3DDevice[VT_MAX_MONITOR_COUNT]; 
 
	IDirect3DDevice9 * m_pCurDevice; 
	HWND   m_hWindow; 
	HINSTANCE m_hInstance; 
 
	IDirect3DVertexBuffer9 * m_pVertex[VT_MAX_MONITOR_COUNT]; 
}; 
#endif //__d_multiScreen_h__