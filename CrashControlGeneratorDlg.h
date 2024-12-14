
// CrashControlGeneratorDlg.h : header file
//

#pragma once


// CCrashControlGeneratorDlg dialog
class CCrashControlGeneratorDlg : public CDialogEx
{
// Construction
public:
	CCrashControlGeneratorDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CRASHCONTROLGENERATOR_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

public:
    afx_msg void OnBnClickedCheckRightCtrl();
    afx_msg void OnBnClickedCheckLeftShift();
    afx_msg void OnBnClickedCheckRightShift();
    afx_msg void OnBnClickedCheckLeftAlt();
    afx_msg void OnBnClickedCheckRightAlt();
    afx_msg void OnBnClickedCheckLeftCtrl();

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT OnScanCode(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
private:
	void Generate();

	BOOL m_leftShift{ FALSE };
	BOOL m_rightShift{ FALSE };
	BOOL m_leftAlt{ FALSE };
	BOOL m_rightAlt{ FALSE };
	BOOL m_leftCtrl{ FALSE };
	BOOL m_rightCtrl{ TRUE };
	CString m_scanCodeText;
	CString m_indexText;
	CString m_registryContent;

	UCHAR m_scanCode{0x46};
public:
	afx_msg void OnDestroy();
};
