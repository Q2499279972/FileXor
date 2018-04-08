// FileXorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FileXor.h"
#include "FileXorDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CFileXorDlg dialog


#include "windows.h"

CFileXorDlg::CFileXorDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFileXorDlg::IDD, pParent)
	, m_inputFileNameStr(_T(""))
	, m_maskStr(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CFileXorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_FILE_INPUT, m_inputFileNameStr);
	DDX_Text(pDX, IDC_EDIT_MASK, m_maskStr);
	DDX_Control(pDX,IDC_PROGRESS,m_processCtrl);
}

BEGIN_MESSAGE_MAP(CFileXorDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_START, &CFileXorDlg::OnBnClickedStart)
	ON_BN_CLICKED(IDC_STOP, &CFileXorDlg::OnBnClickedStop)
	ON_WM_DROPFILES()
	ON_EN_CHANGE(IDC_EDIT_MASK, &CFileXorDlg::OnEnChangeEditMask)
END_MESSAGE_MAP()


// CFileXorDlg message handlers

BOOL CFileXorDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	GetDlgItem(IDC_START)->EnableWindow(1);
	GetDlgItem(IDC_STOP)->EnableWindow(1);
	m_processCtrl.SetRange(0,100);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CFileXorDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CFileXorDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


#define BUFFER_SIZE (2*1024*1024)
static unsigned char buffer[BUFFER_SIZE];

UINT CFileXorDlg::XorThread(void *param)
{
	CFileXorDlg *p = (CFileXorDlg *)param;
	p->m_threadRunning = 1;
    p->m_threadExited = 0;

	FILE* fp_i = NULL;
	FILE* fp_o = NULL;
	fp_i = _tfopen(p->m_inputFileNameStr,_T("rb"));
	fp_o = _tfopen(p->m_inputFileNameStr+"_OUT",_T("wb"));
	
	p->m_process = 0;
	p->m_processCtrl.SetPos(p->m_process);//mark
	
	if(!fp_i)
	{
		p->MessageBoxA("Open Input File Failed","ERR",MB_OK);
		goto _THREAD_EXIT;
	}

	if(!fp_o)
	{
		p->MessageBoxA("Open Output File Failed","ERR",MB_OK);
		goto _THREAD_EXIT;
	}

	fseek(fp_i,0,SEEK_END);
	fpos_t pos;
	fgetpos(fp_i,&pos);
	fseek(fp_i,0,SEEK_SET);

	unsigned long long readTotalBytes = 0;
	int xorIndex = 0;

	int len = p->m_maskStr.GetLength();
	if(p->m_maskStr.GetLength()>1000)
	{
		p->MessageBoxA("Xor String Too Long!","ERR",MB_OK);
		goto _THREAD_EXIT;
	}
	char s[1024];
	strcpy(s,(char*)p->m_maskStr.GetBuffer(0));
	p->m_maskStr.ReleaseBuffer();
	do
	{
		size_t readBytes = fread(buffer,1,BUFFER_SIZE,fp_i);
		if(readBytes)
		{
			size_t i;
			for(i=0;i<readBytes;i++)
			{
				buffer[i] ^= s[xorIndex++];
				if(xorIndex>=len)
				{
					xorIndex = 0;
				}
			}
			fwrite(buffer,1,readBytes,fp_o);
			readTotalBytes += readBytes; 
		}
		else
		{
			p->m_threadRunning = 0;
		}
		p->m_process = (int)(readTotalBytes*100ULL/pos);
		p->m_processCtrl.SetPos(p->m_process);//mark
	}while(p->m_threadRunning);

_THREAD_EXIT:
	p->m_threadRunning = 0;
	if(fp_i) fclose(fp_i);
	if(fp_o) fclose(fp_o);
	p->GetDlgItem(IDC_START)->EnableWindow(1);
	p->m_threadExited = 1;
	return 0;
}
CWinThread* Thread;
void CFileXorDlg::OnBnClickedStart()
{
	UpdateData(1);
	if(m_inputFileNameStr.IsEmpty())
	{
		MessageBoxA("Input File Should Be Set!","Msg",MB_OK);
		return;
	}
	if(m_maskStr.IsEmpty())
	{
		MessageBoxA("Mask String Should Be Set!","Msg",MB_OK);
		return;
	}
	GetDlgItem(IDC_START)->EnableWindow(0);
	Thread = AfxBeginThread(XorThread, 
                           this,
                           THREAD_PRIORITY_NORMAL,
                           0,
                           0,
                           NULL);
	
}

void CFileXorDlg::OnBnClickedStop()
{
	m_threadRunning = 0;
	return;
}

void CFileXorDlg::OnDropFiles(HDROP hDropInfo)
{
    int DropCount=DragQueryFileA(hDropInfo,-1,NULL,0); 
    if(DropCount==1)  
    {  
        char s[MAX_PATH];  
        DragQueryFileA(hDropInfo,0,s,MAX_PATH);
		m_inputFileNameStr = s;
    }
	UpdateData(0);
    DragFinish(hDropInfo);
	CDialog::OnDropFiles(hDropInfo);
}

void CFileXorDlg::OnEnChangeEditMask()
{
}
