// fsccterm.h : main header file for the FSCCTERM application
//

#if !defined(AFX_FSCCTERM_H__66EF75A8_F925_48BB_A62B_B06EBE87EE29__INCLUDED_)
#define AFX_FSCCTERM_H__66EF75A8_F925_48BB_A62B_B06EBE87EE29__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols
#define WM_USER_READ_UPDATE (WM_USER + 1)

/////////////////////////////////////////////////////////////////////////////
// CFscctermApp:
// See fsccterm.cpp for the implementation of this class
//

class CFscctermApp : public CWinApp
{
public:
	CFscctermApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFscctermApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CFscctermApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FSCCTERM_H__66EF75A8_F925_48BB_A62B_B06EBE87EE29__INCLUDED_)
