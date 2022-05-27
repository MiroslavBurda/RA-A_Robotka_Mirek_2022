#include "robotka.h"
#include <thread>
#include <atomic>

#define DELAY 50
#define DELAY_BLINK 250
#define LED_COUNT 8

unsigned long last_millis = 0;
bool rotating = false; // otáčí se?
bool finding = false; // našel kostku
bool previousLeft = false;
bool justPressed = true; //je tlačítko stisknuto poprvé
int ledBlink = 10; // blikani zadanou LED, 10 vypne všechny LED
int UltraUp = 5000, UltraDown = 5000, Min;
int found = 0; // kolik kostek našel
int l = 0; // hodnta leveho ultrazvuku
int r = 0; // hodnota praveho ultrazvuku
int IrL[] = { 0, 0, 0, 0 }; // pole pro levy infrazvuk
int IrR[] = { 0, 0, 0, 0 }; // pole pro pravy infrazvuk
int k = 0; // pocitadlo pro IR

std::atomic<int32_t> motor_value;

std::function<void(rb::Motor&)> m_cb = [&](rb::Motor& motor){
    motor_value.store(motor.position());
};


void SmartLedsOff() {
    for (int i = 0; i != LED_COUNT; i++)
        rkSmartLedsRGB(i, 0, 0, 0);
}

void rkIr() { // prumerovani IR
    while (true) {
        k++;
        if (k == 30000)
            k = 0;
        switch (k % 4) {
        case 0:
            IrL[0] = rkIrLeft();
            IrR[0] = rkIrRight();
            break;
        case 1:
            IrL[1] = rkIrLeft();
            IrR[1] = rkIrRight();
            break;
        case 2:
            IrL[2] = rkIrLeft();
            IrR[2] = rkIrRight();
            break;
        case 3:
            IrL[3] = rkIrLeft();
            IrR[3] = rkIrRight();
            break;
        default:
            printf("Chyba v prikazu switch");
            break;
        }
        l = (IrL[0] + IrL[1] + IrL[2] + IrL[3]) / 4;
        r = (IrR[0] + IrR[1] + IrR[2] + IrR[3]) / 4;
        delay(10);
    }
}

void Print() {
    
    printf("L: %f   R: %f  UltraUp: %i  Ultradown: %i \n", rkMotorsGetPositionLeft(), rkMotorsGetPositionRight(), UltraUp, UltraDown);
    // SerialBT.print("L: ");
    // SerialBT.print(l);
    // SerialBT.print("  R: ");
    // SerialBT.print(r);
    // SerialBT.print("  UltraUP: ");
    // SerialBT.print(UltraUp);
    // SerialBT.print("  UltraDown: ");
    // SerialBT.print(UltraDown);
    // SerialBT.print("  Min: ");
    // SerialBT.println(Min);
    last_millis = millis();
}

void setup() {
    
    rkConfig cfg;
    cfg.owner = "mirek"; // Ujistěte se, že v aplikace RBController máte nastavené stejné
    cfg.name = "mojerobotka";
    cfg.motor_max_power_pct = 100; // limit výkonu motorů na xx %

    cfg.motor_enable_failsafe = false;
    cfg.rbcontroller_app_enable = false; // nepoužívám mobilní aplikaci (lze ji vypnout - kód se zrychlí, ale nelze ji odstranit z kódu -> kód se nezmenší)
    rkSetup(cfg);

    rkUltraMeasureAsync(1, [&](uint32_t distance_mm) -> bool { // nesmí být v hlavním cyklu, protože se je napsaná tak, že se cyklí pomocí return true
        UltraUp = distance_mm;
        return true;
    });

    rkUltraMeasureAsync(2, [&](uint32_t distance_mm) -> bool {
        UltraDown = distance_mm;
        return true;
    });

    // std::thread t1(blink); // zajistí blikání v samostatném vlákně
    // std::thread t2(rkIr); // prumerne hodnoty z IR

    delay(300);
    fmt::print("{}'s Robotka '{}' with {} mV started!\n", cfg.owner, cfg.name, rkBatteryVoltageMv());
    rkLedYellow(true); // robot je připraven

    while (true) {

        if (rkButtonUp(true)) {
            rkMotorsDriveAsync(1000, 1000, 100, [](void) {});
            delay(300);
        }

        if (rkButtonLeft(true)) {
            rkMotorsSetSpeed(100, 100);
            delay(300);
        }

        if (rkButtonRight(true)) {
            rkMotorsSetSpeed(0, 0);
            delay(300);
        }

        if (rkButtonDown(true)) { // Tlačítko dolů: otáčej se a hledej kostku
            if (justPressed) {
                justPressed = false; // kdyz by tu tato podminka nebyla, byla by pauza v kazdem cyklu
                delay(300); // prodleva, abyste stihli uhnout rukou z tlačítka
            }
            if (rotating) {
                rkMotorsSetPower(0, 0); // zastavit robota
                rotating = false;
                rkLedBlue(false);
            } else {
                rotating = true;
                rkLedBlue(true);
            }
        }

        Min = (UltraUp < UltraDown) ? UltraUp : UltraDown;
        if (millis() - last_millis > 500)
            Print();
        //    printf("l: %i  r: %i  UA: %i  UD: %i  Up: %i  Down: %i \n", l, r, rkIrLeft(), rkIrRight(), UltraUp, UltraDown);


    }
}
