#include <avr/io.h>           // AVR ����� �������� ���� (DDRx, PORTx ��)
#include <avr/interrupt.h>    // ���ͷ�Ʈ ���� ��ũ��(sei, cli, ISR ��) ����


// ���� ���� ����
uint8_t flag = 0;              // LED ��� ���� ����� �÷��� (0: ����, 1: ����)
volatile unsigned int count = 0; // �����÷ο� ���� ��갪. ISR���� �а� ���Ƿ� volatile

// �Լ� ������Ÿ��
void delay_ms(int data);
void Timer_init(void);
void IO_init(void);

// I/O �ʱ�ȭ: Port A�� ��� ������� �����ϰ�, �⺻ High(LED ��)�� �ʱ�ȭ
void IO_init(void) {
	DDRA  = 0xFF;  // DDRA = 0b11111111; PortA ��� ���� ������� ����
	PORTA = 0xFF;  // PORTA = 0b11111111; ��� ���� High(5V)�� ���� �� ��Ƽ�� �ο� LED�� ����
}

// Ÿ�̸�1 �ʱ�ȭ: ���ֺ� ����, �����÷ο� ���ͷ�Ʈ ���, ���� ���ͷ�Ʈ Ȱ��ȭ
void Timer_init(void) {
	TCCR1A = 0x00;             // TCCR1A = 0b00000000; PWM ��� ��Ȱ��ȭ, �Ϲ� �����÷ο� ���
	TCCR1B = 0b00000100;       // CS12:0 = 100 �� ���ֺ� 256 (16MHz / 256 = 62.5KHz ī��Ʈ)
	TIMSK  |= (1 << TOIE1);    // TIMSK: TOIE1 ��Ʈ(Ÿ�̸�1 �����÷ο� ���ͷ�Ʈ ���) ����
	sei();                     // ���� ���ͷ�Ʈ ��� (global interrupt enable)
}

// Ÿ�̸�1 �����÷ο� ���ͷ�Ʈ ���� ��ƾ(ISR)
// TCNT1�� 0��65535 �Ѿ ������ �� �Լ��� ȣ���
ISR(TIMER1_OVF_vect) {
	// flag ���� ���� PortA�� Low(�ѱ�) / High(����)�� ���
	if (!flag) {
		flag = 1;
		PORTA = 0x00;  // ��� �� Low �� ��Ƽ�� �ο� LED ����
		} else {
		flag = 0;
		PORTA = 0xFF;  // ��� �� High �� ��Ƽ�� �ο� LED ����
	}
	// ���� �����÷ο� ������ ���� TCNT1 �������Ϳ� �ʱⰪ �缳��
	// 65535 - count ��ŭ ���� ī��Ʈ�� �� �����÷ο�
	TCNT1 = 65535 - count;
}

// ���� �ð�(ms) ��� �Լ�
// data: �����ϰ��� �ϴ� �ð�(�и���)
// count ����: (CPU Ŭ�� / ���ֺ� / 2) * data(ms) / 1000 - 1
void delay_ms(int data) {
	count = (((16000000) / (256 * 2)) * data / 1000) - 1;
}

int main(void)
{
	IO_init();            // I/O ��Ʈ �ʱ�ȭ
	Timer_init();         // Ÿ�̸� & ���ͷ�Ʈ �ʱ�ȭ
	delay_ms(500);        // 500ms ���� �ֱ� ���
	TCNT1 = 65535 - count; // Ÿ�̸� ���� �� ī���� �ʱⰪ ����

	while (1) {
		// ���� ���������� ���� ���� ���� ���ͷ�Ʈ���� ����
	}
}

