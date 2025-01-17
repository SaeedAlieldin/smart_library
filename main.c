/*
 * Library_lightingControl.c
 * Created:4/1/2021 03:53:40
 */

/* ******* before main() function ********************************************************************************************** */
/* File Inclusion */
#include <mega16.h>
#include <delay.h>
#include <alcd.h>     // for LCD

/* Functions Prototype */
void init_ADC(void);
unsigned int read_ADC (unsigned char channel);

/* Global Variables Initialization */
int students_no;    // Number of Students Inside Library
unsigned char hrs, mins, secs, clk_overflow_counter;   // LCD Clock Variables (Hours, Minutes, Seconds) & Timer2 Overflow Counter


/* ********** main() function ************************************************************************************************* */
void main(void)
{
    /* Local Variables Initialization */
    unsigned int temperatureRead_c, temp_adc;   // Temperature Reading in Celsius and ADC conversion of temperature
    unsigned char temperatureSensor_channel = 3;  // Temperature Sensor (LM35) pin channel for ADC Conversion (A3)
    unsigned char temperatureSensor = PINA3;  //Naming Temperature Sensor Pin
    unsigned char led = PINC3;    // Naming LED Pin
    unsigned char IR_enter_door = PIND2, IR_exit_door = PIND3;   // Naming IR Sensors Pins (Enter Door D2 & Enter Door D3)
    unsigned char lights1 = PINB0, lights2 = PINB1, lights3 = PINB2;    // Naming Lights Pins (B0, B1, B2)
    unsigned char AC1 = PINB4, AC2 = PINB5, heater1 = PINB6, heater2 = PINB7;  // Naming Air-conditioniners and heaters Pins
    
    /* Setting Inputs and Outputs */
    DDRC |= (1<<led);  // Setting LED Pin as an Output
    DDRB |= (1<<lights1) | (1<<lights2) | (1<<lights3);  // Setting Lights Pins as Output
    DDRA &= ~(1<<temperatureSensor);   // Temperature Sensor Pin is an Input by default
    DDRB |= (1<<AC1) | (1<<AC2) | (1<<heater1) | (1<<heater2);  // Setting Air-conditioners and Heaters Pins as Outputs
       
    #asm("sei");  // Enables Global Interrupt(I-bit in SREG)
    
    /* Enabling Timer/Counter2 Interrupt for Clock Code */
    // Timer/Counter2 is in normal mode by default
    TIMSK |= (1<<TOIE2);   // Enables Overflow Interrupt
    TCCR2 |= (1<<CS22);    // Interrupt Prescalar clk/64 
    
    /* Enabling External Interrupts for IR sensors */
    PORTD |= (1<<IR_enter_door) | (1<<IR_exit_door);   // Enabling PULL-UP Resistors for IR Sensors(External Interrupts)
    GICR |=  (1<<INT0) | (1<<INT1); // Enables External Interrupts (INT0 and INT1) for IR Sensors
    MCUCR |= (1<<ISC01);  // Falling Edge of INT0 generates an interrupt request
    MCUCR |= (1<<ISC11);  // Falling Edge of INT1 generates an interrupt request
    
    /* ADC Initialization */
    init_ADC();
    
    /* LCD Initialization */    
    lcd_init(16);
    
    
    /* *********** Infinite Loop ****************************************************************************************************/
    while (1)
    {
        /* ****** Temperature Calculation ***************************************************************** */
        
        temp_adc = read_ADC(temperatureSensor_channel);  //wrong value
        temperatureRead_c = (temp_adc*5/255)*100;        //wrong value
        delay_ms(100);
        
        /* ****** Displaying Time, Temperature(C) and Students Number on LCD ****************************** */        
        lcd_printf("%u:%u:%u", hrs, mins, secs);    // Clock Format HH:MM:SS (top left)
        lcd_gotoxy(0,1); lcd_printf("%u", temperatureRead_c);    // Temperature in Celsius (bottom left)
        
        /* Number of students Inside library (bottom right of LCD) */
        if(students_no >= 25)
        {
            lcd_gotoxy(11,1); lcd_printf("FULL!");    // Display "FULL!" When there are 25 or more students
            PORTC |=  (1<<led);     // Turn On Red LED
        }
        else if (students_no > 0)
        {
            lcd_gotoxy(5,1); lcd_printf("%u students", students_no);  // Display the Number of Students Inside Library
            PORTC &= ~(1<<led);    // Turn Off Red LED
        }
        else  // When There are No Students
        {
            students_no = 0;
            lcd_gotoxy(5,1); lcd_printf("%u students", students_no);
        }
        
        /* ******* Lights Control *********************************************** */
        if (students_no <= 10 && students_no > 0)
        {
            PORTB |=  (1<<lights1);          //Lights group 1 ON
            PORTB &= ~(1<<lights2);          //Lights group 2 OFF
            PORTB &= ~(1<<lights3);          //Lights group 3 OFF
        }
        else if (students_no <= 20 && students_no > 10)
        {
            PORTB |=  (1<<lights1);          //Lights group 1 ON
            PORTB |=  (1<<lights2);          //Lights group 2 ON
            PORTB &= ~(1<<lights3);          //Lights group 3 OFF
        }
        else if (students_no > 20)
        {
            PORTB |= (1<<lights1);           //Lights group 1 ON
            PORTB |= (1<<lights2);           //Lights group 2 ON
            PORTB |= (1<<lights3);           //Lights group 3 ON
        }
        else
        {
            PORTB &= ~(1<<lights1);          //Lights group 1 OFF
            PORTB &= ~(1<<lights2);          //Lights group 2 OFF
            PORTB &= ~(1<<lights3);          //Lights group 3 OFF
        }
        
        /* ****** Air-Conditioners and Heaters ********************************* */
        if (temperatureRead_c <= 10 && temperatureRead_c >= 0 )  
        {
            PORTB |=  (1<<heater1);   // Heater 1 is ON
            PORTB |=  (1<<heater2);   // Heater 2 is ON
            PORTB &= ~(1<<AC1);       // Air-conditioner 1 is OFF
            PORTB &= ~(1<<AC2);       // Air-conditioner 2 is OFF
        }
        else if (temperatureRead_c > 10 && temperatureRead_c <= 20)
        {
            PORTB |=  (1<<heater1);   // Heater 1 is ON
            PORTB &= ~(1<<heater2);   // Heater 2 is OFF
            PORTB &= ~(1<<AC1);       // Air-conditioner 1 is OFF
            PORTB &= ~(1<<AC2);       // Air-conditioner 2 is OFF
        }
        else if (temperatureRead_c > 20 && temperatureRead_c <= 30)
        {
            PORTB &= ~(1<<heater1);   // Heater 1 is OFF
            PORTB &= ~(1<<heater2);   // Heater 2 is OFF
            PORTB &= ~(1<<AC1);       // Air-conditioner 1 is OFF
            PORTB &= ~(1<<AC2);       // Air-conditioner 2 is OFF
        }
        else if (temperatureRead_c > 30 && temperatureRead_c <= 40)
        {
            PORTB &= ~(1<<heater1);   // Heater 1 is OFF
            PORTB &= ~(1<<heater2);   // Heater 2 is OFF
            PORTB |=  (1<<AC1);       // Air-conditioner 1 is ON
            PORTB &= ~(1<<AC2);       // Air-conditioner 2 is OFF
        }
        else if (temperatureRead_c > 40 && temperatureRead_c <= 50)
        {
            PORTB &= ~(1<<heater1);   // Heater 1 is ON
            PORTB &= ~(1<<heater2);   // Heater 2 is OFF
            PORTB |=  (1<<AC1);       // Air-conditioner 1 is ON
            PORTB |=  (1<<AC2);       // Air-conditioner 2 is ON
        }
        
        /* LCD Refresh Rate */
        delay_ms(1000);  
        lcd_clear();
    }
}


