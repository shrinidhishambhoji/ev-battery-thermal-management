#include<LPC17xx.h>
#include<stdio.h>
#include"lcd.h"

int main(void)
{
    unsigned long adc_temp;
    unsigned int i;
    float in_vtg, temperature;
    unsigned char vtg[7], dval[10];
    int fanSpeed;
    unsigned char Msg3[11] = {"Temperature"};
    unsigned char Msg4[14] = {"Fan Speed"};

    SystemInit();
    SystemCoreClockUpdate();

    LPC_SC->PCONP |= (1<<15); // Power for GPIO block
    lcd_init();
    LPC_PINCON->PINSEL1 &= ~(0x11<<14);
    LPC_PINCON->PINSEL1 |= 0x01<<14;  // P0.23 as AD0.0
    LPC_SC->PCONP |= (1<<12);      // Enable the peripheral ADC

    // Configure additional GPIOs
    LPC_PINCON->PINSEL3 &= ~(0x03 << 30); // P1.31 as GPIO
    LPC_GPIO1->FIODIR |= (1 << 31);       // P1.31 as output
   
    LPC_PINCON->PINSEL3 &= ~(0x03 << 28); // P1.30 as GPIO
    LPC_GPIO1->FIODIR |= (1 << 30);       // P1.30 as output
    

    LPC_PINCON->PINSEL1 &= ~(0x03<<16); // P0.24 as GPIO
    LPC_GPIO0->FIODIR |= (1<<24);       // P0.24 as output

    LPC_PINCON->PINSEL1 &= ~(0x03<<18); // P0.25 as GPIO
    LPC_GPIO0->FIODIR |= (1<<25);       // P0.25 as output

    LPC_PINCON->PINSEL1 &= ~(0x03<<20); // P0.26 as GPIO
    LPC_GPIO0->FIODIR |= (1<<26);       // P0.26 as output

    LPC_PINCON->PINSEL0 &= ~(0x03 << 22); // P0.27 as GPIO
    LPC_GPIO0->FIODIR |= (1 << 27);       // P0.27 as output

    LPC_PINCON->PINSEL0 &= ~(0x03 << 24); // P0.28 as GPIO
    LPC_GPIO0->FIODIR |= (1 << 28);       // P0.28 as output

    // Display headers
    temp1 = 0x80;
    lcd_com();
    delay_lcd(800);
    lcd_puts(&Msg3[0]);

    temp1 = 0xC0;
    lcd_com();
    delay_lcd(800);
    lcd_puts(&Msg4[0]);

    while(1)
    {
			 LPC_GPIO1->FIOSET = (1 << 31);        // P1.31 set high initially
       LPC_GPIO1->FIOSET = (1 << 30);        // P1.30 set high initially
        LPC_ADC->ADCR = (1<<0)|(1<<21)|(1<<24);  // ADC0.0, start conversion and operational  
        for(i=0;i<2000;i++);  // Delay for conversion
        while((adc_temp = LPC_ADC->ADGDR) == 0x80000000);  // Wait till 'done' bit is 1, indicates conversion complete
        adc_temp = LPC_ADC->ADGDR;
        adc_temp >>= 4;
        adc_temp &= 0x00000FFF;  // 12 bit ADC
        in_vtg = ((adc_temp * 3.3)/4095 );  // Calculating input analog voltage

        temperature = ((in_vtg / 3.3)*100)+18;

        for(i=0;i<2000;i++);

        // Check if temperature exceeds 40 degrees Celsius
        if (temperature <= 30.0  )
        {
            fanSpeed = 0;
            LPC_GPIO0->FIOCLR = (1<<24);  // Set P0.24 low...fan off
            LPC_GPIO0->FIOCLR = (1<<26);  
            LPC_GPIO0->FIOCLR = (1<<27);
            LPC_GPIO0->FIOCLR = (1<<28);
            LPC_GPIO0->FIOSET = (1<<25);  // Set P0.25 high...white led on
        }
        else
        {
            LPC_GPIO0->FIOCLR = (1<<25);  // white led low 
            LPC_GPIO0->FIOCLR = (1<<24);  // fan off
        }

        if (temperature > 31.0)
        {
            fanSpeed = 33;
            LPC_GPIO0->FIOSET = (1<<24);  // Set P0.24 high...fan on
            LPC_GPIO0->FIOCLR = (1<<25);
            LPC_GPIO0->FIOCLR = (1<<27);
            LPC_GPIO0->FIOCLR = (1<<28);
            LPC_GPIO0->FIOSET = (1<<26);  // Set P0.26 high...yellow led on 
        }
        else
        {
            LPC_GPIO0->FIOCLR = (1<<26);  // yellow led low 
        }

        if (temperature >= 33.0 )
        {
            fanSpeed = 66;
            LPC_GPIO0->FIOSET = (1<<24);  // Set P0.24 high..fan on
            LPC_GPIO0->FIOCLR = (1<<25);
            LPC_GPIO0->FIOCLR = (1<<26);
            LPC_GPIO0->FIOCLR = (1<<28);
            LPC_GPIO0->FIOSET = (1<<27);  // Set P0.27 high...green led on

            // Set P1.31 low
            LPC_GPIO1->FIOCLR = (1 << 31);
        }
        else
        {
            LPC_GPIO0->FIOCLR = (1<<27);  // green led low 
        }

        if (temperature >= 36.0)
        {
            fanSpeed = 99;
            LPC_GPIO0->FIOSET = (1<<24);  // Set P0.24 high...fan on
            LPC_GPIO0->FIOCLR = (1<<25);
            LPC_GPIO0->FIOCLR = (1<<26);
            LPC_GPIO0->FIOCLR = (1<<27);
            LPC_GPIO0->FIOSET = (1<<28);  // Set P0.28 high...red led on

            // Set P1.31 and P1.30 low
            LPC_GPIO1->FIOCLR = (1 << 31);
            LPC_GPIO1->FIOCLR = (1 << 30);
        }
        else
        {
            LPC_GPIO0->FIOCLR = (1<<28);  // red led low 
        }

        for(i=0;i<200000;i++);

        // Display temperature and fan speed
        sprintf(dval, "%.2fC", temperature); // Convert temperature to string
        temp1 = 0x80;
        lcd_com();
        delay_lcd(800);
        lcd_puts(dval);

        sprintf(dval, "FAN: %d%%", fanSpeed); // Convert fan speed to string
        temp1 = 0xC0;
        lcd_com();
        delay_lcd(800);
        lcd_puts(dval);

        // Delay loop
        for (i = 0; i < 200000; i++); // Delay loop
    }
}
