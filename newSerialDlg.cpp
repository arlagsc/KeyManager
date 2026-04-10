
// newSerialDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "newSerial.h"
#include "newSerialDlg.h"
#include "afxdialogex.h"
//#include <atlconv.h>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CnewSerialDlg 对话框
#define WM_UPDATE_MESSAGE (WM_USER+200)

#define TIME_SHOW_MESSAGE_SERIAL 1000
#define TIME_AUTO_CHANGE_SOURCE 2000
#define TIME_BURN_HDCP		3000
#define TIME_BURN_ULPK			4000
#define TIME_GET_ULPK			5000
#define TIME_GET_SN				6000
#define TIME_GET_RESET_ACK		7000

CnewSerialDlg::CnewSerialDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_NEWSERIAL_DIALOG, pParent)
	, m_strRXData(_T(""))
	, m_EditSend(_T(""))
	, m_EditSend2(_T(""))
	, m_EditSend3(_T(""))
	, m_EditSend4(_T(""))
	, m_EditSend5(_T(""))
	, m_EditSend6(_T(""))
	, m_EditSend7(_T(""))
	, m_EditSend8(_T(""))
	, m_bSendHex(FALSE)
	, m_bSendHex2(FALSE)
	, m_bSendHex3(FALSE)
	, m_bSendHex4(FALSE)
	, m_bSendHex5(FALSE)
	, m_bSendHex6(FALSE)
	, m_bSendHex7(FALSE)
	, m_bSendHex8(FALSE)
	, bShowHex(TRUE)
	, m_bAutoFacMode(FALSE)
	, m_bAutoLF(FALSE)
	, m_bAutoClear(FALSE)
	, m_bFilter(FALSE)
	, m_strFilter(_T(""))
	, m_bAutoEnter(FALSE)
	, m_autoDelSpec(TRUE)
	, m_strEditMAC(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CnewSerialDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MSCOMM1, m_ctrlComm);
	//DDX_Text(pDX, IDC_EDIT_RX, m_strRXData);
	DDX_CBString(pDX, IDC_COMBO_PORT_SELECT, m_port);
	DDX_CBString(pDX, IDC_COMBO_BAUD_SELECT, m_baud);
	//DDX_Text(pDX, IDC_EDIT_TX, m_EditSend);
	//DDX_Text(pDX, IDC_EDIT_TX2, m_EditSend2);
	//DDX_Text(pDX, IDC_EDIT_TX3, m_EditSend3);
	//DDX_Text(pDX, IDC_EDIT_TX4, m_EditSend4);
	//DDX_Text(pDX, IDC_EDIT_TX5, m_EditSend5);
	//DDX_Text(pDX, IDC_EDIT_TX6, m_EditSend6);
	//DDX_Text(pDX, IDC_EDIT_TX7, m_EditSend7);
	//DDX_Text(pDX, IDC_EDIT_TX8, m_EditSend8);
	//DDX_Check(pDX, IDC_CHECK_SEND_HEX, m_bSendHex);
	//DDX_Check(pDX, IDC_CHECK_SEND_HEX2, m_bSendHex2);
	//DDX_Check(pDX, IDC_CHECK_SEND_HEX3, m_bSendHex3);
	//DDX_Check(pDX, IDC_CHECK_SEND_HEX4, m_bSendHex4);
	//DDX_Check(pDX, IDC_CHECK_SEND_HEX5, m_bSendHex5);
	//DDX_Check(pDX, IDC_CHECK_SEND_HEX6, m_bSendHex6);
	//DDX_Check(pDX, IDC_CHECK_SEND_HEX7, m_bSendHex7);
	//DDX_Check(pDX, IDC_CHECK_SEND_HEX8, m_bSendHex8);
	DDX_Check(pDX, IDC_CHECK_SHOW_HEX, bShowHex);
	DDX_Check(pDX, IDC_CHECK_AUTO_FAC_MODE, m_bAutoFacMode);
	
	//DDX_Check(pDX, IDC_CHECK_AUTO_LF, m_bAutoLF);
	//DDX_Check(pDX, IDC_CHECK_AUTO_CLEAR, m_bAutoClear);
	//DDX_Check(pDX, IDC_CHECK_FILTER, m_bFilter);
	//DDX_Text(pDX, IDC_EDIT_FILTER, m_strFilter);
	DDX_Control(pDX, IDC_EDIT_RX, m_editRX);
	DDX_Control(pDX, IDC_PIC, m_image);
	DDX_Control(pDX, IDC_PIC2, m_image2);
	//DDX_Check(pDX, IDC_CHECK_ADD_ENTER, m_bAutoEnter);
	DDX_Control(pDX, IDC_MFCBUTTON_TEST, m_mfcButtonTest);
	//DDX_Control(pDX, IDC_COMBO_COMMAND_LIST, m_comList);
	//DDX_Text(pDX, IDC_EDIT_COMLIST_NAME, m_strEditListName);
	//DDX_Check(pDX, IDC_CHECK_DEL_SPEC, m_autoDelSpec);
	DDX_Text(pDX, IDC_EDIT_SN, m_strEdit_GetSN);

	DDX_Text(pDX, IDC_EDIT_MAC, m_strEditMAC);
	DDX_Text(pDX, IDC_EDIT_SEND_DATA, m_EditSend);
	DDX_Text(pDX, IDC_EDIT_SN_INDEX, m_strSnIndex);
	
}

BEGIN_MESSAGE_MAP(CnewSerialDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WM_UPDATE_MESSAGE, OnUpdateMessage)
	ON_WM_TIMER()
	//ON_EVENT(CMSCOMM1, IDC_MSCOMM1, 1, CnewSerialDlg::OnCommMscomm1, VTS_NONE)
	ON_EN_UPDATE(IDC_EDIT_RX, &CnewSerialDlg::OnEnUpdateEdit1)
	ON_CBN_SETFOCUS(IDC_COMBO_PORT_SELECT, &CnewSerialDlg::OnCbnSetfocusComboPortSelect)
	ON_CBN_CLOSEUP(IDC_COMBO_BAUD_SELECT, &CnewSerialDlg::OnCbnCloseupComboBaudSelect)
	//ON_BN_CLICKED(IDC_BUTTON_SERIAL_CONTROL, &CnewSerialDlg::OnBnClickedButtonSerialControl)
	//ON_BN_CLICKED(IDC_BUTTON_SEND, &CnewSerialDlg::OnBnClickedButtonSend1)
	//ON_BN_CLICKED(IDC_BUTTON_SEND2, &CnewSerialDlg::OnBnClickedButtonSend2)
	//ON_BN_CLICKED(IDC_BUTTON_SEND3, &CnewSerialDlg::OnBnClickedButtonSend3)
	//ON_BN_CLICKED(IDC_BUTTON_SEND4, &CnewSerialDlg::OnBnClickedButtonSend4)
	//ON_BN_CLICKED(IDC_BUTTON_SEND5, &CnewSerialDlg::OnBnClickedButtonSend5)
	//ON_BN_CLICKED(IDC_BUTTON_SEND7, &CnewSerialDlg::OnBnClickedButtonSend6)
	//ON_BN_CLICKED(IDC_BUTTON_SEND8, &CnewSerialDlg::OnBnClickedButtonSend7)
	//ON_BN_CLICKED(IDC_BUTTON_SEND9, &CnewSerialDlg::OnBnClickedButtonSend8)
	//ON_BN_CLICKED(IDC_BUTTON_TEST, &CnewSerialDlg::OnBnClickedButtonTest)
	//ON_BN_CLICKED(IDC_BUTTON_SAVE_COMLIST, &CnewSerialDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, &CnewSerialDlg::OnBnClickedButtonSave)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR, &CnewSerialDlg::OnBnClickedButtonClear)
	ON_BN_CLICKED(IDC_CHECK_SHOW_HEX, &CnewSerialDlg::OnBnClickedCheckShowHex)
	//ON_BN_CLICKED(IDC_CHECK_SEND_HEX, &CnewSerialDlg::OnBnClickedCheckSendHex)
	//ON_BN_CLICKED(IDC_CHECK_SEND_HEX2, &CnewSerialDlg::OnBnClickedCheckSendHex2)
	//ON_BN_CLICKED(IDC_CHECK_SEND_HEX3, &CnewSerialDlg::OnBnClickedCheckSendHex3)
	//ON_BN_CLICKED(IDC_CHECK_SEND_HEX4, &CnewSerialDlg::OnBnClickedCheckSendHex4)
	//ON_BN_CLICKED(IDC_CHECK_SEND_HEX5, &CnewSerialDlg::OnBnClickedCheckSendHex5)
	//ON_BN_CLICKED(IDC_CHECK_SEND_HEX6, &CnewSerialDlg::OnBnClickedCheckSendHex6)
	//ON_BN_CLICKED(IDC_CHECK_SEND_HEX7, &CnewSerialDlg::OnBnClickedCheckSendHex7)
	//ON_BN_CLICKED(IDC_CHECK_SEND_HEX8, &CnewSerialDlg::OnBnClickedCheckSendHex8)
	//ON_BN_CLICKED(IDC_CHECK_AUTO_CLEAR, &CnewSerialDlg::OnBnClickedCheckAutoClear)
	//ON_BN_CLICKED(IDC_CHECK_AUTO_LF, &CnewSerialDlg::OnBnClickedCheckAutoLf)
	//ON_BN_CLICKED(IDC_CHECK_FILTER, &CnewSerialDlg::OnBnClickedCheckFilter)
	//ON_EN_CHANGE(IDC_EDIT_FILTER, &CnewSerialDlg::OnEnChangeEditFilter)
	ON_BN_CLICKED(IDC_BUTTON_PAUSE, &CnewSerialDlg::OnBnClickedButtonPause)
	//ON_BN_CLICKED(IDC_BUTTON4, &CnewSerialDlg::OnBnClickedButton4)
	//ON_BN_CLICKED(IDC_CHECK_ADD_ENTER, &CnewSerialDlg::OnBnClickedCheckAddEnter)
	ON_BN_CLICKED(IDC_MFCBUTTON_TEST, &CnewSerialDlg::OnBnClickedMfcbuttonTest)
	//ON_BN_CLICKED(IDC_BUTTON_LOAD_COMLIST, &CnewSerialDlg::OnBnClickedButtonLoadComlist)
	//ON_CBN_SELCHANGE(IDC_COMBO_COMMAND_LIST, &CnewSerialDlg::OnCbnSelchangeComboCommandList)
	//ON_BN_CLICKED(IDC_CHECK_DEL_SPEC, &CnewSerialDlg::OnBnClickedCheckDelSpec)
	//ON_BN_CLICKED(IDC_BUTTON1, &CnewSerialDlg::OnBnClickedButton1)
	//ON_BN_CLICKED(IDC_BUTTON5, &CnewSerialDlg::OnBnClickedButton5)
	//ON_BN_CLICKED(IDC_BUTTON6, &CnewSerialDlg::OnBnClickedButton6)
	//ON_BN_CLICKED(IDC_BUTTON7, &CnewSerialDlg::OnBnClickedButton7)
	ON_BN_CLICKED(IDC_BUTTON_SET_SN, &CnewSerialDlg::OnBnClickedButtonSetSn)
	ON_BN_CLICKED(IDC_BUTTON_OPEN_KEY, &CnewSerialDlg::OnBnClickedButtonOpenKey)
	ON_BN_CLICKED(IDC_BUTTON_BURN_ULPK, &CnewSerialDlg::OnBnClickedButtonBurnUlpk)
	ON_BN_CLICKED(IDC_BUTTON_BURN_MAC, &CnewSerialDlg::OnBnClickedButtonBurnMac)
	ON_BN_CLICKED(IDC_BUTTON_GET_UID, &CnewSerialDlg::OnBnClickedButtonGetUid)
	ON_BN_CLICKED(IDC_BUTTON_LOAD_HDCP14, &CnewSerialDlg::OnBnClickedButtonLoadHdcp14)
	ON_BN_CLICKED(IDC_BUTTON_LOAD_HDCP22, &CnewSerialDlg::OnBnClickedButtonLoadHdcp22)
	ON_BN_CLICKED(IDC_BUTTON_BURN_HDCP, &CnewSerialDlg::OnBnClickedButtonBurnHdcp)
	ON_BN_CLICKED(IDC_BUTTON_SEND, &CnewSerialDlg::OnBnClickedButtonSend)
	ON_BN_CLICKED(IDC_BUTTON10, &CnewSerialDlg::OnBnClickedButton10)
	ON_BN_CLICKED(IDC_BUTTON_READ_SN, &CnewSerialDlg::OnBnClickedButtonReadSn)
	ON_BN_CLICKED(IDC_BUTTON12, &CnewSerialDlg::OnBnClickedButton12)
	ON_BN_CLICKED(IDC_BUTTON_SET_SN2, &CnewSerialDlg::OnBnClickedButtonSetSn2)
	ON_BN_CLICKED(IDC_BUTTON11, &CnewSerialDlg::OnBnClickedButton11)
	ON_EN_CHANGE(IDC_EDIT_SN_INDEX, &CnewSerialDlg::OnEnChangeEditSnIndex)
	ON_BN_CLICKED(IDC_BUTTON_TEST1, &CnewSerialDlg::OnBnClickedButtonTest1)
	ON_BN_CLICKED(IDC_CHECK_AUTO_FAC_MODE, &CnewSerialDlg::OnBnClickedCheckAutoFacMode)
END_MESSAGE_MAP()


// CnewSerialDlg 消息处理程序

BOOL CnewSerialDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	//ShowWindow(SW_MINIMIZE);

	// TODO: 在此添加额外的初始化代码
	m_bIsComOpen = FALSE;
	GetCurrentDirectory(100, m_strConfigFile.GetBuffer(100));
	m_strConfigFile.ReleaseBuffer();
	m_strConfigFile += _T("\\config.ini");

	baud = 38400;

	((CComboBox*)GetDlgItem(IDC_COMBO_BAUD_SELECT))->SetCurSel(5);

	//OnCbnSetfocusComboPortSelect();
	m_comList.AddString(_T("1"));
	m_comList.AddString(_T("2"));
	m_comList.AddString(_T("3"));
	m_comList.AddString(_T("4"));
	m_comList.AddString(_T("5"));
	m_comList.AddString(_T("6"));
	m_comList.AddString(_T("7"));
	m_comList.AddString(_T("8"));

	Readconfig();

	SetTimer(TIME_SHOW_MESSAGE_SERIAL, 20, NULL);
	m_mfcButtonTest.SetButtonBgColor(0x0000FF);
	m_mfcButtonTest.SetButtonTextColor(0xFFFFFF);
	//m_cbuttonConnect.SetButtonBgColor(0x00FF00);
	//m_cbuttonConnect.SetButtonTextColor(0xFF0000);

	GetDlgItem(IDC_BUTTON_BURN_ULPK)->EnableWindow(FALSE);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CnewSerialDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

typedef unsigned char  U8;
typedef unsigned short U16;
typedef unsigned long  U32;
const unsigned short CRC_TABLE[] = {
	0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
		0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF };
U16 calculateCRC_ROKU(const U8* pu8DataBuf, U32 u32DataSize)//(ctx as Object, buffer as Dynamic)
{
	U16 checksum;
	{
		checksum = 0xFFFF;
		for (U32 i = 0; i < u32DataSize; ++i)//(i = 0; to buffer.count() - 1)
		{
			U16 ucTemp = (unsigned char)(checksum >> 0x0C);
			checksum <<= 4;
			checksum = checksum & 0xffff;
			checksum ^= CRC_TABLE[ucTemp ^ ((pu8DataBuf[i]) >> 0x04)];
			checksum = checksum & 0xffff;
			ucTemp = (unsigned char)(checksum >> 0x0C);
			checksum <<= 4;
			checksum ^= CRC_TABLE[ucTemp ^ ((pu8DataBuf[i]) & 0x0F)];
			checksum = checksum & 0xffff;
		}
	}
	return checksum;
}
//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CnewSerialDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

LRESULT CnewSerialDlg::OnUpdateMessage(WPARAM wParam, LPARAM lParam)
{
	//UpdateData(false);
	if ((wParam == 0) && (lParam == 0))
	{
		if (m_bAutoClear)
		{
			if ((((CEdit*)(GetDlgItem(IDC_EDIT_RX)))->GetLineCount() > 30) || (m_strRXData.GetLength() > 2000))
			{
				m_strRXData.Empty();
				GetDlgItem(IDC_EDIT_RX)->SetWindowText(m_strRXData);
			}
		}
			
		//UpdateData(FALSE);
		GetDlgItem(IDC_EDIT_RX)->SetWindowText(m_strRXData);
		//int line = ((CEdit*)(GetDlgItem(IDC_EDIT_RX)))->GetLineCount();//选中最后一行
		//((CEdit*)(GetDlgItem(IDC_EDIT_RX)))->LineScroll(line);
		//((CEdit*)GetDlgItem(IDC_EDIT_RX))->LineScroll(((CEdit*)GetDlgItem(IDC_EDIT_RX))->GetLineCount() - 1, 0);
		this->SendDlgItemMessage(IDC_EDIT_RX, WM_VSCROLL, SB_BOTTOM, 0); //滚动条始终在底部
		//if(m_bAutoLF)
		
#if ENABLE_DEBUG
		((CEdit*)GetDlgItem(IDC_EDIT_TV_RXD))->LineScroll(((CEdit*)GetDlgItem(IDC_EDIT_TV_RXD))->GetLineCount() - 1, 0);
#endif
	}
	/*else if ((wParam == 1) && (lParam == 1))
	{
		CString tmp;
		tmp.Format("%d", m_intTimer);
		GetDlgItem(IDC_STATIC_TIMER)->SetWindowText(tmp);
	}*/
	return 1;
}

BOOL CnewSerialDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN)    //捕捉到键盘
	{
		CString tmp;
		tmp.Format(_T("%x"), pMsg->wParam);
		//MessageBox(tmp);
		if (!m_bIsComOpen)
			return FALSE;
		switch (pMsg->wParam)
		{
		case VK_RETURN:
			{
				//CString tmp;
				//tmp.Format(_T("%x"), pMsg->wParam);
				//MessageBox(tmp);
			if (GetDlgItem(IDC_EDIT_SEND_DATA) == GetFocus())
			{
				m_bEnterKey = TRUE;
				OnBnClickedButtonSend();
				m_bEnterKey = FALSE;
				m_EditSend.Empty();
				UpdateData(FALSE);
			}
#if 0
				if (GetDlgItem(IDC_EDIT_RX) == GetFocus())
				{
#if 1
					if (m_strRXData.Find("input#") != -1)
					{
						int pos;
						GetDlgItem(IDC_EDIT_RX)->GetWindowText(m_strRXData);
						pos = m_strRXData.Find("input#");
						m_strRXData.Mid(pos);
						m_strRXData += "\r\n";
						m_ctrlComm.put_Output(COleVariant(m_strRXData));
					}
					else
					{
						m_strRXData = "\r\ninput#";
						GetDlgItem(IDC_EDIT_RX)->SetWindowText(m_strRXData);
					}
#else
					if (!m_bInput)
					{
						m_bPause = TRUE;
						m_bInput = TRUE;
						m_strRXData.Empty();
						UpdateData(FALSE);
						m_ctrlComm.put_Output(COleVariant(_T("\r\n")));
					}
					else
					{
						m_bPause = FALSE;
						m_bInput = FALSE;
						UpdateData(TRUE);
						m_strRXData += "\r\n";
						m_ctrlComm.put_Output(COleVariant(m_strRXData));
			
					}
#endif
				}
#endif
				
			}
			return TRUE;
		case 'C'://Ctrl+C
			if (GetKeyState(VK_CONTROL))
			{
				CByteArray HexDataBuf;
				//HexDataBuf.RemoveAll();
				HexDataBuf.SetSize(2);
				HexDataBuf.SetAt(0, 0x03);
				HexDataBuf.SetAt(1, 0x1A);
				
				m_ctrlComm.put_Output(COleVariant(HexDataBuf));
			}
			break;

		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}

void CnewSerialDlg::OnEnUpdateEdit1()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数，以将 EM_SETEVENTMASK 消息发送到该控件，
	// 同时将 ENM_UPDATE 标志“或”运算到 lParam 掩码中。

	// TODO:  在此添加控件通知处理程序代码
}
BEGIN_EVENTSINK_MAP(CnewSerialDlg, CDialogEx)
	ON_EVENT(CnewSerialDlg, IDC_MSCOMM1, 1, CnewSerialDlg::OnComm, VTS_NONE)
