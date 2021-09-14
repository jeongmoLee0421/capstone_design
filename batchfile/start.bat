@echo off
REM 불필요한 명령어 출력 차단

cd "C:\Users\leejeongmo\Documents\repos\server\Debug"
start serv.bat

REM 클라이언트 프로그램이 있는 디렉터리로 이동
cd "C:\Users\leejeongmo\Documents\repos\client\Debug"

REM 여러 클라이언트 프로그램 실행을 위한 배치 프로그램 실행
REM start 명령어를 통해 새로운 cmd.exe 창을 띄워 프로그램 실행
start clnt1.bat
start clnt2.bat
start clnt3.bat
start clnt4.bat
start clnt5.bat
start clnt6.bat
start clnt7.bat

REM 일괄 프로그램 처리를 보류하고 다음 메시지를 보여줌
pause