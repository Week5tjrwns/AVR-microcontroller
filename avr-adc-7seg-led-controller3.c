#define F_CPU 16000000UL
#include <avr/io.h>

void T1i(void){
    TCCR1A=0;
    TCCR1B=(1<<WGM12)|(1<<CS11)|(1<<CS10);
    OCR1A=249;
    TIFR=1<<OCF1A;
}
void d1(void){
    TCNT1=0;
    TIFR=1<<OCF1A;
    while(!(TIFR&1<<OCF1A));
}
unsigned r0(void){
    ADMUX=1<<REFS0;
    ADCSRA|=1<<ADSC;
    while(ADCSRA&1<<ADSC);
    return ADC;
}
const unsigned char seg7[]={0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x90},
                  seq[] ={1<<PG0,1<<PG0|1<<PG1,1<<PG1,1<<PG1|1<<PG2,
                          1<<PG2,1<<PG2|1<<PG3,1<<PG3,1<<PG3|1<<PG0};

int main(void){
    T1i();
    ADCSRA=1<<ADEN|1<<ADPS2|1<<ADPS1|1<<ADPS0;
    DDRA=DDRC=0xFF; PORTA=PORTC=0xFF;
    DDRG|=0x0F; PORTG|=0x0F;

    unsigned char cd=200, t=0, i=0, v;
    unsigned int raw;

    while(1){
        d1();
        raw=r0();
        v=raw*100UL/1024; if(v>99)v=99;
        PORTA=seg7[v/10];
        PORTC=seg7[v%10];

        unsigned tgt = v<50
            ? 200 - (150UL * v)/50       // decel: 0→50→200→50
            : 50  - (48UL*(v-50))/49;    // accel: 50→99→50→2
        if(tgt<2)   tgt=2;
        if(tgt>200) tgt=200;

        if(cd<tgt) cd++;
        else if(cd>tgt) cd--;

        if(t--==0){
            if(cd<200){
                PORTG=(PORTG&0xF0)|((~seq[i])&0x0F);
                i=(i+1)&7;
            }
            t=cd;
        }
    }
}
