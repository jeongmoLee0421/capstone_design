write 함수 호출: 데이터가 출력버퍼로 이동   
read 함수 호출: 입력버퍼에 저장된 데이터를 읽음   
write함수가 반환되는 시점: 전송할 데이터가 출력버퍼로 이동이 완료되는 시점   
TCP의 sliding window프로토콜은 상대의 입력버퍼 크기를 초과하는 분량의 데이터 전송을 하지 않는다.(흐름 제어)

TCP 소켓 연결: three-way handshaking   
TCP 소켓 연결종료: four-way handshaking

데이터 전송시 상대는 전송한 데이터를 전부 받을 수 있도록 반복문을 설정해야 한다.