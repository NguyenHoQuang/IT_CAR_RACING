//NguyenHoQuang
//Update lan cuoi 03/05/2018
#include "function.h"

inline void case_10(void);
inline void case_11(void);
inline void case_12(void);
inline void case_20(void);
inline void case_21(void);
inline void case_31(void);
inline void case_41(void);
inline void case_51(void);
inline void case_52(void);
inline void case_61(void);
inline void case_62(void);

bool control_speed(void);

int check_line(void);
int check_line_while_turn(void);
bool check_rightline(void);
bool check_leftline(void);

uint8_t  pattern;
uint16_t cnt, cnt_temp, pulse_v;
extern uint16_t pulse_ratio;
extern uint16_t velocity;

int state = -1;

int percent_run = 100;

int8_t vitri = 0;

uint8_t cnt_stop = 0,
		cnt_led = 0,
		t;
		
uint16_t slow = 0;

bool check = true;

#define PERCENT_TURN		100		// percent of turn left right									
#define PERCENT_CHANGE_LINE	50		// percent of motor when change line
#define PERCENT_NO_LINE		30

#define time_checkline		50		// delay (ms) waiting check line


int main(void)
{
	INIT();
	sel_mode();
	///////////////////////////////////////////////////////////////////////
	
	get_speed();

	///////////////////////////////////////////////////////////////////////
	
	pattern = 10;
	 
	while(1)
	{				
		switch(pattern)
		{
			case 10:							// Run STRANGE
			{
				state = check_line();		
				if (state == 0)
				{
					cnt = 0;
					pattern = 20;
					break;
				}
				else if (state == 1)
				{
					cnt = 0;
					pattern = 51;
					break;
				}
				else if (state == 2)
				{
					cnt = 0;
					pattern = 61;
					break;
				}
 	
				led7(10);
				case_10();
				break;				
			}							
			
			case 11:							// Turn RIGHT
			{
				if (check == true)
				{
					state = check_line();
					if (state == 0)
					{
						cnt = 0;
						pattern = 20;
						break;
					}
					else if (state == 1)
					{
						cnt = 0;
						pattern = 51;
						break;
					}
					else if (state == 2)
					{
						cnt = 0;
						pattern = 61;
						break;
					}
				}
				led7(11);
				case_11();
				break;
			}
			
			case 12:							// Turn LEFT
			{
				if (check == true)
				{
					state = check_line();
					if (state == 0)
					{
						cnt = 0;
						pattern = 20;
						break;
					}
					else if (state == 1)
					{
						cnt = 0;
						pattern = 51;
						break;
					}
					else if (state == 2)
					{
						cnt = 0;
						pattern = 61;
						break;
					}
				}
				led7(12);
				case_12();
				break;
			}	
			
			/***** Turn 90 degree & No line ******/			
			
			case 20:							// CROSS line waiting
			{
				if ( check_rightline() )
				{
					cnt = 0;
					pattern = 31;
					break;
				}
				if ( check_leftline() )
				{
					cnt = 0;
					pattern = 41;
					break;
				}
				if (sensor_cmp(0xFF) == 0x00)
				{
					cnt = 0;
					handle(0);
					speed(100,100, PERCENT_NO_LINE);
					pattern = 21;
					break;
				}
				
				led7(20);
				case_20();
				break;
			}
			
			case 21:			// NO line
			{
				led7(21);
				case_21();
				break;
			}
			
			case 31:			// Turn 90 degree RIGHT
			{
				led7(31);
				case_31();
				break;
			}
						 
			case 41:			// Turn 90 degree LEFT
			{
				led7(41);
				case_41();
				break;
			}
						
			/************ CHANGE LINE ************/			
			
			case 51:							// RIGHT line waiting
			{
				led7(51);
				case_51();
				break;
			}
						
			case 52:							// Change RIGHT line
			{
				led7(52);
				case_52();
				break;
			}
						
			case 61:							// LEFT line waiting
			{
				led7(61);
				case_61();
				break;
			}
						
			case 62:							// Change LEFT line
			{
				led7(62);
				case_62();	
				break;
			}				
			
			case 100:
			{
				fast_brake();
				break;
			}
 		}
	}
}		

/************** INTERRUPT FUNTION *******************/
ISR(TIMER0_COMP_vect)
{
	cnt++;
	cnt_temp++;
	
	cnt_led++;
	print();			//Quét LED7 doan
}
ISR(INT0_vect) 
{
}

