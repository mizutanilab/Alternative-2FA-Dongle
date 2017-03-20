// AuthKeyDlg.h : header file
//

#pragma once
#include "afxwin.h"
#define BASE32SIZE 20
#define LABELSIZE 8
#define MAX_ENTRY 8


// CAuthKeyDlg dialog
class CAuthKeyDlg : public CDialog
{
// Construction
public:
	CAuthKeyDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_AUTHKEY_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
	unsigned int AdjustTime();

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
//	afx_msg void OnBnClickedSend();
//	afx_msg void OnBnClickedAdjust();
	afx_msg void OnBnClickedRegisterkey();
	CComboBox m_cmbEntries;
	afx_msg void OnBnClickedUpdate();
	afx_msg void OnBnClickedClear();
	afx_msg void OnBnClickedAdjusttime();
};
