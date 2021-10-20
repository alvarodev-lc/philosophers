/*
 * File:   ensayo-1.c
 * Author: norberto
 *
 * Created on 2 de octubre de 2021, 13:29
 */

// DSPIC33FJ128MC802 Configuration Bit Settings

// 'C' source line config statements

// FBS
#pragma config BWRP = WRPROTECT_OFF     // Boot Segment Write Protect (Boot Segment may be written)
#pragma config BSS = NO_FLASH           // Boot Segment Program Flash Code Protection (No Boot program Flash segment)
#pragma config RBS = NO_RAM             // Boot Segment RAM Protection (No Boot RAM)

// FSS
#pragma config SWRP = WRPROTECT_OFF     // Secure Segment Program Write Protect (Secure segment may be written)
#pragma config SSS = NO_FLASH           // Secure Segment Program Flash Code Protection (No Secure Segment)
#pragma config RSS = NO_RAM             // Secure Segment Data RAM Protection (No Secure RAM)

// FGS
#pragma config GWRP = OFF               // General Code Segment Write Protect (User program memory is not write-protected)
#pragma config GSS = OFF                // General Segment Code Protection (User program memory is not code-protected)

// FOSCSEL
#pragma config FNOSC = PRI              // Oscillator Mode (Primary Oscillator (XT, HS, EC))
#pragma config IESO = OFF               // Internal External Switch Over Mode (Start-up device with user-selected oscillator source)

// FOSC
#pragma config POSCMD = XT              // Primary Oscillator Source (XT Oscillator Mode)
#pragma config OSCIOFNC = OFF           // OSC2 Pin Function (OSC2 pin has clock out function)
#pragma config IOL1WAY = ON             // Peripheral Pin Select Configuration (Allow Only One Re-configuration)
#pragma config FCKSM = CSECMD           // Clock Switching and Monitor (Clock switching is enabled, Fail-Safe Clock Monitor is disabled)

// FWDT
#pragma config WDTPOST = PS16384        // Watchdog Timer Postscaler (1:16,384)
#pragma config WDTPRE = PR128           // WDT Prescaler (1:128)
#pragma config WINDIS = OFF             // Watchdog Timer Window (Watchdog Timer in Non-Window mode)
#pragma config FWDTEN = OFF             // Watchdog Timer Enable (Watchdog timer enabled/disabled by user software)

// FPOR
#pragma config FPWRT = PWR128           // POR Timer Value (128ms)
#pragma config ALTI2C = OFF             // Alternate I2C  pins (I2C mapped to SDA1/SCL1 pins)
#pragma config LPOL = ON                // Motor Control PWM Low Side Polarity bit (PWM module low side output pins have active-high output polarity)
#pragma config HPOL = ON                // Motor Control PWM High Side Polarity bit (PWM module high side output pins have active-high output polarity)
#pragma config PWMPIN = OFF             // Motor Control PWM Module Pin Mode bit (PWM module pins controlled by PWM module at device Reset)

// FICD
#pragma config ICS = PGD1               // Comm Channel Select (Communicate on PGC1/EMUC1 and PGD1/EMUD1)
#pragma config JTAGEN = OFF             // JTAG Port Enable (JTAG is Disabled)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.


#include "xc.h"
#include "AuK_v1_1_6.h"
#include <libpic30.h>
#include <stdio.h>

Tsemaphore Tenedor[5];
Tmutex impresora;

void init_semaphores(void)
{
    int x;
    for (x = 0; x<5; x++){
        init_semaphore(&Tenedor[x], 1);
    }
}

void init_micro(void)
{
    RCONbits.SWDTEN = 0; // Disable Watchdog Timer

    // Configure Oscillator to operate the device at 40 Mhz
    // Fosc = Fin*M/(N1*N2), Fcy = Fosc/2
    // Fosc = 7.3728*43/(2*2) = 79.2576 Mhz
    // Fcy = Fosc/2 = 39.6288 MHz

    // Configure PLL prescaler, PLL postscaler and PLL divisor

    PLLFBDbits.PLLDIV = 41; // M = PLLDIV + 2 = 43 -> PLLDIV = 43 - 2 = 41
    CLKDIVbits.PLLPOST = 0; // N2 = 2 (Output/2)
    CLKDIVbits.PLLPRE = 0; // N1 = 2 (Input/2)
    
    // clock switching to incorporate PLL
    __builtin_write_OSCCONH(0x03); // Initiate Clock Switch to Primary
    __builtin_write_OSCCONL(0x01); // Start clock switching

    while (OSCCONbits.COSC != 0b011); // Wait for Clock switch to occur
    while (OSCCONbits.LOCK != 1) {}; // Wait for PLL to lock (If LOCK = 1 -> PLL start-up timer is satisfied)

}

void init_ports(void)
{
    /* All possible analog bits are configured as digital */
    AD1PCFGL = 0xFFFF;
    
    /* Set TRISx registers for uart */
    TRISBbits.TRISB6 = 1; /* RB6 is configured as input (UART1 RX) */
    TRISBbits.TRISB7 = 0; /* RB7 is configured as output (UART1 TX) */
    
}

