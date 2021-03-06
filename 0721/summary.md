비동기 IO   
입출력 함수의 반환시점과 데이터 송수신의 완료 시점이 일치하지 않는 경우

비동기 IO 사용 이유   
동기 IO는 송수신하고자 하는 데이터가 버퍼에 전송될 때까지 스레드가 블로킹 상태에 있기 때문에 다른 작업을 처리하지 못해서 수행 성능이 떨어지게 된다.   
비동기 IO는 디바이스 드라이버가 장치로부터의 응답을 대기해주기 때문에 애플리케이션의 스레드는 IO요청이 완료될 때까지 대기할 필요 없이 다른 작업을 수행할 수 있다.   
이후에 IO요청 완료 통지를 수신하여 처리하면 된다. IO가 완료된 시점과 그에 대한 처리 시점을 분리하기 때문에 프로그램에 굉장한 유연성을 제공한다.

IO 완료통지 수신 방법   
디바이스 커널 오브젝트의 시그널링   
스레드는 비동기 IO요청이 완료되었는지 여부를 확인하기 위해 WaitForSingleObject나 WaitForMultipleObjects함수를 사용할 수 있다.   
WaitForSingleObject 함수는 매개변수로 전달된 디바이스 커널 오브젝트가 시그널 상태가 될때까지 스레드를 대기 상태로 유지한다.   
IO작업이 완료되어 디바이스 드라이버가 디바이스 커널 오브젝트를 시그널 상태로 만들면 WaitForSingleObject 함수가 반환된다.   
문제점: 단일 장치에 대해 다수의 IO요청에 대한 처리를 수행할 수 없다. 어떤 작업이 완료된 것인지 구분할 방법이 없기 때문이다.

비동기 notification IO   
IO 상태에 상관없이 반환이 이뤄지는 방식으로, 이후에 상태에 변화가 있었는지 확인해야 한다.

WSAEventSelect 함수   
event 오브젝트와 소켓을 연결하는 함수로, 소켓에서 감시하고자 하는 이벤트가 발생하면 event 커널 오브젝트를 signaled 상태로 바꾸는 함수

비동기 notification IO 모델 과정   
1. event 오브젝트 생성(WSACreateEvent)   
2. event 오브젝트와 소켓 연결(WSAEventSelect)   
3. 이벤트 발생 확인(WSAWaitMultipleEvents)   
4. 발생된 이벤트 유형 확인(WSAEnumNetworkEvents)