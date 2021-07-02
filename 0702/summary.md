server = socket -> bind -> listen -> accept   
client = socket -> connect

리눅스는 소켓을 파일로 취급하지만 윈도우는 소켓과 파일을 구분한다.

저수준 파일 입출력은 운영체제에서 제공하는 함수이고 표준 파일 입출력은 ANSI에서 정의한 함수이다.
