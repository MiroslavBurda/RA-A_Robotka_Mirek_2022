#include "robotka.h"
#include <thread>

unsigned long startTime = 0; // zacatek programu 
bool red = true;
byte readData[10]= { 1 }; //The character array is used as buffer to read into.

void ultrasonic() {
    while (true) {
        if (Serial1.available() > 0) { 
            int x = Serial1.readBytes(readData, 10); //It require two things, variable name to read into, number of bytes to read.
            printf("bytes: "); 
            // Serial.println(x); //display number of character received in readData variable.
            printf("h: %i, ", readData[0]);
            printf("h: %i, ", readData[1]);
            for(int i = 2; i<10; i++) {
                printf("%i: %i, ", i-2, readData[i]); // ****************
            }
            printf("\n ");
        } 
        delay(10);            
    }
}


void stopTime() { // STOP jizde po x milisec 
    while(true) {
        if (( millis() - startTime ) > 30000000) { // konci cca o 700ms driv real: 127000
            printf("cas vyprsel: ");
            printf("%lu, %lu \n", startTime, millis() );
            rkSmartLedsRGB(0, 255, 0, 0);
            delay(100); // aby stihla LED z predchoziho radku rozsvitit - z experimentu
            abort(); // program skonci -> dojde k resetu a zustane cekat pred stiskem tlacitka Up
        }
        delay(10); 
    }
}

void setup() {
    
    Serial1.begin(115200, SERIAL_8N1, 17, 16); // Rx = 17 Tx = 16   

    rkConfig cfg;
    cfg.motor_max_power_pct = 100; // limit výkonu motorů na xx %
    cfg.motor_enable_failsafe = false;
    cfg.rbcontroller_app_enable = false; // nepoužívám mobilní aplikaci (lze ji vypnout - kód se zrychlí, ale nelze ji odstranit z kódu -> kód se nezmenší)
    cfg.motor_polarity_switch_left = true;
    cfg.motor_polarity_switch_right = false;
    rkSetup(cfg);

    rkLedBlue(true); // cekani na stisk Up, take po resetu stop tlacitkem, aby se zase hned nerozjela
    printf("cekani na stisk Up\n");
    while(true) {   
        if(rkButtonUp(true)) {
            break;
        }
        delay(10);
    }
    rkLedBlue(false);

    startTime = millis();   

    std::thread t2(ultrasonic);
    std::thread t3(stopTime); // vlakno pro zastaveni po uplynuti casu 

    fmt::print("{}'s Robotka '{}' with {} mV started!\n", cfg.owner, cfg.name, rkBatteryVoltageMv());
    rkLedYellow(true); // robot je připraven
    if(red) {  
        rkLedRed(true);
        rkLedBlue(false);
    }
    else {
        rkLedRed(false);
        rkLedBlue(true);               
    }

    printf("vyber strany - tlacitkko Down\n");
    while(true) {   
        if(!rkButtonLeft(false)) { // vytazeni startovaciho lanka na tl. Left rozjede robota  
            break;
        }
        if(rkButtonDown(true)) {
            red = !red;
            if(red) {
                rkLedRed(true);
                rkLedBlue(false);
            }
            else {
                rkLedRed(false);
                rkLedBlue(true);               
            }
        }
        delay(10);
    }
// jizda vpred - predek je tam, kde je radlice 

    if(red) {   // startuje na cervene barve
                rkMotorsSetSpeed(0, 50);
                delay(1000);
                rkMotorsSetSpeed(0, 0);
    }
    else { //startuje na modre barve
                rkMotorsSetSpeed(50, 0);
                delay(1000);  
                rkMotorsSetSpeed(0, 0);          
    }

    while(true)     // po dokončení jízdy si v klidu odpočívá 
        delay(10); 
}