// shutdown.cpp : Defines the entry point for the console application.
// Copyright 2011 Celtic Wolf, Inc.  All rights reserved.
//
// This software is licensed under the terms of the GNU GPL version 2.  See
// http://www.gnu.org/licenses/gpl-2.0.html for a copy of the license.

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
	DWORD timeout;
	_TCHAR computername[63 * sizeof(_TCHAR)];
	_TCHAR message[128 * sizeof(_TCHAR)];
	bool verbose;

} argstype;

typedef std::basic_string<_TCHAR> tstring;
typedef std::basic_stringstream<_TCHAR> tstringstream;

/*******************************************************************************
** Prototypes
*******************************************************************************/

bool AbortShutdown(argstype &args);
bool CheckForConflictingArgs(bool logoff, bool reboot, bool abort, bool other_pc);
void CheckGetLastError(BOOL res, LPTSTR lpError, bool &ret);
bool EnsurePrivileges(argstype &args);
bool Logoff(argstype &args);
bool ParseArgs(int argc, _TCHAR* argv[], argstype *pargs);
DWORD ParseReason(const _TCHAR* const pszReason);
time_t ParseTime(const _TCHAR* const pszTime);
void split(const tstring &s, _TCHAR delim, std::vector<tstring> &elems);
bool Shutdown(argstype &args);
void Syntax();

/*******************************************************************************
** Functions
*******************************************************************************/


/*******************************************************************************
** _tmain
**
** Standard entry point function.
**
** Arguments
**
**	int argc		- count of arguments
**	_TCHAR* argv[]	- array of arguments
**
** Returns: 0 for success, 1 for failure
*******************************************************************************/
int _tmain(int argc, _TCHAR* argv[])
{
	if (argc < 2)
	{
		Syntax();
		return 1;
	}

	argstype args;
	// Set defaults
	memset(&args, 0, sizeof(argstype));
	args.action = ActionShutdown;

	if (false == ParseArgs(argc, argv, &args))
		return 1;

	if (args.action == ActionShutdown || args.action == ActionReboot)
	{
		if (false == Shutdown(args))
			return 1;
	}
	else if (args.action == ActionLogoff)
	{
		if (false == Logoff(args))
			return 1;
	}
	else if (args.action == ActionAbort)
	{
		if (false == AbortShutdown(args))
			return 1;
	}

	return 0;
}

/*******************************************************************************
** AbortShutdown
**
** Aborts a shutdown in progress.  Calls EnsurePrivileges to make sure that the
** process has the required privileges enabled, then calls AbortSystemShutdown.
** If that call fails, it will print an error from GetLastError and return 
** failure.
**
** Arguments
**
**	argstype &args	- struct containing the arguments specified by the user
**
** Returns: true for success, false for failure
*******************************************************************************/
bool AbortShutdown(argstype &args)
{
	bool ret = true;

	if (true == args.verbose)
		_tprintf(_T("aborting the system shutdown ..."));

	if (false == EnsurePrivileges(args))
		return false;

	BOOL bRes = ::AbortSystemShutdown(args.computername);
	CheckGetLastError(bRes, _T("Error aborting shutdown"), ret);

	return ret;
}

/*******************************************************************************
** Logoff
**
** Logs the current user off the system, closing all applications, using
** ExitWindowsEx.  If that call fails, it will print an error from GetLastError
** and return failure.
**
** Arguments
**
**	argstype &args	- struct containing the arguments specified by the user
**
** Returns: true for success, false for failure
*******************************************************************************/
bool Logoff(argstype &args)
{
	bool ret = true;

	UINT uFlags = EWX_LOGOFF;
	if (args.force)
		uFlags |= EWX_FORCE;

	BOOL res = ::ExitWindowsEx(uFlags, args.reason);
	CheckGetLastError(res, _T("Error logging off"), ret);

	return ret;
}

