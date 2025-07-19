#include <avr/io.h>      // IO 레지스터 정의
#include <util/delay.h>  // _delay_ms() 사용
#include <avr/interrupt.h> // 인터럽트 관련

#define SEG_PORT_L PORTA // 상위 7-세그먼트 포트
#define SEG_PORT_R PORTC // 하위 7-세그먼트 포트

#define SLAVE_ADDRESS 0b0011001 // 7비트 슬레이브 주소

volatile uint8_t received_data = 0; // I2C로 받은 데이터 저장

// 7-세그먼트 출력 함수
void SEG_WRITE(uint8_t data) {
	uint8_t FND_TBL[] = {
		0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D,
		0x7D, 0x27, 0x7F, 0x6F, 0x77, 0x7C,
		0x39, 0x5E, 0x79, 0x71, 0x40
	}; // 0~F 및 '-' 패턴

	SEG_PORT_L = FND_TBL[data >> 4];    // 상위 4비트 표시
	SEG_PORT_R = FND_TBL[data & 0x0F];  // 하위 4비트 표시
}

// I/O 포트 초기화
void IO_init(void) {
	DDRA  = 0xFF;   // PORTA 출력 설정 (세그먼트)
	PORTA = 0x00;   // PORTA 초기값 0

	DDRC  = 0xFF;   // PORTC 출력 설정 (세그먼트)
	PORTC = 0x00;   // PORTC 초기값 0

	DDRF  = 0xFF;   // PORTF 출력 설정 (상태 표시)
	PORTF = 0x00;   // PORTF 초기값 0
}

// I2C 슬레이브 모드 초기화
void I2C_Slave_Init(void) {
	TWAR = (SLAVE_ADDRESS << 1);            // Own Address 설정, GCEN=0
	TWCR = (1<<TWEN) | (1<<TWIE) | (1<<TWEA);// TWI Enable, ISR Enable, ACK 허용
	// TWDR 초기화는 불필요함 (HW에서 자동 관리)
}

// I2C 인터럽트 서비스 루틴
ISR(TWI_vect) {
	uint8_t status = TWSR & 0xF8;    // 현재 상태 코드 읽기

	switch (status) {
		// 슬레이브로 쓰기 시도된 주소 수신
		case 0x60: case 0x68:
		case 0x70: case 0x78:
		TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE)|(1<<TWEA); // ACK 후 대기
		break;

		// 데이터 수신 완료
		case 0x80: case 0x90:
		received_data = TWDR;       // 받은 데이터 저장
		TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE)|(1<<TWEA); // 다음 바이트 대기
		break;

		// STOP 또는 반복 START 수신
		case 0xA0:
		TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE)|(1<<TWEA); // 대기 상태 복귀
		break;

		// 마스터가 해당 슬레이브로부터 읽기 요청
		case 0xA8: case 0xB8:
		case 0xC0: case 0xC8:
		TWDR = received_data;       // 마지막 받은 데이터 송신
		TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE)|(1<<TWEA); // 송신 후 대기
		break;

		// 기타 상태: 모두 무시하고 ACK 후 대기
		default:
		TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE)|(1<<TWEA);
		break;
	}
}

int main(void) {
	IO_init();          // 포트 설정
	I2C_Slave_Init();   // I2C 슬레이브 초기화
	sei();              // 전역 인터럽트 허용

	SEG_WRITE(0);       // 초기값 표시

	while (1) {
		SEG_WRITE(received_data); // 수신된 값 루프에서 표시
	}
	return 0; // 실행 종료 지점 (실제 도달 안 함)
}