END_EVENTSINK_MAP()



void CnewSerialDlg::Readconfig()
{
	CFileFind f;
	int i = 0;
	CString str;
	BOOL bFind = f.FindFile(_T(".\\config.ini"));
	if (!bFind)
	{
		try {
			CStdioFile cFlie;
			cFlie.Open(_T("config.ini"), CFile::modeCreate | CFile::modeWrite | CFile::typeText);
			//UpdateData(FALSE);
			//cFlie.WriteString(m_strRXData);
			cFlie.Close();
		}
		catch (CFileException* e)
		{
			e->ReportError();
			e->Delete();
		}
		//return;
		::WritePrivateProfileString(_T("COMM"), _T("port"), _T("COM5"), m_strConfigFile);
		::WritePrivateProfileString(_T("COMM"), _T("baudRate"), _T("115200"), m_strConfigFile);
		::WritePrivateProfileString(_T("COMM"), _T("openBtn"), _T("0"), m_strConfigFile);
		::WritePrivateProfileString(_T("COMM"), _T("send1"), _T("SetReg 2B 84"), m_strConfigFile);
		::WritePrivateProfileString(_T("COMM"), _T("send2"), _T("getReg 01 2B"), m_strConfigFile);
		::WritePrivateProfileString(_T("COMM"), _T("send3"), _T("CurrentTest"), m_strConfigFile);
		::WritePrivateProfileString(_T("COMM"), _T("send4"), _T("SpiTest"), m_strConfigFile);
		::WritePrivateProfileString(_T("COMM"), _T("send5"), _T("getVersion"), m_strConfigFile);
		::WritePrivateProfileString(_T("COMM"), _T("send6"), _T("07 51 00 12 00 00 00"), m_strConfigFile);
		::WritePrivateProfileString(_T("COMM"), _T("send7"), _T("07 51 00 13 00 00 00"), m_strConfigFile);
		::WritePrivateProfileString(_T("COMM"), _T("send8"), _T("79 00 12 9A 00 00 01 53 FE"), m_strConfigFile);
		::WritePrivateProfileString(_T("SN"), _T("SnIndex"), _T("0"), m_strConfigFile);
	}

	::GetPrivateProfileString(_T("COMM"), _T("port"), _T(""), m_port.GetBuffer(20), 20, m_strConfigFile);			//串口号
	::GetPrivateProfileString(_T("COMM"), _T("baudRate"), _T(""), m_baud.GetBuffer(20), 20, m_strConfigFile);    //波特率e);    //波特率
	m_baud.ReleaseBuffer();

	

	str = m_port.Mid(3, m_port.GetLength());
	m_port.ReleaseBuffer();
	
	i = _ttoi(str); //i = atoi(str);
	((CComboBox*)GetDlgItem(IDC_COMBO_PORT_SELECT))->SetCurSel(i);

	CString tmp;
	::GetPrivateProfileString(_T("COMM"), _T("AutoEnter"), _T(""), tmp.GetBuffer(20), 20, m_strConfigFile);			
	if (tmp == _T("1"))
	{
		m_bAutoEnter = TRUE;
	}
	else
	{
		m_bAutoEnter = FALSE;
	}

	::GetPrivateProfileString(_T("COMM"), _T("AutoDelSpecCharater"), _T(""), tmp.GetBuffer(20), 20, m_strConfigFile);
	if (tmp == _T("1"))
	{
		m_autoDelSpec = TRUE;
	}
	else
	{
		m_autoDelSpec = FALSE;
	}

#if 1
	::GetPrivateProfileString(_T("COMM"), _T("ComList"), _T(""), tmp.GetBuffer(3), 3, m_strConfigFile);
	//str = tmp.Mid(3, tmp.GetLength());
	i = _ttoi(tmp); //i = atoi(str);
	m_comList.SetCurSel(i-1);
	OnBnClickedButtonLoadComlist();
#else
	//CString tmp;
	::GetPrivateProfileString(_T("SEND"), _T("send1"), _T(""), m_EditSend.GetBuffer(100), 100, m_strConfigFile);	//串口打开按钮状态，1开，0 关
	::GetPrivateProfileString(_T("SEND"), _T("send2"), _T(""), m_EditSend2.GetBuffer(100), 100, m_strConfigFile);	//串口打开按钮状态，1开，0 关
	::GetPrivateProfileString(_T("SEND"), _T("send3"), _T(""), m_EditSend3.GetBuffer(100), 100, m_strConfigFile);	//串口打开按钮状态，1开，0 关
	::GetPrivateProfileString(_T("SEND"), _T("send4"), _T(""), m_EditSend4.GetBuffer(100), 100, m_strConfigFile);	//串口打开按钮状态，1开，0 关
	::GetPrivateProfileString(_T("SEND"), _T("send5"), _T(""), m_EditSend5.GetBuffer(100), 100, m_strConfigFile);	//串口打开按钮状态，1开，0 关
	::GetPrivateProfileString(_T("SEND"), _T("send6"), _T(""), m_EditSend6.GetBuffer(100), 100, m_strConfigFile);	//串口打开按钮状态，1开，0 关
	::GetPrivateProfileString(_T("SEND"), _T("send7"), _T(""), m_EditSend7.GetBuffer(100), 100, m_strConfigFile);	//串口打开按钮状态，1开，0 关
	::GetPrivateProfileString(_T("SEND"), _T("send8"), _T(""), m_EditSend8.GetBuffer(100), 100, m_strConfigFile);	//串口打开按钮状态，1开，0 关
#endif
	UpdateData(FALSE);
	CString strOpenBtn;
	::GetPrivateProfileString(_T("COMM"), _T("openBtn"), _T("0"), strOpenBtn.GetBuffer(2), 2, m_strConfigFile);	//串口打开按钮状态，1开，0 关
	//4800; 9600; 19200; 38400; 57600; 115200; 230400; 460800;
	if (m_baud.Find(_T("4800")) != -1)
	{
		((CComboBox*)GetDlgItem(IDC_COMBO_BAUD_SELECT))->SetCurSel(0);
		baud = 4800;
	}
	else if (m_baud.Find(_T("9600")) != -1)
	{
		((CComboBox*)GetDlgItem(IDC_COMBO_BAUD_SELECT))->SetCurSel(1);
		baud = 9600;
	}
	else if (m_baud.Find(_T("19200")) != -1)
	{
		((CComboBox*)GetDlgItem(IDC_COMBO_BAUD_SELECT))->SetCurSel(2);
		baud = 19200;
	}
	else if (m_baud.Find(_T("38400")) != -1)
	{
		((CComboBox*)GetDlgItem(IDC_COMBO_BAUD_SELECT))->SetCurSel(3);
		baud = 38400;
	}
	else if (m_baud.Find(_T("57600")) != -1)
	{
		((CComboBox*)GetDlgItem(IDC_COMBO_BAUD_SELECT))->SetCurSel(4);
		baud = 57600;
	}
	else if (m_baud.Find(_T("115200")) != -1)
	{
		((CComboBox*)GetDlgItem(IDC_COMBO_BAUD_SELECT))->SetCurSel(5);
		baud = 115200;
	}
	else if (m_baud.Find(_T("230400")) != -1)
	{
		((CComboBox*)GetDlgItem(IDC_COMBO_BAUD_SELECT))->SetCurSel(6);
		baud = 230400;
	}
	else if (m_baud.Find(_T("460800")) != -1)
	{
		((CComboBox*)GetDlgItem(IDC_COMBO_BAUD_SELECT))->SetCurSel(7);
		baud = 460800;
	}

	if (strOpenBtn == _T("1"))
	{
		//OnBnClickedButtonSerialControl();
	}

	::GetPrivateProfileString(_T("SN"), _T("SnIndex"), _T(""), tmp.GetBuffer(3), 3, m_strConfigFile);
	m_SN_index = _ttoi(tmp); //i = atoi(str);
	
	//UpdateData(FALSE);
}
CString CnewSerialDlg::Ascii2Hex(CString str_ASCII)
{//转HEX
	int i;
	int length = str_ASCII.GetLength();
	CString str_HEX;
	CString temp;

	for (i = 0; i < length; i++)
	{
		temp.Format(_T("%02X "), str_ASCII.GetAt(i));
		str_HEX = str_HEX + temp;
	}

	return str_HEX;
}
void CnewSerialDlg::OnComm()
{
	// TODO: 在此处添加消息处理程序代码
	VARIANT variant_inp;
	COleSafeArray safearray_inp;
	LONG len, k;
	BYTE rxdata[4096]; //设置BYTE数组 An 8-bit integerthat is not signed.  
	CString strtemp;
	CString getStr;
	if ((m_ctrlComm.get_CommEvent() == 2) && (!m_bPause)) //事件值为2表示接收缓冲区内有字符  
	{
		//以下你可以根据自己的通信协议加入处理代码
		variant_inp = m_ctrlComm.get_Input(); //读缓冲区  
		safearray_inp = variant_inp; //VARIANT型变量转换为ColeSafeArray型变量  
		len = safearray_inp.GetOneDimSize(); //得到有效数据长度  
		getStr.Empty();
		for (k = 0; k < len; k++)
			safearray_inp.GetElement(&k, rxdata + k);//转换为BYTE型数组  
		for (k = 0; k < len; k++) //将数组转换为Cstring型变量  
		{
			char bt = *(char*)(rxdata + k); //字符型  
			//USES_CONVERSION;
			//CString wbt = A2W(bt);
			if (bShowHex)
			{
				strtemp.Format(_T("%02X "), bt);
				if (strtemp.Find("FFFFFF") != -1)
					strtemp = strtemp.Mid(6);
				getStr += strtemp;
			}
			else
			{
				strtemp.Format(_T("%c"), bt); //将字符送入临时变量strtemp存放  
				getStr += strtemp; //加入接收编辑框对应字符串  
			}

			//strtemp = wbt;
			//getStr += strtemp;
#if 0
			if (strtemp == '\r')
			{
				if (bShowHex)
				{
					getStr += Ascii2Hex(strtemp);
					//GetDlgItem(IDC_EDIT_RECEIVE)->SetWindowText(curretBufString_test);
				}
				else
					getStr += "\n";
				m_u16strCount = 0;
			}
			else if (strtemp == '\n')
			{
				m_u16strCount = 0;
			}
			else
#endif
			{
				m_u16strCount++;
#if 0
				if (m_bAutoLF)
				{
					if (bShowHex)
					{
						if (m_u16strCount >= 15)
						{
							getStr += "\r\n";
							m_u16strCount = 0;
						}
					}
					else
					{ 
						if (m_u16strCount >= 88)
						{
							getStr += "\r\n";
							m_u16strCount = 0;
						}
					}
				}
#endif
			}		
		}
		if (m_bFilter)
		{
			if (getStr.Find(m_strFilter) == -1)
				getStr.Empty();
		}
#if 1
		if (!bShowHex && m_autoDelSpec)
		{
			CString specialCharater1 = "[1;34m";
			CString specialCharater2 = "[0m";
			CString specialCharater3 = "[1;32";
			CString specialCharater4 = "[0;0m";
			CString specialCharater5 = "[m";
			CString specialCharater6 = "[K";
			CString specialCharater7 = "[7m";
			CString specialCharater8 = "[1m";
			//char specialCharater1[7] = { 0x1B,0x5B,0x31,0x3B,0x33,0x34,0x6D };
			//char specialCharater2[4] = { 0x1B,0x5B,0x30,0x6D };
			//char specialCharater3[4] = { 0x1B,0x5B,0x31,0x6D };
			//char specialCharater4[6] = { 0x1B,0x5B,0x30,0x3B,0x30,0x6D };
			getStr.Replace(specialCharater1, _T(""));
			getStr.Replace(specialCharater2, _T(""));
			getStr.Replace(specialCharater3, _T(""));
			getStr.Replace(specialCharater4, _T(""));
			getStr.Replace(specialCharater5, _T(""));
			getStr.Replace(specialCharater6, _T(""));
			getStr.Replace(specialCharater7, _T(""));
			getStr.Replace(specialCharater8, _T(""));
		}
#endif
		//getStr.Remove(*specialCharater1);
		//getStr.Remove(*specialCharater2);
		//getStr.Remove(*specialCharater3);
		//getStr.Remove(*specialCharater4);
		m_strRXData += getStr;
		m_strShowRX += getStr;
		m_getAck += getStr;
		bNeedShow = TRUE;
		//PostMessage(WM_UPDATE_MESSAGE, 0, 0);		
	}
	//PostMessage(WM_UPDATE_MESSAGE, 0, 0);
	//int line = ((CEdit*)(GetDlgItem(IDC_EDIT_RX)))->GetLineCount();//选中最后一行
	//((CEdit*)(GetDlgItem(IDC_EDIT_RX)))->LineScroll(line);
	//UpdateData(FALSE); //更新编辑框内容 
}

void CnewSerialDlg::openComm()
{
#if 1
	int i = 0;
	CString str;
	CString strComSetting;
	//((CComboBox*)GetDlgItem(IDC_COMBO_PORT_SELECT))->GetWindowText(str);
	str = m_port.Mid(3, m_port.GetLength());
	i = _ttoi(str); //i = atoi(str);
	m_ctrlComm.put_CommPort(i);

	if (!m_ctrlComm.get_PortOpen())
		m_ctrlComm.put_PortOpen(TRUE);//打开串口  
	else
		AfxMessageBox(_T("cannot open serial port"));

	strComSetting.Format(_T("%d"), baud);
	strComSetting = strComSetting + _T(",n,8,1");
	m_ctrlComm.put_Settings(strComSetting); //波特率9600，无校验，8个数据位，1个停止位  

	//m_ctrlComm.put_OutBufferSize(1024*2);
	//m_ctrlComm.put_OutBufferCount(0);

	m_ctrlComm.put_InputMode(1); //1：表示以二进制方式检取数据  
	m_ctrlComm.put_RThreshold(1);
	//参数1表示每当串口接收缓冲区中有多于或等于1个字符时将引发一个接收数据的OnComm事件  
	m_ctrlComm.put_InputLen(0); //设置当前接收区数据长度为0 
	m_ctrlComm.get_Input();//先预读缓冲区以清除残留数据 
	//运行到这里代表打开成功
	//com_port = i;
	//serial_isopen = TRUE;
	//sio_flowctrl(com_port, 0x00);//关闭硬件流控制
	//sio_lctrl(com_port, 0x00);//关闭RTS DTR
	//sio_ioctl(com_port, baud, config);
	//sio_flush(com_port, 2);
	m_bIsComOpen = TRUE;
	((CComboBox*)GetDlgItem(IDC_COMBO_PORT_SELECT))->EnableWindow(FALSE);
	((CComboBox*)GetDlgItem(IDC_COMBO_BAUD_SELECT))->EnableWindow(FALSE);
	((CButton*)GetDlgItem(IDC_MFCBUTTON_TEST))->SetWindowText(_T("关闭串口"));
	UpdateData();
	::WritePrivateProfileString(_T("COMM"), _T("port"), m_port, m_strConfigFile);
	::WritePrivateProfileString(_T("COMM"), _T("baudRate"), m_baud, m_strConfigFile);
	//::WritePrivateProfileString(_T("COMM"), _T("openBtn"), _T("1"), m_strConfigFile);
	
	HBITMAP hBmp;    // 保存CBitmap加载的位图的句柄   
	CBitmap m_bitmap;
	m_bitmap.LoadBitmap(IDB_BITMAP_READY);  // 将位图IDB_BITMAP1加载到bitmap   
	hBmp = (HBITMAP)m_bitmap.GetSafeHandle();  // 获取bitmap加载位图的句柄  
	//m_image.ModifyStyle(0xf, SS_BITMAP | SS_CENTERIMAGE);
	m_image.SetBitmap(hBmp);    // 设置图片控件m_jzmPicture的位图图片为IDB_BITMAP1  
	if (!m_bPause)
	{
		m_image2.SetBitmap(hBmp);
	}
#else
	CString strComSetting;
	if (m_ctrlComm.get_PortOpen())
		m_ctrlComm.put_PortOpen(FALSE);

	m_ctrlComm.put_CommPort(5); //选择com1  
	if (!m_ctrlComm.get_PortOpen())
		m_ctrlComm.put_PortOpen(TRUE);//打开串口  
	else
		AfxMessageBox("cannot open serial port");
	strComSetting.Format("%d", baud);
	strComSetting = strComSetting + ",n,8,1";
	m_ctrlComm.put_Settings(strComSetting); //波特率9600，无校验，8个数据位，1个停止位  

	m_ctrlComm.put_InputMode(1); //1：表示以二进制方式检取数据  
	m_ctrlComm.put_RThreshold(1);
	//参数1表示每当串口接收缓冲区中有多于或等于1个字符时将引发一个接收数据的OnComm事件  
	m_ctrlComm.put_InputLen(0); //设置当前接收区数据长度为0 
	m_ctrlComm.get_Input();//先预读缓冲区以清除残留数据 
#endif
}

void CnewSerialDlg::OnCbnSetfocusComboPortSelect()
{
	//搜索系统当前串口，并实时显示在选择框中
	HANDLE hCom;
	short i = 0;
	CString str;
	str.Empty();
	((CComboBox*)GetDlgItem(IDC_COMBO_PORT_SELECT))->ResetContent();//清除显示项
	for (i = 1; i <= 16; i++)
	{
		//m_ctrlComm.put_CommPort(i); //选择com1  
		str.Format(_T("\\\\.\\COM%d"),i);
		hCom = CreateFile(str,0,0,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
		if (INVALID_HANDLE_VALUE != hCom)//(!m_ctrlComm.get_PortOpen())
		{
			CloseHandle(hCom);//m_ctrlComm.put_PortOpen(TRUE);
			m_port = str.Mid(4);
			//str.AppendFormat("COM%d", i);
			((CComboBox*)GetDlgItem(IDC_COMBO_PORT_SELECT))->AddString(m_port);
			str.Empty();	
		}
		/*if (SIO_OK == sio_open(i))
		{
			sio_close(i);
			str.AppendFormat("COM%d", i);
			((CComboBox*)GetDlgItem(IDC_COMBO_PORT_SELECT))->AddString(str);
			str.Empty();
		}*/
	}
	i = ((CComboBox*)GetDlgItem(IDC_COMBO_PORT_SELECT))->GetCount();
	if (i == 0)
	{
		GetDlgItem(IDC_MFCBUTTON_TEST)->EnableWindow(FALSE);
	}
	else
	{
		GetDlgItem(IDC_MFCBUTTON_TEST)->EnableWindow(TRUE);
	}
}

void CnewSerialDlg::OnCbnCloseupComboBaudSelect()
{
	// TODO: 在此添加控件通知处理程序代码
	int i = 0;
	i = ((CComboBox*)GetDlgItem(IDC_COMBO_BAUD_SELECT))->GetCurSel();
	if (-1 == i)
	{
		baud = 115200;
		MessageBox(_T("请选择一个波特率"), _T("错误"), MB_OK);
	}
	else
	{
		switch (i)
		{
		case 0:
			baud = 4800;
			break;
		case 1:
			baud = 9600;
			break;
		case 2:
			baud = 19200;
			break;
		case 3:
			baud = 38400;
			break;
		case 4:
			baud = 57600;
			break;
		case 5:
			baud = 115200;
			break;
		case 6:
			baud = 230400;
			break;
		case 7:
			baud = 460800;
			break;
		}
	}
}



void CnewSerialDlg::OnBnClickedButtonSerialControl()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_bIsComOpen)
	{
		if (m_ctrlComm.get_PortOpen())
		{
			m_bIsComOpen = FALSE;
			m_ctrlComm.put_PortOpen(FALSE);//打开串口  
			((CComboBox*)GetDlgItem(IDC_COMBO_PORT_SELECT))->EnableWindow(TRUE);
			((CComboBox*)GetDlgItem(IDC_COMBO_BAUD_SELECT))->EnableWindow(TRUE);
			((CButton*)GetDlgItem(IDC_MFCBUTTON_TEST))->SetWindowText(_T("打开串口"));

			HBITMAP hBmp;    // 保存CBitmap加载的位图的句柄   
			CBitmap m_bitmap;
			m_bitmap.LoadBitmap(IDB_BITMAP_NG);  // 将位图IDB_BITMAP1加载到bitmap   
			hBmp = (HBITMAP)m_bitmap.GetSafeHandle();  // 获取bitmap加载位图的句柄  
				//m_image.ModifyStyle(0xf, SS_BITMAP | SS_CENTERIMAGE);
			m_image.SetBitmap(hBmp);    // 设置图片控件m_jzmPicture的位图图片为IDB_BITMAP1  
			m_image2.SetBitmap(hBmp);
			KillTimer(TIME_SHOW_MESSAGE_SERIAL);
		
		}		
	}
	else
	{
		openComm();
		SetTimer(TIME_SHOW_MESSAGE_SERIAL, 100, NULL);
	}
}

