// fscctermView.cpp : implementation of the CFscctermView class
//

#include "stdafx.h"
#include "fsccterm.h"

#include "fscctermDoc.h"
#include "fscctermView.h"
#include "PortSelect.h"
#include "..\fscc.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


struct threaddata threadparam[10];
BOOL connected;
CWinThread *rT;
char rtbuf[10][4096];
int rtcount[10];
int portlist[10];
ULONG rtstatus[10];

/////////////////////////////////////////////////////////////////////////////
// CFscctermView

IMPLEMENT_DYNCREATE(CFscctermView, CView)

BEGIN_MESSAGE_MAP(CFscctermView, CView)
	ON_MESSAGE(WM_USER_READ_UPDATE, OnReadUpdate)
	//{{AFX_MSG_MAP(CFscctermView)
	ON_WM_CHAR()
	ON_COMMAND(IDM_PORT, OnPort)
	ON_COMMAND(IDM_DISCONNECT, OnDisconnect)
	ON_COMMAND(IDM_SETTINGS, OnSettings)
	ON_WM_DESTROY()
	ON_COMMAND(IDC_FLUSHTX, OnFlushtx)
	ON_COMMAND(IDC_FLUSH_RX, OnFlushRx)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFscctermView construction/destruction

CFscctermView::CFscctermView()
{
	// TODO: add construction code here

}

CFscctermView::~CFscctermView()
{
}

BOOL CFscctermView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CFscctermView drawing

void CFscctermView::OnDraw(CDC* pDC)
{
	CFscctermDoc* pDoc = GetDocument();
	RECT mycrect;
	RECT myrrect;
	RECT myrrect2;
	RECT mytrect;
	RECT calcrect;
	TEXTMETRIC tm;
	CSize csz;
	char xbuf[256];
	int i,x,y;
	POINT p1;

	ASSERT_VALID(pDoc);
	GetClientRect(&mycrect);
	GetClientRect(&mytrect);
	GetClientRect(&myrrect);
	pDC->GetTextMetrics(&tm);
	mytrect.bottom = 5*tm.tmHeight;
	myrrect.top = mytrect.bottom;
	
	
	mytrect.top += tm.tmHeight;

	memcpy(&myrrect2,&myrrect,sizeof(RECT));
	myrrect2.top = ((myrrect.bottom - myrrect.top)/2)+myrrect.top;
	myrrect.bottom = myrrect2.top;
	p1.x = mytrect.left;
	p1.y = tm.tmHeight;
	pDC->MoveTo(p1);
	p1.x = mytrect.right;
	pDC->LineTo(p1);

	p1.x = mytrect.left;
	p1.y = mytrect.bottom;
	pDC->MoveTo(p1);
	p1.x = mytrect.right;
	pDC->LineTo(p1);

	p1.x = myrrect.left;
	p1.y = myrrect.bottom;
	pDC->MoveTo(p1);
	p1.x = myrrect.right;
	pDC->LineTo(p1);

	//sprintf(xbuf,"count:%d :%x",pDoc->txloc,curchar);
	sprintf(xbuf,"tcount:%d , rcount:%d , rx status:%4.4x",pDoc->txloc,pDoc->rxloc,pDoc->status);



	pDC->TextOut(0,0,xbuf);
	sprintf(xbuf,"TX Data:");
	pDC->TextOut(0,mytrect.top+2,xbuf);
	sprintf(xbuf,"RX Data (ASCII):");
	pDC->TextOut(0,myrrect.top+2,xbuf);
	sprintf(xbuf,"RX Data (HEX):");
	pDC->TextOut(0,myrrect2.top+2,xbuf);

	mytrect.top += tm.tmHeight+2;
	myrrect.top += tm.tmHeight+2;
	myrrect2.top += tm.tmHeight+2;

	// TODO: add draw code for native data here
	x = mytrect.left;
	y = mytrect.top;
	for(i=0;i<pDoc->txloc;i++)
		{
		sprintf(xbuf,"%c",pDoc->txbuffer[i]);
		if(y<mytrect.bottom)
		{
		pDC->TextOut(x,y,xbuf);
		memset(&calcrect,0,sizeof(RECT));
		pDC->DrawText(xbuf,&calcrect,DT_CALCRECT);

		if(x+2*calcrect.right > mytrect.right)
			{
			y+=tm.tmHeight;
			x=mytrect.left;
			}
		else x += calcrect.right;
		}
		}
	x = myrrect.left;
	y = myrrect.top;
	for(i=0;i<pDoc->rxloc;i++)
		{
		sprintf(xbuf,"%c",pDoc->rxbuffer[i]);
		if(y<myrrect.bottom)
		{
		pDC->TextOut(x,y,xbuf);
		memset(&calcrect,0,sizeof(RECT));
		pDC->DrawText(xbuf,&calcrect,DT_CALCRECT);

		if(x+2*calcrect.right > myrrect.right)
			{
			y+=tm.tmHeight;
			x=myrrect.left;
			}
		else x += calcrect.right;
		}
		}
	x = myrrect2.left;
	y = myrrect2.top;
	for(i=0;i<pDoc->rxloc;i++)
		{
		sprintf(xbuf,"%2.2x:",pDoc->rxbuffer[i]&0xff);
		if(y<myrrect2.bottom)
		{
		pDC->TextOut(x,y,xbuf);
		memset(&calcrect,0,sizeof(RECT));
		pDC->DrawText(xbuf,&calcrect,DT_CALCRECT);

		if(x+2*calcrect.right > myrrect2.right)
			{
			y+=tm.tmHeight;
			x=myrrect2.left;
			}
		else x += calcrect.right;
		}
		}

}

