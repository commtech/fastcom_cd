// fscctermDoc.h : interface of the CFscctermDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_FSCCTERMDOC_H__90E9FB16_9A66_4FD5_AECA_B727D9172186__INCLUDED_)
#define AFX_FSCCTERMDOC_H__90E9FB16_9A66_4FD5_AECA_B727D9172186__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000




struct threaddata
{
HWND msgwnd;
int port;
char portname[256];
char *pdata;
int *count;
DWORD *status;
BOOL connected;
};



class CFscctermDoc : public CDocument
{
protected: // create from serialization only
	CFscctermDoc();
	DECLARE_DYNCREATE(CFscctermDoc)

// Attributes
public:
	char m_portname[256];
	HANDLE m_hfscc;
	char txbuffer[4096];
	int txloc;
	OVERLAPPED osw;
	
	char rxbuffer[4096];
	int rxloc;
	int port;
	ULONG status;
// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFscctermDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CFscctermDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CFscctermDoc)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FSCCTERMDOC_H__90E9FB16_9A66_4FD5_AECA_B727D9172186__INCLUDED_)
