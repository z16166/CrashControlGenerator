
// CrashControlGeneratorDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "CrashControlGenerator.h"
#include "CrashControlGeneratorDlg.h"
#include "afxdialogex.h"

#include <string>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WM_USER_SCAN_CODE (WM_USER + 1)

// https://learn.microsoft.com/en-us/windows-hardware/drivers/debugger/forcing-a-system-crash-from-the-keyboard

constexpr int leftShiftMask = 0x10;
constexpr int leftAltMask = 0x40;
constexpr int leftCtrlMask = 0x20;
constexpr int rightShiftMask = 0x01;
constexpr int rightAltMask = 0x04;
constexpr int rightCtrlMask = 0x02;

static const WCHAR registryTemplate[] = 
LR"(Windows Registry Editor Version 5.00

[HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\i8042prt\Parameters]
"CrashOnCtrlScroll"=dword:00000000

[HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\kbdhid\Parameters]
"CrashOnCtrlScroll"=dword:00000000

[HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\hyperkbd\Parameters]
"CrashOnCtrlScroll"=dword:00000000

[HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\i8042prt\crashdump]
"Dump1Keys"=dword:%08x
"Dump2Key"=dword:%08x

[HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\kbdhid\crashdump]
"Dump1Keys"=dword:%08x
"Dump2Key"=dword:%08x

[HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\hyperkbd\crashdump]
"Dump1Keys"=dword:%08x
"Dump2Key"=dword:%08x

[HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\CrashControl]
"AutoReboot"=dword:00000001
"CrashDumpEnabled"=dword:00000001
"DumpFile"=hex(2):25,00,53,00,79,00,73,00,74,00,65,00,6d,00,52,00,6f,00,6f,00,\
  74,00,25,00,5c,00,4d,00,45,00,4d,00,4f,00,52,00,59,00,2e,00,44,00,4d,00,50,\
  00,00,00
"DumpLogLevel"=dword:00000000
"EnableLogFile"=dword:00000001
"LogEvent"=dword:00000001
"MinidumpDir"=hex(2):25,00,53,00,79,00,73,00,74,00,65,00,6d,00,52,00,6f,00,6f,\
  00,74,00,25,00,5c,00,4d,00,69,00,6e,00,69,00,64,00,75,00,6d,00,70,00,00,00
"MinidumpsCount"=dword:00000005
"Overwrite"=dword:00000001
"DumpFilters"=hex(7):64,00,75,00,6d,00,70,00,66,00,76,00,65,00,2e,00,73,00,79,\
  00,73,00,00,00,00,00
"AlwaysKeepMemoryDump"=dword:00000000
"FilterPages"=dword:00000001
)";

HHOOK g_hKeyboardHook = NULL;

// 钩子回调函数
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0 && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)) {
        KBDLLHOOKSTRUCT* pKeyboard = (KBDLLHOOKSTRUCT*)lParam;

        // 获取虚拟键码和扫描码
        UINT virtualKey = pKeyboard->vkCode;
        UINT scanCode = pKeyboard->scanCode;

        // 获取主窗口句柄
        HWND hMainWnd = AfxGetMainWnd()->GetSafeHwnd();
        if (hMainWnd) {
            // 发送扫描码到主窗口
            PostMessage(hMainWnd, WM_USER_SCAN_CODE, virtualKey, scanCode);
        }
    }

    return CallNextHookEx(g_hKeyboardHook, nCode, wParam, lParam);
}

// 设置钩子
void SetKeyboardHook() {
    g_hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, NULL, 0);
}

// 卸载钩子
void RemoveKeyboardHook() {
    if (g_hKeyboardHook) {
        UnhookWindowsHookEx(g_hKeyboardHook);
        g_hKeyboardHook = NULL;
    }
}

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CCrashControlGeneratorDlg dialog



CCrashControlGeneratorDlg::CCrashControlGeneratorDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CRASHCONTROLGENERATOR_DIALOG, pParent)
	, m_leftShift(FALSE)
	, m_rightShift(FALSE)
	, m_leftAlt(FALSE)
	, m_rightAlt(FALSE)
	, m_rightCtrl(TRUE)
	, m_scanCodeText(_T(""))
	, m_indexText(_T(""))
	, m_registryContent(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CCrashControlGeneratorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK_LEFT_SHIFT, m_leftShift);
	DDX_Check(pDX, IDC_CHECK_RIGHT_SHIFT, m_rightShift);
	DDX_Check(pDX, IDC_CHECK_LEFT_ALT, m_leftAlt);
	DDX_Check(pDX, IDC_CHECK_RIGHT_ALT, m_rightAlt);
	DDX_Check(pDX, IDC_CHECK_LEFT_CTRL, m_leftCtrl);
	DDX_Check(pDX, IDC_CHECK_RIGHT_CTRL, m_rightCtrl);
	DDX_Text(pDX, IDC_EDIT_SCAN_CODE, m_scanCodeText);
	DDX_Text(pDX, IDC_EDIT_INDEX, m_indexText);
	DDX_Text(pDX, IDC_EDIT_REGISTRY_CONTENT, m_registryContent);
}