/////////////////////////////////////////////////////////////////////////////
// CFscctermView printing

BOOL CFscctermView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CFscctermView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CFscctermView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CFscctermView diagnostics

#ifdef _DEBUG
void CFscctermView::AssertValid() const
{
	CView::AssertValid();
}

void CFscctermView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CFscctermDoc* CFscctermView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CFscctermDoc)));
	return (CFscctermDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CFscctermView message handlers

void CFscctermView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
	CFscctermDoc * pDoc = GetDocument();
	int i;
	curchar = nChar;
	if(nChar!=0x0d)
	{
	if(nChar==8)
	{
	if(pDoc->txloc>0)
		{
		pDoc->txloc--;
		pDoc->txbuffer[pDoc->txloc]=' ';
		}
	}
	else
	{
	for(i=0;i<nRepCnt;i++)pDoc->txbuffer[pDoc->txloc++]=nChar;
	}
	Invalidate(TRUE);
	}
	else
	{
	/*send frame here
	*/
	unsigned long bytesout;
	unsigned long er;
	unsigned long j;
	if(!WriteFile(pDoc->m_hfscc,pDoc->txbuffer,pDoc->txloc,&bytesout,&pDoc->osw))
		{
		if((er=GetLastError())==ERROR_IO_PENDING)
			{
			do
				{
				j = WaitForSingleObject( pDoc->osw.hEvent, 1000 );//1 second timeout
				if(j==WAIT_TIMEOUT)
					{
					//this will execute every 1 second
					//you could put a counter in here and if the 
					//driver takes an inordinate ammout of time
					//to complete, you could issue a flush RX command
					//and break out of this loop
					MessageBox("txtimeout","",MB_OK);
					DeviceIoControl(pDoc->m_hfscc,IOCTL_FSCCDRV_FLUSH_TX,NULL,0,NULL,0,&er,NULL);
					}
				if(j==WAIT_ABANDONED)
					{
					}
		
				}while(j!=WAIT_OBJECT_0);//stay here until we get signaled or the main thread exits

			GetOverlappedResult(pDoc->m_hfscc,&pDoc->osw,&bytesout,TRUE);
			}
		else
			{
			char xbuf[256];
			sprintf(xbuf,"WRITE ERROR:%8.8x\n",er);
			MessageBox(xbuf,"ERROR!",MB_OK);
			}
		}

	pDoc->txloc=0;
	memset(pDoc->txbuffer,0,sizeof(pDoc->txbuffer));
	Invalidate(TRUE);
	}
	CView::OnChar(nChar, nRepCnt, nFlags);
}


