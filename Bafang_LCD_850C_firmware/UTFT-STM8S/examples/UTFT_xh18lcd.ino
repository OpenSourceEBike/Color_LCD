#include <UTFT.h>


extern uint8_t SmallFont[];


#define POWER_LATCH PD7

#define LCD_LED_1 PD2
#define LCD_LED_2 PD3

#define LCD_CS PA2
#define LCD_RESET PA3
#define LCD_RS PA4
#define LCD_WR PA5
#define LCD_RD PA6



void setup()
{
  // XH18-LCD soft power latching, not related to LCD
  pinMode(POWER_LATCH, OUTPUT);
  digitalWrite(POWER_LATCH, HIGH);

  // Power on display backlight on the lower of two steps
  pinMode(LCD_LED_1, OUTPUT);
  pinMode(LCD_LED_2, OUTPUT);
  digitalWrite(LCD_LED_1, HIGH);
  digitalWrite(LCD_LED_2, LOW);

  // /RD of LCD not used, set it to HIGH
  pinMode(LCD_RD, OUTPUT);
  digitalWrite(LCD_RD, HIGH);
  
  UTFT(ST7735_8, LCD_RS ,LCD_WR ,LCD_CS ,LCD_RESET);

  // LCD of my XH18-LCD had an offset...
  UTFT_offset_x = -1;
  UTFT_offset_y = 2;
  
  UTFT_InitLCD();
  UTFT_setFont(SmallFont);
}

void loop()
{
  // do some nonsense
  UTFT_clrScr();
  UTFT_fillScr(VGA_AQUA);
  UTFT_setColor_rgb(255, 0, 0);
  UTFT_fillRect(0, 0, 100, 50);
  UTFT_setColor_rgb(255, 255, 0);
  UTFT_drawPixel(0,0);
  UTFT_drawPixel(0,10);
  UTFT_drawPixel(10,0);
  UTFT_drawPixel(10,10);
  UTFT_print("Hello world!",10 ,60 );
  UTFT_setColor(VGA_FUCHSIA);
  UTFT_drawLine(5,80,154,80);
  UTFT_drawLine(5,81,154,81);
  delay(5000);
}

