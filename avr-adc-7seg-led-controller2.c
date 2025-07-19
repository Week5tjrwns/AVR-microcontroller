
#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
static const uint8_t Seg_array[10] = {0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x90};
// 7세그먼트 배열 선언 (액티브 로우)
int main(void) {
	uint8_t mode = 0, v, c;
	uint16_t r;
	DDRD &= ~((1<<PD0)|(1<<PD1));
	PORTD |= (1<<PD0)|(1<<PD1);
	// 버튼 PD0/PD1 입력 풀업 설정
	// PortA/PortC 출력→OFF
	DDRA = 0xFF; PORTA = 0xFF;
	DDRC = 0xFF; PORTC = 0xFF;
	ADMUX  = 0x00;// ARef를 레퍼런스로, ADC0번 채널사용, 오른쪽 정렬
	ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);//ADC인에이블, 분주128로 설정
	
	PORTA = Seg_array[0];// 기본값 00으로 설정
	PORTC = Seg_array[0];
	_delay_ms(100);
	while (1) {
		if (!(PIND & (1<<PD0)))//pd0버튼이 눌리면
		{
			_delay_ms(50);
			if (!(PIND & (1<<PD0)))
			{
				while (!(PIND & (1<<PD0))); //버튼이 눌린동안 대기
				_delay_ms(50);
				mode = 0; // 모드를 0으로 변경
				PORTA = Seg_array[0];
				PORTC = Seg_array[0]; //세그먼트에 00출력
			}
		}
		else if (!(PIND & (1<<PD1))) //pd1버튼이 눌리면
		{
			_delay_ms(50);
			if (!(PIND & (1<<PD1)))
			{
				while (!(PIND & (1<<PD1))); //pd1 버튼이 눌린동안 대기
				_delay_ms(50);
				mode = 1; //모드1로 변경
				PORTA = 0xFF;
				PORTC = 0xFF; // 세그먼트 모드 off
			}
		}
		
		if (!mode) {
			//0번 모드일때
			ADMUX = 0x00; // 0번채널, AREF참조전압, 오른쪽정렬
			ADCSRA |= (1<<ADSC); //single conversion mode로 설정
			while (ADCSRA & (1<<ADSC)); // ADC변환이 완료될때까지 대기
			r = ADCL | (ADCH<<8); // 16비트 uint16_t 변수에 10비트만큼 ADC변환결과 저장
			v = (r > 1013 ? 99 : (uint8_t)((uint32_t)r * 99 / 1023));
			//3항 연산자 사용: r이 1013보다 크면 99를 r에 저장(고정시킴), 거짓이면 r * 99/ 1023을 저장
			PORTA = Seg_array[v / 10]; // 10으로 나눈 몫을 10의 자리 세그먼트에 출력
			PORTC = Seg_array[v % 10]; // 10으로 나눈 나머지를 1의 자리 세그먼트에 출력
		}
		else //1번 모드일때
		{
			ADMUX = 0x01; // 1번채널, AREF참조전압, 오른쪽정렬
			ADCSRA |= (1<<ADSC); //single conversion mode로 설정
			while (ADCSRA & (1<<ADSC)); // ADC변환이 완료될때까지 대기
			r = ADCL | (ADCH<<8); // 16비트 uint16_t 변수에 10비트만큼 ADC변환결과 저장
			c = (r > 1013 ? 8 : (uint8_t)((uint32_t)r * 8 / 1023));//세그먼트가를 8단계로 나눔
			//3항 연산자 사용:  r이 1013보다 크면 8를 r에 저장(고정시킴), 거짓이면 r * 8/ 1023을 저장
			PORTA = (c ? (uint8_t)~((1 << c) - 1) : 0xFF); // c가 0이 아니면 참, 0이면 거짓
			// c가 0이 아니면 누적한 led를 10의 자리 세그먼트에 출력하고 0이면 led모두 소등시킴
			PORTC = 0xFF; // 포트c를 모두 소등시킴
		}
		_delay_ms(100);
	}
	return 0;
}