UINT ReadProc( LPVOID lpData )
{
struct threaddata *tp;
HANDLE hfscc;
DWORD j;			//temp
BOOL t;				//temp
DWORD nobytestoread;	//the number of bytes that can be put in data[] (max)
DWORD nobytesread;		//the number of bytes that the driver put in data[]
OVERLAPPED  os ;	//overlapped struct for overlapped I/O
tp = (struct threaddata*)lpData;

memset( &os, 0, sizeof( OVERLAPPED ) ) ;        //wipe the overlapped struct

// create I/O event used for overlapped read

os.hEvent = CreateEvent( NULL,    // no security
			    TRUE,    // explicit reset req
			    FALSE,   // initial event reset
			    NULL ) ; // no name
if (os.hEvent == NULL)
   {
      MessageBox(NULL,  "Failed to create event for thread!", "READ Error!",
		  MB_ICONEXCLAMATION | MB_OK ) ;
      return ( FALSE ) ;
   }

hfscc = CreateFile(tp->portname,GENERIC_WRITE | GENERIC_READ , FILE_SHARE_READ | FILE_SHARE_WRITE,
	NULL,OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED, NULL);
	
	if(hfscc == INVALID_HANDLE_VALUE)
	{
	MessageBox(NULL,"Failed to open port","ERROR",MB_OK);
	CloseHandle(os.hEvent);
	return(FALSE);
	}

nobytestoread = 4096;		///should allways be 4096, read call will fail with invalid param if it is less than 4096
//MessageBox(NULL,"read thread start","",MB_OK);
do
	{
	//start a read request by calling ReadFile() with the esccdevice handle
	//if it returns true then we received a frame 
	//if it returns false and ERROR_IO_PENDING then there are no
	//receive frames available so we wait until the overlapped struct
	//gets signaled.

	t = ReadFile(hfscc,tp->pdata,nobytestoread,&nobytesread,&os);
	
	if(t==TRUE)
		{
		//we have some data here so do something with it ... display it?
		//display our newly received frame
		//printf("received %u bytes:\n\r",nobytesread);
		if(nobytesread>=2)
		{
		tp->status[0] = (tp->pdata[nobytesread-1]<<8)|tp->pdata[nobytesread-2];
		tp->pdata[nobytesread-2] = 0;
		tp->count[0] = nobytesread-2;
		::PostMessage(tp->msgwnd,WM_USER_READ_UPDATE,0,0);
		}
		else MessageBox(NULL,"received 0 bytes","",MB_OK);
		}
	else
		{
		if (GetLastError() == ERROR_IO_PENDING)
			{
			// wait for a receive frame to come in, note it will wait forever
			do
				{
				j = WaitForSingleObject( os.hEvent, 1000 );//1 second timeout
				if(j==WAIT_TIMEOUT)
					{
					//this will execute every 1 second
					//you could put a counter in here and if the 
					//driver takes an inordinate ammout of time
					//to complete, you could issue a flush RX command
					//and break out of this loop
//					MessageBox(NULL,"rxtimeout","",MB_OK);
					}
				if(j==WAIT_ABANDONED)
					{
					}
				
				}while((j!=WAIT_OBJECT_0)&&(tp->connected));//stay here until we get signaled or the main thread exits
			
				if(tp->connected)
					{
					GetOverlappedResult(hfscc,&os,&nobytesread,TRUE); //here to get the actual nobytesread!!!
					if(nobytesread>=2)
					{
					tp->status[0] = (tp->pdata[nobytesread-1]<<8)|tp->pdata[nobytesread-2];
					tp->pdata[nobytesread-2] = 0;
					tp->count[0] = nobytesread-2;
					t=TRUE;
					::PostMessage(tp->msgwnd,WM_USER_READ_UPDATE,0,0);
					}
					else MessageBox(NULL,"received 0 bytes","",MB_OK);
					}
			}
		}
	}while(tp->connected);              //do until we want to terminate

	CloseHandle( os.hEvent ) ;      //done with event
	CloseHandle(hfscc);
//	MessageBox(NULL,"read thread done","",MB_OK);
	return(TRUE);                   //outta here
	

}

