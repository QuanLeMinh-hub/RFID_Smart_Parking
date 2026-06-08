#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/eeprom.h>

// K?t n?i LCD
#define LCD_DR DDRB
#define LCD_OUT PORTB
#define LCD_RS PB0
#define LCD_RW PB1
#define LCD_EN PB2
#define LCD_D4 PB4
#define LCD_D5 PB5
#define LCD_D6 PB6
#define LCD_D7 PB7

unsigned char c_flag=0;
unsigned char key=0;

void gpio_set_up(void);
void init_lcd(void);
void power_reset_lcd(void);
void lcd_out4(unsigned char data1, char rs1);
void lcd_out4_2(unsigned char data2, char rs2);
void KEY_RD(void);
void SCAN_LED(void);
unsigned char DtoH(unsigned char data);

int main(void)
{
	unsigned char tmp=0;
	gpio_set_up();
	power_reset_lcd();
	init_lcd();
	tmp= eeprom_read_byte((unsigned char*)0x00);
    while (1)
    {
		lcd_out4_2(tmp,1);
		KEY_RD();
		tmp=DtoH(key);
		eeprom_write_byte((unsigned char*)0x00, tmp);
		lcd_out4_2(0x01,0);
		_delay_ms(2);
		
    }
}

void gpio_set_up(void)
{
	LCD_DR=0xFF;
	LCD_OUT=0xF0;
	DDRA=0x0F;
	PORTA=0xFF;
	DDRC = 0xFF;   // Toàn b? PORTC là input
	PORTC = 0x00;
}

void power_reset_lcd(void)
{
	_delay_ms(20);
	lcd_out4(0x30,0);
	_delay_ms(5);
	lcd_out4(0x30,0);
	_delay_us(200);
	lcd_out4_2(0x32,0);
}

void lcd_out4(unsigned char data1, char rs1)
{
	LCD_OUT=data1;
	if(rs1==1) LCD_OUT|=(1<<LCD_RS);
	else LCD_OUT&=~(1<<LCD_RS);
	LCD_OUT&= ~(1 << LCD_RW);
	LCD_OUT|=(1<<LCD_EN);
	_delay_us(1);
	LCD_OUT&=~(1<<LCD_EN);
	_delay_us(100);
}

void lcd_out4_2(unsigned char data2, char rs2)
{
	unsigned char tmp;
	tmp=data2&(0xF0);
	lcd_out4(tmp,rs2);
	tmp=(data2<<4)&(0xF0);
	lcd_out4(tmp,rs2);
}

void init_lcd(void)
{
	lcd_out4_2(0x28,0);
	_delay_us(100);
	lcd_out4_2(0x0C,0);
	_delay_us(100);
	lcd_out4_2(0x01,0);
	_delay_ms(2);
	lcd_out4_2(0x06,0);
	_delay_us(100);
}

void KEY_RD(void)
{
	do {
		SCAN_LED();
	} while (c_flag == 0);  // ch? nh?n

	do {
		SCAN_LED();
	} while (c_flag == 1);  // ch? nh?
		
}

void SCAN_LED(void)
{
	c_flag=0;
	signed int cnt=3;
	char code=0xFE;
	for(;cnt>=0;cnt--)
	{
		PORTA=code;
		int tmp=PINA&0xF0;
		tmp=PINA&0xF0;
		if(tmp==0xF0)
		{
			code=(code<<1)|1;
			continue;
		}
		else
		{
			c_flag=1;
			tmp=tmp>>4;
			int n=0;
			while((tmp&1)!=0)
			{
				n++;
				tmp=tmp>>1;
			}
			cnt=3-cnt;
			key=n*4+cnt;
			return;
		}
	}
}

unsigned char DtoH(unsigned char data)
{
	if (data < 0x0A)
	return data + 0x30;  // '0'–'9'
	else
	return data + 0x37;  // 'A'–'F'
}

//hàm ðo