#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

#ifndef sbi
#define sbi(port,bit)	port |= (1 << bit)
#endif

#ifndef cbi
#define cbi(port,bit)	port &= ~(1 << bit)
#endif

#define LATCH	4
#define DATA	5
#define SCK		7
#define BTN0	0b11111101
#define BTN1	0b11111011
#define BTN2	0b11110111

/* PORTC */
#define DIR00		4
#define DIR01		5
#define DIR0_DDR	DDRC
#define DIR0_PORT	PORTC

/* PORTD */
#define DIR10		3
#define DIR11		6
#define DIR1_DDR	DDRD
#define DIR1_PORT	PORTD

#define SERVO_CENTER		3000 + (22*9)	//Sai s? c?a c?n sensor trên xe
#define STEP				22				//B??c quay c?a servo
#define ANGLE_MAX			45				//Goc quay toi da servo
#define vach_xam			19/20			//B?ng 1 n?u ???ng line không có v?ch xám

//Variable ADC
uint16_t ADC_average[8];		//ADC trung bình
uint16_t linetrang[8];			//ADC line tr?ng
uint16_t lineden[8];			//ADC line ?en

//Variable LED7
struct led7 {
	uint8_t i;
	uint8_t unit;
	uint8_t ten;
	uint8_t hundred;
	uint8_t thousand;
	uint8_t sensor_out;	
} led7_data;

//Variable RATIO
#define ratio_default 0.5
uint16_t cnt_ratio,	max,		// Thoi gian tinh van toc (50ms)
		 pulse_ratio;			// Bien dem encoder cho van toc
uint16_t velocity;				// Van toc
float ratio;					// Ti so toc do
float ratio_base;				// Ti so toc do nen

//===================BUTTON + SWITCH=====================
uint8_t get_button(uint8_t keyid)
{
	if ( (PINB & 0x0e) != 0x0e)
	{
		_delay_ms(80);
		if ((PINB|keyid) == keyid) {
			_delay_ms(50);
			return 1;
		}
	}
	return 0;
}
uint8_t get_switch() // tr? v? t? 0->15
{
	uint8_t x=0;
	x = ~PINC;
	x = x & 0x0f;
	return x;
}
float get_switch_2() //tr? v? 0.1 -> 0.4
{
	float val = 0;
	for(uint8_t i=0; i<4; i++)
	{
		if ( (((~PINC)>>i)&0x1) == 0x1 ) val += 0.1;
	}
	return val;
}

//================RATIO + SERVO + MOTOR ================

void cal_ratio()
{
	
}

void handle(int goc)
{
	if (goc > ANGLE_MAX)		 goc = ANGLE_MAX;
	else if(goc < -ANGLE_MAX)	 goc = -ANGLE_MAX;
	OCR1A = SERVO_CENTER+goc*STEP;
}

void speed(float left, float right, float percent)
{
	left  = left  *  ratio * (percent/100.0);
	right = right *  ratio * (percent/100.0);

	if(right >= 0)
	{
		sbi(DIR0_PORT, DIR00);
		cbi(DIR0_PORT, DIR01);
		OCR1B = right*200;
	}
	else
	{
		cbi(DIR0_PORT, DIR00);
		sbi(DIR0_PORT, DIR01);
		OCR1B = -right*200;
	}
	
	if(left >= 0)
	{
		sbi(DIR1_PORT, DIR10);
		cbi(DIR1_PORT, DIR11);
		OCR2 = left*255/100;
	}
	else
	{
		cbi(DIR1_PORT, DIR10);
		sbi(DIR1_PORT, DIR11);
		OCR2 = -left*255/100;
	}
}

inline void fast_brake_right()
{
	sbi(DIR0_PORT, DIR00);
	sbi(DIR0_PORT, DIR01);
	OCR1B=20000;
}

inline void fast_brake_left()
{
	sbi(DIR1_PORT, DIR10);
	sbi(DIR1_PORT, DIR11);
	OCR2=255;
}

void fast_brake()
{
	fast_brake_left();
	fast_brake_right();
}