void CFscctermView::OnPort() 
{
	// TODO: Add your command handler code here
	// TODO: Add your command handler code here
CPortSelect dlg;
CFscctermDoc * pDoc = GetDocument();


if(dlg.DoModal()==IDOK)	
	{
	memcpy(pDoc->m_portname,dlg.m_selection,strlen(dlg.m_selection));
	}

pDoc->m_hfscc =  CreateFile(pDoc->m_portname,GENERIC_WRITE | GENERIC_READ , FILE_SHARE_READ | FILE_SHARE_WRITE,
	NULL,OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED, NULL);
	
	if(pDoc->m_hfscc == INVALID_HANDLE_VALUE)
	{
		char xbuf[256];
		sprintf(xbuf,"Cannot open %s",pDoc->m_portname);
		MessageBox(xbuf,"ERROR!",MB_OK);
	}
	pDoc->txloc=0;
	pDoc->rxloc=0;
pDoc->SetTitle(pDoc->m_portname);
pDoc->osw.hEvent = CreateEvent(NULL,TRUE,FALSE,NULL);

int i;
int port;
connected = TRUE;
for(i=0;i<10;i++)
{
if(portlist[i]==0)
	{
	portlist[i]=1;
	port=i;
	break;
	}
}

threadparam[port].msgwnd = m_hWnd;
memcpy(threadparam[port].portname,pDoc->m_portname,sizeof(pDoc->m_portname));
threadparam[port].pdata = &rtbuf[port][0];
threadparam[port].count = &rtcount[port];
threadparam[port].status = &rtstatus[port];
threadparam[port].connected = TRUE;
threadparam[port].port = port;
pDoc->port = port;
rT = AfxBeginThread(ReadProc,&threadparam[port],THREAD_PRIORITY_NORMAL,0,0,NULL);

	
}

LRESULT CFscctermView::OnReadUpdate(WPARAM wp, LPARAM lp) 
{
CFscctermDoc * pDoc = GetDocument();
memcpy(pDoc->rxbuffer,&rtbuf[pDoc->port][0],rtcount[pDoc->port]);
memset(&pDoc->rxbuffer[rtcount[pDoc->port]],0,4096-rtcount[pDoc->port]);
pDoc->rxloc=rtcount[pDoc->port];
pDoc->status = rtstatus[pDoc->port];
Invalidate();
return 0;
}

void CFscctermView::OnDisconnect() 
{
	// TODO: Add your command handler code here
CFscctermDoc * pDoc = GetDocument();

threadparam[pDoc->port].connected=FALSE;
CloseHandle(pDoc->m_hfscc);
CloseHandle(pDoc->osw.hEvent);
pDoc->m_hfscc=NULL;
pDoc->osw.hEvent=NULL;
portlist[pDoc->port]=0;	
pDoc->SetTitle("Disconnected");
memset(pDoc->txbuffer,0,sizeof(pDoc->txbuffer));
memset(pDoc->rxbuffer,0,sizeof(pDoc->rxbuffer));
pDoc->txloc=0;
pDoc->rxloc=0;
Invalidate();
}

