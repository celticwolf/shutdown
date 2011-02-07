// shutdown.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <time.h>
#include <string>
#include <sstream>
#include <vector>

/*******************************************************************************
** Types
*******************************************************************************/

enum ActionType
{
	ActionShutdown,
	ActionReboot,
	ActionLogoff,
	ActionAbort
};

typedef struct argsstruct
{
	ActionType action;
	bool force;
	DWORD reason;
	_TCHAR computername[63 * sizeof(_TCHAR)];
	_TCHAR message[128 * sizeof(_TCHAR)];

} argstype;

typedef std::basic_string<_TCHAR> tstring;
typedef std::basic_stringstream<_TCHAR> tstringstream;

/*******************************************************************************
** Prototypes
*******************************************************************************/

void Syntax();
bool ParseArgs(int argc, _TCHAR* argv[], argstype *pargs);
DWORD ParseReason(_TCHAR *pszReason);
time_t ParseTime(_TCHAR *pszTime);
std::vector<tstring> &split(const tstring &s, _TCHAR delim);

/*******************************************************************************
** Functions
*******************************************************************************/
int _tmain(int argc, _TCHAR* argv[])
{
	if (argc < 2)
	{
		Syntax();
		return 1;
	}

	argstype args;
	if (false == ParseArgs(argc, argv, &args))
		return 1;

	return 0;
}

void Syntax()
{
	_tprintf(_T("shutdown.exe version 1.0 - Copyright 2011 Celtic Wolf, Inc.\n\n"));
	_tprintf(_T("usage: shutdown [options] [time] [message]\n\n"));
	_tprintf(_T("-l                      Log off (cannot be used with -m option)\n"));
//	_tprintf(_T("-s                      Shutdown the computer\n"));
	_tprintf(_T("-r                      Shutdown and restart the computer\n"));
	_tprintf(_T("-a                      Abort a system shutdown\n"));
	_tprintf(_T("-m \\\\computername       Remote computer to shutdown/restart/abort\n"));
//	_tprintf(_T("-c \"comment\"            Shutdown comment (maximum of 127 characters)\n"));
	_tprintf(_T("-f                      Forces running applications to close without warning\n"));
	_tprintf(_T("-d [u|p]:xx:yy         The reason code for the shutdown\n"));
	_tprintf(_T("                        u is the user code\n"));
	_tprintf(_T("                        p is a planned shutdown code\n"));
	_tprintf(_T("                        xx is the major reason code (positive integer less than 256)\n"));
	_tprintf(_T("                        yy is the minor reason code (positive integer less than 65536)\n"));
}

bool ParseArgs(int argc, _TCHAR* argv[], argstype *pargs)
{
	bool logoff = false;
	bool reboot = false;
	bool abort = false;
	bool arg_error = false;

	for (int i = 1; i < argc; ++i)
	{
		if (0 == _tcscmp(argv[i], _T("-l")))
		{
			pargs->action = ActionLogoff;
			logoff = true;
		}
		else if (0 == _tcscmp(argv[i], _T("-r")))
		{
			pargs->action = ActionReboot;
			reboot = true;
		}
		else if (0 == _tcscmp(argv[i], _T("-a")))
		{
			pargs->action = ActionAbort;
			abort = true;
		}
		else if (0 == _tcscmp(argv[i], _T("-m")))
		{
			if (i + 1 < argc)
				_tcscpy(pargs->computername, argv[++i]);
			else
			{
				_tprintf(_T("if you specify -m, you must specify a computer name"));
				arg_error = true;
			}
		}
		else if (0 == _tcscmp(argv[i], _T("-f")))
			pargs->force = true;
		else if (0 == _tcscmp(argv[i], _T("-d")))
			pargs->reason = ParseReason(argv[i]); // figure out codes and update
		else
		{
			time_t shutdown_time = ParseTime(argv[i]);
		}

		// Check for arg_erroring arguments
		if (logoff && reboot)
		{
			_tprintf(_T("you cannot specify both -l and -r\n"));
			arg_error = true;
		}

		if (logoff && abort)
		{
			_tprintf(_T("you cannot specify both -l and -a\n"));
			arg_error = true;
		}

		if (reboot && abort)
		{
			_tprintf(_T("you cannot specify both -r and -a\n"));
			arg_error = true;
		}
	}

	return ! arg_error;
}

DWORD ParseReason(_TCHAR *pszReason)
{
	DWORD ret = 0;
	std::vector<tstring> elems = split(tstring(pszReason), _T(':'));
	for (std::vector<tstring>::iterator it = elems.begin(); it != elems.end(); ++it)
		ret &= _ttoi((*it).c_str());
	return ret;
}

time_t ParseTime(_TCHAR *pszTime)
{
	return time(NULL);
}

std::vector<tstring> &split(const tstring &s, _TCHAR delim)
{
	std::vector<tstring> elems;
    tstringstream ss(s);
    tstring item;
    while(std::getline(ss, item, delim))
        elems.push_back(item);
    return elems;
}
