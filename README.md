# CobraBot
CobraBot - A IRC net written in pure C back in 2012




-COMPILE AS FOLLOWS-


gcc -m32 %SourceFiles% -Wl,--relax -Wl,--gc-sections -fno-asynchronous-unwind-tables -Wall -Wextra -o %ProjectName%.%Extention% %Libraries% -nostdlib -Wl,-e__main -O1 -O2 -O3 -Os -s -mwindows