void CFscctermView::OnSettings() 
{
	// TODO: Add your command handler code here
//well now we get to have some fun...we need to run fsccp_basic_settings.exe but we need
//the parameters for the PCI bus/slot/function to pass to it...
//do it the hard way, because I don't know of an easy one
//there is probably a setupdi function that will get us there, but short of that
//enumerate the \\HKLM\SYSTEM\CCS\Services\fsccpdrv\parameters keys looking for a portname that matches ours

HKEY mkey;
HKEY skey;
char kname[512];
char pnvalue[512];
char *c;
long result;
unsigned long type;
unsigned long pnvaluesz;
CFscctermDoc * pDoc = GetDocument();
int i;
unsigned long pcibus,pciport;
int goodresult;

goodresult=0;
result = RegOpenKey(HKEY_LOCAL_MACHINE,"SYSTEM\\CurrentControlSet\\Services\\fsccpdrv\\parameters",&mkey);
if(result!=ERROR_SUCCESS)
{
char xbuf[256];
sprintf(xbuf,"Cannot open Registry Key! error#:%8.8x",result);
MessageBox(xbuf,"ERROR",MB_OK);
return;
}

i = 0;
do
	{
	result = RegEnumKey(mkey,i,(char*)&kname,sizeof(kname));
	if(result==ERROR_SUCCESS)
		{
		RegOpenKey(mkey,kname,&skey);
		pnvaluesz=sizeof(pnvalue);
		result = RegQueryValueEx(skey,"portname",0,&type,(unsigned char *)pnvalue,(unsigned long*)&pnvaluesz);
		if(result!=ERROR_SUCCESS)
			{
			char xbuf[256];
			sprintf(xbuf,"Cannot get portname from registry! error#:%8.8x",result);
			MessageBox(xbuf,"ERROR",MB_OK);
			RegCloseKey(skey);
			RegCloseKey(mkey);
			return;
			}
		RegCloseKey(skey);
		if(strstr(pnvalue,&pDoc->m_portname[4])!=NULL)
			{
			goodresult=1;
			goto donescan;
			}
		}
	i++;
	}while(result!=ERROR_NO_MORE_ITEMS);
donescan:
RegCloseKey(mkey);

if(goodresult==1)
	{
	c = strchr(kname,'_');
	if(c!=NULL)
		{
		pcibus=atol(c+1);
		c = strchr(c+1,'_');
		if(c!=NULL)
			{
			pciport=atol(c+1);
			}
		}
	char sysbuf[512];
	SHELLEXECUTEINFO execinfo;

	sprintf(sysbuf,"loc:%d port:%d",pcibus,pciport);
	memset(&execinfo,0,sizeof(SHELLEXECUTEINFO));
	execinfo.cbSize = sizeof(SHELLEXECUTEINFO);
	execinfo.fMask = SEE_MASK_FLAG_NO_UI;
	execinfo.hwnd = NULL;
	execinfo.lpVerb = "open";
	execinfo.lpFile = "fscc_basic_settings.exe";
	execinfo.lpDirectory = "";
	execinfo.nShow = SW_SHOWNORMAL;
	execinfo.lpParameters = sysbuf;
	ShellExecuteEx(&execinfo);
	}
else
	{
	char xbuf[256];
	sprintf(xbuf,"Did not find port! Aborting Settings");
	MessageBox(xbuf,"ERROR!",MB_OK);
	}

}

void CFscctermView::OnDestroy() 
{

	
	// TODO: Add your message handler code here
CFscctermDoc* pDoc = GetDocument();
threadparam[pDoc->port].connected=FALSE;
if(pDoc->m_hfscc!=NULL) CloseHandle(pDoc->m_hfscc);
if(pDoc->osw.hEvent!=NULL)     CloseHandle(pDoc->osw.hEvent);
pDoc->m_hfscc=NULL;
pDoc->osw.hEvent=NULL;
portlist[pDoc->port]=0;	
pDoc->SetTitle("Disconnected");	
	CView::OnDestroy();
}

void CFscctermView::OnFlushtx() 
{
char xbuf[512];
ULONG er;
CFscctermDoc* pDoc = GetDocument();
	// TODO: Add your command handler code here
DeviceIoControl(pDoc->m_hfscc,IOCTL_FSCCDRV_FLUSH_TX,NULL,0,NULL,0,&er,NULL);
sprintf(xbuf,"TX on %s flushed",pDoc->m_portname);	
MessageBox(xbuf,"",MB_OK);
}

void CFscctermView::OnFlushRx() 
{
char xbuf[512];
	// TODO: Add your command handler code here
ULONG er;
CFscctermDoc* pDoc = GetDocument();
	// TODO: Add your command handler code here
DeviceIoControl(pDoc->m_hfscc,IOCTL_FSCCDRV_FLUSH_RX,NULL,0,NULL,0,&er,NULL);	
sprintf(xbuf,"RX on %s flushed",pDoc->m_portname);	
MessageBox(xbuf,"",MB_OK);
	
}
