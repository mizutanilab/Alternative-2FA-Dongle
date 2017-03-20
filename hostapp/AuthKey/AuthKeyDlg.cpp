// AuthKeyDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AuthKey.h"
#include "AuthKeyDlg.h"
#include "clongint.h"

#include "Usbhidioc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CAuthKeyDlg dialog




CAuthKeyDlg::CAuthKeyDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAuthKeyDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CAuthKeyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ENTRIES, m_cmbEntries);
}

BEGIN_MESSAGE_MAP(CAuthKeyDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
//	ON_BN_CLICKED(IDC_SEND, &CAuthKeyDlg::OnBnClickedSend)
//	ON_BN_CLICKED(IDC_ADJUST, &CAuthKeyDlg::OnBnClickedAdjust)
	ON_BN_CLICKED(IDC_REGISTERKEY, &CAuthKeyDlg::OnBnClickedRegisterkey)
	ON_BN_CLICKED(IDC_UPDATE, &CAuthKeyDlg::OnBnClickedUpdate)
	ON_BN_CLICKED(IDC_CLEAR, &CAuthKeyDlg::OnBnClickedClear)
	ON_BN_CLICKED(IDC_ADJUSTTIME, &CAuthKeyDlg::OnBnClickedAdjusttime)
END_MESSAGE_MAP()


// CAuthKeyDlg message handlers

BOOL CAuthKeyDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	///////////////////////////////////////////////////////
	CString sKey;
	sKey = "Enter your code here";
	SetDlgItemText(IDC_KEY, sKey);
	CString sLabel;
	sLabel = "My site";
	SetDlgItemText(IDC_LABEL, sLabel);
	((CButton*)GetDlgItem(IDC_ADJUSTTIME))->SetCheck(TRUE);
	((CButton*)GetDlgItem(IDC_ENCRYPTION))->SetCheck(TRUE);
	///////////////////////////////////////////////////////

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CAuthKeyDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CAuthKeyDlg::OnPaint()
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
HCURSOR CAuthKeyDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

int DecodeBASE32(CString sKey, unsigned char* pcDecode, int* pinDecode) {
	const int istrlen = sKey.GetLength();
	int ilen = 0;
	for (int i=0; i<istrlen; i++) {
		if (sKey.GetAt(i) == ' ') continue;
		ilen++;
	}
	*pinDecode = (ilen * 5 % 8) ? (ilen * 5 / 8 + 1) : (ilen * 5 / 8);
	if (*pinDecode > BASE32SIZE) return 16111820;

	for (int i=0; i<*pinDecode; i++) {pcDecode[i] = 0;}

	int iout = 128;
	int idx = 0;
	for (int i=0; i<istrlen; i++) {
		char cLett = sKey.GetAt(i);
		if ((cLett >= 'A')&&(cLett <= 'Z')) cLett -= 'A';
		else if ((cLett >= 'a')&&(cLett <= 'z')) cLett -= 'a';
		else if ((cLett >= '2')&&(cLett <= '7')) cLett = cLett - '2' + 26;
		else if (cLett == '=') cLett = 0;
		else continue;
		int imask = 32;
		for (int j=0; j<5; j++) {
			imask = imask >> 1;
			if (cLett & imask) pcDecode[(idx*5+j)>>3] |= iout;
			iout = iout >> 1;
			if (iout == 0) iout = 128;
		}
		idx++;
	}
	return 0;
}

void GetUTC(unsigned long long* pullUTC) {
	__time64_t t64time;
	time( &t64time );
	*pullUTC = t64time;
}

unsigned int CAuthKeyDlg::AdjustTime() {
	unsigned char buf[CUSBHIDIOC_BUFSIZE];//110920
	CUsbhidioc USB;//110920

	bool bResetCalib = false;
	if (((CButton*)GetDlgItem(IDC_RESETCALIB))->GetCheck() == BST_CHECKED) bResetCalib = true;

	buf[0] = 'T';
	unsigned long long ullUTC = 0;
	GetUTC(&ullUTC);
	for (int i=7; i>=0; i--) {
		buf[i+1] = (unsigned char)(ullUTC & 0xff);
		ullUTC = ullUTC >> 8;
	}
	if (bResetCalib) buf[9] = 1; else buf[9] = 0;
	
    if (!USB.WriteReport (buf, CUSBHIDIOC_BUFSIZE)) {
		AfxMessageBox("Device not found");
		return 17032001;
	}
	USB.CloseReport();
	return 0;
}

void CAuthKeyDlg::OnBnClickedRegisterkey()
{
	// TODO: ここにコントロール通知ハンドラ コードを追加します。
	unsigned char buf[CUSBHIDIOC_BUFSIZE];//110920
	CUsbhidioc USB;//110920

	CString sKey;
	GetDlgItemText(IDC_KEY, sKey);

	CString sLabel;
	GetDlgItemText(IDC_LABEL, sLabel);

	int isel = m_cmbEntries.GetCurSel();
	CString sItemText = "1: ";
	if (isel >= 0) m_cmbEntries.GetLBText(isel, sItemText);
	isel = atoi(sItemText.SpanExcluding(":")) - 1;
	if ((isel < 0)||(isel >= MAX_ENTRY)) {
		AfxMessageBox("Entry# error"); return;
	}
	if (AfxMessageBox("Register the key to entry " + sItemText + "?", MB_YESNO) == IDNO) return;

	bool bEncrypt = false;
	if (((CButton*)GetDlgItem(IDC_ENCRYPTION))->GetCheck() == BST_CHECKED) bEncrypt = true;

	unsigned char pcKey[BASE32SIZE];
	int ncKey = 0;
	if (DecodeBASE32(sKey, pcKey, &ncKey)) {
		AfxMessageBox("Too long key"); return;
	}

	CLongInt ib, im, ie;
	LongIntInit(&ib);
	LongIntInit(&im);
	LongIntInit(&ie);
	unsigned int uiLen1 = 0;

	if (bEncrypt) {
		buf[0] = 'C';//get encryption keys
		if (!USB.WriteReport (buf, CUSBHIDIOC_BUFSIZE)) {
			AfxMessageBox("Device not found");
			return;
		}

		for(int i=0; i<CUSBHIDIOC_BUFSIZE; i++) buf[i] = 0;
		if (!USB.ReadReport(buf)) {
			AfxMessageBox("Device error");
			return;
		}

//		CString msg2 = "encrypt keys\r\n", line2;
//		for (int i=0; i<CUSBHIDIOC_BUFSIZE; i+=8) {
//			line2.Format("%2d %2d %2d %2d %2d %2d %2d %2d\r\n",
//				buf[i+0], buf[i+1], buf[i+2], buf[i+3], buf[i+4], buf[i+5], buf[i+6], buf[i+7]);
//			msg2 += line2;
//		}

		LongIntSet(&ib, pcKey, 20);
//		msg2 += "\r\noriginal code\r\n";
//		msg2 += LongIntGetHex(&ib) + "\r\n";

		uiLen1 = buf[0];
		const unsigned int uiLen2 = buf[uiLen1+1];

		LongIntSet(&im, &(buf[1]), uiLen1);
		LongIntSet(&ie, &(buf[uiLen1+2]), uiLen2);
		//
		LongIntPowerMod(&ib, &ie, &im);
//		msg2 += "\r\nencrypted code\r\n";
//		msg2 += LongIntGetHex(&ib) + "\r\n";
		//AfxMessageBox(msg2);
	}

	buf[0] = 'K';
	buf[1] = isel;//position
	buf[2] = ncKey;
	buf[3] = uiLen1;
	//buf[4-11]: label
	for (int i=0; i<LABELSIZE; i++) {
		if (sLabel.GetLength() > i) {buf[i+4] = sLabel.GetAt(i);}
		else buf[i+4] = ' ';
	}
	CString msg = "Key: " + sLabel + "\r\n", line;
	line.Format("buf2=%d buf3=%d\r\n", buf[2], buf[3]); msg += line;
	if (bEncrypt) {
		msg += "\r\nsending enrypted code\r\n";
		for (int i=0; i<(int)uiLen1; i++) {
			buf[i+12] = ib.pElem[i];
			line.Format("%2x ", buf[i+12]); msg += line;
		}
	} else {
		msg += "\r\nsending plane code\r\n";
		for (int i=0; i<ncKey; i++) {
			buf[i+12] = pcKey[i];
			line.Format("%2x ", buf[i+12]); msg += line;
		}
	}
	msg += "\r\n";

//	AfxMessageBox(msg);
    if (!USB.WriteReport (buf, CUSBHIDIOC_BUFSIZE)) {
		AfxMessageBox("Device not found");
		return;
	}
	USB.CloseReport();

	SetDlgItemText(IDC_KEY, "Please wait.");
	GetDlgItem(IDC_KEY)->Invalidate();
	GetDlgItem(IDC_KEY)->UpdateWindow();

	if (bEncrypt) {
		Sleep(13000);
		OnBnClickedUpdate();
	} else {
		Sleep(300);
	}
	OnBnClickedUpdate();
	const int nitem = m_cmbEntries.GetCount();
	for (int i=0; i<nitem; i++) {
		m_cmbEntries.GetLBText(i, sItemText);
		if (isel == atoi(sItemText.SpanExcluding(":"))-1) {
			m_cmbEntries.SetCurSel(i);
			break;
		}
	}
	SetDlgItemText(IDC_KEY, "stored");
}

void CAuthKeyDlg::OnBnClickedUpdate()
{
	if (((CButton*)GetDlgItem(IDC_ADJUSTTIME))->GetCheck() == BST_CHECKED) {
		if (AdjustTime()) return;
	}//170320

	unsigned char buf[CUSBHIDIOC_BUFSIZE];//110920
	CUsbhidioc USB;//110920

	buf[0] = 'L';
    if (!USB.WriteReport (buf, CUSBHIDIOC_BUFSIZE)) {
		AfxMessageBox("Device not found");
		return;
	}

	for(int i=0; i<CUSBHIDIOC_BUFSIZE; i++) buf[i] = 0;
	if (!USB.ReadReport(buf)) {
		AfxMessageBox("Device error");
		return;
	}

	m_cmbEntries.ResetContent();
//	CString msg = "", line;
	int isum = 0;
	for (int i=0; i<CUSBHIDIOC_BUFSIZE; i+=LABELSIZE) {
//		line.Format("%2x %2x %2x %2x %2x %2x %2x %2x\r\n",
//			buf[i+0], buf[i+1], buf[i+2], buf[i+3], buf[i+4], buf[i+5], buf[i+6], buf[i+7]);
//		msg += line;
		CString sLabel;
		sLabel.Format("%d:", (i / LABELSIZE)+1 );
		if (buf[i] == 0xff) sLabel += "(blank)";
		else {
			for (int j=0; j<LABELSIZE; j++) {sLabel += buf[i+j];}
		}
		m_cmbEntries.AddString(sLabel);
		//msg += sLabel + "\r\n";
	}
	m_cmbEntries.SetCurSel(0);

	USB.CloseReport();
}

void CAuthKeyDlg::OnBnClickedClear()
{
	unsigned char buf[CUSBHIDIOC_BUFSIZE];//110920
	CUsbhidioc USB;//110920

	int isel = m_cmbEntries.GetCurSel();
	CString sItemText = "1: ";
	if (isel >= 0) m_cmbEntries.GetLBText(isel, sItemText);
	isel = atoi(sItemText.SpanExcluding(":")) - 1;
	if ((isel < 0)||(isel >= MAX_ENTRY)) {
		AfxMessageBox("Entry# error"); return;
	}
	if (AfxMessageBox("Delete " + sItemText + "?", MB_YESNO) == IDNO) return;

	buf[0] = 'K';
	buf[1] = isel;//position
	buf[2] = BASE32SIZE;
	buf[3] = 0;
	for (int i=0; i<LABELSIZE; i++) {buf[i+4] = 0xff;}
	//buf[3-10]: label
	for (int i=0; i<BASE32SIZE; i++) {
		buf[i+12] = 0xff;
	}
    if (!USB.WriteReport (buf, CUSBHIDIOC_BUFSIZE)) {
		AfxMessageBox("Device not found");
		return;
	}
	USB.CloseReport();

	Sleep(200);
	OnBnClickedUpdate();
}

void CAuthKeyDlg::OnBnClickedAdjusttime()
{
	if (((CButton*)GetDlgItem(IDC_ADJUSTTIME))->GetCheck() == BST_CHECKED) {
		GetDlgItem(IDC_RESETCALIB)->EnableWindow(TRUE);
	} else {
		GetDlgItem(IDC_RESETCALIB)->EnableWindow(FALSE);
	}
}
