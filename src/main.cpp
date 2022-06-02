#include "robotka.h"
#include <thread>

// startuje z pravého horního rohu 
// tlačítko Up spouští přípravu, tlačítko Down vybírá strany; startovací lanko rozjíždí 

unsigned long startTime = 0; // zacatek programu 
bool red = true;
const byte readSize = 8;
const byte header = 250; //hlavicka zpravy ma tvar: {250, 250+k}, k = 1 ... 3    
constexpr byte msgHeader[3] = {251, 252, 253};
const byte minVzdal = 50; // minimalni vzdalenost, na ktere se sousedni robot muze priblizit, if se priblizi vic, tak abort();

byte readData0[readSize]= {0}; //The character array is used as buffer to read into.
byte readData1[readSize]= {0};
byte readData2[readSize]= {0};
byte state = 1; // stav programu
bool startState = false; // if je odstartovano vytazenim lanka 
byte speed = 35; // obvykla rychlost robota
byte speedSlow = 20; // pomala = zataceci rychlost robota  

/**
 * @brief Funkce na vyhledání nejmenší hodnoty z byte pole. !!! Nulu to nebere jako nejmenší !!!
 * 
 * @param arr   Ukazatel na pole hodnot
 * @param index Adresa kam má funkce vrátit pozici nejmenší hodnoty
 * @return byte Vrací nejmenší hodnotu pole 
 */
byte min_arr(byte *arr, int &index){
    byte tmp = 255;
    index = -1;
    for (size_t i = 0; i < 8; i++) {
        if (arr[i] < tmp && arr[i] != 0) {
            tmp = arr[i];
            index = i;
        }
    }
    return tmp;
}

void ultrasonic() {
    int pozice0, pozice1, pozice2;
    while (true) {
            if (Serial1.available() > 0) { 
                int temp = Serial1.read();
                if(temp == header) {
                    // printf("bytes: %i \n", header); 
                    if (Serial1.available() > 0) {
                        int head = Serial1.read();
                        printf("head: %i ", head); 
                        switch (head) {
                        case msgHeader[0]: 
                            Serial1.readBytes(readData0, readSize); //It require two things, variable name to read into, number of bytes to read.
                            for(int i = 0; i<8; i++) { printf("%i: %i, ", i, readData0[i]); } printf("\n ");
                            break;        
                        case msgHeader[1]:
                            Serial1.readBytes(readData1, readSize); 
                            for(int i = 0; i<8; i++) { printf("%i: %i, ", i, readData1[i]); } printf("\n ");
                            break;        
                        case msgHeader[2]:
                            Serial1.readBytes(readData2, readSize); 
                            for(int i = 0; i<8; i++) { printf("%i: %i, ", i, readData2[i]); } printf("\n ");
                            break;
                        default:
                            printf("Nenasel druhy byte hlavicky !! "); 
                        }
                    }
                    int min0 = min_arr(readData0, pozice0); 
                    int min1 = min_arr(readData1, pozice0); 
                    if ( (min0 == min1) && (min0 < minVzdal) ) {
                        printf("Souper blizi...");
                        if(startState) {
                            printf("Souper se prilis priblizil...");
                            abort();
                        }
                    }
                }
             
        }
        delay(50);            
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
                for(int i = 1; i<5; i++) {
                    rkLedById(i, true);
                }
                delay(500);
                for(int i = 1; i<5; i++) {
                    rkLedById(i, false);
                }
                delay(500);
            }
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

    printf("vyber strany - tlacitko Down\n");
    while(true) {   
        if(!rkButtonLeft(false)) { // vytazeni startovaciho lanka na tl. Left rozjede robota  
            startState = true;
            printf("Rozjizdim se\n");
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