void init_uart(void)
{
    /* Specified pins for UART1 */
    RPINR18bits.U1RXR = 6; /* Pin RP6 asigned to UART1 RX */
    RPOR3bits.RP7R = 0b00011; /* UART1 TX asigned to RP7 */
    
    U1MODEbits.USIDL = 1;   // Stop on idle mode
    U1MODEbits.IREN = 0;    // disable IrDA operation
    U1MODEbits.UEN = 0;     // Only RX and TX are used (non CTS, RTS and BCLK)
    U1MODEbits.WAKE = 0;    // Wake up on start bit is disabled
    U1MODEbits.LPBACK = 0;  // Loopback mode disabled
    U1MODEbits.ABAUD = 0;   // No automatic baud rate
    U1MODEbits.URXINV = 0;  // Non polarity inversion. Idle state is 1
    U1MODEbits.BRGH = 0;    // High baude rate disabled
    U1MODEbits.PDSEL = 0;   // 8 bit data with no parity
    U1MODEbits.STSEL = 0;   // One stop bit.
    
    U1STAbits.URXISEL = 0;  // Interrupt on each character received
    
    U1BRG = 257; // 9600 Baudios. 39.6288*10**6/(16*9600) - 1
    
    /* In this configuration uart interrupts are not allowed */
    IPC2bits.U1RXIP = 0; 
    IFS0bits.U1RXIF = 0;
    IEC0bits.U1RXIE = 0;
    
    U1MODEbits.UARTEN = 1; // Enable UART operation
    U1STAbits.UTXEN = 1;    // Enable uart1 TX (must be done after UARTEN)
    
    /* It is needed to wait one transmision bit to ensure start bit detection 
     When 9600 Baud rate is selected it is necessary to wait 104 us */
    __delay32(4122);
}

void printer(char *texto)
{
    mutex_lock(&impresora);
    printf(texto);
    mutex_unlock(&impresora);
}

void filo1(void)
{
    while(1)
    {
        printer("Filo1: Estoy durmiendo \r\n");
        printer("Filo1: Estoy intentando comer \r\n");
        wait(&Tenedor[0]);
        wait(&Tenedor[1]);
        printer("Filo1: Estoy comiendo con tenedores 0 y 1\r\n");
        signal(&Tenedor[0]);
        signal(&Tenedor[1]);
        printer("Filo1: He terminado de usar tenedores 0 y 1\r\n");
        printer("Filo1: Estoy meditando\r\n");
    }
}

void filo2(void)
{
    while(1)
    {
        printer("Filo2: Estoy durmiendo \r\n");
        printer("Filo2: Estoy intentando comer \r\n");
        wait(&Tenedor[1]);
        wait(&Tenedor[2]);
        printer("Filo2: Estoy comiendo con tenedores 1 y 2\r\n");
        signal(&Tenedor[1]);
        signal(&Tenedor[2]);
        printer("Filo2: He terminado de usar tenedores 1 y 2\r\n");
        printer("Filo2: Estoy meditando\r\n");
    }
}

void filo3(void)
{
    while(1)
    {
        printer("Filo3: Estoy durmiendo \r\n");
        printer("Filo3: Estoy intentando comer \r\n");
        wait(&Tenedor[2]);
        wait(&Tenedor[3]);
        printer("Filo3: Estoy comiendo con tenedores 2 y 3\r\n");
        signal(&Tenedor[2]);
        signal(&Tenedor[3]);
        printer("Filo3: He terminado de usar tenedores 2 y 3\r\n");
        printer("Filo3: Estoy meditando\r\n");
    }
}

void filo4(void)
{
    while(1)
    {
        printer("Filo4: Estoy durmiendo \r\n");
        printer("Filo4: Estoy intentando comer \r\n");
        wait(&Tenedor[3]);
        wait(&Tenedor[4]);
        printer("Filo4: Estoy comiendo con tenedores 3 y 4\r\n");
        signal(&Tenedor[3]);
        signal(&Tenedor[4]);
        printer("Filo4: He terminado de usar tenedores 3 y 4\r\n");
        printer("Filo4: Estoy meditando\r\n");
    }
}

void filo5(void)
{
    while(1)
    {
        printer("Filo5: Estoy durmiendo \r\n");
        printer("Filo5: Estoy intentando comer \r\n");
        wait(&Tenedor[0]);
        wait(&Tenedor[4]);
        printer("Filo5: Estoy comiendo con tenedores 0 y 4\r\n");
        signal(&Tenedor[0]);
        signal(&Tenedor[4]);
        printer("Filo5: He terminado de usar tenedores 0 y 4\r\n");
        printer("Filo5: Estoy meditando\r\n");
    }
}

int main(void) 
{
    mutex_init(&impresora, 1);
    init_semaphores();
    
    init_micro();
    init_ports();
    init_uart();
    
    init_task_manager();
    
    create_task((unsigned int)&filo1, 200, 1);
    create_task((unsigned int)&filo2, 200, 1);
    create_task((unsigned int)&filo3, 200, 1);
    create_task((unsigned int)&filo4, 200, 1);
    create_task((unsigned int)&filo5, 200, 1);
    
    idle_task();
    
    return 0;
}
