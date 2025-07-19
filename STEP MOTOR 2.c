// 1상 구동
/*
#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

int main(void) {
	DDRC  = 0x0F;    // PC0~PC3 출력
	PORTC = 0x00;

	const uint8_t seq1[4] = {
		(1<<PC0),
		(1<<PC1),
		(1<<PC2),
		(1<<PC3)
	};
	uint8_t idx = 0;

	while (1) {
		PORTC = seq1[idx];
		idx = (idx + 1) & 0x03;
		_delay_ms(10);
	}
} */

//2상 구동
/*
#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

int main(void) {
	DDRC  = 0x0F;
	PORTC = 0x00;

	const uint8_t seq2[4] = {
		(1<<PC0)|(1<<PC1),
		(1<<PC1)|(1<<PC2),
		(1<<PC2)|(1<<PC3),
		(1<<PC3)|(1<<PC0)
	};
	uint8_t idx = 0;

	while (1) {
		PORTC = seq2[idx];
		idx = (idx + 1) & 0x03;
		_delay_ms(10);
	}
}

*/
// 1-2상 구동
/*
#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

int main(void) {
	DDRC  = 0x0F;
	PORTC = 0x00;

	const uint8_t seq3[8] = {
		(1<<PC0),               // A
		(1<<PC0)|(1<<PC1),      // A+B
		(1<<PC1),               // B
		(1<<PC1)|(1<<PC2),      // B+C
		(1<<PC2),               // C
		(1<<PC2)|(1<<PC3),      // C+D
		(1<<PC3),               // D
		(1<<PC3)|(1<<PC0)       // D+A
	};
	uint8_t idx = 0;

	while (1) {
		PORTC = seq3[idx];
		idx = (idx + 1) & 0x07;
		_delay_ms(10);
	}
}
*/

/*
  ATmega128 스텝모터 가감속 제어 예제
  - 인터럽트 INT0 (PD2) : 가속 시작 (throw up)
  - 인터럽트 INT1 (PD3) : 감속 시작 (throw down → 정지)
  - PC0~PC3 에 Active-High 시퀀스로 스텝코일 구동
  - Timer1 CTC 모드에서 OCR1A 를 조절해 속도 제어
*/
/*
  ATmega128 스텝모터 가감속 제어 — Active-Low 버전
  - INT0 (PD2): 가속 시작
  - INT1 (PD3): 감속 시작 → 정지
  - PC0~PC3: Active-Low 시퀀스
  - Timer1 CTC 모드로 OCR1A 조절
*/

/*
  ATmega128 스텝모터 가감속 제어 — Active-Low on PORTG
  - 스위치0 → PD0(INT0, 눌러질 때 페일링 엣지)
  - 스위치1 → PD1(INT1, 눌러질 때 페일링 엣지)
  - PG0~PG3   → ULN2003 IN1~IN4 (Active-Low)
  - Timer1 CTC 모드에서 OCR1A 조절로 속도 제어
*/


/*
  ATmega128 스텝모터 90° 회전 제어 — Active-Low on PORTG
  - 버튼 CW → PD0(INT0): 90° 시계방향 회전
  - 버튼 CCW → PD1(INT1): 90° 반시계방향 회전
  - PG0~PG3   → ULN2003 IN1~IN4 (Active-Low)
  - Half‐step(0.9°) 기준 90° = 100 스텝
*/

/*
  ATmega128 스텝모터 90° 회전 예제 — Active-Low on PORTG
  - 스위치 CW → PD0(INT0, 눌러질 때 FALLING edge)
  - 스위치 CCW → PD1(INT1, 눌러질 때 FALLING edge)
  - PG0~PG3   → ULN2003 IN1~IN4 (Active-Low)
  - Half-step (0.08789°) 기준 90° = 1024 스텝
*/

#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

/* 회전 관련 정의 */
#define STEP_ANGLE      (5.625f/64.0f)   // 0.087890625°
#define STEPS_PER_REV   4096             // 360°/0.08789°
#define STEPS_PER_90    (STEPS_PER_REV/4) // 1024 스텝
#define STEP_DELAY      2                // 스텝 간 지연(ms), 필요에 따라 조정

volatile uint8_t step_idx   = 0;
volatile uint8_t rotate_dir = 0;  // 1=CW, 2=CCW

// Half-step 시퀀스 (Active-High 패턴)
const uint8_t seq[8] = {
    (1<<PG0),               // A
    (1<<PG0)|(1<<PG1),      // A+B
    (1<<PG1),               // B
    (1<<PG1)|(1<<PG2),      // B+C
    (1<<PG2),               // C
    (1<<PG2)|(1<<PG3),      // C+D
    (1<<PG3),               // D
    (1<<PG3)|(1<<PG0)       // D+A
};

/* CW 버튼 ↔ INT0 */
ISR(INT0_vect) {
    rotate_dir = 1;
}

/* CCW 버튼 ↔ INT1 */
ISR(INT1_vect) {
    rotate_dir = 2;
}

int main(void) {
    // PG0~PG3 출력, Idle: 모두 HIGH (코일 OFF)
    DDRG  |= 0x0F;
    PORTG  = 0x0F;

    // PD0, PD1 입력 + 내부 풀업
    DDRD  &= ~((1<<PD0)|(1<<PD1));
    PORTD |=  (1<<PD0)|(1<<PD1);

    // 외부 인터럽트: INT0/INT1 모두 FALLING edge
    EICRA  = (1<<ISC01)|(0<<ISC00)   // INT0
           | (1<<ISC11)|(0<<ISC10); // INT1
    EIMSK  = (1<<INT0)|(1<<INT1);

    sei();  // 전역 인터럽트 허용

    while (1) {
        if (rotate_dir == 1) {
            // 시계방향 90° 회전
            for (uint16_t i = 0; i < STEPS_PER_90; i++) {
                PORTG = (~seq[step_idx] & 0x0F);
                step_idx = (step_idx + 1) & 0x07;
                _delay_ms(STEP_DELAY);
            }
            rotate_dir = 0;
        }
        else if (rotate_dir == 2) {
            // 반시계방향 90° 회전
            for (uint16_t i = 0; i < STEPS_PER_90; i++) {
                PORTG = (~seq[step_idx] & 0x0F);
                step_idx = (step_idx + 7) & 0x07;  // -1 mod 8
                _delay_ms(STEP_DELAY);
            }
            rotate_dir = 0;
        }
    }
}
