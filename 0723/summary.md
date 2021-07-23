이벤트 커널 오브젝트의 시그널링을 사용하지 않을 때 WSAOVERLAPPED 구조체의 멤버 hEvent를 사용자 임의로 사용할 수 있다(event 오브젝트의 시그널링을 사용하지 않으면 멤버 hEvent는 큰 의미를 갖지 못하기 때문).   
위 코드에서는 입출력 완료 시 자동으로 호출되는 Completion Routine 내부로 클라이언트 정보(소켓과 버퍼)를 전달하기 위해서 WSAOVERLAPPED 구조체의 멤버 hEvent를 사용하였다.

OVERLAPPED IO 기반 echo server 동작 원리   
1. 클라이언트가 연결되면 WSARecv 함수를 호출하면서 non-blocking 모드로 데이터가 수신되게 하고, 수신이 완료되면 ReadCompRoutine 함수가 호출되게 한다.   
2. ReadCompRoutine 함수가 호출되면 WSASend 함수를 호출하면서 non-blocking 모드로 데이터가 송신되게 하고, 송신이 완료되면 WriteCompRoutine 함수가 호출되게 한다.   
3. 이렇게 해서 호출된 WriteCompRoutine 함수는 다시 WSARecv 함수를 호출하면서 non-blocking 모드로 데이터의 수신을 기다린다.