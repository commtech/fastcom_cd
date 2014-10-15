; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CFscctermView
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "fsccterm.h"
LastPage=0

ClassCount=7
Class1=CFscctermApp
Class2=CFscctermDoc
Class3=CFscctermView
Class4=CMainFrame

ResourceCount=4
Resource1=IDR_MAINFRAME
Resource2=IDR_FSCCTETYPE
Resource3=IDD_ABOUTBOX
Class5=CChildFrame
Class6=CAboutDlg
Class7=CPortSelect
Resource4=IDD_PORT_SELECT

[CLS:CFscctermApp]
Type=0
HeaderFile=fsccterm.h
ImplementationFile=fsccterm.cpp
Filter=N

[CLS:CFscctermDoc]
Type=0
HeaderFile=fscctermDoc.h
ImplementationFile=fscctermDoc.cpp
Filter=N
BaseClass=CDocument
VirtualFilter=DC
LastObject=CFscctermDoc

[CLS:CFscctermView]
Type=0
HeaderFile=fscctermView.h
ImplementationFile=fscctermView.cpp
Filter=C
LastObject=IDC_FLUSH_RX
BaseClass=CView
VirtualFilter=VWC


[CLS:CMainFrame]
Type=0
HeaderFile=MainFrm.h
ImplementationFile=MainFrm.cpp
Filter=T
LastObject=IDC_FLUSHTX


[CLS:CChildFrame]
Type=0
HeaderFile=ChildFrm.h
ImplementationFile=ChildFrm.cpp
Filter=M


[CLS:CAboutDlg]
Type=0
HeaderFile=fsccterm.cpp
ImplementationFile=fsccterm.cpp
Filter=D

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889

[MNU:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_PRINT_SETUP
Command4=ID_FILE_MRU_FILE1
Command5=ID_APP_EXIT
Command6=ID_VIEW_TOOLBAR
Command7=ID_VIEW_STATUS_BAR
Command8=ID_APP_ABOUT
CommandCount=8

[TB:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_FILE_NEW
Command2=IDM_PORT
Command3=IDM_DISCONNECT
Command4=IDM_SETTINGS
Command5=IDC_FLUSHTX
Command6=IDC_FLUSH_RX
Command7=ID_APP_ABOUT
CommandCount=7

[MNU:IDR_FSCCTETYPE]
Type=1
Class=CFscctermView
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_CLOSE
Command4=ID_FILE_SAVE
Command5=ID_FILE_SAVE_AS
Command6=ID_FILE_PRINT
Command7=ID_FILE_PRINT_PREVIEW
Command8=ID_FILE_PRINT_SETUP
Command9=ID_FILE_MRU_FILE1
Command10=ID_APP_EXIT
Command11=ID_EDIT_UNDO
Command12=ID_EDIT_CUT
Command13=ID_EDIT_COPY
Command14=ID_EDIT_PASTE
Command15=ID_VIEW_TOOLBAR
Command16=ID_VIEW_STATUS_BAR
Command17=IDM_PORT
Command18=IDM_DISCONNECT
Command19=IDM_SETTINGS
Command20=ID_WINDOW_NEW
Command21=ID_WINDOW_CASCADE
Command22=ID_WINDOW_TILE_HORZ
Command23=ID_WINDOW_ARRANGE
Command24=ID_APP_ABOUT
CommandCount=24

[ACL:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_FILE_PRINT
Command5=ID_EDIT_UNDO
Command6=ID_EDIT_CUT
Command7=ID_EDIT_COPY
Command8=ID_EDIT_PASTE
Command9=ID_EDIT_UNDO
Command10=ID_EDIT_CUT
Command11=ID_EDIT_COPY
Command12=ID_EDIT_PASTE
Command13=ID_NEXT_PANE
Command14=ID_PREV_PANE
CommandCount=14

[DLG:IDD_PORT_SELECT]
Type=1
Class=CPortSelect
ControlCount=3
Control1=IDC_PORTSELECT,combobox,1344340226
Control2=IDOK,button,1342242817
Control3=IDCANCEL,button,1342242816

[CLS:CPortSelect]
Type=0
HeaderFile=PortSelect.h
ImplementationFile=PortSelect.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=IDOK