/* ********** after main() function *************************************************************************************************/
/* Timer/Counter2 Overflow Interrupt Service Routine */
interrupt [5] void Clock (void)
{
    clk_overflow_counter++;
    /* 
     * Microcontroller Frequency = 1MHz, Prescalar = 64, Timer2 is 8bit => 256 steps 
     * Frequency / (prescalar * 256 steps) = 61.035 overflows/sec
     */
    if (clk_overflow_counter >= 61)
    {
        secs++; clk_overflow_counter = 0;
        if (secs >= 60)
        {
            mins++; secs=0;
            /*
             * to reduce the acccumulated error resulted from the 0.035 overflow/sec
             * we have to add 1 count every 28.5 secs (1/0.035=28.5)
             * using approximation we'll add 2 counts for every 60 secs
             */
            clk_overflow_counter = 2;
            if (mins >= 60)
            {
                hrs++; mins=0;
                if (hrs>=24) hrs=0;
            }
         }
    }
}

/* Interrupt0 (IR Enter Door) Service Routine */
interrupt [2] void ENTER_DOOR (void)
{students_no++;}   // Someone Entered the Library

/* Interrupt1 (IR Exit Door) Service Routine */
interrupt [3] void EXIT_DOOR (void)
{students_no--;}   // Someone Left the Library


/* ********** FUNCTIONS ************************************************************************** */
/* ADC Initialization Function */
void init_ADC (void)
{
    ADCSRA |= (1<<ADEN);    // ADC Enable
    ADMUX  |= (1<<REFS0);   // AVcc with External Capacitor at AREF pin
    ADMUX  |= (1<<ADLAR);   // Left Adjust the ADC Result
    ADCSRA |= (1<<ADPS0) | (1<<ADPS1) | (1<<ADPS2);  // Prescalar = 128, 1MHz/128=7812.5    
}

/* ADC Function */
unsigned int read_ADC (unsigned char channel)
{
    channel &= 0b00000111;  // Select ADC Channel Between 0 to 7
    ADMUX &= 0xF0;   // Clear Older Channel That was Read Before
    ADMUX |= channel;   // Selecting Where The Analog Input will be Connected for Digital Conversion
    delay_us(10);    // Wait Till Channel is Read
    ADCSRA |= (1<<ADSC);  // Start AD Conversion
    while((ADCSRA & (1<<ADIF)) == 0);	/* Monitor end of conversion interrupt */
	delay_us(10);
    return ADCH;  // Return Calculated ADC value (8bit not 10bit)
}