//==========================LED7=========================
void SPI(uint8_t data)			//Truy?n d?  li?u sang led7, s? d?ng SPI
{
	SPDR = data;
	while(!(SPSR & (1<<SPIF)));	//??i ph?n c?ng truy?n xong data
}
void led7(unsigned int num)		// Tính toán d? li?u cho 4 led 7 ?o?n
{
	led7_data.unit		 = (1<<7) |(unsigned int)(num%10);
	led7_data.ten		 = (unsigned int)(num%100 / 10);
	led7_data.hundred	 = (unsigned int)(num%1000 / 100);
	led7_data.thousand	 = (unsigned int)(num/ 1000);
	led7_data.thousand	|= (led7_data.thousand!=0)? 1<<4 : 0;
	led7_data.hundred	|= (led7_data.thousand!=0 || led7_data.hundred!=0)? 1<<5 : 0;
	led7_data.ten		|= (led7_data.thousand!=0 || led7_data.hundred !=0 || led7_data.ten!=0)? 1<<6 : 0;
}
void print()					//Luôn th?c thi m?i vài ms ?? quét LED
{
	uint8_t value=0;
	if(led7_data.i++ == 4) led7_data.i=0;
	switch(led7_data.i)
	{
		case 0: value=led7_data.thousand;	break;
		case 1: value=led7_data.hundred;	break;
		case 2: value=led7_data.ten;		break;
		case 3: value=led7_data.unit;		break;
		default: break;
	}
	SPI(~led7_data.sensor_out);
	SPI(value);
	sbi(PORTB,LATCH);
	cbi(PORTB,LATCH);
}

//==========================ADC==========================
void read_adc_eeprom()
{
	for(uint8_t j=0; j<8; j++)
	{
		while(!eeprom_is_ready());
		linetrang[j] = eeprom_read_word((uint16_t*)(j*2));
		while(!eeprom_is_ready());
		lineden[j] = eeprom_read_word((uint16_t*)((j+8)*2));
	}
	for(uint8_t i=0; i<8; i++)
	{
		ADC_average[i]=(linetrang[i]+lineden[i])/2;
		ADC_average[i]=ADC_average[i]*vach_xam;
	}
}
void write_adc_eeprom()
{
	for(uint8_t j=0; j<8; j++)
	{
		while(!eeprom_is_ready());
		eeprom_write_word((uint16_t*)(j*2), (uint16_t)linetrang[j]);
		while(!eeprom_is_ready());
		eeprom_write_word((uint16_t*)((j+8)*2), (uint16_t)lineden[j]);
	}
}
uint16_t adc_read(uint8_t ch)
{
	ADMUX = (1<< REFS0)|ch;									// selecting channel
	ADCSRA|=(1<<ADSC);										// start conversion
	while(!(ADCSRA & (1<<ADIF)));							// waiting for ADIF, conversion complete
	return ADC;											// Giá tr? tr? v? t? [0 -> 1024] t??ng ?ng [0V -> 5V]
}

uint8_t sensor_cmp(uint8_t mask)							//Sensor compare: ??c v? và so sánh v?i trung bình
{															//Thêm tính n?ng che m?t n?: mask m?c ??nh là: 0xff (0b11111111)
	uint8_t ADC_value=0;
	for(uint8_t i=0; i<8; i++)
	{
		if(adc_read(i)<ADC_average[i]) sbi(ADC_value,i);	//Nh? h?n trung bình -> g?n v? 0V -> led thu h?ng ngo?i d?n -> có nhi?u h?ng ngo?i -> v?ch tr?ng
		//else    cbi(ADC_value,i);
	}
	led7_data.sensor_out=ADC_value;							//C?p nh?t giá tr? xu?t ra 8 led ??n
	return (ADC_value & mask);
}

void learn_color()
{
	uint16_t ADC_temp=0;
	for (uint8_t i=0; i<8; i++)
	{
		linetrang[i]=1024;
		lineden[i]=0;
	}
	
	led7(2017);
	while(1)
	{
		if(get_button(BTN0)) return;
		else if(get_button(BTN2)) break;
		for (uint8_t i=0; i<8; i++)
		{
			ADC_temp=adc_read(i);
			if (ADC_temp < linetrang[i]) linetrang[i]=ADC_temp;
			if(ADC_temp>lineden[i]) lineden[i]=ADC_temp;
		}
	}
	for (uint8_t i=0; i<8; i++)
	{
		ADC_average[i]=(linetrang[i]+lineden[i])/2;
		
		ADC_average[i]=ADC_average[i] * vach_xam;
	}
	write_adc_eeprom();										//Ghi vào eeprom ?? cho các l?n sau
}