/*******************************************************************************
** Shutdown
**
** Shuts down the system, optionally rebooting.  Calls EnsurePrivileges to
** make sure that the process has the required privileges enabled, then calls
** InitiateSystemShutdownEx.  If that call fails, it will print an error from
** GetLastError and return failure.
**
** Arguments
**
**	argstype &args	- struct containing the arguments specified by the user
**
** Returns: true for success, false for failure
*******************************************************************************/
bool Shutdown(argstype &args)
{
	bool ret = true;

	if (args.verbose)
	{
		_tprintf(_T("Shutting down computer with following arguments\n"));
		_tprintf(_T("computer name: %s\n"), args.computername);
		_tprintf(_T("message: %s\n"), args.message);
		_tprintf(_T("timeout: %d seconds\n"), args.timeout);
		_tprintf(_T("force: %d\n"), args.force);
		_tprintf(_T("reboot: %d\n"), args.action == ActionReboot);
		_tprintf(_T("reason: %X\n\n"), args.reason);
	}

	if (false == EnsurePrivileges(args))
		return false;

	BOOL bRes = ::InitiateSystemShutdownEx(
		args.computername, 
		args.message, 
		args.timeout, 
		args.force, 
		args.action == ActionReboot, 
		args.reason);

	CheckGetLastError(bRes, _T("error initiating system shutdown"), ret);

	return ret;
}

/*******************************************************************************
** Syntax
**
** Prints out the version and usage information, including a list of possible
** arguments.
*******************************************************************************/
void Syntax()
{
	_tprintf(_T("shutdown.exe version 1.0 - Copyright 2011 Celtic Wolf, Inc.\n\n"));
	_tprintf(_T("usage: shutdown [options] [time] [message]\n\n"));
	_tprintf(_T("-l                      Log off current session (cannot be used with -m option)\n"));
//	_tprintf(_T("-s                      Shutdown the computer\n"));
	_tprintf(_T("-r                      Reboot the computer\n"));
	_tprintf(_T("-a                      Abort a shutdown already in progress\n"));
	_tprintf(_T("-m \\\\computername       Remote computer to shutdown, reboot or abort\n"));
//	_tprintf(_T("-c \"comment\"            Shutdown comment (maximum of 127 characters)\n"));
	_tprintf(_T("-f                      Forces running applications to close without warning (may lose unsaved data)\n"));
	_tprintf(_T("-d [u|p]:MM:mm          The reason code for the shutdown (see System Shutdown Codes in Technet)\n"));
	_tprintf(_T("                        u indicates a user-defined code\n"));
	_tprintf(_T("                        p indicates a planned shutdown code\n"));
	_tprintf(_T("                        MM is the major reason code (positive integer less than 256)\n"));
	_tprintf(_T("                        mm is the minor reason code (positive integer less than 65536)\n"));
	_tprintf(_T("-V                      Verbose output\n"));
}

/*******************************************************************************
** ParseArgs
**
** Evaluates the arguments passed into the program and populates the supplied
** instance of argstype.  Also checks for invalid values or combinations of
** arguments and prints out errors.
**
** Arguments:
**
**	int argc		- count of arguments
**	_TCHAR* argv[]	- array of arguments
**	argstype *pargs	- receives information about the arguments
**
** Returns: true if arguments successfully parsed, false if error found in
** arguments, including conflicting arguments.
*******************************************************************************/
bool ParseArgs(int argc, _TCHAR* argv[], argstype *pargs)
{
	bool logoff = false;
	bool reboot = false;
	bool abort = false;
	bool other_pc = false;
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

			other_pc = true;
		}
		else if (0 == _tcscmp(argv[i], _T("-f")))
			pargs->force = true;
		else if (0 == _tcscmp(argv[i], _T("-d")))
			pargs->reason = ParseReason(argv[++i]); // figure out codes and update
		else if (0 == _tcscmp(argv[i], _T("-V")))
			pargs->verbose = true;
		else
		{
			pargs->timeout = ParseTime(argv[i]);
		}
	}

	if (CheckForConflictingArgs(logoff, reboot, abort, other_pc))
		arg_error = true;

	return ! arg_error;
}

