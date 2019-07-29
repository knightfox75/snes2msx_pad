/*  ----------------------------------------------------------------------------
 SNES Game pad auto-fire
 (c)2015 by Cesar Rincon "NightFox"
 File created July 25 2015
 -----------------------------------------------------------------------------*/


// Includes del programa
#include <stdio.h>          // Includes de C
#include <stdlib.h>

#include <xc.h>             // Includes del PIC
#include <htc.h>



// Parametros del PIC
#pragma config BOREN = OFF, MCLRE = OFF, PWRTE = ON, WDTE = OFF, FOSC = INTRCIO
#define _XTAL_FREQ 4000000



// Definiciones propias
#define SW_TIME 500         // Tiempo minimo de pulsacion
#define REPEAT_TIME 2000    // Tiempo de auto repeticion
#define SW_ACTIVE 200       // Tiempo activo
#define BA_ADDR 0x01        // Direccion para guardar los parametros
#define BB_ADDR 0x03


/*
 *      Variables globales
 */
unsigned int cntA = 0;
unsigned int cntB = 0;
unsigned char trigger = 0;
unsigned char eepromData = 0;
signed char buttonA = 0;
signed char buttonB = 0;
unsigned int speed[] = {200, 300, 400, 500, 600, 800, 1000, 1200, 1500, 2000};
unsigned int autoA = 2000;
unsigned int autoB = 2000;
unsigned char oldButtonA = 255;
unsigned char oldButtonB = 255;
unsigned char outA = 0;
unsigned char outB = 0;




/*
 *      Main
 */


void main(void) {

    // Inicializa los puertos
    CMCON = 0x07;               // Desactiva el comparador de los puertos 0 y 1
    TRISIO = 0x0F;              // Ports 0 al 3 IN / 4 y 5 OUT
    GPIO = 0x00;                // Todos los puertos a 0

    // Lee de la eeprom los datos
    eepromData = eeprom_read(BA_ADDR);      // Modo cartucho
    if (eepromData > 9) eepromData = 5;     // Si hay corrupcion de datos
    buttonA = eepromData;                   // Por defecto en modo 5
    oldButtonA = buttonA;

    eepromData = eeprom_read(BB_ADDR);      // Modo mapper
    if (eepromData > 9) eepromData = 5;     // Si hay corrupcion de datos
    buttonB = eepromData;                   // Por defecto en modo 5
    oldButtonB = buttonB;

    // GP0      Button A
    // GP1      Button B
    // GP2      Button L <<<    Less Speed, More Delay
    // GP3      Button R >>>    More Speed, Less Delay
    // GP4      Out Button A
    // GP5      Out Button B

    // Bucle del programa
    while (1) {

        // Reset del autofire
        if ((GP2 == 0) && (GP3 == 0)) {

            buttonA = 5;
            buttonB = 5;

        } else if (GP2 == 0) {          // Si has pulsado L, más lento

            trigger = 1;        // Id de ultimo gatillo L

            if (GP0 == 0) {     // Si has pulsado A
                cntA ++;
                if (cntA > REPEAT_TIME) {
                    cntA = 0;
                    buttonA ++;
                    if (buttonA > 9) buttonA = 9;
                }
            }

            if (GP1 == 0) {     // Si has pulsado B
                cntB ++;
                if (cntB > REPEAT_TIME) {
                    cntB = 0;
                    buttonB ++;
                    if (buttonB > 9) buttonB = 9;
                }
            }

        } else if (GP3 == 0) {  // Si has pulsado R, más rapido

            trigger = 2;        // Id de ultimo gatillo R

            if (GP0 == 0) {     // Si has pulsado A
                cntA ++;
                if (cntA > REPEAT_TIME) {
                    cntA = 0;
                    buttonA --;
                    if (buttonA < 0) buttonA = 0;
                }
            }

            if (GP1 == 0) {     // Si has pulsado B
                cntB ++;
                if (cntB > REPEAT_TIME) {
                    cntB = 0;
                    buttonB --;
                    if (buttonB < 0) buttonB = 0;
                }
            }

        } else {

            // Si ninguno de los dos gatillos está activo
            if (trigger == 1) {

                if (cntA > SW_TIME) {
                    buttonA ++;
                    if (buttonA > 9) buttonA = 9;
                }

                if (cntB > SW_TIME) {
                    buttonB ++;
                    if (buttonB > 9) buttonB = 9;
                }

            } else if (trigger == 2) {

                if (cntA > SW_TIME) {
                    buttonA --;
                    if (buttonA < 0) buttonA = 0;
                }

                if (cntB > SW_TIME) {
                    buttonB --;
                    if (buttonB < 0) buttonB = 0;
                }

            }

            // Reinicia los contadores
            cntA = 0;
            cntB = 0;
            trigger = 0;

        }


        // Gestion normal de los botones

        // Boton A
        if (GP0 == 0) {
            autoA ++;
            if (autoA > speed[buttonA]) {
                outA = 1;
                if (autoA > (speed[buttonA] + SW_ACTIVE)) {
                    autoA = 0;
                }
            } else {
                outA = 0;
            }
        } else {
            autoA = speed[9];
            outA = 0;
        }

        // Boton B
        if (GP1 == 0) {
            autoB ++;
            if (autoB > speed[buttonB]) {
                outB = 1;
                if (autoB > (speed[buttonB] + SW_ACTIVE)) {
                    autoB = 0;
                }
            } else {
                outB = 0;
            }
        } else {
            autoB = speed[9];
            outB = 0;
        }



        // Si hay cambio en la SRAM...
        if (GP2 == 1 && GP3 == 1) {
            if (buttonA != oldButtonA) {
                oldButtonA = buttonA;
                eeprom_write(BA_ADDR, buttonA);
            }
            if (buttonB != oldButtonB) {
                oldButtonB = buttonB;
                eeprom_write(BB_ADDR, buttonB);
            }
        }


        // Salida
        GPIO = (outB << 5) | (outA << 4);

    }

}
