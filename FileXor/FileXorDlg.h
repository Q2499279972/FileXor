// FileXorDlg.h : header file
//

#pragma once


// CFileXorDlg dialog
class CFileXorDlg : public CDialog
{
// Construction
public:
	CFileXorDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_FILEXOR_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedStart();
	afx_msg void OnBnClickedStop();
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnEnChangeEditMask();

	static UINT XorThread(void *param);
public:
	CString m_inputFileNameStr;
	CString m_maskStr;
	CProgressCtrl m_processCtrl;
	volatile int m_threadRunning;
    volatile int m_threadExited;
    int m_process;
};
