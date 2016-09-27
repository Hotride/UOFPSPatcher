//---------------------------------------------------------------------------

#ifndef MainScriptH
#define MainScriptH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <vector.h>
//---------------------------------------------------------------------------
//incremented ordinary for
#define IFOR(var, value, target) for (int (##var) = value; ##var < target; ##var ++)
//decremented ordinary for
#define DFOR(var, value, target) for (int (##var) = value; ##var >= target; ##var --)
//---------------------------------------------------------------------------
#define FPSPATCH_INSTALL				WM_USER + 666
#define FPSPATCH_INFO_REQUEST			FPSPATCH_INSTALL + 1
#define FPSPATCH_INFO_ANSWER			FPSPATCH_INFO_REQUEST + 1
#define FPSPATCH_ENABLE					FPSPATCH_INFO_ANSWER + 1
#define FPSPATCH_DISABLE				FPSPATCH_ENABLE + 1
//---------------------------------------------------------------------------
class TFPSPatcher : public TForm
{
__published:	// IDE-managed Components
	TButton *bt_patch;
	TButton *bt_refresh;
	TLabel *l_status;
	TLabel *Label1;
	TLabel *Label2;
	TListBox *lb_client_list;
	void __fastcall bt_patchClick(TObject *Sender);
	void __fastcall lb_client_listClick(TObject *Sender);
	void __fastcall bt_refreshClick(TObject *Sender);
	void __fastcall FormCreate(TObject *Sender);
public:		// User declarations
	__fastcall TFPSPatcher(TComponent* Owner);

	std::vector<HWND> m_client_list;

	void __fastcall OnMessagePathInfo(TMessage &Message);
#pragma warn -inl
BEGIN_MESSAGE_MAP
	MESSAGE_HANDLER(FPSPATCH_INFO_ANSWER, TMessage, OnMessagePathInfo)
END_MESSAGE_MAP(TForm)
};
//---------------------------------------------------------------------------
extern PACKAGE TFPSPatcher *FPSPatcher;
//---------------------------------------------------------------------------
#endif