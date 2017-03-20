// Usbhidioc.h : ヘッダー ファイル
//

#define CUSBHIDIOC_BUFSIZE 64

/////////////////////////////////////////////////////////////////////////////
// CUsbhidioc ウィンドウ

class CUsbhidioc : public CWnd
{
// コンストラクション
public:
	CUsbhidioc();

// アトリビュート
public:

// オペレーション
public:

// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。

	//{{AFX_VIRTUAL(CUsbhidioc)
	//}}AFX_VIRTUAL

public:
    bool ReadReport (unsigned char *);
    bool WriteReport(unsigned char *, unsigned int);
    void CloseReport ();

protected:
    bool FindTheHID();
    void GetDeviceCapabilities();
    void PrepareForOverlappedTransfer();

// インプリメンテーション
public:
	virtual ~CUsbhidioc();

	// 生成されたメッセージ マップ関数
protected:
	//{{AFX_MSG(CUsbhidioc)
		// メモ - ClassWizard はこの位置にメンバ関数を追加または削除します。
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