/*******************************************************************************
** CheckForConflictingArgs
**
** Checks the program arguments to make sure that there are no logical conflicts
** in what was specified.
**
** Arguments
**
**	bool logoff		- whether the user specified the -l argument
**	bool reboot		- whether the user specified the -r argument
**	bool abort		- whether the user specified the -a argument
**	bool other_pc	- whether the user specified the -m argument
**
** Returns: true if there are conflicts, false if not
*******************************************************************************/
bool CheckForConflictingArgs(bool logoff, bool reboot, bool abort, bool other_pc)
{
	bool arg_error = false;

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

	if (logoff && other_pc)
	{
		_tprintf(_T("you can only log the user off of the local PC"));
		arg_error = true;
	}

	if (reboot && abort)
	{
		_tprintf(_T("you cannot specify both -r and -a\n"));
		arg_error = true;
	}

	return arg_error;
}

/*******************************************************************************
** CheckGetLastError
**
** If res is FALSE, calls GetLastError to get the error code, then calls
** FormatMessage and prints out both the numeric error code and the message.
**
** Arguments:
**
**	BOOL res	- return value that tells us whether the API function succeeded
**				  or failed.  If TRUE, GetLastError will not be called.
**	LPTSTR lpError	- A custom string that will be added to the error output
**	bool &ret	- sets to false if GetLastError was called
**
** Returns: void
*******************************************************************************/
void CheckGetLastError(BOOL res, LPTSTR lpError, bool &ret)
{
	if (FALSE == res)
	{
		TCHAR szBuf[1024];
		memset(szBuf, 0, 1024 * sizeof(TCHAR));
		DWORD err = ::GetLastError();
		DWORD dwRes = ::FormatMessage(
			0,
			NULL,
			err,
			0,
			szBuf,
			1024,
			NULL);
		_tprintf(_T("%s: %X - %s\n"), lpError, err, szBuf);
		ret = false;
	}
}

/*******************************************************************************
** EnsurePrivileges
**
** Enables the SE_SHUTDOWN_NAME (for local) or SE_REMOTE_SHUTDOWN_NAME (for 
** remote) privilege for the current process.
**
** Arguments:
**
**	argstype &args	- struct containing the arguments specified by the user
**
** Returns: true for success, false for failure.
*******************************************************************************/
bool EnsurePrivileges(argstype &args)
{
	bool ret = true;

	HANDLE hToken = INVALID_HANDLE_VALUE;
	TOKEN_PRIVILEGES tp;
	memset(&tp, 0, sizeof(TOKEN_PRIVILEGES));

	HANDLE hProc = INVALID_HANDLE_VALUE;
	hProc = ::GetCurrentProcess();
	BOOL bRes = ::OpenProcessToken(hProc, TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &hToken);
	CheckGetLastError(bRes, _T("Error opening process token"), ret);
 
	if (_tcslen(args.computername) > 0)
		bRes = ::LookupPrivilegeValue(NULL, SE_REMOTE_SHUTDOWN_NAME, &tp.Privileges[0].Luid);
	else
		bRes = ::LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tp.Privileges[0].Luid);
	CheckGetLastError(bRes, _T("error looking up privilege value"), ret);

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	bRes = ::AdjustTokenPrivileges(
		hToken,
		FALSE,
		&tp,
		sizeof(TOKEN_PRIVILEGES),
		(PTOKEN_PRIVILEGES) NULL,
		(PDWORD) NULL
	);
	CheckGetLastError(bRes, _T("error adjusting token privileges"), ret);

	return ret;
}

