set src=../src/

cd bin

gcc -std=c99 ^
%src%str/str.c ^
%src%main.c ^
-o regless-asm.exe

regless-asm.exe
@pause
