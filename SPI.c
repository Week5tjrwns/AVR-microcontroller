#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

// 액티브로우 7-세그먼트 패턴 (인덱스 = 0~9)
static const uint8_t SEG[10] = {
	0xC0, // 0
	0xF9, // 1
	0xA4, // 2
	0xB0, // 3
	0x99, // 4
	0x92, // 5
	0x82, // 6
	0xF8, // 7
	0x80, // 8
	0x90  // 9
};

int main(void) {
	uint8_t rx;

		// PA0~PA6 = 출력, PA7 = 입력(미사용)
	DDRA  = 0x7F;   // 0b0111_1111
	// 모두 HIGH → 액티브-로우이므로 세그먼트 OFF
	PORTA = 0xFF;

	//── SPI 슬레이브 초기화 ──
	// PB3(MISO) = 출력, PB2(MOSI), PB1(SCK), PB0(SS) = 입력
	DDRB  |=  (1<<PB3);
	DDRB  &= ~((1<<PB2)|(1<<PB1)|(1<<PB0));
	// SPI Enable, 슬레이브 모드 (MSTR=0)
	SPCR   = (1<<SPE);
	// (SPSR는 기본 0 → SPI2X=0, CPOL=0, CPHA=0, MSB first)

	

	while (1) {
		// SPI 수신 대기 (SPIF=1)
		while (!(SPSR & (1<<SPIF)));
		rx = SPDR;          // 수신된 1바이트 읽기
		SPSR |= (1<<SPIF);  //  SPIF 클리어

		// ASCII '0'~'9' 범위면 7-세그먼트에 표시
		if (rx >= '0' && rx <= '9') {
			PORTA = SEG[rx - '0'];
		}
		
	}
}