BEGIN_MESSAGE_MAP(CCrashControlGeneratorDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_CHECK_RIGHT_CTRL, &CCrashControlGeneratorDlg::OnBnClickedCheckRightCtrl)
	ON_BN_CLICKED(IDC_CHECK_LEFT_SHIFT, &CCrashControlGeneratorDlg::OnBnClickedCheckLeftShift)
	ON_BN_CLICKED(IDC_CHECK_RIGHT_SHIFT, &CCrashControlGeneratorDlg::OnBnClickedCheckRightShift)
	ON_BN_CLICKED(IDC_CHECK_LEFT_ALT, &CCrashControlGeneratorDlg::OnBnClickedCheckLeftAlt)
	ON_BN_CLICKED(IDC_CHECK_RIGHT_ALT, &CCrashControlGeneratorDlg::OnBnClickedCheckRightAlt)
	ON_BN_CLICKED(IDC_CHECK_LEFT_CTRL, &CCrashControlGeneratorDlg::OnBnClickedCheckLeftCtrl)
	ON_MESSAGE(WM_USER_SCAN_CODE, &CCrashControlGeneratorDlg::OnScanCode)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CCrashControlGeneratorDlg message handlers

BOOL CCrashControlGeneratorDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
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

	SetKeyboardHook();
	Generate();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CCrashControlGeneratorDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CCrashControlGeneratorDlg::OnPaint()
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
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CCrashControlGeneratorDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CCrashControlGeneratorDlg::OnBnClickedCheckRightCtrl()
{
    UpdateData();
    Generate();
}

void CCrashControlGeneratorDlg::OnBnClickedCheckLeftShift()
{
    UpdateData();
    Generate();
}

void CCrashControlGeneratorDlg::OnBnClickedCheckRightShift()
{
    UpdateData();
    Generate();
}

void CCrashControlGeneratorDlg::OnBnClickedCheckLeftAlt()
{
    UpdateData();
    Generate();
}

void CCrashControlGeneratorDlg::OnBnClickedCheckRightAlt()
{
    UpdateData();
    Generate();
}

void CCrashControlGeneratorDlg::OnBnClickedCheckLeftCtrl()
{
    UpdateData();
    Generate();
}

void CCrashControlGeneratorDlg::Generate() {
	int dump1Keys = 0;

	if (m_leftShift) dump1Keys |= leftShiftMask;
	if (m_leftAlt) dump1Keys |= leftAltMask;
	if (m_leftCtrl) dump1Keys |= leftCtrlMask;

    if (m_rightShift) dump1Keys |= rightShiftMask;
    if (m_rightAlt) dump1Keys |= rightAltMask;
    if (m_rightCtrl) dump1Keys |= rightCtrlMask;

	if (dump1Keys == 0) {
		m_registryContent.Empty();
	}
	else {
        static const UCHAR keyToScanTbl[134] = {
        0x00,0x29,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,
        0x0A,0x0B,0x0C,0x0D,0x7D,0x0E,0x0F,0x10,0x11,0x12,
        0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x00,
        0x3A,0x1E,0x1F,0x20,0x21,0x22,0x23,0x24,0x25,0x26,
        0x27,0x28,0x2B,0x1C,0x2A,0x00,0x2C,0x2D,0x2E,0x2F,
        0x30,0x31,0x32,0x33,0x34,0x35,0x73,0x36,0x1D,0x00,
        0x38,0x39,0xB8,0x00,0x9D,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0xD2,0xD3,0x00,0x00,0xCB,
        0xC7,0xCF,0x00,0xC8,0xD0,0xC9,0xD1,0x00,0x00,0xCD,
        0x45,0x47,0x4B,0x4F,0x00,0xB5,0x48,0x4C,0x50,0x52,
        0x37,0x49,0x4D,0x51,0x53,0x4A,0x4E,0x00,0x9C,0x00,
        0x01,0x00,0x3B,0x3C,0x3D,0x3E,0x3F,0x40,0x41,0x42,
        0x43,0x44,0x57,0x58,0x00,0x46,0x00,0x00,0x00,0x00,
        0x00,0x7B,0x79,0x70 };

		int dump2Keys = -1;
		for (int k = 0; k < sizeof(keyToScanTbl); k++) {
			if (keyToScanTbl[k] == m_scanCode) {
				dump2Keys = k;
				break;
			}
		}

		if (dump2Keys == -1)
		{
		}
		else {
			m_scanCodeText.Format(L"0x%02x", m_scanCode);
			m_indexText.Format(L"0x%08x", dump2Keys);

			CString s;
			s.Format(registryTemplate, dump1Keys, dump2Keys, dump1Keys, dump2Keys, dump1Keys, dump2Keys);

            std::wstring processedText((LPCWSTR)s);
            size_t pos = 0;
            while ((pos = processedText.find(L'\n', pos)) != std::wstring::npos) {
                processedText.replace(pos, 1, L"\r\n");
                pos += 2;
            }

			m_registryContent = processedText.c_str();
		}
	}

	UpdateData(FALSE);
}

LRESULT CCrashControlGeneratorDlg::OnScanCode(WPARAM wParam, LPARAM lParam) {
    UINT virtualKey = static_cast<UINT>(wParam);
    UINT scanCode = static_cast<UINT>(lParam);

	m_scanCode = (UCHAR)scanCode;
	Generate();

    return 0;
}


void CCrashControlGeneratorDlg::OnDestroy()
{
	RemoveKeyboardHook();

	CDialogEx::OnDestroy();
}