void StringtoHex(BYTE* GB, int glen, BYTE* SB, int* slen)
{
	int i; //遍历输入的字符串
	int a = 0;
	char temp; //接收字符，用来判断是否为空格，若是则跳过
	char temp1, temp2; //接收一个字节的两个字符 例如EB，则temp1='E',temp2 = 'B'
	*slen = 0; //输出的16进制字符串长度
	for (i = 0; i < glen; i++)
	{
		temp = GB[i];
		if (temp == ' ')
			continue;

		if (a == 0)
			temp1 = GB[i];
		if (a == 1)
			temp2 = GB[i];
		a++;

		if (a == 2)
		{
			a = 0;
			temp1 = temp1 - '0';
			if (temp1 > 10)
				temp1 = temp1 - 7;
			temp2 = temp2 - '0';
			if (temp2 > 10)
				temp2 = temp2 - 7;

			SB[*slen] = temp1 * 16 + temp2;
			(*slen)++;
		}
	}

}

void CnewSerialDlg::OnBnClickedButtonSend1()
{
	// TODO: 在此添加控件通知处理程序代码
	CString str1;
	//sendString.Empty();
	//没有对\r\n特殊处理
	UpdateData(true); //读取编辑框内容 
	::WritePrivateProfileString(_T("SEND"), _T("send1"), m_EditSend, m_strConfigFile);
	if (m_bSendHex)//发送16进制数据
	{
		CByteArray HexDataBuf;
		int i = 0;
		BYTE SendBuf[1024] = { 0 };
		BYTE GetData[1024] = { 0 };
		int SendLen = 0;
		int GetLen = 0;

		//UpdateData(TRUE);//获取编辑框内容
		if (m_EditSend.IsEmpty())
		{
			AfxMessageBox(_T("发送数据为空!"));
			return;
		}
		HexDataBuf.RemoveAll();    //清空数组 

		GetLen = m_EditSend.GetLength();
		#if 0
		if (m_EditSend.Find(' ') == -1)
		{
			int j = 0;
			for (i = 0; i < GetLen; i++)
			{			
				if ((i%2 == 0)&&(i>1))
				{
					GetData[j] = ' ';
					j++;
					GetData[j] = (BYTE)m_EditSend.GetBuffer()[i];
				}
				else
					GetData[j] = (BYTE)m_EditSend.GetBuffer()[i];
				j++;
			}
			GetLen = j;
		}
		else
		#endif
		{
			for (i = 0; i < GetLen; i++)
			{
				GetData[i] = (BYTE)m_EditSend.GetBuffer()[i];
			}
		}
		StringtoHex(GetData, GetLen, SendBuf, &SendLen);//将字符串转化为字节数据
		/*const int limitNum = 20;
		if (SendLen > limitNum)
		{
			HexDataBuf.SetSize(limitNum);
			//while (i < SendLen)
			{
				int j;
				for (j = 0; j < (SendLen / limitNum); j++)
				{
					for (i = 0; i < limitNum; i++)
					{
						HexDataBuf.SetAt(i, SendBuf[j * limitNum + i]);
					}
					if (m_bIsComOpen)
						m_ctrlComm.put_Output(COleVariant(HexDataBuf));
				}
			}
		}
		else*/
		{
			if (m_bAutoEnter)
				HexDataBuf.SetSize(SendLen + 2);   //设置数组大小为帧长度 
			else
				HexDataBuf.SetSize(SendLen);   //设置数组大小为帧长度 

			for (i = 0; i < SendLen; i++)
			{
				HexDataBuf.SetAt(i, SendBuf[i]);
			}
			if (m_bAutoEnter)
			{
				HexDataBuf.SetAt(SendLen, 0x0D);
				HexDataBuf.SetAt(SendLen + 1, 0x0A);
			}
			if (m_bIsComOpen)
				m_ctrlComm.put_Output(COleVariant(HexDataBuf)); //发送十六进制数据
		}
		//UpdateData(false); //更新编辑框内容
	}
	else
	{
		//UpdateData(true); //读取编辑框内容 
		//((CEdit*)GetDlgItem(IDC_EDIT_TX))->GetWindowText(str1);
		if(m_bAutoEnter || m_bEnterKey)
			m_EditSend += "\r\n";
		if (m_bIsComOpen)
			m_ctrlComm.put_Output(COleVariant(m_EditSend));
	}
	
}
void CnewSerialDlg::OnBnClickedButtonSend2()
{
	// TODO: 在此添加控件通知处理程序代码
	CString str1;
	//sendString.Empty();
	//没有对\r\n特殊处理
	UpdateData(true); //读取编辑框内容 
	::WritePrivateProfileString(_T("SEND"), _T("send2"), m_EditSend2, m_strConfigFile);
	if (m_bSendHex2)//发送16进制数据
	{
		CByteArray HexDataBuf;
		int i = 0;
		BYTE SendBuf[1024] = { 0 };
		BYTE GetData[1024] = { 0 };
		int SendLen = 0;
		int GetLen = 0;

		//UpdateData(TRUE);//获取编辑框内容
		if (m_EditSend2.IsEmpty())
		{
			AfxMessageBox(_T("发送数据为空!"));
			return;
		}
		HexDataBuf.RemoveAll();    //清空数组 

		GetLen = m_EditSend2.GetLength();
		for (i = 0; i < GetLen; i++)
		{
			GetData[i] = (BYTE)m_EditSend2.GetBuffer()[i];
		}
		StringtoHex(GetData, GetLen, SendBuf, &SendLen);//将字符串转化为字节数据

		if (m_bAutoEnter)
			HexDataBuf.SetSize(SendLen + 2);   //设置数组大小为帧长度 
		else
			HexDataBuf.SetSize(SendLen);   //设置数组大小为帧长度 
		for (i = 0; i < SendLen; i++)
		{
			HexDataBuf.SetAt(i, SendBuf[i]);
		}
		if (m_bAutoEnter)
		{
			HexDataBuf.SetAt(SendLen, 0x0D);
			HexDataBuf.SetAt(SendLen + 1, 0x0A);
		}
		if (m_bIsComOpen)
			m_ctrlComm.put_Output(COleVariant(HexDataBuf)); //发送十六进制数据

		//UpdateData(false); //更新编辑框内容
	}
	else
	{
		//UpdateData(true); //读取编辑框内容 
		//((CEdit*)GetDlgItem(IDC_EDIT_TX))->GetWindowText(str1);
		if (m_bAutoEnter || m_bEnterKey)
			m_EditSend2 += "\r\n";
		if (m_bIsComOpen)
			m_ctrlComm.put_Output(COleVariant(m_EditSend2));
	}
	//::WritePrivateProfileString("SEND", "send2", m_EditSend2, m_strConfigFile);
}
void CnewSerialDlg::OnBnClickedButtonSend3()
{
	// TODO: 在此添加控件通知处理程序代码
	CString str1;
	//sendString.Empty();
	//没有对\r\n特殊处理
	UpdateData(true); //读取编辑框内容 
	::WritePrivateProfileString(_T("SEND"), _T("send3"), m_EditSend3, m_strConfigFile);
	if (m_bSendHex3)//发送16进制数据
	{
		CByteArray HexDataBuf;
		int i = 0;
		BYTE SendBuf[1024] = { 0 };
		BYTE GetData[1024] = { 0 };
		int SendLen = 0;
		int GetLen = 0;

		//UpdateData(TRUE);//获取编辑框内容
		if (m_EditSend3.IsEmpty())
		{
			AfxMessageBox(_T("发送数据为空!"));
			return;
		}
		HexDataBuf.RemoveAll();    //清空数组 

		GetLen = m_EditSend3.GetLength();
		for (i = 0; i < GetLen; i++)
		{
			GetData[i] = (BYTE)m_EditSend3.GetBuffer()[i];
		}
		StringtoHex(GetData, GetLen, SendBuf, &SendLen);//将字符串转化为字节数据

		if (m_bAutoEnter)
			HexDataBuf.SetSize(SendLen + 2);   //设置数组大小为帧长度 
		else
			HexDataBuf.SetSize(SendLen);   //设置数组大小为帧长度 
		for (i = 0; i < SendLen; i++)
		{
			HexDataBuf.SetAt(i, SendBuf[i]);
		}
		if (m_bAutoEnter)
		{
			HexDataBuf.SetAt(SendLen, 0x0D);
			HexDataBuf.SetAt(SendLen + 1, 0x0A);
		}
		if (m_bIsComOpen)
			m_ctrlComm.put_Output(COleVariant(HexDataBuf)); //发送十六进制数据

		//UpdateData(false); //更新编辑框内容
	}
	else
	{
		//UpdateData(true); //读取编辑框内容 
		//((CEdit*)GetDlgItem(IDC_EDIT_TX))->GetWindowText(str1);
		if (m_bAutoEnter || m_bEnterKey)
			m_EditSend3 += "\r\n";
		if (m_bIsComOpen)
			m_ctrlComm.put_Output(COleVariant(m_EditSend3));
	}
	//::WritePrivateProfileString("SEND", "send3", m_EditSend3, m_strConfigFile);
}
void CnewSerialDlg::OnBnClickedButtonSend4()
{
	// TODO: 在此添加控件通知处理程序代码
	CString str1;
	//sendString.Empty();
	//没有对\r\n特殊处理
	UpdateData(true); //读取编辑框内容 
	::WritePrivateProfileString(_T("SEND"), _T("send4"), m_EditSend4, m_strConfigFile);
	if (m_bSendHex4)//发送16进制数据
	{
		CByteArray HexDataBuf;
		int i = 0;
		BYTE SendBuf[1024] = { 0 };
		BYTE GetData[1024] = { 0 };
		int SendLen = 0;
		int GetLen = 0;

		//UpdateData(TRUE);//获取编辑框内容
		if (m_EditSend4.IsEmpty())
		{
			AfxMessageBox(_T("发送数据为空!"));
			return;
		}
		HexDataBuf.RemoveAll();    //清空数组 

		GetLen = m_EditSend4.GetLength();
		for (i = 0; i < GetLen; i++)
		{
			GetData[i] = (BYTE)m_EditSend4.GetBuffer()[i];
		}
		StringtoHex(GetData, GetLen, SendBuf, &SendLen);//将字符串转化为字节数据

		if (m_bAutoEnter)
			HexDataBuf.SetSize(SendLen + 2);   //设置数组大小为帧长度 
		else
			HexDataBuf.SetSize(SendLen);   //设置数组大小为帧长度 
		for (i = 0; i < SendLen; i++)
		{
			HexDataBuf.SetAt(i, SendBuf[i]);
		}
		if (m_bAutoEnter)
		{
			HexDataBuf.SetAt(SendLen, 0x0D);
			HexDataBuf.SetAt(SendLen + 1, 0x0A);
		}
		if (m_bIsComOpen)
			m_ctrlComm.put_Output(COleVariant(HexDataBuf)); //发送十六进制数据

		//UpdateData(false); //更新编辑框内容
	}
	else
	{
		//UpdateData(true); //读取编辑框内容 
		//((CEdit*)GetDlgItem(IDC_EDIT_TX))->GetWindowText(str1);
		if (m_bAutoEnter || m_bEnterKey)
			m_EditSend4 += "\r\n";
		if (m_bIsComOpen)
			m_ctrlComm.put_Output(COleVariant(m_EditSend4));
	}
	//::WritePrivateProfileString("SEND", "send4", m_EditSend4, m_strConfigFile);
}
void CnewSerialDlg::OnBnClickedButtonSend5()
{
	// TODO: 在此添加控件通知处理程序代码
	CString str1;
	//sendString.Empty();
	//没有对\r\n特殊处理
	UpdateData(true); //读取编辑框内容 
	::WritePrivateProfileString(_T("SEND"), _T("send5"), m_EditSend5, m_strConfigFile);
	if (m_bSendHex5)//发送16进制数据
	{
		CByteArray HexDataBuf;
		int i = 0;
		BYTE SendBuf[1024] = { 0 };
		BYTE GetData[1024] = { 0 };
		int SendLen = 0;
		int GetLen = 0;

		//UpdateData(TRUE);//获取编辑框内容
		if (m_EditSend5.IsEmpty())
		{
			AfxMessageBox(_T("发送数据为空!"));
			return;
		}
		HexDataBuf.RemoveAll();    //清空数组 

		GetLen = m_EditSend5.GetLength();
		for (i = 0; i < GetLen; i++)
		{
			GetData[i] = (BYTE)m_EditSend5.GetBuffer()[i];
		}
		StringtoHex(GetData, GetLen, SendBuf, &SendLen);//将字符串转化为字节数据

		if (m_bAutoEnter)
			HexDataBuf.SetSize(SendLen + 2);   //设置数组大小为帧长度 
		else
			HexDataBuf.SetSize(SendLen);   //设置数组大小为帧长度 
		for (i = 0; i < SendLen; i++)
		{
			HexDataBuf.SetAt(i, SendBuf[i]);
		}
		if (m_bAutoEnter)
		{
			HexDataBuf.SetAt(SendLen, 0x0D);
			HexDataBuf.SetAt(SendLen + 1, 0x0A);
		}
		if (m_bIsComOpen)
			m_ctrlComm.put_Output(COleVariant(HexDataBuf)); //发送十六进制数据

		//UpdateData(false); //更新编辑框内容
	}
	else
	{
		//UpdateData(true); //读取编辑框内容 
		//((CEdit*)GetDlgItem(IDC_EDIT_TX))->GetWindowText(str1);
		if (m_bAutoEnter || m_bEnterKey)
			m_EditSend5 += "\r\n";
		if (m_bIsComOpen)
			m_ctrlComm.put_Output(COleVariant(m_EditSend5));
	}

	//::WritePrivateProfileString("SEND", "send5", m_EditSend5, m_strConfigFile);
}
void CnewSerialDlg::OnBnClickedButtonSend6()
{
	// TODO: 在此添加控件通知处理程序代码
	CString str1;
	//sendString.Empty();
	//没有对\r\n特殊处理
	UpdateData(true); //读取编辑框内容 
	::WritePrivateProfileString(_T("SEND"), _T("send6"), m_EditSend6, m_strConfigFile);
	if (m_bSendHex6)//发送16进制数据
	{
		CByteArray HexDataBuf;
		int i = 0;
		BYTE SendBuf[1024] = { 0 };
		BYTE GetData[1024] = { 0 };
		int SendLen = 0;
		int GetLen = 0;

		//UpdateData(TRUE);//获取编辑框内容
		if (m_EditSend6.IsEmpty())
		{
			AfxMessageBox(_T("发送数据为空!"));
			return;
		}
		HexDataBuf.RemoveAll();    //清空数组 

		GetLen = m_EditSend6.GetLength();
		for (i = 0; i < GetLen; i++)
		{
			GetData[i] = (BYTE)m_EditSend6.GetBuffer()[i];
		}
		StringtoHex(GetData, GetLen, SendBuf, &SendLen);//将字符串转化为字节数据

		if (m_bAutoEnter)
			HexDataBuf.SetSize(SendLen + 2);   //设置数组大小为帧长度 
		else
			HexDataBuf.SetSize(SendLen);   //设置数组大小为帧长度 
		for (i = 0; i < SendLen; i++)
		{
			HexDataBuf.SetAt(i, SendBuf[i]);
		}
		if (m_bAutoEnter)
		{
			HexDataBuf.SetAt(SendLen, 0x0D);
			HexDataBuf.SetAt(SendLen + 1, 0x0A);
		}
		if (m_bIsComOpen)
			m_ctrlComm.put_Output(COleVariant(HexDataBuf)); //发送十六进制数据

		//UpdateData(false); //更新编辑框内容
	}
	else
	{
		//UpdateData(true); //读取编辑框内容 
		//((CEdit*)GetDlgItem(IDC_EDIT_TX))->GetWindowText(str1);
		if (m_bAutoEnter || m_bEnterKey)
			m_EditSend6 += "\r\n";
		if (m_bIsComOpen)
			m_ctrlComm.put_Output(COleVariant(m_EditSend6));
	}
	//::WritePrivateProfileString("SEND", "send6", m_EditSend6, m_strConfigFile);
}
void CnewSerialDlg::OnBnClickedButtonSend7()
{
	// TODO: 在此添加控件通知处理程序代码
	CString str1;
	//sendString.Empty();
	//没有对\r\n特殊处理
	UpdateData(true); //读取编辑框内容 
	::WritePrivateProfileString(_T("SEND"), _T("send7"), m_EditSend7, m_strConfigFile);
	if (m_bSendHex7)//发送16进制数据
	{
		CByteArray HexDataBuf;
		int i = 0;
		BYTE SendBuf[1024] = { 0 };
		BYTE GetData[1024] = { 0 };
		int SendLen = 0;
		int GetLen = 0;

		//UpdateData(TRUE);//获取编辑框内容
		if (m_EditSend7.IsEmpty())
		{
			AfxMessageBox(_T("发送数据为空!"));
			return;
		}
		HexDataBuf.RemoveAll();    //清空数组 

		GetLen = m_EditSend7.GetLength();
		for (i = 0; i < GetLen; i++)
		{
			GetData[i] = (BYTE)m_EditSend7.GetBuffer()[i];
		}
		StringtoHex(GetData, GetLen, SendBuf, &SendLen);//将字符串转化为字节数据

		if (m_bAutoEnter)
			HexDataBuf.SetSize(SendLen + 2);   //设置数组大小为帧长度 
		else
			HexDataBuf.SetSize(SendLen);   //设置数组大小为帧长度 
		for (i = 0; i < SendLen; i++)
		{
			HexDataBuf.SetAt(i, SendBuf[i]);
		}
		if (m_bAutoEnter)
		{
			HexDataBuf.SetAt(SendLen, 0x0D);
			HexDataBuf.SetAt(SendLen + 1, 0x0A);
		}
		if (m_bIsComOpen)
			m_ctrlComm.put_Output(COleVariant(HexDataBuf)); //发送十六进制数据

		//UpdateData(false); //更新编辑框内容
	}
	else
	{
		//UpdateData(true); //读取编辑框内容 
		//((CEdit*)GetDlgItem(IDC_EDIT_TX))->GetWindowText(str1);
		if (m_bAutoEnter || m_bEnterKey)
			m_EditSend7 += "\r\n";
		if (m_bIsComOpen)
			m_ctrlComm.put_Output(COleVariant(m_EditSend7));
	}
	//::WritePrivateProfileString("SEND", "send7", m_EditSend7, m_strConfigFile);
}
void CnewSerialDlg::OnBnClickedButtonSend8()
{
	// TODO: 在此添加控件通知处理程序代码
	CString str1;
	//sendString.Empty();
	//没有对\r\n特殊处理
	UpdateData(true); //读取编辑框内容 
	::WritePrivateProfileString(_T("SEND"), _T("send8"), m_EditSend8, m_strConfigFile);
	if (m_bSendHex8)//发送16进制数据
	{
		CByteArray HexDataBuf;
		int i = 0;
		BYTE SendBuf[1024] = { 0 };
		BYTE GetData[1024] = { 0 };
		int SendLen = 0;
		int GetLen = 0;

		//UpdateData(TRUE);//获取编辑框内容
		if (m_EditSend8.IsEmpty())
		{
			AfxMessageBox(_T("发送数据为空!"));
			return;
		}
		HexDataBuf.RemoveAll();    //清空数组 

		GetLen = m_EditSend8.GetLength();
		for (i = 0; i < GetLen; i++)
		{
			GetData[i] = (BYTE)m_EditSend8.GetBuffer()[i];
		}
		StringtoHex(GetData, GetLen, SendBuf, &SendLen);//将字符串转化为字节数据

		if (m_bAutoEnter)
			HexDataBuf.SetSize(SendLen + 2);   //设置数组大小为帧长度 
		else
			HexDataBuf.SetSize(SendLen);   //设置数组大小为帧长度 
		for (i = 0; i < SendLen; i++)
		{
			HexDataBuf.SetAt(i, SendBuf[i]);
		}
		if (m_bAutoEnter || m_bEnterKey)
		{
			HexDataBuf.SetAt(SendLen, 0x0D);
			HexDataBuf.SetAt(SendLen + 1, 0x0A);
		}
		if (m_bIsComOpen)
			m_ctrlComm.put_Output(COleVariant(HexDataBuf)); //发送十六进制数据

		//UpdateData(false); //更新编辑框内容
	}
	else
	{
		//UpdateData(true); //读取编辑框内容 
		//((CEdit*)GetDlgItem(IDC_EDIT_TX))->GetWindowText(str1);
		if (m_bAutoEnter)
			m_EditSend8 += "\r\n";
		if (m_bIsComOpen)
			m_ctrlComm.put_Output(COleVariant(m_EditSend8));
	}
	//::WritePrivateProfileString("SEND", "send8", m_EditSend8, m_strConfigFile);
}
void CnewSerialDlg::OnBnClickedButton3()
{
	// TODO: 在此添加控件通知处理程序代码
	CString selList;
	switch (m_comList.GetCurSel())
	{
	case 0:
		selList = "SEND1";
		break;
	case 1:
		selList = "SEND2";
		break;
	case 2:
		selList = "SEND3";
		break;
	case 3:
		selList = "SEND4";
		break;
	case 4:
		selList = "SEND5";
		break;
	case 5:
		selList = "SEND6";
		break;
	case 6:
		selList = "SEND7";
		break;
	case 7:
		selList = "SEND8";
		break;
	default:
		selList = "SEND";
		break;
	}
	UpdateData(TRUE);
	::WritePrivateProfileString(selList, _T("name"), m_strEditListName, m_strConfigFile);
	::WritePrivateProfileString(selList, _T("send1"), m_EditSend, m_strConfigFile);
	::WritePrivateProfileString(selList, _T("send2"), m_EditSend2, m_strConfigFile);
	::WritePrivateProfileString(selList, _T("send3"), m_EditSend3, m_strConfigFile);
	::WritePrivateProfileString(selList, _T("send4"), m_EditSend4, m_strConfigFile);
	::WritePrivateProfileString(selList, _T("send5"), m_EditSend5, m_strConfigFile);
	::WritePrivateProfileString(selList, _T("send6"), m_EditSend6, m_strConfigFile);
	::WritePrivateProfileString(selList, _T("send7"), m_EditSend7, m_strConfigFile);
	::WritePrivateProfileString(selList, _T("send8"), m_EditSend8, m_strConfigFile);

	CString tmp;
	if (m_bSendHex)
		tmp = "1";
	else
		tmp = "0";
	::WritePrivateProfileString(selList, _T("SendHex1"), tmp, m_strConfigFile);
	if (m_bSendHex2)
		tmp = "1";
	else
		tmp = "0";
	::WritePrivateProfileString(selList, _T("SendHex2"), tmp, m_strConfigFile);
	if (m_bSendHex3)
		tmp = "1";
	else
		tmp = "0";
	::WritePrivateProfileString(selList, _T("SendHex3"), tmp, m_strConfigFile);
	if (m_bSendHex4)
		tmp = "1";
	else
		tmp = "0";
	::WritePrivateProfileString(selList, _T("SendHex4"), tmp, m_strConfigFile);
	if (m_bSendHex5)
		tmp = "1";
	else
		tmp = "0";
	::WritePrivateProfileString(selList, _T("SendHex5"), tmp, m_strConfigFile);
	if (m_bSendHex6)
		tmp = "1";
	else
		tmp = "0";
	::WritePrivateProfileString(selList, _T("SendHex6"), tmp, m_strConfigFile);
	if (m_bSendHex7)
		tmp = "1";
	else
		tmp = "0";
	::WritePrivateProfileString(selList, _T("SendHex7"), tmp, m_strConfigFile);
	if (m_bSendHex8)
		tmp = "1";
	else
		tmp = "0";
	::WritePrivateProfileString(selList, _T("SendHex8"), tmp, m_strConfigFile);
}

