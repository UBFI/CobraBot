# CobraBot
CobraBot - A IRC net written in pure C back in 2012




-COMPILE AS FOLLOWS-




@echo off

SET ProjectName=CobraBot
SET Libraries=-lmsvcrt -lkernel32 -luser32 -lws2_32 -lgcc -lshell32 -ladvapi32 -lwininet -lcrypt32
SET SourceFiles=%ProjectName%.c BotKiller.c PasswordRecovery.obj
SET Extention=exe

IF EXIST %ProjectName%.exe DEL %ProjectName%.exe
gcc -m32 %SourceFiles% -Wl,--relax -Wl,--gc-sections -fno-asynchronous-unwind-tables -Wall -Wextra -o %ProjectName%.%Extention% %Libraries% -nostdlib -Wl,-e__main -O1 -O2 -O3 -Os -s -mwindows
IF EXIST %ProjectName%.%Extention% (
echo Compiled.
) ELSE (
echo Error Compiling !
)
pause


