커널 오브젝트   
운영체제에 의해서 생성되는 리소스(스레드, 파일)들은 관리를 목적으로 정보를 기록하기 위해 내부적으로 데이터 블록을 생성한다.   
이 데이터 블록을 커널 오브젝트라고 한다.

커널 오브젝트의 소유자는 커널(운영체제)이다.

운영체제는 프로세스나 스레드가 종료되면 해당 커널 오브젝트를 signaled 상태로 변경함으로써 프로세스 또는 스레드의 종료 여부를 커널 오브젝트에 기록해둔다.