void CnewSerialDlg::OnBnClickedButtonSave()
{
	// TODO: 在此添加控件通知处理程序代码
#if 0//new method
	CFile mFile;
	if (mFile.Open(_T("save.txt"), CFile::modeCreate | CFile::modeWrite) == 0)
		return;
	CArchive ar(&mFile, CArchive::store);
	ar << m_strRXData;
	ar.Close();
	mFile.Close();
#else//old method
	try {
		CStdioFile cFlie;
		cFlie.Open(_T("save.txt"), CFile::modeCreate | CFile::modeWrite | CFile::typeText);
		//UpdateData(FALSE);
		m_strRXData.Replace(_T("\r\n"), _T("\n"));
		cFlie.WriteString(m_strRXData);
		cFlie.Close();
	}
	catch (CFileException* e)
	{
		e->ReportError();
		e->Delete();
	}
#endif
	CString str;
	CString str1 = _T("save.txt");
	str = _T("notepad ") + str1;
	WinExec(str, SW_SHOW);
}

void CnewSerialDlg::OnBnClickedButtonClear()
{
	// TODO: 在此添加控件通知处理程序代码
	//UpdateData(true);
	m_strRXData.Empty();
	//UpdateData(FALSE);
	GetDlgItem(IDC_EDIT_RX)->SetWindowText(m_strRXData);
}

U8 BLOCK_NO=9;
U8 BLOCK_SIZE = 128;
#define DELAY_TIMES 10
void CnewSerialDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (TIME_SHOW_MESSAGE_SERIAL == nIDEvent)
	{
		if (bNeedShow)
		{
			PostMessage(WM_UPDATE_MESSAGE, 0, 0);
			bNeedShow = FALSE;
		}
		//UpdateData(FALSE);
		//int line = ((CEdit*)(GetDlgItem(IDC_EDIT_RX)))->GetLineCount();//选中最后一行
		//((CEdit*)(GetDlgItem(IDC_EDIT_RX)))->LineScroll(((CEdit*)(GetDlgItem(IDC_EDIT_RX)))->GetLineCount());
	}
	else if (TIME_AUTO_CHANGE_SOURCE == nIDEvent)
	{
		m_intTimer++;
		PostMessage(WM_UPDATE_MESSAGE, 1, 1);
		switch (m_step)
		{
			default:
			case 0:
			{
				CString str1 = "07 51 01 07 0B 6A 9B";
				SendData(str1);
				m_step = 1;
			}
				break;
			case 1:
			{
				CString str1 = "07 51 01 07 12 E9 83";
				SendData(str1);
				m_step = 2;
			}
				break;
			case 2:
			{
				CString str1 = "07 51 01 07 13 F9 A2";
				SendData(str1);
				m_step = 0;
			}
				break;
		}

	}
	else if (TIME_GET_ULPK == nIDEvent)
	{
		switch(m_step)
		{
			case 5:
				KillTimer(TIME_GET_ULPK);
				MessageBox("Get key Fail !");
				break;
			case 0:
			if (m_getAck.Find("03 0C F1") != -1)
			{
				CString tmp;
				tmp = "07 51 01 07 D3 20 EE";
				m_getAck.Empty();
				SendData(tmp);
				m_step = 1;
				m_iCheckAckCount = 0;
			}
			else
			{
				if (m_iCheckAckCount++ > 5)
					m_step = 5;
			}
			break;
			case 1:
			if (m_getAck.Find("03 0C 29") != -1)
			{
				CString tmp;
				CString strUlpk_path;
				KillTimer(TIME_GET_ULPK);
			//03 0C 29 01 31 2D 64 1D 6D 7E 50 FB E6 60 BA D3 9A 38 19 CD 4A CB EB 38 9C A3 84 88 54 C5 20 06 50 77 26 0F 83 9E 3F 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FA C3 
			//01 31 2D 64
				for (int i = 9; i < 20; i++)
				{
					if (m_getAck.GetAt(i) == 0x20)
						continue;
					tmp +=m_getAck.GetAt(i);
				}
				int v_dec;

				//v_dec = wcstol(v_hex, NULL, 16);
				m_longUid = strtol(tmp, NULL, 16);
				m_strUid.Format("%ld", m_longUid);
				//m_strUid = m_getAck;
				if ((m_strUid.Find("4000") != -1) || (m_strUid.Find("2000") != -1))
				{

					if (m_strUid.Find("4000") != -1)
					{
						int pos = m_strUid.Find("4000");
						m_strUid= m_strUid.Mid(pos, 8);
						m_strRXData += "\r\nget TV 5691 UID:" + m_strUid;
						strUlpk_path = "key\\5691_309";
					}
					if (m_strUid.Find("2000") != -1)
					{
						int pos = m_strUid.Find("2000");
						m_strUid = m_strUid.Mid(pos, 8);
						m_strRXData += "\r\nget TV 5583 UID:" + m_strUid;
						strUlpk_path = "key\\5583_339";
					}
					GetDlgItem(IDC_STATIC_SHOW_ULPK)->SetWindowText("UID=" + m_strUid);
					PostMessage(WM_UPDATE_MESSAGE, 0, 0);
					m_step = 2;
					SearchKeybyUID(strUlpk_path);
				}
			}
			else
			{
				if (m_iCheckAckCount++ > 5)
					m_step = 5;
			}
			break;
			default:
				break;
		}
	}
	else if (TIME_BURN_ULPK == nIDEvent)
	{
		if ((m_getAck.Find("03 0C E0") != -1) || (m_getAck.Find("03 0C E1") != -1) || (m_getAck.Find("03 0C E2") != -1))
		{
			KillTimer(TIME_BURN_ULPK);
			MessageBox("Burn key Fail! ");
		}
		switch (m_step)
		{
			case 5:
				KillTimer(TIME_BURN_ULPK);
				MessageBox("Burn key Fail!");
				break;
			case 0:
			{				
				CString tmp;
				m_getAck.Empty();
				if (m_bAutoFacMode)
					tmp = "07 51 01 07 01 CB D1";
				else
					tmp = "07 51 01 08 1B 00 79 AE";
				SendData(tmp);
				m_iCheckAckCount = 0;
				m_step = 1;
			}
			case 1:
				if (m_getAck.Find("03 0C") != -1)
				{
					m_getAck.Empty();
					SendData(strUlpkPotk);
					if(m_bIs5586)
						m_step = 3;
					else
						m_step = 2;
					m_iCheckAckCount = 0;
				}
				else
				{
					if (m_iCheckAckCount++ > 5)
						m_step = 5;					
				}
				break;
			case 2:
				if (m_getAck.Find("03 0C F1") != -1)
				{
					m_getAck.Empty();
					SendData(strPotk);
					m_step = 3;
					m_iCheckAckCount = 0;
				}
				else
				{
					if (m_iCheckAckCount++ > 5)
						m_step = 5;
				}
				break;
			case 3:
			if (m_getAck.Find("03 0C F1") != -1)
			{
				m_step = 4;
				//CString filePath = _T("要删除的文件路径");
				//CFile::Remove(m_HDCP14_path);
				//CFile::Remove(m_HDCP22_path);
				KillTimer(TIME_BURN_ULPK);
				MessageBox("Burn key OK");
			}
			else
			{
				if (m_iCheckAckCount++ > 5)
					m_step = 5;
			}
			break;
			/*else
			{
				if (m_iCheckAckCount++ < 3)
					m_step = 2;
				else
					m_step = 5;
			}
			break;*/
			default:
				break;
		}
	}
	else if (TIME_BURN_HDCP == nIDEvent)
	{
	if ((m_getAck.Find("03 0C E0") != -1) || (m_getAck.Find("03 0C E1") != -1) || (m_getAck.Find("03 0C E2") != -1))
	{
		KillTimer(TIME_BURN_HDCP);
		MessageBox("烧key 失败！");
	}
	switch (m_step)
	{
	case 0:
		if (m_getAck.Find("03 0C F1") != -1)
		{
			m_getAck.Empty();
			SendData(strHdcp14_0);
			m_step = 1;
			m_iCheckAckCount = 0;
		}
		else
		{
			if (m_iCheckAckCount++ > 5)
				m_step = 55;
		}
		break;
	case 1:
		if (m_getAck.Find("03 0C F1") != -1)
		{
			m_getAck.Empty();
			SendData(strHdcp14_1);
			m_step = 2;
			m_iCheckAckCount = 0;
		}
		else
		{
			if (m_iCheckAckCount++ > DELAY_TIMES)
				m_step = 55;
		}
		break;
	case 2:
		if (m_getAck.Find("03 0C F1") != -1)
		{
			m_getAck.Empty();
			SendData(strHdcp14_2);
			if(m_totalBlock_of_HDCP14 >= 3)
				m_step = 3;
			else
				m_step = 30;
			m_iCheckAckCount = 0;
		}
		else
		{
			if (m_iCheckAckCount++ > DELAY_TIMES)
				m_step = 55;
		}
		break;
	case 3:
		if (m_getAck.Find("03 0C F1") != -1)
		{
			m_getAck.Empty();
			SendData(strHdcp14_3);
			if (m_totalBlock_of_HDCP14 >= 4)
				m_step = 4;
			else
				m_step = 30;
			m_iCheckAckCount = 0;
		}
		else
		{
			if (m_iCheckAckCount++ > DELAY_TIMES)
				m_step = 55;
		}
		break;
	case 4:
		if (m_getAck.Find("03 0C F1") != -1)
		{
			m_getAck.Empty();
			SendData(strHdcp14_4);
			if(m_burnHDCP22)
				m_step = 30;
			else
				m_step = 36;
			m_iCheckAckCount = 0;
		}
		else
		{
			if (m_iCheckAckCount++ > DELAY_TIMES)
				m_step = 55;
		}
		break;
	case 30://burn HDCP22
		if ((m_getAck.Find("03 0C F2") != -1) || ((!m_burnHDCP14) && m_getAck.Find("03 0C F1") != -1))
		{
			CString tmp, strSendData;
			UINT16 checkSum;
			m_strRXData += "\r\n";
			checkSum = calculateCRC_ROKU((const UINT8*)m_sReadHdcp22, 1044);
			strSendData = "07 51 01 FE 00 0F E4 00 ";// 05 04 14 ";
			tmp.Format("%02X ", BLOCK_NO);
			strSendData += tmp;

			tmp.Format("%02X ", (U8)(BLOCK_SIZE >> 8));
			strSendData += tmp;
			tmp.Format("%02X ", (U8)(BLOCK_SIZE));
			strSendData += tmp;
			//strSendData += "04 14  ";//key len
			tmp.Format("%02X ", (U8)(checkSum >> 8));
			strSendData += tmp;
			tmp.Format("%02X ", (U8)checkSum);
			strSendData += tmp;
			strSendData += "00 00";
			strSendData = clrCRC(strSendData);
			SendData(strSendData);

			m_strRXData += "\r\n烧HDCP22_0: ";
			m_strRXData += strSendData;
			m_strRXData += "\r\n";
			m_getAck.Empty();
			m_step = 31;
			m_iCheckAckCount = 0;
			m_Curblock = 0;
		}
		else
		{
			if (m_iCheckAckCount++ > DELAY_TIMES)
				m_step = 55;
		}
		break;
	case 31:
		if (m_getAck.Find("03 0C F1") != -1)
		{
			CString tmp, strSendData;
			int maxNum;
			strSendData = "07 51 01 FE 00 ";// 00 F0 E4 ";// 01 ";
			if (m_Curblock == (BLOCK_NO - 1))
				tmp.Format("%02X ", 1044 - (BLOCK_SIZE * (BLOCK_NO - 1)) + 10);
			else
				tmp.Format("%02X ", BLOCK_SIZE + 10);
			strSendData += tmp;
			strSendData += "E4 ";
			tmp.Format("%02X ", m_Curblock + 1);
			strSendData += tmp;
			if (m_Curblock == (BLOCK_NO - 1))
				maxNum = 1044 - (BLOCK_SIZE * (BLOCK_NO - 1));
			else
				maxNum = BLOCK_SIZE;
			for (int i = 0; i < maxNum; i++)//304
			{
				tmp.Format("%02X ", m_sReadHdcp22[i + m_Curblock * BLOCK_SIZE] & 0xFF);
				//m_strRXData += tmp;
				strSendData += tmp;
			}
			strSendData += "00 00";
			strSendData = clrCRC(strSendData);
			m_strRXData += "\r\n烧HDCP22_";
			tmp.Format("%01X: ", m_Curblock + 1);
			m_strRXData += tmp;
			m_strRXData += strSendData;
			m_strRXData += "\r\n";
			m_getAck.Empty();
			SendData(strSendData);
			m_iCheckAckCount = 0;
			if (m_Curblock == 8)
				m_step = 36;
			else
				m_step = 32;
		}
		else
		{
			if (m_iCheckAckCount++ > DELAY_TIMES)
				m_step = 55;
		}
		break;
	case 32:
		if (m_getAck.Find("03 0C F1") != -1)
		{
			m_Curblock++;
			m_step = 31;
		}
		else
		{
			if (m_iCheckAckCount++ > DELAY_TIMES)
				m_step = 55;
		}
	case 36:
		if (m_getAck.Find("03 0C F2") != -1)
		{
			m_strRXData += "\r\n烧KEK OK!!!! ";
			PostMessage(WM_UPDATE_MESSAGE, 0, 0);
			m_step = 37;
		}
		else
		{
			if (m_iCheckAckCount++ > DELAY_TIMES)
				m_step = 55;
		}
		break;
	case 37:
	{
		m_step = 38;
		//CString filePath = _T("要删除的文件路径");
		CFile::Remove(m_HDCP14_path);
		CFile::Remove(m_HDCP22_path);
		KillTimer(TIME_BURN_HDCP);
		MessageBox("烧key OK");
	}
	break;
	case 55:
		KillTimer(TIME_BURN_HDCP);
		MessageBox("烧key FAIL");
		break;
	default:
		break;
	}
	}
	else if (TIME_GET_SN== nIDEvent)
	{
	switch (m_step)
	{
	case 5:
		KillTimer(TIME_GET_SN);
		MessageBox("Get key Fail !");
		break;
	case 0:
		if (m_getAck.Find("LMC1") != -1)
		{
			CString tmp;
			//CString strUlpk_path;
			KillTimer(TIME_GET_SN);
			//03 0C 15 4C 4D 43 31 56 31 47 5A 30 30 32 30 30 36 36 00 78 CF 
			//01 31 2D 64
			int start = m_getAck.Find("LMC1");
			tmp=m_getAck.Mid(start, start + 12);
			/*for (int i = 9; i < 29; i++)
			{
				if (m_getAck.GetAt(i) == 0x20)
					continue;
				tmp += m_getAck.GetAt(i);
			}*/
			//int v_dec;
			GetDlgItem(IDC_STATIC_SHOW_SN)->SetWindowText(tmp);
			if (m_load_SN_list.GetAt(m_SN_index).Find(tmp) != -1)
			{	
				MessageBox("SN burn OK!");
				if (m_SN_index < m_load_SN_list.GetSize())
				{
					m_SN_index++;
					if (m_SN_index == m_load_SN_list.GetSize())
					{
						m_SN_index = 0;
						tmp.Format("%d", m_SN_index);
						::WritePrivateProfileString(_T("SN"), _T("SnIndex"), tmp, m_strConfigFile);
						GetDlgItem(IDC_EDIT_SN2)->SetWindowText(m_load_SN_list.GetAt(m_SN_index));
						GetDlgItem(IDC_EDIT_SN_INDEX)->SetWindowText(tmp);
						MessageBox("SN号用完了");
					}
					else
					{
						tmp.Format("%d", m_SN_index);
						::WritePrivateProfileString(_T("SN"), _T("SnIndex"), tmp, m_strConfigFile);
						GetDlgItem(IDC_EDIT_SN2)->SetWindowText(m_load_SN_list.GetAt(m_SN_index));
						GetDlgItem(IDC_EDIT_SN_INDEX)->SetWindowText(tmp);
					}
					
				}	
				else
					MessageBox("SN号用完了");
			}
			else
			{
				MessageBox("get SN is not = burn SN!");
			}
			
		}
		else
		{
			if (m_iCheckAckCount++ > 5)
				m_step = 5;
			else
				m_step = 1;
		}
		break;
	case 1:
	{
		CString tmp;
		tmp = "07 51 01 08 1B 01 69 8F";
		m_getAck.Empty();
		SendData(tmp);
		m_step = 0;
	}
	default:
		break;
	}	
	}
	else if (TIME_GET_RESET_ACK == nIDEvent)
	{
	if ((m_getAck.Find("03 0C E0") != -1))
	{
		KillTimer(TIME_GET_RESET_ACK);
		MessageBox("reset 失败！");
	}
	switch (m_step)
	{
	case 0:
		if (m_getAck.Find("03 0C F1") != -1)
		{
			//CString tmp;
			//tmp = "07 51 01 07 D3 20 EE";
			//m_getAck.Empty();
			//SendData(tmp);
			m_step = 1;
			m_iCheckAckCount = 0;
		}
		else
		{
			if (m_iCheckAckCount++ > 50)
				m_step = 5;
		}
		break;
	case 1:
		KillTimer(TIME_GET_RESET_ACK);
		MessageBox("Reset OK!");
		break;
	case 5:
		KillTimer(TIME_GET_RESET_ACK);
		MessageBox("Reset NG!");
		break;
	default:
		break;
	}
	}
	CDialog::OnTimer(nIDEvent);
}

