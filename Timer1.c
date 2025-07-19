#include <avr/io.h>           // AVR 입출력 레지스터 정의 (DDRx, PORTx 등)
#include <avr/interrupt.h>    // 인터럽트 관련 매크로(sei, cli, ISR 등) 정의


// 전역 변수 선언
uint8_t flag = 0;              // LED 토글 상태 저장용 플래그 (0: 꺼짐, 1: 켜짐)
volatile unsigned int count = 0; // 오버플로우 간격 계산값. ISR에서 읽고 쓰므로 volatile

// 함수 프로토타입
void delay_ms(int data);
void Timer_init(void);
void IO_init(void);

// I/O 초기화: Port A를 모두 출력으로 설정하고, 기본 High(LED 끔)로 초기화
void IO_init(void) {
	DDRA  = 0xFF;  // DDRA = 0b11111111; PortA 모든 핀을 출력으로 설정
	PORTA = 0xFF;  // PORTA = 0b11111111; 출력 핀을 High(5V)로 설정 → 액티브 로우 LED는 꺼짐
}

// 타이머1 초기화: 분주비 설정, 오버플로우 인터럽트 허용, 전역 인터럽트 활성화
void Timer_init(void) {
	TCCR1A = 0x00;             // TCCR1A = 0b00000000; PWM 모드 비활성화, 일반 오버플로우 모드
	TCCR1B = 0b00000100;       // CS12:0 = 100 → 분주비 256 (16MHz / 256 = 62.5KHz 카운트)
	TIMSK  |= (1 << TOIE1);    // TIMSK: TOIE1 비트(타이머1 오버플로우 인터럽트 허용) 설정
	sei();                     // 전역 인터럽트 허용 (global interrupt enable)
}

// 타이머1 오버플로우 인터럽트 서비스 루틴(ISR)
// TCNT1이 0→65535 넘어갈 때마다 이 함수가 호출됨
ISR(TIMER1_OVF_vect) {
	// flag 값에 따라 PortA를 Low(켜기) / High(끄기)로 토글
	if (!flag) {
		flag = 1;
		PORTA = 0x00;  // 모든 핀 Low → 액티브 로우 LED 켜짐
		} else {
		flag = 0;
		PORTA = 0xFF;  // 모든 핀 High → 액티브 로우 LED 꺼짐
	}
	// 다음 오버플로우 시점을 위해 TCNT1 레지스터에 초기값 재설정
	// 65535 - count 만큼 먼저 카운트된 뒤 오버플로우
	TCNT1 = 65535 - count;
}

// 지연 시간(ms) 계산 함수
// data: 지연하고자 하는 시간(밀리초)
// count 계산식: (CPU 클록 / 분주비 / 2) * data(ms) / 1000 - 1
void delay_ms(int data) {
	count = (((16000000) / (256 * 2)) * data / 1000) - 1;
}

int main(void)
{
	IO_init();            // I/O 포트 초기화
	Timer_init();         // 타이머 & 인터럽트 초기화
	delay_ms(500);        // 500ms 지연 주기 계산
	TCNT1 = 65535 - count; // 타이머 시작 시 카운터 초기값 설정

	while (1) {
		// 메인 루프에서는 별도 동작 없이 인터럽트에만 의존
	}
}

