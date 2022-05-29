#include "robotka.h"
#include <thread>

// startuje z pravého horního rohu 
// tlačítko Up spouští přípravu, tlačítko Down vybírá strany; startovací lanko rozjíždí 

unsigned long startTime = 0; // zacatek programu 
bool red = true;
byte readData[10]= { 1 }; //The character array is used as buffer to read into.
byte state = 1; // stav programu
byte speed = 50; // obvykla rychlost robota
byte speedSlow = 20; // pomala = zataceci rychlost robota  

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
        if (( millis() - startTime ) > 127000) { // konci cca o 700ms driv real: 127000
            printf("cas vyprsel: ");
            printf("%lu, %lu \n", startTime, millis() );
            rkMotorsSetSpeed(0, 0);
            //rkSmartLedsRGB(0, 255, 0, 0);
            delay(100); // aby stihla LED z predchoziho radku rozsvitit - z experimentu
            //rkSmartLedsRGB(0, 255, 0, 0)
            for(int i = 0; i<5; i++)  { // zaverecne zablikani vsemi LED 
                for(int i = 0; i<5; i++) {
                    rkLedById(i, true);
                }
                delay(500);
                for(int i = 0; i<5; i++) {
                    rkLedById(i, false);
                }
                delay(500);
            }
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
    //cfg.motor_polarity_switch_left = true;
    //cfg.motor_polarity_switch_right = false;
    cfg.motor_wheel_diameter = 66;
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

    // std::thread t2(ultrasonic);
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

    printf("vyber strany - tlacitko Down\n");
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
    delay(500); // robot se pri vytahovani lanka musi pridrzet -> pocka, nez oddelam ruku, 

   while(true){  // hlavni smycka 
        if(state == 1) {
            state = 2;
            rkMotorsDriveAsync(350, 350, speed, [&](){printf("ze startu\n"); state = 3;});
        }
        if(state == 3) {
            state = 4;
            if (red){
                rkMotorsDriveAsync(130, -130, speedSlow, [&](){printf("zatocil k nakladaku\n"); state = 5;});
            }
            else {
                rkMotorsDriveAsync(-130, 130, speedSlow, [&](){printf("zatocil k nakladaku\n"); state = 5;});
                delay(500);
            }
        }
        if(state == 5) {
            state = 6;
            rkMotorsDriveAsync(1010, 1010, speed, [&](){printf("vytlacil\n"); state = 7;}); // ************ bez couvani - state 9 
        }

        if(state == 7) { // ************ couvani - nebezpecne bez ultrazvuku 
            state = 8;
            rkMotorsDriveAsync(-50, -50, speedSlow, [&](){printf("couvl\n"); state = 9;});
        }

        if(state == 9) { 
            state = 10;
            if (red){
                rkMotorsDriveAsync(260, -260, speedSlow, [&](){printf("otocil se zpet\n"); state = 11;});
            }
            else {
                rkMotorsDriveAsync(-260, 260, speedSlow, [&](){printf("otocil se zpet\n"); state = 11;});
                delay(500);
            }
        }

        if(state == 11) {
            state = 12;
            rkMotorsDriveAsync(1120, 1120, speed, [&](){printf("vraci se zpet\n"); state = 13;});
        }

        if(state == 13) { 
            state = 14;
            if (red){
                rkMotorsDriveAsync(-140, 140, speedSlow, [&](){printf("otocil se na start\n"); state = 15;});
            }
            else {
                rkMotorsDriveAsync(140, -140, speedSlow, [&](){printf("otocil se na start\n"); state = 15;});
                delay(500);
            }
        }
        if(state == 15) {
            state = 16;
            rkMotorsDriveAsync(650, 650, speed, [&](){printf("zpet na start\n"); state = 17;});
        }

        delay(10); 
    }
}