/*************** CHECK LINE FUNCTION ****************/
int check_line()
{
	/*************** CHECK RIGHT LINE ****************/
	t = sensor_cmp(0xFF);
	if( (t==0b00111111) || (t==0b00011111) || (t==0b00001111) || (t==0b00000111) )
	{
		cnt_temp = 0;
		while(cnt_temp <= time_checkline)
		{
			switch (sensor_cmp(0b11111111))
			{
				case 0b11111111:
				_delay_ms(time_checkline);
				return 0;
				break;
				
				case 0b01100000:
				case 0b00110000:
				case 0b00011000:
				case 0b00001100:
				case 0b00000110:
				return 1;
				break;
			}
		}
	}
	/*************** CHECK LEFT LINE ****************/
	if( (t==0b11100000) || (t==0b11110000) || (t==0b11111000) || (t==0b11111100) )
	{
		cnt_temp = 0;
		while(cnt_temp <= time_checkline)
		{
			switch (sensor_cmp(0b11111111))
			{
				case 0b11111111:
				_delay_ms(time_checkline);
				return 0;
				break;
				
				case 0b01100000:
				case 0b00110000:
				case 0b00011000:
				case 0b00001100:
				case 0b00000110:
				return 2;
				break;
			}
		}
	}
	/*************** CHECK CROSS LINE ****************/
	if( t==0b11111111 )
	{
		_delay_ms(time_checkline);
		return 0;
	}
	
	return -1;
}

bool check_rightline()
{   
	t = sensor_cmp(0b00111111);
	if( (t==0b00111111) || (t==0b00011111) || (t==0b00001111) || (t==0b00000111) )
		{ return true; }
	
	return false;
}

bool check_leftline()
{
	t = sensor_cmp(0b11111100);
	if( (t==0b11100000) || (t==0b11110000) || (t==0b11111000) || (t==0b11111100) ) 
		{ return true; }
	
	return false;
}

/*************** CONTROL FUNCTION ******************/
			
void case_10()			// Run STRANGE
{
	switch(sensor_cmp(0b11111111))
	{
		case 0b00011000:
		handle(0);
		speed(100,100, 100);
		break;
		
		/******************************/			//lech trai 1
		case 0b00001000:
		handle(3);
		speed(100,100, 100);
		break;
		
		case 0b00010000:							//lech phai 1
		handle(-3);
		speed(100,100, 100);
		break;
		
		/******************************/			//lech trai 2
		case 0b00001100:
		handle(5);
		speed(100,100, 100);
		break;

		case 0b00110000:							//lech phai 2
		handle(-5);
		speed(100,100, 100);
		break;
		
		/******************************/			//lech trai 3
		case 0b00000100:
		handle(7);
		speed(100,90, 95);
		break;
		
		case 0b00100000:							//lech phai 3
		handle(-7);
		speed(90,100, 95);
		break;
		
		/******************************/			//lech trai goc lon
		case 0b00000011:
		case 0b00000010:
		case 0b00000110:		
		handle(10);
		speed(100,80, 90);
		slow = cnt;
		cnt = 0;
		pulse_v = 0;
		pattern = 11;
		break;
		
		/******************************/			//lech phai goc lon
		case 0b11000000:
		case 0b01000000:
		case 0b01100000:
		handle(-10);
		speed(80,100, 90);
		slow = cnt;
		cnt = 0;
		pulse_v = 0;
		pattern = 12;
		break;
	}
}

bool control_speed()
{ 
	if (slow > 1700) {
		if (cnt < 100)					{ speed(100,100, -70);	return false; }
		else							{ percent_run = 80;		return true; }
	}		
}

void case_11()			// Turn RIGHT
{
	switch (sensor_cmp(0xFF))
	{
		case 0b11000000:
		case 0b10000000:
		handle(30);
		speed(100,0, 100);
		fast_brake_right();
		check = false;
		break;
		
		case 0b00000001:
		case 0b10000001:
		case 0b10000011:
		case 0b11000001:
		handle(25);
		speed(100,0, 100);
		fast_brake_right();
		check = false;
		break;
		
		case 0b00000011:
		handle(20);
		if ( control_speed() )	{ speed(100,50, percent_run); }
		check = false;
		break;
	
		case 0b00000010:
		handle(15);
		if ( control_speed() )	{ speed(100,50, percent_run); }
		check = false;
		break;

		case 0b00000110:
		handle(13);
		if ( control_speed() )	{ speed(100,70, percent_run); }				
		check = true;
		break;

		case 0b00000100:
		handle(10);
		if ( control_speed() )	{ speed(100,90, percent_run); }		
		check = true;
		break;
		
		case 0b00001100:
		handle(7);
		if ( control_speed() )	{ speed(100,90, percent_run); }	
		check = true;
		break;
		
		case 0b00001000:		
		case 0b00011000:
		case 0b00010000:
		handle(5);
		speed(100,100, percent_run);
		check = true;
		cnt = 0;
		pulse_v = 0;
		pattern = 10;
		break;
		
		case 0b11111000:
		case 0b11110000:
		case 0b11100000:
		handle(30);
		speed(100,-50, 100);
		fast_brake_right();
		check = false;
		break;
	}
}