//=======================INITIAL=========================
void INIT()
{
	//ADC
	ADMUX=(1<<REFS0);										// 0b0100000000 Ch?n ?i?n áp tham chi?u t? chân AVCC, thêm t? ? AREF
	ADCSRA=(1<<ADEN) | (1<<ADPS2)|(1<<ADPS2)|(1<<ADPS0);	// 0b10000111 Enable ADC and set Prescaler = 128
	read_adc_eeprom();										// T? ??ng ??c Eeprom ra khi b?t ngu?n chip
	
	//PORT
	DDRB  = 0b11110001;
	PORTB = 0b11111111;
	
	DDRC  = 0b00000000;			// Dipswitch
	PORTC = 0b11111111;
	
	DDRD  = 0b11111011;
	
	DIR0_DDR |= (1 << DIR00) | (1 << DIR01);
	DIR0_PORT = (DIR0_PORT & ~((1 << DIR00) | (1 << DIR01))) | (1 << DIR00);
	
	DIR1_DDR |= (1 << DIR10) | (1 << DIR11);
	DIR1_PORT = (DIR1_PORT & ~((1 << DIR10) | (1 << DIR11))) | (1 << DIR10);				// DIR00 = 1, DIR01 = 0, DIR10 = 1, DIR11 = 0
	
	//SPI
	SPCR = (1<<SPE)|(1<<MSTR);							//Enable spi, Master
	SPSR = (1<<SPI2X);									//SCK Mode 2X: Fosc/2
	
	//TIMER
	TCCR0=(1<<WGM01) | (1<<CS02);							// Mode 2 CTC,  Prescaler = 256
	OCR0=62;												// 1ms
	TIMSK=(1<<OCIE0);
	
	TCCR1A = (1<<COM1A1)|(1<<COM1B1)|(1<<WGM11);			// SET OCR1A & OCR1B at BOTTOM, CLEAR at Compare Match (Non-invert), Mode 14 Fast PWM
	TCCR1B = (1<<WGM13)|(1<<WGM12)|(1<<CS11);				// Prescaler = 8
	ICR1 = 20000;											// Time Period = 10ms
	
	TCCR2=(1<<WGM20)|(1<<WGM21)|(1<<COM21)|(1<<CS22)|(1<<CS21)|(1<<CS20);  //SET OC2 at BOTTOM, CLEAR OC2 on compare match,(non-invert), Mode 3 Fast PWM,  Prescaler = 1024
	OCR2=0;
	sei();
	
	//ENCODER
	MCUCR |= (1<<ISC11)|(1<<ISC01);
	GICR |= (1<<INT0);
}

void test_hardware()
{
	int8_t _index=0,
	temp = 0;
	while(1)
	{
		if (get_button(BTN0))
		{
			speed(100,100,100);
			handle(--temp);
			if (temp <= -ANGLE_MAX)		temp = -ANGLE_MAX;
			led7(-temp);
		}
		else if (get_button(BTN2))
		{
			speed(-100,-100,100);
			handle(++temp);
			if (temp >= ANGLE_MAX)		temp = ANGLE_MAX;
			led7(temp);
		}
		else if (get_button(BTN1))
		{
			if(++_index == 8)			_index=0;
			led7(adc_read(_index));
		}
		else
		{
			speed(0,0,0);
			handle(0);
			led7(adc_read(_index));
			temp = 0;
		}
		
		led7_data.sensor_out = 0 | (1<<_index);
		/*sensor_cmp(0xff);*/
	}
}

void get_speed()
{
	float speed = ratio_base;

	while(1)
	{
	if (get_button(BTN1))	{ ratio = ratio_base = speed;	return; }
	else if (get_button(BTN0))
	{
		speed += 0.05;
	if (speed > 1.01)				{ speed = ratio_base; }
}

led7(speed*100);

sensor_cmp(0xff);
	}
}

//========================START==========================
void sel_mode()
{
	handle(0);
	speed(0,0,0);
	ratio_base = ratio_default;
	while(1)
	{

		led7(ratio_base*100);
		sensor_cmp(0xff);
		if(get_button(BTN0))		return;
		else if (get_button(BTN1))	test_hardware();
		else if (get_button(BTN2))	learn_color();
	}
}