#if 0//test
#define ElementType char
bool isEven(unsigned int N)
{
	if (N % 2 == 0)
		return TRUE;
	else
		return FALSE;
}
unsigned long long int Pow(unsigned long long int X, unsigned int N)
{
	if (N == 0)
		return 1;
	if (N == 1)
		return X;
	if (isEven(N))
		return Pow(X * X, N / 2);
	else
		return Pow(X * X, N / 2) * X;
}
void shellsort(ElementType A[], int N)
{
	int i, j, Increment;
	ElementType Tmp;

	for (Increment = N / 2; Increment > 0; Increment /= 2)
	{
		for (i = Increment; i < N; i++)
		{
			Tmp = A[i];
			for (j = i; j >= Increment; j -= Increment)
			{
				if (Tmp < A[j - Increment])
					A[j] = A[j - Increment];
				else
					break;
			}
			A[j] = Tmp;
		}
	}
}

void InsertionSort(ElementType A[], int N)
{
	int j, p;
	ElementType tmp;
	for (p = 0; p < N; p++)
	{
		tmp = A[p];
		for (j = p; j > 0 && A[j - 1] > tmp; j--)
		{
			A[j] = A[j - 1];
		}
		A[j] = tmp;
	}
}
void Swap(ElementType* A, ElementType* B)
{
	ElementType tmp;
	tmp = *A;
	*A = *B;
	*B = tmp;
}
ElementType Median3(ElementType A[], int left, int right)
{
	int center = (left + right) / 2;
	if (A[left] > A[center])
		Swap(&A[left], &A[center]);
	if (A[left] > A[right])
		Swap(&A[left], &A[right]);
	if (A[center] > A[right])
		Swap(&A[center], &A[right]);

	Swap(&A[center], &A[right - 1]);
	return A[right - 1];
}
#define Cutoff (3)
void Qsort(ElementType A[], int left, int right)
{
	int i, j;
	ElementType Pivot;
	if (left + Cutoff <= right)
	{
		Pivot = Median3(A, left, right);
		i = left; j = right - 1;
		for (;;)
		{
			while(A[++i]<Pivot){}
			while(A[--j]>Pivot){}
			if (i < j)
				Swap(&A[i], &A[j]);
			else
				break;
		}
		Swap(&A[i], &A[right - 1]);
		Qsort(A, left, i - 1);
		Qsort(A, i + 1, right);
	}
	else
	{
		InsertionSort(A + left, right - left + 1);
	}
}
void QuickSort(ElementType A[], int N)
{
	Qsort(A, 0, N - 1);
}
void CnewSerialDlg::OnBnClickedButtonTest()
{
	// TODO: 在此添加控件通知处理程序代码
	unsigned long long int X;
	CString x,tmp;
	char A[] = { 10,5,6,12,5,6,2,1,8,98,45,17 };
	QuickSort(A, 12);
	//X = Pow(2,96)-1;
	for (int i = 0; i < 12; i++)
	{
		tmp.Format(_T("%d,"), A[i]);
		x += tmp;
	}	

	MessageBox(x);
}

int binarySeach(const ElementType A[], ElementType X, int N)
{
	int low, mid, high;
	low = 0; high = N - 1;
	while (low <= high)
	{
		mid = (low + high) / 2;
		if (X > A[mid])
			low = mid + 1;
		else if (X < A[mid])
			high = mid - 1;
		else
			return mid;
	}
	return -1;

}

#define MinPQSize 100
#define MinData 0
struct HeapStruct;

typedef struct HeapStruct* priortyqueue;
struct HeapStruct
{
	int Capacity;
	int Size;
	ElementType* Elements;
};
priortyqueue initialize(int MaxElements)
{
	priortyqueue H;
	if (MaxElements < MinPQSize)
		return NULL;// Error("");
	H = (priortyqueue)malloc(sizeof(struct HeapStruct));
	if (H == NULL)
	{
		//FatalError();
	}	
	
	H->Elements = (ElementType*)malloc((MaxElements + 1) * sizeof(ElementType));
	if (H->Elements == NULL)
	{
		//FatalError();
	}
	H->Capacity = MaxElements;
	H->Size = 0;
	H->Elements[0] = MinData;
	return H;
}


#endif










void CnewSerialDlg::OnBnClickedCheckShowHex()
{
	// TODO: 在此添加控件通知处理程序代码
	bShowHex = !bShowHex;
}


void CnewSerialDlg::OnBnClickedCheckSendHex()
{
	// TODO: 在此添加控件通知处理程序代码
	m_bSendHex = !m_bSendHex;
}


void CnewSerialDlg::OnBnClickedCheckSendHex2()
{
	// TODO: 在此添加控件通知处理程序代码
	m_bSendHex2 = !m_bSendHex2;
}


void CnewSerialDlg::OnBnClickedCheckSendHex3()
{
	// TODO: 在此添加控件通知处理程序代码
	m_bSendHex3 = !m_bSendHex3;
}


void CnewSerialDlg::OnBnClickedCheckSendHex4()
{
	// TODO: 在此添加控件通知处理程序代码
	m_bSendHex4 = !m_bSendHex4;
}


void CnewSerialDlg::OnBnClickedCheckSendHex5()
{
	// TODO: 在此添加控件通知处理程序代码
	m_bSendHex5 = !m_bSendHex5;
}


void CnewSerialDlg::OnBnClickedCheckSendHex6()
{
	// TODO: 在此添加控件通知处理程序代码
	m_bSendHex6 = !m_bSendHex6;
}


void CnewSerialDlg::OnBnClickedCheckSendHex7()
{
	// TODO: 在此添加控件通知处理程序代码
	m_bSendHex7 = !m_bSendHex7;
}


void CnewSerialDlg::OnBnClickedCheckSendHex8()
{
	// TODO: 在此添加控件通知处理程序代码
	m_bSendHex8 = !m_bSendHex8;
}


void CnewSerialDlg::OnBnClickedCheckAutoClear()
{
	// TODO: 在此添加控件通知处理程序代码
	m_bAutoClear = !m_bAutoClear;
}

void CnewSerialDlg::OnBnClickedCheckAutoLf()
{
	// TODO: 在此添加控件通知处理程序代码
	m_bAutoLF = !m_bAutoLF;
#if 0
	if (m_bAutoLF)
	{
		//m_editRX.EnableScrollBar(WM_HSCROLL, 0);
		//m_editRX.EnableScrollBarCtrl(WM_HSCROLL, 0);
		//m_editRX.Create(WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_TABSTOP, CRect(10, 10, 100, 200), this, 122);
		//m_editRX.ModifyStyle(WS_HSCROLL | ES_AUTOHSCROLL, 0);
		//m_editRX.ModifyStyle(ES_AUTOHSCROLL| WS_HSCROLL, NULL, NULL);
		m_editRX.ModifyStyleEx(ES_AUTOHSCROLL | WS_HSCROLL, NULL, NULL);//WS_AUTOHSCROLL
		m_editRX.GetWindowLong();
		m_editRX.GetWindowInfo();
		//m_editRX.ModifyStyle(ES_LEFT, 1);
		//m_editRX.enab
	}	
	else
	{
		//m_editRX.ModifyStyle(WS_HSCROLL | ES_AUTOHSCROLL, 1);
		//m_editRX.ModifyStyle(ES_AUTOHSCROLL| ES_LEFT, 1);
		m_editRX.ModifyStyleEx(NULL,ES_AUTOHSCROLL| WS_HSCROLL, NULL);
	}	
	//UpdateWindow();
	//UpdateData(TRUE);
#endif
}


void CnewSerialDlg::OnBnClickedCheckFilter()
{
	// TODO: 在此添加控件通知处理程序代码
	m_bFilter = !m_bFilter;
}


void CnewSerialDlg::OnEnChangeEditFilter()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	//UpdateData(FALSE);
	//((CEdit*)(GetDlgItem(IDC_EDIT_FILTER)))->GetWindowText(m_strFilter);
}


void CnewSerialDlg::OnBnClickedButtonPause()
{
	// TODO: 在此添加控件通知处理程序代码
	m_bPause = !m_bPause;
	if (m_bPause)
	{
		HBITMAP hBmp;    // 保存CBitmap加载的位图的句柄   
		CBitmap m_bitmap;
		m_bitmap.LoadBitmap(IDB_BITMAP_NG);  // 将位图IDB_BITMAP1加载到bitmap   
		hBmp = (HBITMAP)m_bitmap.GetSafeHandle();  // 获取bitmap加载位图的句柄  
		//m_image.ModifyStyle(0xf, SS_BITMAP | SS_CENTERIMAGE);
		m_image2.SetBitmap(hBmp);    // 设置图片控件m_jzmPicture的位图图片为IDB_BITMAP1  
	}
	else
	{
		if (m_bIsComOpen)
		{
			HBITMAP hBmp;    // 保存CBitmap加载的位图的句柄   
			CBitmap m_bitmap;
			m_bitmap.LoadBitmap(IDB_BITMAP_READY);  // 将位图IDB_BITMAP1加载到bitmap   
			hBmp = (HBITMAP)m_bitmap.GetSafeHandle();  // 获取bitmap加载位图的句柄  
			//m_image.ModifyStyle(0xf, SS_BITMAP | SS_CENTERIMAGE);
			m_image2.SetBitmap(hBmp);    // 设置图片控件m_jzmPicture的位图图片为IDB_BITMAP1  
		}	
	}
}

void CnewSerialDlg::OnBnClickedButton4()
{
	// TODO: 在此添加控件通知处理程序代码
	//if (m_bSendHex)//发送16进制数据
	{
		//CByteArray HexDataBuf;
		int i = 0;
		BYTE SendBuf[1024] = { 0 };
		BYTE GetData[1024] = { 0 };
		int SendLen = 0;
		int GetLen = 0;

		UpdateData(TRUE);//获取编辑框内容
		if (m_EditSend.IsEmpty())
		{
			AfxMessageBox(_T("发送数据为空!"));
			return;
		}
		//HexDataBuf.RemoveAll();    //清空数组 

		GetLen = m_EditSend.GetLength();
		for (i = 0; i < GetLen; i++)
		{
			GetData[i] = (BYTE)m_EditSend.GetBuffer()[i];
		}
		StringtoHex(GetData, GetLen, SendBuf, &SendLen);//将字符串转化为字节数据

		U16 checkSum = calculateCRC_ROKU(SendBuf, SendLen-2);
		CString tmp;
		m_EditSend.Empty();
		for (i = 0; i < SendLen-2; i++)
		{
			tmp.Format("%02X ", SendBuf[i]);
			m_EditSend += tmp;
		}
		tmp.Format("%02X ", (U8)(checkSum >> 8));
		m_EditSend += tmp;
		tmp.Format("%02X", (U8)checkSum);
		m_EditSend += tmp;
		UpdateData(FALSE);
		//if (m_bIsComOpen)
			//m_ctrlComm.put_Output(COleVariant(HexDataBuf)); //发送十六进制数据

		//UpdateData(false); //更新编辑框内容
	}
}


void CnewSerialDlg::OnBnClickedCheckAddEnter()
{
	// TODO: 在此添加控件通知处理程序代码
	m_bAutoEnter = !m_bAutoEnter;
	CString tmp;
	if (m_bAutoEnter)
		tmp = "1";
	else
		tmp = "0";
	::WritePrivateProfileString(_T("COMM"), _T("AutoEnter"), tmp, m_strConfigFile);
}


void CnewSerialDlg::OnBnClickedMfcbuttonTest()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_bIsComOpen)
	{
		if (m_ctrlComm.get_PortOpen())
		{
			m_bIsComOpen = FALSE;
			m_ctrlComm.put_PortOpen(FALSE);//打开串口  
			((CComboBox*)GetDlgItem(IDC_COMBO_PORT_SELECT))->EnableWindow(TRUE);
			((CComboBox*)GetDlgItem(IDC_COMBO_BAUD_SELECT))->EnableWindow(TRUE);
			((CButton*)GetDlgItem(IDC_MFCBUTTON_TEST))->SetWindowText(_T("打开串口"));

			HBITMAP hBmp;    // 保存CBitmap加载的位图的句柄   
			CBitmap m_bitmap;
			m_bitmap.LoadBitmap(IDB_BITMAP_NG);  // 将位图IDB_BITMAP1加载到bitmap   
			hBmp = (HBITMAP)m_bitmap.GetSafeHandle();  // 获取bitmap加载位图的句柄  
				//m_image.ModifyStyle(0xf, SS_BITMAP | SS_CENTERIMAGE);
			m_image.SetBitmap(hBmp);    // 设置图片控件m_jzmPicture的位图图片为IDB_BITMAP1  
			m_image2.SetBitmap(hBmp);
			KillTimer(TIME_SHOW_MESSAGE_SERIAL);
			m_mfcButtonTest.SetButtonBgColor(0x0000FF);
			m_mfcButtonTest.SetButtonTextColor(0xFFFFFF);
			KillTimer(TIME_AUTO_CHANGE_SOURCE);

		}
	}
	else
	{
		openComm();
		SetTimer(TIME_SHOW_MESSAGE_SERIAL, 100, NULL);
		m_mfcButtonTest.SetButtonBgColor(0x00FF00);
		m_mfcButtonTest.SetButtonTextColor(0x000000);
		//GetDlgItem(IDC_EDIT_TX)->SetFocus();
	}
}


void CnewSerialDlg::OnBnClickedButtonLoadComlist()
{
	// TODO: 在此添加控件通知处理程序代码
	CString selList;
	switch (m_comList.GetCurSel())
	{
	case 0:
		selList = "SEND1";
		break;
	case 1:
		selList = "SEND2";
		break;
	case 2:
		selList = "SEND3";
		break;
	case 3:
		selList = "SEND4";
		break;
	case 4:
		selList = "SEND5";
		break;
	case 5:
		selList = "SEND6";
		break;
	case 6:
		selList = "SEND7";
		break;
	case 7:
		selList = "SEND8";
		break;
	default:
		selList = "SEND";
		break;
	}
	::GetPrivateProfileString(selList, _T("name"), _T(""), m_strEditListName.GetBuffer(100), 100, m_strConfigFile);	//串口打开按钮状态，1开，0 关
	::GetPrivateProfileString(selList, _T("send1"), _T(""), m_EditSend.GetBuffer(100), 100, m_strConfigFile);	//串口打开按钮状态，1开，0 关
	::GetPrivateProfileString(selList, _T("send2"), _T(""), m_EditSend2.GetBuffer(100), 100, m_strConfigFile);	//串口打开按钮状态，1开，0 关
	::GetPrivateProfileString(selList, _T("send3"), _T(""), m_EditSend3.GetBuffer(100), 100, m_strConfigFile);	//串口打开按钮状态，1开，0 关
	::GetPrivateProfileString(selList, _T("send4"), _T(""), m_EditSend4.GetBuffer(100), 100, m_strConfigFile);	//串口打开按钮状态，1开，0 关
	::GetPrivateProfileString(selList, _T("send5"), _T(""), m_EditSend5.GetBuffer(100), 100, m_strConfigFile);	//串口打开按钮状态，1开，0 关
	::GetPrivateProfileString(selList, _T("send6"), _T(""), m_EditSend6.GetBuffer(100), 100, m_strConfigFile);	//串口打开按钮状态，1开，0 关
	::GetPrivateProfileString(selList, _T("send7"), _T(""), m_EditSend7.GetBuffer(100), 100, m_strConfigFile);	//串口打开按钮状态，1开，0 关
	::GetPrivateProfileString(selList, _T("send8"), _T(""), m_EditSend8.GetBuffer(100), 100, m_strConfigFile);	//串口打开按钮状态，1开，0 关
	CString tmp;
	::GetPrivateProfileString(selList, _T("SendHex1"), _T(""), tmp.GetBuffer(2), 2, m_strConfigFile);	//串口打开按钮状态，1开，0 关
	if (tmp == "1")
	{
		m_bSendHex = TRUE;		
	}
	else
	{
		m_bSendHex = FALSE;
	}
	::GetPrivateProfileString(selList, _T("SendHex2"), _T(""), tmp.GetBuffer(2), 2, m_strConfigFile);	//串口打开按钮状态，1开，0 关
	if (tmp == "1")
	{
		m_bSendHex2 = TRUE;
	}
	else
	{
		m_bSendHex2 = FALSE;
	}
	::GetPrivateProfileString(selList, _T("SendHex3"), _T(""), tmp.GetBuffer(2), 2, m_strConfigFile);	//串口打开按钮状态，1开，0 关
	if (tmp == "1")
	{
		m_bSendHex3 = TRUE;
	}
	else
	{
		m_bSendHex3 = FALSE;
	}
	::GetPrivateProfileString(selList, _T("SendHex4"), _T(""), tmp.GetBuffer(2), 2, m_strConfigFile);	//串口打开按钮状态，1开，0 关
	if (tmp == "1")
	{
		m_bSendHex4 = TRUE;
	}
	else
	{
		m_bSendHex4 = FALSE;
	}
	::GetPrivateProfileString(selList, _T("SendHex5"), _T(""), tmp.GetBuffer(2), 2, m_strConfigFile);	//串口打开按钮状态，1开，0 关
	if (tmp == "1")
	{
		m_bSendHex5 = TRUE;
	}
	else
	{
		m_bSendHex5 = FALSE;
	}
	::GetPrivateProfileString(selList, _T("SendHex6"), _T(""), tmp.GetBuffer(2), 2, m_strConfigFile);	//串口打开按钮状态，1开，0 关
	if (tmp == "1")
	{
		m_bSendHex6 = TRUE;
	}
	else
	{
		m_bSendHex6 = FALSE;
	}
	::GetPrivateProfileString(selList, _T("SendHex7"), _T(""), tmp.GetBuffer(2), 2, m_strConfigFile);	//串口打开按钮状态，1开，0 关
	if (tmp == "1")
	{
		m_bSendHex7 = TRUE;
	}
	else
	{
		m_bSendHex7 = FALSE;
	}
	::GetPrivateProfileString(selList, _T("SendHex8"), _T(""), tmp.GetBuffer(2), 2, m_strConfigFile);	//串口打开按钮状态，1开，0 关
	if (tmp == "1")
	{
		m_bSendHex8 = TRUE;
	}
	else
	{
		m_bSendHex8 = FALSE;
	}
	
	UpdateData(FALSE);
}


void CnewSerialDlg::OnCbnSelchangeComboCommandList()
{
	// TODO: 在此添加控件通知处理程序代码
	OnBnClickedButtonLoadComlist();
	CString tmp;
	tmp.Format(_T("%d"), m_comList.GetCurSel()+1);
	::WritePrivateProfileString(_T("COMM"), _T("ComList"), tmp, m_strConfigFile);
}


void CnewSerialDlg::OnBnClickedCheckDelSpec()
{
	// TODO: 在此添加控件通知处理程序代码
	m_autoDelSpec = !m_autoDelSpec;
	CString tmp;
	if (m_autoDelSpec)
		tmp = "1";
	else
		tmp = "0";
	::WritePrivateProfileString(_T("COMM"), _T("AutoDelSpecCharater"), tmp, m_strConfigFile);
}

