//---------------------------------------------------------------------------
#include <vcl.h>
#include <psapi.h>
#include <stdio.h>
#pragma hdrstop

#include "MainScript.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TFPSPatcher *FPSPatcher;
//---------------------------------------------------------------------------
void __fastcall TFPSPatcher::OnMessagePathInfo(TMessage &M)
{
	HWND hwnd = 0;

	if (lb_client_list->ItemIndex != -1)
		hwnd = m_client_list[lb_client_list->ItemIndex];

	if ((HWND)M.LParam == hwnd)
	{
		if (M.WParam)
		{
			l_status->Font->Color = clGreen;
			l_status->Caption = "Patched";
			bt_patch->Caption = "Unset patch";
		}
		else
		{
			l_status->Font->Color = clRed;
			l_status->Caption = "NOT Patched";
			bt_patch->Caption = "Set patch";
		}
	}
	else
		M.Result=DefWindowProc(Handle,M.Msg,M.WParam,M.LParam);
}
//---------------------------------------------------------------------------
bool __stdcall EnumProc(HWND hWnd,long)
{
	PDWORD pPid;   //LPDWORD
	DWORD result;  //DWORD
	PVOID hg;      //HGLOBAL

	if (!hWnd)
		return false;

	hg = GlobalAlloc(GMEM_SHARE, sizeof(DWORD));
	pPid = (PDWORD)GlobalLock(hg);
	result = GetWindowThreadProcessId(hWnd, pPid);

	if (result)
	{
		char title[150] = {0};
		char className[30] = {0};

		GetClassName(hWnd, className, 30);
		GetWindowText(hWnd, title, 150);

		if (strstr(className, "Ultima Online"))
		{
			FPSPatcher->lb_client_list->Items->Add(title);
			FPSPatcher->m_client_list.push_back(hWnd);
		}
	}
	else
	{
		GlobalUnlock(hg);
		GlobalFree(hg);

		return false;
	}

	GlobalUnlock(hg);
	GlobalFree(hg);

	return true;
}
//---------------------------------------------------------------------------
__fastcall TFPSPatcher::TFPSPatcher(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
BOOL SetPrivilege(HANDLE hToken,LPCTSTR lpszPrivilege,BOOL bEnablePrivilege)
{
	TOKEN_PRIVILEGES tp;
	LUID luid;

	if (!LookupPrivilegeValue(NULL, lpszPrivilege, &luid))
		return FALSE;

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;

	if (bEnablePrivilege)
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	else
		tp.Privileges[0].Attributes = 0;

	if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES)NULL, (PDWORD)NULL))
		return FALSE;

	return TRUE;
}
//---------------------------------------------------------------------------
BOOL EnableDebugPrivilages()
{
	HANDLE hToken;

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
		return FALSE;

	return SetPrivilege(hToken, SE_DEBUG_NAME, TRUE);
}
//---------------------------------------------------------------------------
void __fastcall TFPSPatcher::bt_refreshClick(TObject *Sender)
{
	l_status->Font->Color = clWindowText;
	l_status->Caption = "Unknown";
	lb_client_list->Items->Clear();
	m_client_list.clear();

	long lp = 0;
	EnumWindows((WNDENUMPROC)EnumProc, lp);
}
//---------------------------------------------------------------------------
void __fastcall TFPSPatcher::FormCreate(TObject *Sender)
{
	EnableDebugPrivilages();
	bt_refreshClick(bt_refresh);
}
//---------------------------------------------------------------------------
void __fastcall TFPSPatcher::lb_client_listClick(TObject *Sender)
{
	if (lb_client_list->ItemIndex == -1)
	{
		l_status->Font->Color = clWindowText;
		l_status->Caption = "Unknown";

		return;
	}

	HWND hWnd = m_client_list[lb_client_list->ItemIndex];

	HWND dllWindow = FindWindow(("FPSModWindow_" + IntToHex((int)hWnd, 8)).t_str(), NULL);

	if (dllWindow == NULL)
	{
		DWORD processID = 0;
		GetWindowThreadProcessId(hWnd, &processID);
		HANDLE hp = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, TRUE, processID);

		if (hp != NULL)
		{
			String path = ExtractFilePath(Application->ExeName) + "\\FPSMod.dll";

			HANDLE hProcess = NULL, hThread = NULL;
			PWSTR pszLibFileRemote = NULL;

			__try
			{
				// Get a handle for the target process.
				hProcess = OpenProcess(
					PROCESS_QUERY_INFORMATION |   // Required by Alpha
					PROCESS_CREATE_THREAD     |   // For CreateRemoteThread
					PROCESS_VM_OPERATION      |   // For VirtualAllocEx/VirtualFreeEx
					PROCESS_VM_WRITE,             // For WriteProcessMemory
					FALSE, processID);

				if (hProcess == NULL)
				{
					MessageBox(Handle, "Open process failed.", "Error", MB_OK);
					return;
				}

				// Calculate the number of bytes needed for the DLL's pathname
				int cch = 1 + lstrlenW(path.c_str());
				int cb  = cch * sizeof(WCHAR);

				// Allocate space in the remote process for the pathname
				pszLibFileRemote=(PWSTR)VirtualAllocEx(hProcess, NULL, cb, MEM_COMMIT, PAGE_READWRITE);

				if (pszLibFileRemote == NULL)
				{
					if (hProcess!=NULL)
						CloseHandle(hProcess);

					MessageBox(Handle, "Memory can't be selected.", "Error", MB_OK);

					return;
				}

				// Copy the DLL's pathname to the remote process's address space
				if (!WriteProcessMemory(hProcess, pszLibFileRemote, (PVOID) path.c_str(), cb, NULL))
				{
					if (pszLibFileRemote != NULL)
						VirtualFreeEx(hProcess, pszLibFileRemote, 0, MEM_RELEASE);

					if (hProcess != NULL)
						CloseHandle(hProcess);

					MessageBox(Handle, "Failed to write string in to the memory.", "Error", MB_OK);

					return;
				}

				// Get the real address of LoadLibraryW in Kernel32.dll
				PTHREAD_START_ROUTINE pfnThreadRtn = (PTHREAD_START_ROUTINE)
				GetProcAddress(GetModuleHandle(TEXT("Kernel32")), "LoadLibraryW");

				if (pfnThreadRtn == NULL)
				{
					if (pszLibFileRemote != NULL)
						VirtualFreeEx(hProcess, pszLibFileRemote, 0, MEM_RELEASE);
					if (hProcess != NULL)
						CloseHandle(hProcess);

					MessageBox(Handle, "LoadLibraryW not found in Kernel32.dll", "Error", MB_OK);

					return;
				}

				bool created = false;

				/*HMODULE modNtDll = GetModuleHandle("ntdll.dll");

				if (modNtDll != NULL)
				{
					typedef NTSTATUS (WINAPI *LPFUN_NtCreateThreadEx)
					(
					  OUT PHANDLE hThread,
					  IN ACCESS_MASK DesiredAccess,
					  IN LPVOID ObjectAttributes,
					  IN HANDLE ProcessHandle,
					  IN LPTHREAD_START_ROUTINE lpStartAddress,
					  IN LPVOID lpParameter,
					  IN BOOL CreateSuspended,
					  IN ULONG StackZeroBits,
					  IN ULONG SizeOfStackCommit,
					  IN ULONG SizeOfStackReserve,
					  OUT LPVOID lpBytesBuffer
					);

					LPFUN_NtCreateThreadEx funNtCreateThreadEx = (LPFUN_NtCreateThreadEx)GetProcAddress(modNtDll, "NtCreateThreadEx");

					ShowMessage("funNtCreateThreadEx=0x"+IntToHex((int)funNtCreateThreadEx, 8)+"; 0x"+IntToHex((int)GetProcAddress(modNtDll, "NtCreateThread"),8 ));

					if (funNtCreateThreadEx != NULL)
					{
						typedef struct NtCreateThreadExBuffer
						{
							ULONG Size;
							ULONG Unknown1;
							ULONG Unknown2;
							PULONG Unknown3;
							ULONG Unknown4;
							ULONG Unknown5;
							ULONG Unknown6;
							PULONG Unknown7;
							ULONG Unknown8;
						} NtCreateThreadExBuffer;

						NtCreateThreadExBuffer ntbuffer;

						memset (&ntbuffer,0,sizeof(NtCreateThreadExBuffer));
						DWORD temp1 = 0;
						DWORD temp2 = 0;

						ntbuffer.Size = sizeof(NtCreateThreadExBuffer);
						ntbuffer.Unknown1 = 0x10003;
						ntbuffer.Unknown2 = 0x8;
						ntbuffer.Unknown3 = &temp2;
						ntbuffer.Unknown4 = 0;
						ntbuffer.Unknown5 = 0x10004;
						ntbuffer.Unknown6 = 4;
						ntbuffer.Unknown7 = &temp1;
						ntbuffer.Unknown8 = 0;

						NTSTATUS status = funNtCreateThreadEx(
															&hThread,
															0x1FFFFF,
															NULL,
															hProcess,
															(LPTHREAD_START_ROUTINE)pfnThreadRtn,
															pszLibFileRemote,
															FALSE, //start instantly
															NULL,
															NULL,
															NULL,
															&ntbuffer
															);

						created = (hThread != NULL);

						ShowMessage("Inj from ntdll: " + IntToStr((int)created));
					}
				}*/

				if (!created) // Create a remote thread that calls LoadLibraryW(DLLPathname)
					hThread = CreateRemoteThread(hProcess, NULL, 0, pfnThreadRtn, pszLibFileRemote, 0, NULL);

				if (hThread == NULL)
				{
					if (pszLibFileRemote != NULL)
						VirtualFreeEx(hProcess, pszLibFileRemote, 0, MEM_RELEASE);
					if (hProcess != NULL)
						CloseHandle(hProcess);

					ShowMessage("Thrd error: " + IntToStr((int)GetLastError()));
					MessageBox(Handle, "Remote thread can't be created.", "Error", MB_OK);

					return;
				}

				// Wait for the remote thread to terminate
				WaitForSingleObject(hThread, INFINITE);
			}
			__finally
			{ // Now, we can clean everthing up

				// Free the remote memory that contained the DLL's pathname
				if (pszLibFileRemote != NULL)
					VirtualFreeEx(hProcess, pszLibFileRemote, 0, MEM_RELEASE);

				if (hThread != NULL)
					CloseHandle(hThread);

				if (hProcess != NULL)
					CloseHandle(hProcess);
			}

			CloseHandle(hp);
		}
		else
		{
			bt_refreshClick(bt_refresh);
			//MessageBox(Handle, "UO process not found!", "Error", MB_OK);
			return;
		}

		Sleep(500);
		dllWindow = FindWindow(("FPSModWindow_" + IntToHex((int)hWnd, 8)).t_str(), NULL);
	}

	l_status->Font->Color = clPurple;
	l_status->Caption = "Unknown patch";
	bt_patch->Caption = "Set patch";

	if (dllWindow != 0)
		SendMessage(dllWindow, FPSPATCH_INFO_REQUEST, (DWORD)Handle, 0);
}
//---------------------------------------------------------------------------
void __fastcall TFPSPatcher::bt_patchClick(TObject *Sender)
{
	if (lb_client_list->ItemIndex == -1)
	{
		ShowMessage("No selected client.");
		return;
	}

	HWND hWnd = m_client_list[lb_client_list->ItemIndex];
	HWND dllWindow = FindWindow(("FPSModWindow_" + IntToHex((int)hWnd, 8)).t_str(), NULL);

	if (dllWindow == 0)
	{
		ShowMessage("FPS patch window is not found!");
		return;
	}

	if (bt_patch->Caption == "Set patch")
		SendMessage(dllWindow, FPSPATCH_ENABLE, (DWORD)Handle, 0);
	else
		SendMessage(dllWindow, FPSPATCH_DISABLE, (DWORD)Handle, 0);
}
//---------------------------------------------------------------------------
