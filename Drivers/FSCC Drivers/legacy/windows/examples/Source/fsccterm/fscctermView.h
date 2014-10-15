// fscctermView.h : interface of the CFscctermView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_FSCCTERMVIEW_H__20674346_7049_4D1F_B3C6_3B5EC7F89208__INCLUDED_)
#define AFX_FSCCTERMVIEW_H__20674346_7049_4D1F_B3C6_3B5EC7F89208__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


UINT ReadProc( LPVOID lpData );

class CFscctermView : public CView
{
protected: // create from serialization only
	CFscctermView();
	DECLARE_DYNCREATE(CFscctermView)

// Attributes
public:
	CFscctermDoc* GetDocument();
	unsigned curchar;
// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFscctermView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CFscctermView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	LRESULT OnReadUpdate(WPARAM wParam, LPARAM lParam);
	//{{AFX_MSG(CFscctermView)
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnPort();
	afx_msg void OnDisconnect();
	afx_msg void OnSettings();
	afx_msg void OnDestroy();
	afx_msg void OnFlushtx();
	afx_msg void OnFlushRx();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in fscctermView.cpp
inline CFscctermDoc* CFscctermView::GetDocument()
   { return (CFscctermDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FSCCTERMVIEW_H__20674346_7049_4D1F_B3C6_3B5EC7F89208__INCLUDED_)