void CnewSerialDlg::SendData(CString cData)
{
	CByteArray HexDataBuf;
	int i = 0;
	BYTE SendBuf[1000] = { 0 };
	BYTE GetData[1000] = { 0 };
	int SendLen = 0;
	int GetLen = 0;

	//UpdateData(TRUE);//获取编辑框内容

	HexDataBuf.RemoveAll();    //清空数组 

	GetLen = cData.GetLength();

	{
		for (i = 0; i < GetLen; i++)
		{
			GetData[i] = (BYTE)cData.GetBuffer()[i];
		}
	}
	StringtoHex(GetData, GetLen, SendBuf, &SendLen);//将字符串转化为字节数据

	U16 checkSum = calculateCRC_ROKU(SendBuf, SendLen - 2);
	CString tmp;
	cData.Empty();
	for (i = 0; i < SendLen - 2; i++)
	{
		tmp.Format("%02X ", SendBuf[i]);
		cData += tmp;
	}
	tmp.Format("%02X ", (U8)(checkSum >> 8));
	cData += tmp;
	tmp.Format("%02X", (U8)checkSum);
	cData += tmp;

	{
		for (i = 0; i < GetLen; i++)
		{
			GetData[i] = (BYTE)cData.GetBuffer()[i];
		}
	}
	StringtoHex(GetData, GetLen, SendBuf, &SendLen);

	{
		HexDataBuf.SetSize(SendLen);   //设置数组大小为帧长度 

		for (i = 0; i < SendLen; i++)
		{
			HexDataBuf.SetAt(i, SendBuf[i]);
		}

		if (m_bIsComOpen)
			m_ctrlComm.put_Output(COleVariant(HexDataBuf)); //发送十六进制数据
	}
	Sleep(200);
}
void CnewSerialDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	CString t1;
	t1 = "07 51 01 07 87 3A 9F";//start
	SendData(t1);
	Sleep(100);
	t1 = "07 51 01 0A 55 FF FF FF BC CC";//test patt
	SendData(t1);
	Sleep(100);
	t1 = "07 51 01 07 05 8B 55";//normal
	SendData(t1);
	Sleep(100);
	t1 = "07 51 01 09 01 00 70 22 20";//112
	SendData(t1);
	Sleep(100);
	t1 = "07 51 01 09 02 00 71 22 20";
	SendData(t1);
	Sleep(100);
	t1 = "07 51 01 09 03 00 72 22 20";
	SendData(t1);
	Sleep(100);
	t1 = "07 51 01 0A 00 01 01 01 B3 FE";
	SendData(t1);
	Sleep(100);
	t1 = "07 51 01 07 06 BB 36";//cool
	SendData(t1);
	Sleep(100);
	t1 = "07 51 01 09 01 00 80 22 20";//128
	SendData(t1);
	Sleep(100);
	t1 = "07 51 01 09 02 00 81 22 20";
	SendData(t1);
	Sleep(100);
	t1 = "07 51 01 09 03 00 82 22 20";
	SendData(t1);
	Sleep(100);
	t1 = "07 51 01 0A 00 01 01 01 B3 FE";
	SendData(t1);
	Sleep(100);
	t1 = "07 51 01 07 07 AB 17";//warm
	SendData(t1);
	Sleep(100);
	t1 = "07 51 01 09 01 00 90 22 20";//144
	SendData(t1);
	Sleep(100);
	t1 = "07 51 01 09 02 00 91 22 20";
	SendData(t1);
	Sleep(100);
	t1 = "07 51 01 09 03 00 92 22 20";
	SendData(t1);
	Sleep(100);
	t1 = "07 51 01 0A 00 01 01 01 B3 FE";
	SendData(t1);
	Sleep(100);
	t1 = "07 51 01 07 28 7E 9A";
	SendData(t1);

}

CString CnewSerialDlg::clrCRC(CString EditSend)
{
	// TODO: 在此添加控件通知处理程序代码
	//if (m_bSendHex)//发送16进制数据
	{
		//CByteArray HexDataBuf;
		int i = 0;
		BYTE SendBuf[1000] = { 0 };
		BYTE GetData[1000] = { 0 };
		int SendLen = 0;
		int GetLen = 0;

		if (EditSend.IsEmpty())
		{
			AfxMessageBox(_T("发送数据为空!"));
			return 0;
		}
		//HexDataBuf.RemoveAll();    //清空数组 

		GetLen = EditSend.GetLength();
		for (i = 0; i < GetLen; i++)
		{
			GetData[i] = (BYTE)EditSend.GetBuffer()[i];
		}
		StringtoHex(GetData, GetLen, SendBuf, &SendLen);//将字符串转化为字节数据

		U16 checkSum = calculateCRC_ROKU(SendBuf, SendLen - 2);
		CString tmp;
		EditSend.Empty();
		for (i = 0; i < SendLen - 2; i++)
		{
			tmp.Format("%02X ", SendBuf[i]);
			EditSend += tmp;
		}
		tmp.Format("%02X ", (U8)(checkSum >> 8));
		EditSend += tmp;
		tmp.Format("%02X", (U8)checkSum);
		EditSend += tmp;

		return EditSend;
		//if (m_bIsComOpen)
			//m_ctrlComm.put_Output(COleVariant(HexDataBuf)); //发送十六进制数据

		//UpdateData(false); //更新编辑框内容
	}
}
#define SLEEP_TIME 2000
U8 HDCP14_SIZE = 120;// 102;
U8 HDCP14_BLOCK = 3;
void CnewSerialDlg::doSendkey(CString strUlpkPath)
{
	try {
		CFile cFlie;
	
		char sRead[1044];
		//char sReadHdcp22[1044];
		cFlie.Open(_T("key\\SN_MAC.txt"), CFile::modeRead);
		//UpdateData(FALSE);

		//cFlie.WriteString(m_strRXData);
		cFlie.Read(sRead, 50);
		cFlie.Close();
		int type = 0;
		U8 uid_len = 0;
		U8 mac[12], j = 0;
		//U8 u8Ulpk[32] = {0};
		//U8 u8Potk[16] = { 0 };
		//U8 u8Uid[4] = { 0 };
		CString sn, strmac, uid, strSn,strShowmac, strShowuid;
		CString tmp, strSendData;
		U16 checkSum;
		if (m_bAutoFacMode)
		{
			tmp = "07 51 01 07 01 CB D1";
			SendData(tmp);
			Sleep(100);
		}
		m_strSn = "07 51 01 16 88 ";
		m_strMac = "07 51 01 0D B3 ";
		for (int i = 0; i < 50; i++)
		{
			if (sRead[i] > 122)
				break;
			if (sRead[i] == ',')
				type++;
			else
			{
				if (type == 0)
				{
					strSn += sRead[i];
					tmp.Format("%02X ", sRead[i]);
					m_strSn += tmp;
				}
				else if (type == 1)
				{
					//mac += sRead[i];
					if (sRead[i] == '-')
						continue;
					mac[j++] = sRead[i];
					//tmp.Format("%02X ", sRead[i]);
					strShowmac += sRead[i];
				}
				else if (type == 2)
				{
					if ((sRead[i] > 57) || (sRead[i] < 48))
						break;
					if (uid_len++ > 8)
						break;
					uid += sRead[i];
					//tmp.Format("%02X ", sRead[i]);
					strShowuid += sRead[i];
				}
				else
					break;
			}
		}

		m_strRXData += strSn;
		m_strRXData += ", ";
		//m_strRXData += ", ";
		m_strRXData += strShowmac;
		m_strRXData += ", ";
		//m_strRXData += ", ";
		m_strRXData += strShowuid;
		m_strRXData += "\r\n烧SN: ";

		m_strSn += "00 00";
		m_strSn = clrCRC(m_strSn);
		m_strRXData += m_strSn;
		//SendData(m_strSn);
		//Sleep(100);
		int SendLen;
		BYTE SendBuf[6];
		StringtoHex(mac, sizeof(mac), SendBuf, &SendLen);

		for (U8 i = 0; i < 6; i++)
		{
			tmp.Format("%02X ", SendBuf[i]);
			m_strMac += tmp;
		}
		m_strMac += "00 00";
		m_strMac = clrCRC(m_strMac);
		m_strRXData += "\r\n烧MAC: ";
		m_strRXData += m_strMac;
		//SendData(m_strMac);
		//Sleep(100);

		CString strFileTitle;
		CFileFind finder;
		//CString HDCP14_path, HDCP22_path, ULPK_path, POTK_path;
		CString ULPK_path;

		BOOL bWorking = finder.FindFile("key\\HDCP14\\*.bin");//C:\\windows\\sysbkup\\*.cab
		if (bWorking)
		{
			bWorking = finder.FindNextFile();
			strFileTitle = finder.GetFilePath();
		}
		else
		{
			MessageBox("HDCP14 key错误");
			return;
		}
		m_HDCP14_path = strFileTitle;
		m_strRXData += "\r\n";
		m_strRXData += strFileTitle;

		bWorking = finder.FindFile("key\\HDCP22\\*.bin");//C:\\windows\\sysbkup\\*.cab
		if (bWorking)
		{
			bWorking = finder.FindNextFile();
			strFileTitle = finder.GetFilePath();
		}
		else
		{
			MessageBox("HDCP22 key错误");
			return;
		}
		m_HDCP22_path = strFileTitle;
		m_strRXData += "\r\n";
		m_strRXData += strFileTitle;

		tmp = strUlpkPath + "\\ulpk*" + uid + ".dat";
		bWorking = finder.FindFile(tmp);// ("key\\ULPK\\309\\potk*.dat");//C:\\windows\\sysbkup\\*.cab
		if (bWorking)
		{
			bWorking = finder.FindNextFile();
			strFileTitle = finder.GetFilePath();
		}
		else
		{
			MessageBox("ULPK key错误");
			return;
		}
		ULPK_path = strFileTitle;
		m_strRXData += "\r\n";
		m_strRXData += strFileTitle;
		/*
		tmp = strUlpkPath + "\\potk*" + uid + ".dat";
		bWorking = finder.FindFile(tmp);//("key\\ULPK\\309\\ulpk*.dat");//C:\\windows\\sysbkup\\*.cab
		if (bWorking)
		{
			bWorking = finder.FindNextFile();
			strFileTitle = finder.GetFilePath();//GetFileName();
		}
		else
		{
			MessageBox("POTK key错误");
			return;
		}
		POTK_path = strFileTitle;
		m_strRXData += "\r\n";
		m_strRXData += strFileTitle;*/
		//SendData(t1);
#if 1//ULPK
		cFlie.Open(ULPK_path, CFile::modeRead | CFile::typeBinary);
		cFlie.Read(sRead, 48);//32
		cFlie.Close();
		m_strRXData += "\r\n";
		strUlpkPotk = "07 51 01 3B D1 ";
		long int lUid = atol(uid);
		tmp.Format("%02X ", (U8)(lUid >> 24));
		strUlpkPotk += tmp;
		tmp.Format("%02X ", (U8)(lUid >> 16));
		strUlpkPotk += tmp;
		tmp.Format("%02X ", (U8)(lUid >> 8));
		strUlpkPotk += tmp;
		tmp.Format("%02X ", (U8)lUid);
		strUlpkPotk += tmp;
		for (int i = 0; i < 48; i++)//304
		{
			tmp.Format("%02X ", sRead[i] & 0xFF);
			//m_strRXData += tmp;
			strUlpkPotk += tmp;
		}
		strUlpkPotk += "00 00";
		strUlpkPotk = clrCRC(strUlpkPotk);
		//SendData(strUlpkPotk);
		//Sleep(100);
		m_strRXData += "\r\n烧ULPK+POTK: ";
		m_strRXData += strUlpkPotk;

		/*cFlie.Open(POTK_path, CFile::modeRead | CFile::typeBinary);
		cFlie.Read(sRead, 16);
		cFlie.Close();
		m_strRXData += "\r\n";
		strSendData = "07 51 01 1D D2 ";
		//long int lUid = atol(uid);
		tmp.Format("%02X ", (U8)(lUid >> 24));
		strSendData += tmp;
		tmp.Format("%02X ", (U8)(lUid >> 16));
		strSendData += tmp;
		tmp.Format("%02X ", (U8)(lUid >> 8));
		strSendData += tmp;
		tmp.Format("%02X ", (U8)lUid);
		strSendData += tmp;
		for (int i = 0; i < 16; i++)//304
		{
			tmp.Format("%02X ", sRead[i] & 0xFF);
			//m_strRXData += tmp;
			strSendData += tmp;
		}
		strSendData += "00 00";
		strSendData = clrCRC(strSendData);
		SendData(strSendData);
		Sleep(100);
		m_strRXData += "\r\n烧POTK: ";
		m_strRXData += strSendData;*/
#endif

#if 1//HDCP14	
		cFlie.Open(m_HDCP14_path, CFile::modeRead);
		cFlie.Read(sRead, 304);
		cFlie.Close();
		m_strRXData += "\r\n";

		checkSum = calculateCRC_ROKU((const U8*)sRead, 304);
		strHdcp14_0 = "07 51 01 FE 00 0F E3 00 03 01 30 ";
		tmp.Format("%02X ", (U8)(checkSum >> 8));
		strHdcp14_0 += tmp;
		tmp.Format("%02X ", (U8)checkSum);
		strHdcp14_0 += tmp;
		strHdcp14_0 += "00 00";
		strHdcp14_0 = clrCRC(strHdcp14_0);
		//SendData(strHdcp14_0);
		//Sleep(100);
		m_strRXData += "\r\n烧HDCP14_0: ";
		m_strRXData += strHdcp14_0;
		strHdcp14_1 = "07 51 01 FE 00 ";//00 8A E3 01 ";
		tmp.Format("%02X ", (U8)(HDCP14_SIZE + 10));
		strHdcp14_1 += tmp;
		strHdcp14_1 += "E3 01 ";
		for (int i = 0; i < HDCP14_SIZE; i++)//304
		{
			tmp.Format("%02X ", sRead[i] & 0xFF);
			//m_strRXData += tmp;
			strHdcp14_1 += tmp;
		}
		strHdcp14_1 += "00 00";
		strHdcp14_1 = clrCRC(strHdcp14_1);
		//SendData(strHdcp14_1);
		//Sleep(100);
		m_strRXData += "\r\n烧HDCP14_1: ";
		m_strRXData += strHdcp14_1;

		strHdcp14_2 = "07 51 01 FE 00 ";//8A E3 02 ";
		tmp.Format("%02X ", (U8)(HDCP14_SIZE + 10));
		strHdcp14_2 += tmp;
		strHdcp14_2 += "E3 02 ";
		for (int i = HDCP14_SIZE; i < HDCP14_SIZE*2; i++)//304
		{
			tmp.Format("%02X ", sRead[i] & 0xFF);
			//m_strRXData += tmp;
			strHdcp14_2 += tmp;
		}
		strHdcp14_2 += "00 00";
		strHdcp14_2 = clrCRC(strHdcp14_2);
		//SendData(strHdcp14_2);
		//Sleep(100);
		m_strRXData += "\r\n烧HDCP14_2: ";
		m_strRXData += strHdcp14_2;

		strHdcp14_3 = "07 51 01 FE 00 ";// 3A E3 03 ";
		tmp.Format("%02X ", (U8)((304-HDCP14_SIZE*2) + 10));
		strHdcp14_3 += tmp;
		strHdcp14_3 += "E3 03 ";
		for (int i = HDCP14_SIZE*2; i < 304; i++)//304
		{
			tmp.Format("%02X ", sRead[i] & 0xFF);
			//m_strRXData += tmp;
			strHdcp14_3 += tmp;
		}
		strHdcp14_3 += "00 00";
		strHdcp14_3 = clrCRC(strHdcp14_3);
		//SendData(strHdcp14_2);
		//Sleep(100);
		m_strRXData += "\r\n烧HDCP14_3: ";
		m_strRXData += strHdcp14_3;
	#if 0
		strHdcp14_4 = "07 51 01 FE 00 ";// 3A E3 03 ";
		tmp.Format("%02X ", (U8)((304 - HDCP14_SIZE * 2) + 10));
		strHdcp14_4 += tmp;
		strHdcp14_4 += "E3 04 ";
		for (int i = HDCP14_SIZE * 2; i < 304; i++)//304
		{
			tmp.Format("%02X ", sRead[i] & 0xFF);
			//m_strRXData += tmp;
			strHdcp14_4 += tmp;
		}
		strHdcp14_4 += "00 00";
		strHdcp14_4 = clrCRC(strHdcp14_4);
		//SendData(strHdcp14_2);
		//Sleep(100);
		m_strRXData += "\r\n烧HDCP14_4: ";
		m_strRXData += strHdcp14_4;
	#endif
#endif
		cFlie.Open(m_HDCP22_path, CFile::modeRead);
		cFlie.Read(m_sReadHdcp22, 1044);
		//memcpy(sReadHdcp22, sRead, 1044);
		cFlie.Close();
		SetTimer(TIME_BURN_HDCP, 300, NULL);
		m_step = 0;
		bShowHex = TRUE;
		m_getAck.Empty();
#if 0//HDCP22	
		//CString tmp, strSendData;
		//CFile cFlie;
		cFlie.Open(HDCP22_path, CFile::modeRead);
		cFlie.Read(m_sReadHdcp22, 1044);
		//memcpy(sReadHdcp22, sRead, 1044);
		cFlie.Close();
		m_strRXData += "\r\n";
		Sleep(300);
		checkSum = calculateCRC_ROKU((const U8*)sReadHdcp22, 1044);
		strSendData = "07 51 01 FE 00 0F E4 00 05 04 14 ";
		tmp.Format("%02X ", (U8)(checkSum >> 8));
		strSendData += tmp;
		tmp.Format("%02X ", (U8)checkSum);
		strSendData += tmp;
		strSendData += "00 00";
		strSendData = clrCRC(strSendData);
		SendData(strSendData);
		Sleep(SLEEP_TIME);
		m_strRXData += "\r\n烧HDCP22_0: ";
		m_strRXData += strSendData;
		strSendData = "07 51 01 FE 00 F0 E4 01 ";
		for (int i = 0; i < 230; i++)//304
		{
			tmp.Format("%02X ", m_sReadHdcp22[i] & 0xFF);
			//m_strRXData += tmp;
			strSendData += tmp;
		}
		strSendData += "00 00";
		strSendData = clrCRC(strSendData);
		SendData(strSendData);
		Sleep(SLEEP_TIME);
		m_strRXData += "\r\n烧HDCP22_1: ";
		m_strRXData += strSendData;

		strSendData = "07 51 01 FE 00 F0 E4 02 ";
		for (int i = 230; i < 230 * 2; i++)//304
		{
			tmp.Format("%02X ", m_sReadHdcp22[i] & 0xFF);
			//m_strRXData += tmp;
			strSendData += tmp;
		}
		strSendData += "00 00";
		strSendData = clrCRC(strSendData);
		SendData(strSendData);
		Sleep(SLEEP_TIME);
		m_strRXData += "\r\n烧HDCP22_2: ";
		m_strRXData += strSendData;

		strSendData = "07 51 01 FE 00 F0 E4 03 ";
		for (int i = 230 * 2; i < 230 * 3; i++)//304
		{
			tmp.Format("%02X ", m_sReadHdcp22[i] & 0xFF);
			//m_strRXData += tmp;
			strSendData += tmp;
		}
		strSendData += "00 00";
		strSendData = clrCRC(strSendData);
		SendData(strSendData);
		Sleep(SLEEP_TIME);
		m_strRXData += "\r\n烧HDCP22_3: ";
		m_strRXData += strSendData;

		strSendData = "07 51 01 FE 00 F0 E4 04 ";
		for (int i = 230 * 3; i < 230 * 4; i++)//304
		{
			tmp.Format("%02X ", m_sReadHdcp22[i] & 0xFF);
			//m_strRXData += tmp;
			strSendData += tmp;
		}
		strSendData += "00 00";
		strSendData = clrCRC(strSendData);
		SendData(strSendData);
		Sleep(SLEEP_TIME);
		m_strRXData += "\r\n烧HDCP22_4: ";
		m_strRXData += strSendData;

		strSendData = "07 51 01 FE 00 86 E4 05 ";
		for (int i = 230 * 4; i < 1044; i++)//304
		{
			tmp.Format("%02X ", m_sReadHdcp22[i] & 0xFF);
			//m_strRXData += tmp;
			strSendData += tmp;
		}
		strSendData += "00 00";
		strSendData = clrCRC(strSendData);
		SendData(strSendData);
		Sleep(SLEEP_TIME);
		m_strRXData += "\r\n烧HDCP22_5: ";
		m_strRXData += strSendData;
#endif
		m_strRXData += "\r\n";
		GetDlgItem(IDC_EDIT_RX)->SetWindowText(m_strRXData);
	}
	catch (CFileException* e)
	{
		e->ReportError();
		e->Delete();
	}
}

