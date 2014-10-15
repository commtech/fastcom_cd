// PortSelect.cpp : implementation file
//

#include "stdafx.h"
#include "fsccterm.h"
#include "PortSelect.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPortSelect dialog


CPortSelect::CPortSelect(CWnd* pParent /*=NULL*/)
	: CDialog(CPortSelect::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPortSelect)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CPortSelect::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPortSelect)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPortSelect, CDialog)
	//{{AFX_MSG_MAP(CPortSelect)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPortSelect message handlers

BOOL CPortSelect::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	char namebuf[256];
	HANDLE hfscc;
	int i;

	for(i=0;i<20;i++)
	{
	sprintf(namebuf,"\\\\.\\FSCC%d",i);
	hfscc = CreateFile(namebuf,GENERIC_WRITE | GENERIC_READ , FILE_SHARE_READ | FILE_SHARE_WRITE,
	NULL,OPEN_EXISTING, 0, NULL);
	
	if(hfscc == INVALID_HANDLE_VALUE)
	{
	}
	else
	{
	SendDlgItemMessage(IDC_PORTSELECT,CB_ADDSTRING,0,(LPARAM)namebuf);
	CloseHandle(hfscc);
	}
	SendDlgItemMessage(IDC_PORTSELECT,CB_SETCURSEL,0,0);

	}
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPortSelect::OnOK() 
{
	char buf[256];
	// TODO: Add extra validation here
	SendDlgItemMessage(IDC_PORTSELECT,WM_GETTEXT,sizeof(buf),(LPARAM)buf);	
	memcpy(m_selection,buf,256);
	CDialog::OnOK();
}