/*******************************************************************************
** ParseReason
**
** Converts the supplied reason information into a single DWORD value that can
** be passed to InitiateSystemShutdown.  If errors are encountered, prints out
** error messages and returns -1.
**
** Arguments:
**
**	const _TCHAR const *pszReason	- the string containing the reason information
**		passed to the program.  It should take the form of n:MM:mm where n is
**		either "u" or "p" for "user defined" or "planned", respectively, MM is
**		the major reason code and mm is the minor reason code.
**
** Returns: the reason code or -1 for failure.
*******************************************************************************/
DWORD ParseReason(const _TCHAR* const pszReason)
{
	DWORD ret = 0;
	std::vector<tstring> elems;
	split(tstring(pszReason), _T(':'), elems);
	if (elems.size() != 3)
	{
		_tprintf(_T("Incorrect number of reason codes supplied.  There should be three: [p|u]:major:minor.\n"));
		return -1UL;
	}

	// Get the user or planned code
	std::vector<tstring>::iterator it = elems.begin();
	if (0 == _tcscmp((*it).c_str(), _T("p")))
		ret |= SHTDN_REASON_FLAG_PLANNED;
	else if (0 == _tcscmp((*it).c_str(), _T("u")))
		ret |= SHTDN_REASON_FLAG_USER_DEFINED;
	else
	{
		_tprintf(_T("The first reason code must be either a \"u\" for \"user defined code\" or a \"p\" for \"planned code\"\n"));
		return -1UL;
	}

	// Get the major code
	++it;
	int code = _ttoi((*it).c_str());
	ret |= code << 16;

	// Get the minor code
	++it;
	code = _ttoi((*it).c_str());
	ret |= code;

	return ret;
}

/*******************************************************************************
** ParseTime
**
** Converts the supplied time information to a delay value to be supplied to
** InitiateSystemShutdownEx.  Allowed values are:
**
**	"now"	- results in a delay value of zero
**	"+NN"	- NN is a number of minutes from now
**	"HH:MM"	- a specific hour and minute in the next 24 hours at which to take
**			  action
**
** Arguments:
**
**	const _TCHAR const *pszTime	- the string containing the time information.
**								  It should be one of the values listed above.
**
** Returns: the number of seconds delay before the action is taken
*******************************************************************************/
time_t ParseTime(const _TCHAR* const pszTime)
{
	const TCHAR* const pColon = _tcsstr(pszTime, _T(":"));
	time_t now = time(NULL);

	// If they specified the string "now"
	if (_tcsicmp(pszTime, _T("now")) == 0)
		return 0L;

	// If they specified a string containing minutes
	else if (static_cast<_TCHAR>(pszTime[0]) == _T('+'))
	{
		return 60L * _tstol(pszTime + 1);
	}

	// If they specified the hh:mm format
	else if (pColon != NULL)
	{
#define HOURS_BUF_SIZE 3
		TCHAR szHours[HOURS_BUF_SIZE];
		memset(szHours, 0, sizeof(TCHAR) * HOURS_BUF_SIZE);
		long diff = pColon - pszTime;
		long size = (diff / sizeof(TCHAR)) + 1;
		errno_t err = _tcsncpy_s(szHours, HOURS_BUF_SIZE, pszTime, ((pColon - pszTime) / sizeof(TCHAR)) + 1);
		int mins = _tstol(pColon + 1);
		int hours = _tstol(szHours);

		// Get the current time
		struct tm *pNow = localtime(&now);

		// If we've already passed the specified hour and minute today, assume 
		// that we're doing it tomorrow.
		if (pNow->tm_hour > hours || (pNow->tm_hour == hours && pNow->tm_min > mins))
			++pNow->tm_mday;
		pNow->tm_hour = hours;
		pNow->tm_min = mins;
		
		return mktime(pNow) - now;
	}

	else
		return -1L;
}

/*******************************************************************************
** split
**
** Splits a string into multiple tokens given a delimiter.
**
** Arguments:
**
**	const tstring &s			- the string to split
**	_TCHAR delim				- the delimiter to use when splitting s
**	std::vector<tstring> &elems	- receives the tokens
**
** Returns: void
*******************************************************************************/
void split(const tstring &s, _TCHAR delim, std::vector<tstring> &elems)
{
    tstringstream ss(s);
    tstring item;
    while(std::getline(ss, item, delim))
        elems.push_back(item);
}
