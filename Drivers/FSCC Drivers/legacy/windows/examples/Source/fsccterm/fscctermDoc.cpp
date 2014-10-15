// fscctermDoc.cpp : implementation of the CFscctermDoc class
//

#include "stdafx.h"
#include "fsccterm.h"

#include "fscctermDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFscctermDoc

IMPLEMENT_DYNCREATE(CFscctermDoc, CDocument)

BEGIN_MESSAGE_MAP(CFscctermDoc, CDocument)
	//{{AFX_MSG_MAP(CFscctermDoc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFscctermDoc construction/destruction

CFscctermDoc::CFscctermDoc()
{
	// TODO: add one-time construction code here
m_hfscc=NULL;
osw.hEvent=NULL;
}

CFscctermDoc::~CFscctermDoc()
{
if(m_hfscc!=NULL) CloseHandle(m_hfscc);
if(osw.hEvent!=NULL)CloseHandle(osw.hEvent);
}

BOOL CFscctermDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;
txloc=0;
rxloc=0;
m_hfscc=NULL;
osw.hEvent=NULL;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CFscctermDoc serialization

void CFscctermDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CFscctermDoc diagnostics

#ifdef _DEBUG
void CFscctermDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CFscctermDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CFscctermDoc commands
