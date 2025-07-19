
#include <avr/io.h>
#include <avr/interrupt.h>

#define LED_PORTA PORTA
#define LED_PORTC PORTC
#define BTN0_PORT  PIND
#define BTN0_PIN   PD2   // INT0
#define BTN1_PIN   PD3   // INT1

// 7-세그 Active-Low 코드 (0~9)
const uint8_t SEG_ARRAY[11] = {
	0xC0, 0xF9, 0xA4, 0xB0, 0x99,
	0x92, 0x82, 0xF8, 0x80, 0x90, 0xEF
};

volatile uint16_t ADC_Read = 0;
volatile uint8_t  mode     = 0;  // 0: bar-graph, 1: 0~99 세그

// 0~1023 → 0~99 매핑 (반올림)
static uint8_t map_adc_to_0_99(uint16_t adc) {
	uint32_t v = (uint32_t)adc * 99 + 511;
	v /= 1023;
	return (v > 99 ? 99 : v);
}

// 두 자리 세그먼트 표시
static void display_0_99(uint8_t val) {
	uint8_t ten  = val / 10;
	uint8_t unit = val % 10;
	LED_PORTA = SEG_ARRAY[ten];
	LED_PORTC = SEG_ARRAY[unit];
}

// bar-graph 표시 (0~7 단계, Active-Low)
static void display_bar(uint16_t adc) {
	uint8_t level = adc / 113;  // 0..7
	if (level > 9) level = 9;
	uint8_t mask = (1 << level) - 1;   // 2^level - 1
	LED_PORTA = ~(mask);               // Active-Low
}

void IO_init(void) {
	// LED ports
	DDRA = 0xFF;  PORTA = 0xFF;
	DDRC = 0xFF;  PORTC = 0xFF;
	// 버튼(투입부): PD2, PD3 입력 + 풀업
	DDRD &= ~((1<<BTN0_PIN)|(1<<BTN1_PIN));
	PORTD |=  (1<<BTN0_PIN)|(1<<BTN1_PIN);
}

void ADC_start(uint8_t ch) {
	// REFS0=1(AVCC), ADLAR=0, MUX=ch (0 or 1)
	ADMUX  = (1<<REFS0) | (ch & 0x07);
	ADCSRA = (1<<ADEN)|(1<<ADIE)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
	ADCSRA |= (1<<ADSC);  // 변환 시작
}

void INT_init(void) {
	EICRA  = (1<<ISC01)|(0<<ISC00)   // INT0: 하강엣지 트리거
	|(1<<ISC11)|(0<<ISC10);  // INT1: 하강엣지 트리거
	EIMSK  = (1<<INT0)|(1<<INT1);   // INT0, INT1 허용
}

ISR(INT0_vect) {
	mode = 0;           // bar-graph 모드
	ADC_start(0);       // 채널0 변환 시작
}

ISR(INT1_vect) {
	mode = 1;           // 세그먼트 모드
	ADC_start(1);       // 채널1 변환 시작
}

ISR(ADC_vect) {
	// ADCL 읽고 ADCH 읽어서 10비트 조합
	ADC_Read = ADCL;
	ADC_Read |= (ADCH << 8);
	// 다음 변환 자동 시작
	ADCSRA |= (1<<ADSC);
}

int main(void) {
	IO_init();
	INT_init();
	sei();  // 전역 인터럽트 허용

	// 시작 시 바로 채널0 변환
	ADC_start(0);

	while (1) {
		if (mode == 0) {
			display_bar(ADC_Read);
			} else {
			display_0_99(map_adc_to_0_99(ADC_Read));
		}
		// (필요시 짧은 딜레이)
	}
	return 0;
}