void case_12()			// Turn LEFT
{	
	switch(sensor_cmp(0b11111111))
	{		
		case 0b00000011:
		case 0b00000001:
		handle(-30);
		speed(0,100, 100);
		fast_brake_left();
		check = false;
		break;
		
		case 0b10000000:
		case 0b10000001:
		case 0b11000001:
		case 0b10000011:
		handle(-25);
		speed(0,100, 100);
		fast_brake_left();
		check = false;
		break;
		
		case 0b11000000:
		handle(-20);
		if ( control_speed() )	{ speed(50,100, percent_run); }
		check = false;
		break;

		case 0b01000000:
		handle(-15);
		if ( control_speed() )	{ speed(50,100, percent_run); }		
		check = false;
		break;
		
		case 0b01100000:
		handle(-13);
		if ( control_speed() )	{ speed(70,100, percent_run); }		
		check = true;
		break;

		case 0b00100000:
		handle(-10);
		if ( control_speed() )	{ speed(90,100, percent_run); }
		check = true;
		break;
		
		case 0b00110000:
		handle(-7);
		if ( control_speed() )	{ speed(90,100, percent_run); }		
		check = true;
		break;
		
		case 0b00010000:
		case 0b00011000:
		case 0b00001000:
		handle(-5);
		speed(100,100, percent_run);
		check = true;
		cnt = 0;
		pulse_v = 0;
		pattern = 10;
		break;
		
		case 0b00011111:
		case 0b00001111:
		case 0b00000111:
		handle(-30);
		speed(-50,100, 100);
		fast_brake_left();
		check = false;
		break;
	}
}

void case_20()			// CROSS line waiting
{
	if (cnt < 100)			{ percent_run = -70; }
	else						{ percent_run = 40; }
	switch(sensor_cmp(0b11111111))
	{
		case 0b00011000:
		handle(0);
		speed(100,100, percent_run);
		break;
	
		/******************************/			//lech trai 1
		case 0b00001000:
		handle(2);
		speed(100,100, percent_run);
		vitri = 1;
		break;
	
		case 0b00010000:							//lech phai 1
		handle(-2);
		speed(100,100, percent_run);
		vitri = -1;
		break;
	
		/******************************/			//lech trai 2
		case 0b00001100:
		handle(3);
		speed(100,100, percent_run);
		vitri = 2;
		break;
	
		case 0b00110000:							//lech phai 2
		handle(-3);
		speed(100,100, percent_run);
		vitri = -2;
		break;
	
		/******************************/			//lech trai 3
		case 0b00000100:
		handle(5);
		speed(100,100, percent_run);
		vitri = 3;
		break;
	
		case 0b00100000:							//lech phai 3
		handle(-5);
		speed(100,100, percent_run);
		vitri = -3;
		break;
	
		/******************************/
	
		case 0b00000110:
		handle(7);
		speed(100, 100, percent_run);
		vitri = -4;
		break;
	
		case 0b01100000:
		handle(-7);
		speed(100,100, percent_run);
		vitri = -4;
		break;
	
		/******************************/
	
		case 0b00000010:
		handle(15);
		speed(100,100, percent_run);
		vitri = 5;
		break;
	
		case 0b01000000:
		handle(-15);
		speed(100,100, percent_run);
		vitri = -5;
		break;
	
		/******************************/
	
		case 0b00000011:
		case 0b11000000:
		case 0b10000001:
		if (vitri > 0)
		{
			handle(30);
			speed(100,100, percent_run);
		}
		if (vitri < 0)
		{
			handle(-30);
			speed(100,100, percent_run);
		}
		break;
	}
}