void CnewSerialDlg::OnBnClickedButton5()
{
	// TODO: 在此添加控件通知处理程序代码
	CString strUlpk_path = "key\\ULPK\\339";
	doSendkey(strUlpk_path);
}


void CnewSerialDlg::OnBnClickedButton6()
{
	// TODO: 在此添加控件通知处理程序代码
	CString strUlpk_path = "key\\ULPK\\309";
	doSendkey(strUlpk_path);
}



void CnewSerialDlg::OnBnClickedButton7()
{
	// TODO: 在此添加控件通知处理程序代码
	SetTimer(TIME_AUTO_CHANGE_SOURCE, 120000, NULL);
	m_step = 0;
	CString tmp = "07 51 01 07 01 CB D1";
	SendData(tmp);
}


void CnewSerialDlg::OnBnClickedButtonSetSn()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_bIsComOpen)
	{
		CString sn, strmac, uid, strSn, strShowmac, strShowuid;
		CString tmp, strSendData;
		U16 checkSum;
		UpdateData(TRUE);
		if (m_bAutoFacMode)
		{
			tmp = "07 51 01 07 01 CB D1";
			SendData(tmp);
			Sleep(100);
		}
		m_strSn = "07 51 01 16 88 ";
		for (int i = 0; i < m_strEdit_GetSN.GetLength(); i++)
		{
			tmp.Format("%02X ", m_strEdit_GetSN.GetAt(i));
			m_strSn += tmp;
		}

		m_strRXData += strSn;
		m_strRXData += ", ";
		//m_strRXData += ", ";

		m_strSn += "00 00";
		m_strSn = clrCRC(m_strSn);
		m_strRXData += m_strSn;
		GetDlgItem(IDC_EDIT_RX)->SetWindowText(m_strRXData);
		SendData(m_strSn);
	}
}


void CnewSerialDlg::OnBnClickedButtonOpenKey()
{
	// TODO: 在此添加控件通知处理程序代码
	CFile cFlie;
	char sRead[1044];
	CString sn, strmac, uid,tmp;
	CString m_strFileOut = _T("");  //初始化适应Unicode
	TCHAR szFilter[] = _T("");
	CFileDialog fileDlg(TRUE, NULL, NULL, 0, szFilter, this);
	
	// 显示打开文件对话框   s
	if (IDOK == fileDlg.DoModal())
	{
		// 如果点击了文件对话框上的“打开”按钮，则将选择的文件路径显示到编辑框里   
		m_strFileOut = fileDlg.GetPathName();
	}

	if ((m_strFileOut.Find("ulpk-00199D-0309-") == -1) && (m_strFileOut.Find("ulpk-00199D-0339-") == -1) && (m_strFileOut.Find("ulpk-potk-00199D-0359-") == -1) && (m_strFileOut.Find("ulpk-potk-00199D-0369-") == -1) && (m_strFileOut.Find("ulpk-potk-00199D-1369-") == -1) && (m_strFileOut.Find("ulpk-potk-00199D-1359-") == -1) && (m_strFileOut.Find("ulpk-potk-00199D-136C-") == -1) && (m_strFileOut.Find("ulpk-potk-00199D-135C-") == -1))
	{
		MessageBox("invalid key");
		return;
	}

	if ((m_strFileOut.Find("ulpk-potk-00199D-0359-") != -1)|| (m_strFileOut.Find("ulpk-potk-00199D-0369-") != -1) || (m_strFileOut.Find("ulpk-potk-00199D-1369-") != -1) || (m_strFileOut.Find("ulpk-potk-00199D-1359-") != -1) || (m_strFileOut.Find("ulpk-potk-00199D-136C-") != -1) || (m_strFileOut.Find("ulpk-potk-00199D-135C-") != -1))//5586
	{
		m_bIs5586 = TRUE;
	}
	else
	{
		m_bIs5586 = FALSE;
	}
#if 1
	// 1. 查找.dat的起始索引
	int nDotDatPos = m_strFileOut.Find(_T(".dat"));
	if (nDotDatPos == -1)
	{
		MessageBox("invalid key");
		return;
	}

	// 2. 从.dat位置往前找最后一个'-'
	int nLastDashPos = m_strFileOut.ReverseFind(_T('-'));
	if (nLastDashPos == -1 || nLastDashPos >= nDotDatPos)
	{
		MessageBox("invalid key");
		return;
	}

	// 3. 截取数字部分（最后一个'-'的下一个位置 到 .dat的起始位置）
	int nStart = nLastDashPos + 1;
	int nLength = nDotDatPos - nStart;
	uid = m_strFileOut.Mid(nStart, nLength);
#else
	int pos = m_strFileOut.Find("9-");

	int endpos = m_strFileOut.Find(".dat");
	m_UID_length = endpos - pos-2;
	uid = m_strFileOut.Mid(pos + 2, m_UID_length);
#endif
	GetDlgItem(IDC_STATIC_SHOW_ULPK)->SetWindowText("UID="+uid);
	cFlie.Open(m_strFileOut, CFile::modeRead | CFile::typeBinary);
	if(m_bIs5586)
		cFlie.Read(sRead, 160);//32
	else
		cFlie.Read(sRead, 48);//32
	cFlie.Close();
	m_strRXData += "UID:";
	m_strRXData += uid;
	//m_strRXData += "\r\n";
	if((m_strFileOut.Find("ulpk-potk-00199D-0359-") != -1)|| (m_strFileOut.Find("ulpk-potk-00199D-0369-") != -1) || (m_strFileOut.Find("ulpk-potk-00199D-1369-") != -1) || (m_strFileOut.Find("ulpk-potk-00199D-1359-") != -1) || (m_strFileOut.Find("ulpk-potk-00199D-136C-") != -1) || (m_strFileOut.Find("ulpk-potk-00199D-135C-") != -1))//5586
	{
		strUlpkPotk = "07 51 01 AB D1 ";
		long int lUid = atol(uid);
		tmp.Format("%02X ", (U8)(lUid >> 24));
		strUlpkPotk += tmp;
		tmp.Format("%02X ", (U8)(lUid >> 16));
		strUlpkPotk += tmp;
		tmp.Format("%02X ", (U8)(lUid >> 8));
		strUlpkPotk += tmp;
		tmp.Format("%02X ", (U8)lUid);
		strUlpkPotk += tmp;
		for (int i = 0; i < 160; i++)//304
		{
			tmp.Format("%02X ", sRead[i] & 0xFF);
			//m_strRXData += tmp;
			strUlpkPotk += tmp;
		}
		strUlpkPotk += "00 00";
		strUlpkPotk = clrCRC(strUlpkPotk);
		//SendData(strUlpkPotk);
		//Sleep(100);
		m_strRXData += "\r\nBurn ULPK: ";
		m_strRXData += strUlpkPotk;
		//GetDlgItem(IDC_EDIT_RX)->SetWindowText(m_strRXData);
		//LoadSTEP_Path = m_strFileOut;
		//SetDlgItemText(IDC_EDIT2, m_strFileOut);//文本编辑框显示所选文件绝对路径
		m_strRXData += "\r\n";
	}
	else//5583//5691
	{
		strUlpkPotk = "07 51 01 3B D1 ";
		long int lUid = atol(uid);
		tmp.Format("%02X ", (U8)(lUid >> 24));
		strUlpkPotk += tmp;
		tmp.Format("%02X ", (U8)(lUid >> 16));
		strUlpkPotk += tmp;
		tmp.Format("%02X ", (U8)(lUid >> 8));
		strUlpkPotk += tmp;
		tmp.Format("%02X ", (U8)lUid);
		strUlpkPotk += tmp;
		for (int i = 0; i < 48; i++)//304
		{
			tmp.Format("%02X ", sRead[i] & 0xFF);
			//m_strRXData += tmp;
			strUlpkPotk += tmp;
		}
		strUlpkPotk += "00 00";
		strUlpkPotk = clrCRC(strUlpkPotk);
		//SendData(strUlpkPotk);
		//Sleep(100);
		m_strRXData += "\r\nBurn ULPK: ";
		m_strRXData += strUlpkPotk;
		//GetDlgItem(IDC_EDIT_RX)->SetWindowText(m_strRXData);
		//LoadSTEP_Path = m_strFileOut;
		//SetDlgItemText(IDC_EDIT2, m_strFileOut);//文本编辑框显示所选文件绝对路径

		strPotk = "07 51 01 1B D2 ";
		//lUid = atol(uid);
		tmp.Format("%02X ", (U8)(lUid >> 24));
		strPotk += tmp;
		tmp.Format("%02X ", (U8)(lUid >> 16));
		strPotk += tmp;
		tmp.Format("%02X ", (U8)(lUid >> 8));
		strPotk += tmp;
		tmp.Format("%02X ", (U8)lUid);
		strPotk += tmp;
		for (int i = 32; i < 48; i++)//304
		{
			tmp.Format("%02X ", sRead[i] & 0xFF);
			//m_strRXData += tmp;
			strPotk += tmp;
		}
		strPotk += "00 00";
		strPotk = clrCRC(strPotk);
		m_strRXData += "\r\nBurn POTK: ";
		m_strRXData += strPotk;
		m_strRXData += "\r\n";
	}

	GetDlgItem(IDC_EDIT_RX)->SetWindowText(m_strRXData);
	GetDlgItem(IDC_BUTTON_BURN_ULPK)->EnableWindow(TRUE);
}


void CnewSerialDlg::OnBnClickedButtonBurnUlpk()
{
	// TODO: 在此添加控件通知处理程序代码
	//CString tmp;
	//tmp = "07 51 01 07 01 CB D1";
	//SendData(tmp);
	if (m_bIsComOpen)
	{
		m_iCheckAckCount = 0;
		SetTimer(TIME_BURN_ULPK, 1000, NULL);
		m_step = 0;
		bShowHex = TRUE;
		m_getAck.Empty();
	}
	else
	{
		MessageBox("COM invalid");
	}
	
	//SendData(strUlpkPotk);
	//Sleep(300);
	//SendData(strPotk);
}


void CnewSerialDlg::OnBnClickedButtonBurnMac()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_bIsComOpen)
	{
		if (m_bAutoFacMode)
		{
			CString tmp;
			tmp = "07 51 01 07 01 CB D1";
			SendData(tmp);
		}
		UpdateData(TRUE);
		m_strMac = "07 51 01 0D B3 ";
		for (int i = 0; i < m_strEditMAC.GetLength(); i++)
		{
			if ((m_strEditMAC.GetAt(i) == 0x20) || (m_strEditMAC.GetAt(i) == 0x2D))
				continue;
			//tmp.Format("%02X ", m_strEditMAC.GetAt(i));
			m_strMac += m_strEditMAC.GetAt(i);
		}
		//m_strMac += m_strEditMAC;
		m_strMac += "00 00";
		m_strMac = clrCRC(m_strMac);
		m_strRXData += "\r\nBurn MAC: ";
		m_strRXData += m_strMac;
		m_strRXData += "\r\n";
		GetDlgItem(IDC_EDIT_RX)->SetWindowText(m_strRXData);
		SendData(m_strMac);
	}
}


void CnewSerialDlg::OnBnClickedButtonGetUid()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_bIsComOpen)
	{
		if (m_bAutoFacMode)
		{
			CString tmp;
			tmp = "07 51 01 07 01 CB D1";
			SendData(tmp);
		}
		m_iCheckAckCount = 0;
		SetTimer(TIME_GET_ULPK, 500, NULL);
		m_step = 0;
		bShowHex = TRUE;
	}
}

void CnewSerialDlg::SearchKeybyUID(CString strUlpkPath)
{
	try {
		CFile cFlie;
		char sRead[1044];
		
		U8 uid_len = 0;
		U8 j = 0;

		CString strShowuid;
		CString tmp, strSendData;
		U16 checkSum;

		CString strFileTitle;
		CFileFind finder;
		//CString HDCP14_path, HDCP22_path, ULPK_path, POTK_path;
		CString ULPK_path;

		tmp = strUlpkPath + "\\ulpk*" + m_strUid + ".dat";
		BOOL bWorking = finder.FindFile(tmp);// ("key\\ULPK\\309\\potk*.dat");//C:\\windows\\sysbkup\\*.cab
		if (bWorking)
		{
			bWorking = finder.FindNextFile();
			strFileTitle = finder.GetFilePath();
		}
		else
		{
			MessageBox("ULPK key错误");
			return;
		}
		ULPK_path = strFileTitle;
		m_strRXData += "\r\n";
		m_strRXData += strFileTitle;

#if 1//ULPK
		cFlie.Open(ULPK_path, CFile::modeRead | CFile::typeBinary);
		cFlie.Read(sRead, 48);//32
		cFlie.Close();
		m_strRXData += "\r\n";
		strUlpkPotk = "07 51 01 3B D1 ";
		//long int lUid = atol(m_strUid);
		tmp.Format("%02X ", (U8)(m_longUid >> 24));
		strUlpkPotk += tmp;
		tmp.Format("%02X ", (U8)(m_longUid >> 16));
		strUlpkPotk += tmp;
		tmp.Format("%02X ", (U8)(m_longUid >> 8));
		strUlpkPotk += tmp;
		tmp.Format("%02X ", (U8)m_longUid);
		strUlpkPotk += tmp;
		for (int i = 0; i < 48; i++)//304
		{
			tmp.Format("%02X ", sRead[i] & 0xFF);
			//m_strRXData += tmp;
			strUlpkPotk += tmp;
		}
		strUlpkPotk += "00 00";
		strUlpkPotk = clrCRC(strUlpkPotk);
		//SendData(strUlpkPotk);
		//Sleep(100);
		m_strRXData += "\r\n烧ULPK+POTK: ";
		m_strRXData += strUlpkPotk;

		strPotk = "07 51 01 1B D2 ";
		//lUid = atol(uid);
		tmp.Format("%02X ", (U8)(m_longUid >> 24));
		strPotk += tmp;
		tmp.Format("%02X ", (U8)(m_longUid >> 16));
		strPotk += tmp;
		tmp.Format("%02X ", (U8)(m_longUid >> 8));
		strPotk += tmp;
		tmp.Format("%02X ", (U8)m_longUid);
		strPotk += tmp;
		for (int i = 32; i < 48; i++)//304
		{
			tmp.Format("%02X ", sRead[i] & 0xFF);
			//m_strRXData += tmp;
			strPotk += tmp;
		}
		strPotk += "00 00";
		strPotk = clrCRC(strPotk);
		m_strRXData += "\r\nBurn POTK: ";
		m_strRXData += strPotk;
		m_strRXData += "\r\n";
		GetDlgItem(IDC_EDIT_RX)->SetWindowText(m_strRXData);
		GetDlgItem(IDC_BUTTON_BURN_ULPK)->EnableWindow(TRUE);
#endif
		//m_strRXData += "\r\n";
		//GetDlgItem(IDC_EDIT_RX)->SetWindowText(m_strRXData);
	}
	catch (CFileException* e)
	{
		e->ReportError();
		e->Delete();
	}
}
#define HDCP14_TOTAL_SIZE 463//436
#define HDCP22_TOTAL_SIZE 1044
void CnewSerialDlg::burnHDCP()
{
	CFile cFlie;
	char sRead[1044];
	U16 u16KeySize;
	U16 checkSum;
	CString tmp;
	m_burnHDCP14 = FALSE;
	m_burnHDCP22 = FALSE;
	if (m_bAutoFacMode)
	{
		tmp = "07 51 01 07 01 CB D1";
		SendData(tmp);
		//Sleep(100);
	}
#if 1//HDCP14	
	if (m_HDCP14_path.Find(".bin") != -1)
	{
		cFlie.Open(m_HDCP14_path, CFile::modeRead);
		u16KeySize = cFlie.GetLength();
		tmp.Format("\r\nu16KeySize=%d \r\n", u16KeySize);
		m_strRXData += tmp;
		if (u16KeySize == 512)
		{
			u16KeySize = 304;
			cFlie.Read(sRead, 304);
				
		}
		else
			cFlie.Read(sRead, u16KeySize);//cFlie.Read(sRead, 304);
		cFlie.Close();

		m_totalBlock_of_HDCP14 = (u16KeySize / HDCP14_SIZE) + 1;

		m_strRXData += "\r\n";

		checkSum = calculateCRC_ROKU((const U8*)sRead, u16KeySize);
		strHdcp14_0 = "07 51 01 FE 00 0F E3 00 "; //strHdcp14_0 = "07 51 01 FE 00 0F E3 00 03 01 30 ";
		if ((u16KeySize % HDCP14_SIZE) != 0)
			tmp.Format("%02X ", (U8)(u16KeySize / HDCP14_SIZE + 1));
		else
			tmp.Format("%02X ", (U8)(u16KeySize / HDCP14_SIZE));
		strHdcp14_0 += tmp;

		tmp.Format("%02X ", (U8)(HDCP14_SIZE >> 8));
		strHdcp14_0 += tmp;
		tmp.Format("%02X ", (U8)(HDCP14_SIZE));
		strHdcp14_0 += tmp;

		tmp.Format("%02X ", (U8)(checkSum >> 8));
		strHdcp14_0 += tmp;
		tmp.Format("%02X ", (U8)checkSum);
		strHdcp14_0 += tmp;
		strHdcp14_0 += "00 00";
		strHdcp14_0 = clrCRC(strHdcp14_0);
		//SendData(strHdcp14_0);
		//Sleep(100);
		m_strRXData += "\r\n烧HDCP14_0: ";
		m_strRXData += strHdcp14_0;
		strHdcp14_1 = "07 51 01 FE 00 ";//00 8A E3 01 ";
		tmp.Format("%02X ", (U8)(HDCP14_SIZE + 10));
		strHdcp14_1 += tmp;
		strHdcp14_1 += "E3 01 ";
		for (int i = 0; i < HDCP14_SIZE; i++)//304
		{
			tmp.Format("%02X ", sRead[i] & 0xFF);
			//m_strRXData += tmp;
			strHdcp14_1 += tmp;
		}
		strHdcp14_1 += "00 00";
		strHdcp14_1 = clrCRC(strHdcp14_1);
		//SendData(strHdcp14_1);
		//Sleep(100);
		m_strRXData += "\r\n烧HDCP14_1: ";
		m_strRXData += strHdcp14_1;

		strHdcp14_2 = "07 51 01 FE 00 ";//8A E3 02 ";
		tmp.Format("%02X ", (U8)(HDCP14_SIZE + 10));
		strHdcp14_2 += tmp;
		strHdcp14_2 += "E3 02 ";
		for (int i = HDCP14_SIZE; i < HDCP14_SIZE * 2; i++)//304
		{
			tmp.Format("%02X ", sRead[i] & 0xFF);
			//m_strRXData += tmp;
			strHdcp14_2 += tmp;
		}
		strHdcp14_2 += "00 00";
		strHdcp14_2 = clrCRC(strHdcp14_2);
		//SendData(strHdcp14_2);
		//Sleep(100);
		m_strRXData += "\r\n烧HDCP14_2: ";
		m_strRXData += strHdcp14_2;


		strHdcp14_3 = "07 51 01 FE 00 ";// 3A E3 03 ";
		if (u16KeySize > HDCP14_SIZE * 3)
		{
			tmp.Format("%02X ", (U8)(HDCP14_SIZE + 10));
			strHdcp14_3 += tmp;
			strHdcp14_3 += "E3 03 ";
			for (int i = HDCP14_SIZE * 2; i < HDCP14_SIZE * 3; i++)//304
			{
				tmp.Format("%02X ", sRead[i] & 0xFF);
				//m_strRXData += tmp;
				strHdcp14_3 += tmp;
			}
		}
		else
		{
			tmp.Format("%02X ", (U8)((u16KeySize - HDCP14_SIZE * 2) + 10));
			strHdcp14_3 += tmp;
			strHdcp14_3 += "E3 03 ";
			for (int i = HDCP14_SIZE * 2; i < u16KeySize; i++)//304
			{
				tmp.Format("%02X ", sRead[i] & 0xFF);
				//m_strRXData += tmp;
				strHdcp14_3 += tmp;
			}
		}
		//strHdcp14_3 += tmp;
		strHdcp14_3 += "00 00";
		strHdcp14_3 = clrCRC(strHdcp14_3);
		//SendData(strHdcp14_2);
		//Sleep(100);
		m_strRXData += "\r\n烧HDCP14_3: ";
		m_strRXData += strHdcp14_3;

#if 1
		if (u16KeySize > HDCP14_SIZE * 3)
		{
			strHdcp14_4 = "07 51 01 FE 00 ";// 3A E3 03 ";
			tmp.Format("%02X ", (U8)((u16KeySize - HDCP14_SIZE * 3) + 10));
			strHdcp14_4 += tmp;
			strHdcp14_4 += "E3 04 ";
			for (int i = HDCP14_SIZE * 3; i < u16KeySize; i++)//304
			{
				tmp.Format("%02X ", sRead[i] & 0xFF);
				//m_strRXData += tmp;
				strHdcp14_4 += tmp;
			}
			strHdcp14_4 += "00 00";
			strHdcp14_4 = clrCRC(strHdcp14_4);
			//SendData(strHdcp14_2);
			//Sleep(100);
			m_strRXData += "\r\n烧HDCP14_4: ";
			m_strRXData += strHdcp14_4;
		}
#endif
		m_burnHDCP14 = TRUE;
	}
	
#endif
	m_strRXData += "\r\n";
	if (m_HDCP22_path.Find(".bin") != -1)
	{
		cFlie.Open(m_HDCP22_path, CFile::modeRead);
		cFlie.Read(m_sReadHdcp22, HDCP22_TOTAL_SIZE);
		//memcpy(sReadHdcp22, sRead, 1044);
		cFlie.Close();
		m_burnHDCP22 = TRUE;
	}
	if (m_burnHDCP14 || m_burnHDCP22)
	{
		SetTimer(TIME_BURN_HDCP, 300, NULL);
		if (m_burnHDCP14)
		{
			if (!m_bAutoFacMode)
			{
				m_getAck.Empty();
				SendData(strHdcp14_0);
				m_step = 1;
				m_iCheckAckCount = 0;
			}
			else
			{
				m_step = 0;
			}
		}
		else
			m_step = 30;
		bShowHex = TRUE;
		m_getAck.Empty();
	}
}

