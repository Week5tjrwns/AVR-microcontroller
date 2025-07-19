
#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 16000000UL

volatile uint8_t led_state = 0;

/* 함수 선행 선언 */
void IO_init(void);
void UART_init(void);
void putch(char data);
void TIMER1_CTC_init(void);

/* UART 초기화: 16MHz, 38400bps → UBRR0L = 25 */
void UART_init(void)
{
    UBRR0H = 0;
    UBRR0L = 25;                       // 38400bps 설정
    UCSR0B = (1 << TXEN0);            // 송신기만 활성화
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);  // 8비트, 패리티 없음, 1스톱비트
}

/* 한 글자 전송 */
void putch(char data)
{
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = data;
}

/* I/O 초기화 */
void IO_init(void)
{
    /* PORTA 전체를 출력으로 설정 (LED) */
    DDRA  = 0xFF;
    PORTA = 0x00;  // 초기 LED OFF

    /* UART0 TXD0 = PE1 출력 */
    DDRE |= (1 << PE1);
}

/* Timer1 CTC 모드로 2초마다 인터럽트 설정 */
void TIMER1_CTC_init(void)
{
    /* CTC 모드: WGM13:0 = 0100 → WGM12=1 */
    TCCR1B = (1 << WGM12)
           /* prescaler = 1024 */
           | (1 << CS12) | (1 << CS10);
    /* OCR1A = 2s * F_CPU / prescaler - 1
         = 2 * 16,000,000 / 1024 - 1 ≈ 31250 - 1 = 31249 */
    OCR1A = 31249;
    /* Compare A 인터럽트 허용 */	
    TIMSK = (1 << OCIE1A);
    /* 전역 인터럽트 허용 */
    sei();
}

/* Timer1 Compare A 인터럽트 서비스 루틴 */
ISR(TIMER1_COMPA_vect)
{
    led_state ^= 1;         // 토글
    if (led_state) {
        PORTA = 0xFF;       // LED ON	
        putch('O');         // UART로 'O' 출력
    } else {
        PORTA = 0x00;       // LED OFF
        putch('X');         // UART로 'X' 출력
    }
}

int main(void)
{
    IO_init();
    UART_init();
    TIMER1_CTC_init();

    while (1) {
        /* 메인 루프 비움: ISR에서 LED 토글 및 UART 출력 제어 */
    }
    return 0;
}