void case_21()			// NO line
{
	speed(100,100, PERCENT_NO_LINE);	
		
	switch(sensor_cmp(0xFF))
	{
		case 0b01100000:
		case 0b00110000:
		case 0b00011000:
		case 0b00001100:
		case 0b00000110:
		pattern = 10;
		break;
		
		case 0b10000000:
		handle(10);
		cnt_temp = 0;
		while(cnt_temp < 100) {
			while(sensor_cmp(0xFF)!=0b10000000) {
				t = sensor_cmp(0xFF);
				if ( (t==0x00) )								break;
				if ( (t==0b10000001) || (t==0b00000011) )		{ handle(-25);	break;}
			}
			break;
		}
		break;
		
		case 0b11000000:
		handle(10);
		cnt_temp = 0;
		while(cnt_temp < 100) {
			while(sensor_cmp(0xFF)!=0b11000000) {
				t = sensor_cmp(0xFF);
				if ( (t==0b10000000) || (t==0x00) )				break;
				if ( (t==0b10000001) || (t==0b00000011) )		{ handle(-25);	break;}	
			}
			break;	
		}	
		break;	
		
		case 0b00000001:
		handle(-10);
		cnt_temp = 0;
		while(cnt_temp < 100) {
			while(sensor_cmp(0xFF) != 0b00000001) {
				t = sensor_cmp(0xFF);
				if ( (t==0x00) )								break;
				if ( (t==0b10000001) || (t==0b11000000) )		{ handle(25);	break;}
			}
			break;
		}		
		
		case 0b00000011:
		handle(-10);
		cnt_temp = 0;
			while(cnt_temp < 100) {
				while(sensor_cmp(0xFF) != 0b00000011) {
					t = sensor_cmp(0xFF);
					if ( (t==0b00000001) || (t==0x00) )				break;
					if ( (t==0b10000001) || (t==0b11000000) )		{ handle(25);	break;}
			}
			break;
		}
		
		case 0b10000001:
		handle(-10);
		cnt_temp = 0;
		while(cnt_temp < 100) {
			while(sensor_cmp(0xFF) != 0b10000001) {
				t = sensor_cmp(0xFF);
				if ( (t==0b01100000) )							break;
				if ( (t==0b11100000) )							{ handle(25);	break;}
			}
		}		
		break;	
	}
}

void case_31()			// Turn 90 degree RIGHT
{
	while(cnt <= 40)
	{
		handle(45);
		speed(100,-100, 100);
	}
	switch (sensor_cmp(0b11111111))
	{
		case 0b01100000:
		case 0b00110000:
		case 0b00011000:
		case 0b00001100:
		case 0b00000110:
		speed(100,100, 100);
		cnt = 0;
		pulse_v = 0;
		pattern = 10;
		break;

		default:
		handle(45);
		speed(100,-100, 100);
		break;
	}
}

void case_41()			// Turn 90 degree LEFT
{
	while(cnt <= 40)
	{
		handle(-45);
		speed(-100,100, 100);
	}
	switch (sensor_cmp(0b11111111))
	{
		case 0b01100000:
		case 0b00110000:
		case 0b00011000:
		case 0b00001100:
		case 0b00000110:
		speed(100,100, 100);
		cnt = 0;
		pulse_v = 0;
		pattern = 10;
		break;

		default:
		handle(-45);
		speed(-100,100, 100);
		break;
	}
}

void case_51()			// RIGHT line waiting
{	
	switch(sensor_cmp(0b11111111))
	{
		case 0b00000000:
		cnt = 0;
		pulse_v = 0;
		pattern = 52;
		break;

		default:
		handle(5);
		speed(100,100, 80);
		break;
	}	
}			

void case_52()			// Change RIGHT line
{
	switch(sensor_cmp(0b11111111))
	{
		case 0b01100000:
		case 0b00110000:
		case 0b00011000:
		case 0b00001100:
		case 0b00000110:
		cnt = 0;
		pulse_v = 0;
		pattern = 10;
		break;

		default:
		handle(5);
		speed(100,100, 80);
		break;
		
	}
}

void case_61()			// LEFT line waiting
{
	switch(sensor_cmp(0b11111111))
	{
		case 0b00000000:
		cnt = 0;
		pulse_v = 0;
		pattern = 62;
		break;
		
		default:
		handle(-5);
		speed(100,100,80 );
		break;
	}		
}

void case_62()			// Change LEFT line
{
	switch(sensor_cmp(0b11111111))
	{		
		case 0b00000110:		
		case 0b00001100:
		case 0b00011000:
		case 0b00110000:
		case 0b01100000:
		cnt = 0;
		pulse_v = 0;
		pattern = 10;
		break;
		
		default:
		handle(-5);
		speed(100,100, 80);
		break;	
	}
}
