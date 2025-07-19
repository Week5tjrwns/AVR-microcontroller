#include <avr/io.h>
#include <avr/interrupt.h>

#define STEPPER_DDR    DDRC
#define STEPPER_PORT   PORTC
#define STEPPER_MASK   0x0F    // 하위 4비트 사용

/* 가감속 제어 매개변수 */
volatile uint8_t speed_delay = 200;    // 초기 딜레이값 (클럭 단위)
const uint8_t speed_min = 50;          // 최대속도(최소 딜레이)
const uint8_t speed_max = 250;         // 최소속도(최대 딜레이)
const uint8_t accel_step = 10;         // 가/감속 단계

/* 스텝 테이블 */
const uint8_t step_tbl[8] = {0x08,0x0C,0x04,0x06,0x02,0x03,0x01,0x09};
volatile uint8_t step_idx = 0;

int main(void) {
	/* 스텝퍼 출력핀 설정 */
	STEPPER_DDR |= STEPPER_MASK;

	/* INT0(가속), INT1(감속) 설정: 하강 에지 트리거 */
	EICRA  = (1<<ISC01)|(1<<ISC11);  // ISC01=1, ISC00=0 ; ISC11=1, ISC10=0
	EIMSK  = (1<<INT0)|(1<<INT1);    // INT0, INT1 인터럽트 활성

	/* 타이머0: 분주비 1024, 오버플로우 인터럽트 */
	TCCR0  = (1<<CS02)|(1<<CS00);
	TIMSK |= (1<<TOIE0);
	TCNT0   = 0;                      // 초기 카운트

	sei();  // 전역 인터럽트 활성화

	while (1) {
		/* 메인 루프는 가감속 ISR과 타이머 ISR에 의해 동작 */
	}
}

/* INT0_vect: 가속 (딜레이 감소) */
ISR(INT0_vect) {
	if (speed_delay > speed_min + accel_step) {
		speed_delay -= accel_step;
		} else {
		speed_delay = speed_min;
	}
}

/* INT1_vect: 감속 (딜레이 증가, 멈출 때까지) */
ISR(INT1_vect) {
	if (speed_delay < speed_max - accel_step) {
		speed_delay += accel_step;
		} else {
		speed_delay = speed_max;
	}
}

/* TIMER0 오버플로우: 스텝 실행 및 속도 제어 */
ISR(TIMER0_OVF_vect) {
	static uint16_t cnt = 0;
	TCNT0 = 0;
	if (++cnt >= speed_delay) {
		cnt = 0;
		/* 다음 스텝 출력 */
		STEPPER_PORT = (STEPPER_PORT & ~STEPPER_MASK)
		| step_tbl[step_idx];
		if (++step_idx >= 8) step_idx = 0;
	}
}
