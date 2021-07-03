IP: Internet Protocol의 약자로 컴퓨터에 부여하는 값   
PORT: 프로그램상에서 생성되는 소켓을 구분하기 위해 소켓에 부여되는 번호   
htons: short형 데이터를 호스트 바이트 순서에서 네트워크 바이트 순서로 변환   
bind: 초기화된 주소정보를 소켓에 할당하는 함수

TCP: transmission control protocol   
LINK 계층: 물리적인 연결   
IP 계층: 경로 설정   
TCP 계층: 신뢰성 있는 데이터 전송 담당   
APPLICATION 계층: 프로그램의 성격에 따라 프로토콜 설계, 구현

listen: 연결요청 대기상태 진입   
accept: '연결요청 대기 큐'에서 대기중인 클라이언트의 연결요청을 수락   
connect: 서버에 연결요청