void CnewSerialDlg::OnBnClickedButtonLoadHdcp14()
{
	// TODO: 在此添加控件通知处理程序代码
	//CFile cFlie;
	//char sRead[1044];
	//CString sn, strmac, uid, tmp;
	//CString m_strFileOut = _T("");  //初始化适应Unicode
	TCHAR szFilter[] = _T("");
	CFileDialog fileDlg(TRUE, NULL, NULL, 0, szFilter, this);

	// 显示打开文件对话框   s
	if (IDOK == fileDlg.DoModal())
	{
		// 如果点击了文件对话框上的“打开”按钮，则将选择的文件路径显示到编辑框里   
		m_HDCP14_path = fileDlg.GetPathName();
	}
	//cFlie.Open(m_strFileOut, CFile::modeRead | CFile::typeBinary);
	//cFlie.Read(sRead, 512);//32
	//cFlie.Close();

	m_strRXData += "\r\n";
	m_strRXData += m_HDCP14_path;
	m_strRXData += "\r\n";
	GetDlgItem(IDC_EDIT_RX)->SetWindowText(m_strRXData);
}


void CnewSerialDlg::OnBnClickedButtonLoadHdcp22()
{
	// TODO: 在此添加控件通知处理程序代码
	TCHAR szFilter[] = _T("");
	CFileDialog fileDlg(TRUE, NULL, NULL, 0, szFilter, this);

	// 显示打开文件对话框   s
	if (IDOK == fileDlg.DoModal())
	{
		// 如果点击了文件对话框上的“打开”按钮，则将选择的文件路径显示到编辑框里   
		m_HDCP22_path = fileDlg.GetPathName();
	}
	
	m_strRXData += "\r\n";
	m_strRXData += m_HDCP22_path;
	m_strRXData += "\r\n";
	GetDlgItem(IDC_EDIT_RX)->SetWindowText(m_strRXData);
}


void CnewSerialDlg::OnBnClickedButtonBurnHdcp()
{
	// TODO: 在此添加控件通知处理程序代码
	burnHDCP();
}


void CnewSerialDlg::OnBnClickedButtonSend()
{
	// TODO: 在此添加控件通知处理程序代码
	// TODO: 在此添加控件通知处理程序代码
	CString str1;
	//sendString.Empty();
	//没有对\r\n特殊处理
	UpdateData(true); //读取编辑框内容 
	::WritePrivateProfileString(_T("SEND"), _T("send1"), m_EditSend, m_strConfigFile);
	if (m_bSendHex)//发送16进制数据
	{
		CByteArray HexDataBuf;
		int i = 0;
		BYTE SendBuf[1024] = { 0 };
		BYTE GetData[1024] = { 0 };
		int SendLen = 0;
		int GetLen = 0;

		//UpdateData(TRUE);//获取编辑框内容
		if (m_EditSend.IsEmpty())
		{
			AfxMessageBox(_T("发送数据为空!"));
			return;
		}
		HexDataBuf.RemoveAll();    //清空数组 

		GetLen = m_EditSend.GetLength();
#if 0
		if (m_EditSend.Find(' ') == -1)
		{
			int j = 0;
			for (i = 0; i < GetLen; i++)
			{
				if ((i % 2 == 0) && (i > 1))
				{
					GetData[j] = ' ';
					j++;
					GetData[j] = (BYTE)m_EditSend.GetBuffer()[i];
				}
				else
					GetData[j] = (BYTE)m_EditSend.GetBuffer()[i];
				j++;
			}
			GetLen = j;
		}
		else
#endif
		{
			for (i = 0; i < GetLen; i++)
			{
				GetData[i] = (BYTE)m_EditSend.GetBuffer()[i];
			}
		}
		StringtoHex(GetData, GetLen, SendBuf, &SendLen);//将字符串转化为字节数据
		/*const int limitNum = 20;
		if (SendLen > limitNum)
		{
			HexDataBuf.SetSize(limitNum);
			//while (i < SendLen)
			{
				int j;
				for (j = 0; j < (SendLen / limitNum); j++)
				{
					for (i = 0; i < limitNum; i++)
					{
						HexDataBuf.SetAt(i, SendBuf[j * limitNum + i]);
					}
					if (m_bIsComOpen)
						m_ctrlComm.put_Output(COleVariant(HexDataBuf));
				}
			}
		}
		else*/
		{
			if (m_bAutoEnter)
				HexDataBuf.SetSize(SendLen + 2);   //设置数组大小为帧长度 
			else
				HexDataBuf.SetSize(SendLen);   //设置数组大小为帧长度 

			for (i = 0; i < SendLen; i++)
			{
				HexDataBuf.SetAt(i, SendBuf[i]);
			}
			if (m_bAutoEnter)
			{
				HexDataBuf.SetAt(SendLen, 0x0D);
				HexDataBuf.SetAt(SendLen + 1, 0x0A);
			}
			if (m_bIsComOpen)
				m_ctrlComm.put_Output(COleVariant(HexDataBuf)); //发送十六进制数据
		}
		//UpdateData(false); //更新编辑框内容
	}
	else
	{
		//UpdateData(true); //读取编辑框内容 
		//((CEdit*)GetDlgItem(IDC_EDIT_TX))->GetWindowText(str1);
		if (m_bAutoEnter || m_bEnterKey)
			m_EditSend += "\r\n";
		if (m_bIsComOpen)
			m_ctrlComm.put_Output(COleVariant(m_EditSend));
	}
}


void CnewSerialDlg::OnBnClickedButton10()
{
	// TODO: 在此添加控件通知处理程序代码
	//if (m_bSendHex)//发送16进制数据
	{
		//CByteArray HexDataBuf;
		int i = 0;
		BYTE SendBuf[1024] = { 0 };
		BYTE GetData[1024] = { 0 };
		int SendLen = 0;
		int GetLen = 0;

		UpdateData(TRUE);//获取编辑框内容
		if (m_EditSend.IsEmpty())
		{
			AfxMessageBox(_T("发送数据为空!"));
			return;
		}
		//HexDataBuf.RemoveAll();    //清空数组 

		GetLen = m_EditSend.GetLength();
		for (i = 0; i < GetLen; i++)
		{
			GetData[i] = (BYTE)m_EditSend.GetBuffer()[i];
		}
		StringtoHex(GetData, GetLen, SendBuf, &SendLen);//将字符串转化为字节数据

		U16 checkSum = calculateCRC_ROKU(SendBuf, SendLen - 2);
		CString tmp;
		m_EditSend.Empty();
		for (i = 0; i < SendLen - 2; i++)
		{
			tmp.Format("%02X ", SendBuf[i]);
			m_EditSend += tmp;
		}
		tmp.Format("%02X ", (U8)(checkSum >> 8));
		m_EditSend += tmp;
		tmp.Format("%02X", (U8)checkSum);
		m_EditSend += tmp;
		UpdateData(FALSE);
		//if (m_bIsComOpen)
			//m_ctrlComm.put_Output(COleVariant(HexDataBuf)); //发送十六进制数据

		//UpdateData(false); //更新编辑框内容
	}
}


void CnewSerialDlg::OnBnClickedButtonReadSn()
{
	// TODO: 在此添加控件通知处理程序代码
	CString tmp;
	tmp = "07 51 01 08 1B 01 69 8F";
	m_getAck.Empty();
	SendData(tmp);
	SetTimer(TIME_GET_SN, 100, NULL);
	m_step = 0;
	bShowHex = FALSE;
	m_iCheckAckCount = 0;
}


void CnewSerialDlg::OnBnClickedButton12()
{
	// TODO: 在此添加控件通知处理程序代码
	CFile cFlie;
	char sRead[8044];
	CString sn, strmac, uid, tmp;
	CString m_strFileOut = _T("");  //初始化适应Unicode
	TCHAR szFilter[] = _T("");
	CFileDialog fileDlg(TRUE, NULL, NULL, 0, szFilter, this);

	// 显示打开文件对话框   s
	if (IDOK == fileDlg.DoModal())
	{
		// 如果点击了文件对话框上的“打开”按钮，则将选择的文件路径显示到编辑框里   
		m_strFileOut = fileDlg.GetPathName();
	}

	if ((m_strFileOut.Find(".txt") == -1) )
	{
		MessageBox("invalid key");
		return;
	}

	cFlie.Open(m_strFileOut, CFile::modeRead);
	//cFlie.Flush();
	int a = (int)cFlie.GetLength();
	cFlie.SeekToBegin();
	cFlie.Read(sRead, a);//32
	cFlie.Close();
	m_strRXData += "SN:\r\n";
	//m_strRXData += sRead;
	//int index = 0;
	//CString tmp;
	for (int i = 0; i < a; i++)
	{
		//tmp.Format("%c", sRead[i]);
		//m_load_SN_list.GetAt(index) += tmp;// sRead[i];
		if (sRead[i] == '\r')
		{
			if (i + 1 < a)
			{
				if (sRead[i + 1] == '\n')
				{
					m_load_SN_list.Add(tmp);
					//index++;		
					tmp.Empty();
					i += 1;
					continue;
				}
			}
			tmp.Empty();
			m_load_SN_list.Add(tmp);
			//index++;
		}
		else
			tmp += sRead[i];
	}
#if 0
	//m_strRXData += "\r\n";
	if (m_strFileOut.Find("ulpk-potk-00199D-0359-") != -1)//5586
	{
		strUlpkPotk = "07 51 01 AB D1 ";
		long int lUid = atol(uid);
		tmp.Format("%02X ", (U8)(lUid >> 24));
		strUlpkPotk += tmp;
		tmp.Format("%02X ", (U8)(lUid >> 16));
		strUlpkPotk += tmp;
		tmp.Format("%02X ", (U8)(lUid >> 8));
		strUlpkPotk += tmp;
		tmp.Format("%02X ", (U8)lUid);
		strUlpkPotk += tmp;
		for (int i = 0; i < 160; i++)//304
		{
			tmp.Format("%02X ", sRead[i] & 0xFF);
			//m_strRXData += tmp;
			strUlpkPotk += tmp;
		}
		strUlpkPotk += "00 00";
		strUlpkPotk = clrCRC(strUlpkPotk);
		//SendData(strUlpkPotk);
		//Sleep(100);
		m_strRXData += "\r\nBurn ULPK: ";
		m_strRXData += strUlpkPotk;
		//GetDlgItem(IDC_EDIT_RX)->SetWindowText(m_strRXData);
		//LoadSTEP_Path = m_strFileOut;
		//SetDlgItemText(IDC_EDIT2, m_strFileOut);//文本编辑框显示所选文件绝对路径
		m_strRXData += "\r\n";
	}
	else//5583//5691
	{
		strUlpkPotk = "07 51 01 3B D1 ";
		long int lUid = atol(uid);
		tmp.Format("%02X ", (U8)(lUid >> 24));
		strUlpkPotk += tmp;
		tmp.Format("%02X ", (U8)(lUid >> 16));
		strUlpkPotk += tmp;
		tmp.Format("%02X ", (U8)(lUid >> 8));
		strUlpkPotk += tmp;
		tmp.Format("%02X ", (U8)lUid);
		strUlpkPotk += tmp;
		for (int i = 0; i < 48; i++)//304
		{
			tmp.Format("%02X ", sRead[i] & 0xFF);
			//m_strRXData += tmp;
			strUlpkPotk += tmp;
		}
		strUlpkPotk += "00 00";
		strUlpkPotk = clrCRC(strUlpkPotk);
		//SendData(strUlpkPotk);
		//Sleep(100);
		m_strRXData += "\r\nBurn ULPK: ";
		m_strRXData += strUlpkPotk;
		//GetDlgItem(IDC_EDIT_RX)->SetWindowText(m_strRXData);
		//LoadSTEP_Path = m_strFileOut;
		//SetDlgItemText(IDC_EDIT2, m_strFileOut);//文本编辑框显示所选文件绝对路径

		strPotk = "07 51 01 1B D2 ";
		//lUid = atol(uid);
		tmp.Format("%02X ", (U8)(lUid >> 24));
		strPotk += tmp;
		tmp.Format("%02X ", (U8)(lUid >> 16));
		strPotk += tmp;
		tmp.Format("%02X ", (U8)(lUid >> 8));
		strPotk += tmp;
		tmp.Format("%02X ", (U8)lUid);
		strPotk += tmp;
		for (int i = 32; i < 48; i++)//304
		{
			tmp.Format("%02X ", sRead[i] & 0xFF);
			//m_strRXData += tmp;
			strPotk += tmp;
		}
		strPotk += "00 00";
		strPotk = clrCRC(strPotk);
		m_strRXData += "\r\nBurn POTK: ";
		m_strRXData += strPotk;
		m_strRXData += "\r\n";
	}
#endif
	tmp.Format("SN数量=%d\r\n", m_load_SN_list.GetSize());
	m_strRXData += tmp;
	m_strRXData += "\r\n-----------\r\n";
	for (int j = 0; j < m_load_SN_list.GetSize(); j++)
	{
		m_strRXData += m_load_SN_list.GetAt(j);
		m_strRXData += "\r\n";
	}
	m_strRXData += "\r\n-----------\r\n";
	GetDlgItem(IDC_EDIT_RX)->SetWindowText(m_strRXData);
	//GetDlgItem(IDC_BUTTON_SET_SN2)->EnableWindow(TRUE);
#if 0
	m_SN_index = 0;
	tmp.Format("%d", m_SN_index);
	::WritePrivateProfileString(_T("SN"), _T("SnIndex"), tmp, m_strConfigFile);
#endif
	tmp.Format("%d", m_SN_index);
	GetDlgItem(IDC_EDIT_SN)->SetWindowText(m_load_SN_list.GetAt(m_SN_index));
	GetDlgItem(IDC_EDIT_SN2)->SetWindowText(m_load_SN_list.GetAt(m_SN_index));
	GetDlgItem(IDC_EDIT_SN_INDEX)->SetWindowText(tmp);
	
}


void CnewSerialDlg::OnBnClickedButtonSetSn2()
{
	// TODO: 在此添加控件通知处理程序代码
	CString tmp;
	if (m_bIsComOpen)
	{
		CString sn, strmac, uid, strSn, strShowmac, strShowuid;
		CString tmp, strSendData;
		U16 checkSum;
		if (m_bAutoFacMode)
		{
			tmp = "07 51 01 07 01 CB D1";
			SendData(tmp);
			Sleep(100);
		}
		m_strSn = "07 51 01 16 88 ";

		for (int i = 0; i < m_load_SN_list.GetAt(m_SN_index).GetLength(); i++)
		{
			tmp.Format("%02X ", m_load_SN_list.GetAt(m_SN_index).GetAt(i));
			m_strSn += tmp;
		}

		//m_strRXData += strSn;
		//m_strRXData += ", ";
		//m_strRXData += ", ";

		m_strSn += "00 00";
		m_strSn = clrCRC(m_strSn);
		m_strRXData += m_strSn;
		m_strRXData += "\r\n";
		GetDlgItem(IDC_EDIT_RX)->SetWindowText(m_strRXData);
		//GetDlgItem(IDC_STATIC_SHOW_SN)->SetWindowText(m_load_SN_list.GetAt(m_SN_index));
		m_strEdit_GetSN = m_load_SN_list.GetAt(m_SN_index);
		SendData(m_strSn);
		UpdateData(FALSE);
	}
	//Sleep(500);
	//OnBnClickedButtonReadSn();
	
	
}


void CnewSerialDlg::OnBnClickedButton11()
{
	// TODO: 在此添加控件通知处理程序代码
	CString tmp;
	tmp = "07 51 01 07 FA 95 A5";
	SendData(tmp);
	m_getAck.Empty();
	SetTimer(TIME_GET_RESET_ACK, 500, NULL);
	m_step = 0;
	bShowHex = TRUE;
	m_iCheckAckCount = 0;
}


void CnewSerialDlg::OnEnChangeEditSnIndex()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	UpdateData(TRUE);
	//m_SN_index = 0;
	//tmp.Format("%d", m_SN_index);
	m_strRXData += m_strSnIndex;
	::WritePrivateProfileString(_T("SN"), _T("SnIndex"), m_strSnIndex, m_strConfigFile);
}


void CnewSerialDlg::OnBnClickedButtonTest1()
{
	// TODO: 在此添加控件通知处理程序代码
	CString str1;
	UpdateData(true); //读取编辑框内容 
	m_bSendHex = TRUE;
	if (m_bSendHex)//发送16进制数据
	{
		CByteArray HexDataBuf;
		int i = 0;
		BYTE SendBuf[1024] = { 0 };
		BYTE GetData[1024] = { 0 };
		int SendLen = 0;
		int GetLen = 0;

		//UpdateData(TRUE);//获取编辑框内容
		if (m_EditSend.IsEmpty())
		{
			AfxMessageBox(_T("发送数据为空!"));
			return;
		}
		HexDataBuf.RemoveAll();    //清空数组 

		GetLen = m_EditSend.GetLength();

		{
			for (i = 0; i < GetLen; i++)
			{
				GetData[i] = (BYTE)m_EditSend.GetBuffer()[i];
			}
		}
		StringtoHex(GetData, GetLen, SendBuf, &SendLen);//将字符串转化为字节数据
		str1.Format("%s", SendBuf);
		GetDlgItem(IDC_EDIT_RX)->SetWindowText(str1);
	}
	else
	{
		//UpdateData(true); //读取编辑框内容 
		//((CEdit*)GetDlgItem(IDC_EDIT_TX))->GetWindowText(str1);
		if (m_bAutoEnter || m_bEnterKey)
			m_EditSend += "\r\n";
	}
}


void CnewSerialDlg::OnBnClickedCheckAutoFacMode()
{
	// TODO: 在此添加控件通知处理程序代码
	m_bAutoFacMode = !m_bAutoFacMode;
}
