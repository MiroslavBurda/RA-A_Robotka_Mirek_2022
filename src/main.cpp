#include "robotka.h"
#include <thread>

unsigned long startTime = 0; // zacatek programu 
bool red = true;
byte readData[10]= { 1 }; //The character array is used as buffer to read into.

void ultrasonic() {
    while (true) {
        if (Serial1.available() > 0) { 
            Serial1.readBytes(readData, 10); //It require two things, variable name to read into, number of bytes to read.
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

void forward(float, int);

void setup() {
    
    Serial1.begin(115200, SERIAL_8N1, 17, 16); // Rx = 17 Tx = 16   

    rkConfig cfg;
    cfg.motor_max_power_pct = 100; // limit výkonu motorů na xx %
    cfg.motor_enable_failsafe = false;
    cfg.rbcontroller_app_enable = false; // nepoužívám mobilní aplikaci (lze ji vypnout - kód se zrychlí, ale nelze ji odstranit z kódu -> kód se nezmenší)
    cfg.motor_polarity_switch_left = true;
    cfg.motor_polarity_switch_right = false;
    cfg.motor_wheel_diameter = 69;
    cfg.motor_id_left = 4;
    cfg.motor_id_right = 1;
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
                rkMotorsSetSpeed(50, 50);
                delay(1000);
                rkMotorsSetSpeed(0, 0);
    }
    else { //startuje na modre barve
                rkMotorsSetSpeed(30, 30);
                delay(1000);  
                rkMotorsSetSpeed(0, 0);          
    }

  //  forward(500,30);

    while(true)     // po dokončení jízdy si v klidu odpočívá 
        delay(10); 
}

void forward(float distance, int speed) {  // vzdalenost v mm 0 .. 32000, rychlost -100 .. 100, interne po nasobcich 10
    
    // int lastL = 0;
    // int lastR = 0;
    int actualL = 0;
    int actualR = 0;
    float actualRD =0; // chtena hodnota na pravem enkoderu
    float corr = 0; // korekce rychlosti
    float const maxCorr = 5; // maximalni korekce rychlosti
    float err = 0;
    float const PP = 1; // clen P v PID regulatoru ( zatim pouze P regulator :-) )
    //float distEnc = distance / 0.4256; // z experimentu; vzdalenot v ticich enkoderu teoreticky vychazi 0.4375
    float distEncReduced = distance;
    
    rkMotorsSetPositionLeft();
    rkMotorsSetPositionRight();

    while ( abs(distEncReduced) > abs(rkMotorsGetPositionLeft()) ) {
        actualL = rkMotorsGetPositionLeft();
        actualR = rkMotorsGetPositionRight();
        actualRD = actualL * 0.99013;
        err = actualRD - actualR;
        corr = PP*err;
        corr = round(corr);
        if (corr > maxCorr) corr = maxCorr;  // zabraneni prilis velke korekce rychlosti
            if (corr < -maxCorr) corr = -maxCorr;
        if (corr >0) {
            rkMotorsSetSpeed(speed, speed - corr);
        }
        else {
            rkMotorsSetSpeed(speed + corr, speed);
        }
        // writeDebugStreamLine("jizda: time1: %i vL: %i vR: %i EncL: %i EncR: %i EncRD: %4.2f err: %4.2f corr: %4.2f", time1[T1], actualL - lastL, actualR - lastR, actualL , actualR, actualRD, err, corr );
        // lastL = rkMotorsGetPositionLeft();
        // lastR = rkMotorsGetPositionRight();
        delay(10);
    }
}

