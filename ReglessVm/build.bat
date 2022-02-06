set src=../src/

cd bin

gcc -std=c99 ^
%src%trap/out/out.c ^
%src%error/error.c ^
%src%main.c ^
-o regless-vm.exe

regless-vm.exe
@pause
