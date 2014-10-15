#if !defined(AFX_PORTSELECT_H__F0DA3096_577E_4391_8379_8EEAC6BCAB0C__INCLUDED_)
#define AFX_PORTSELECT_H__F0DA3096_577E_4391_8379_8EEAC6BCAB0C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PortSelect.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPortSelect dialog

class CPortSelect : public CDialog
{
// Construction
public:
	char m_selection[256];
	CPortSelect(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPortSelect)
	enum { IDD = IDD_PORT_SELECT };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPortSelect)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPortSelect)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PORTSELECT_H__F0DA3096_577E_4391_8379_8EEAC6BCAB0C__INCLUDED_)
