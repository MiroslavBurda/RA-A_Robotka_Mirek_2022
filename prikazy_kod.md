// jizda vpred - predek je tam, kde je radlice 

    // if(red) {   // startuje na cervene barve
    //             rkMotorsSetSpeed(50, 50);
    //             delay(1000);
    //             rkMotorsSetSpeed(0, 0);
    // }
    // else { //startuje na modre barve
    //             rkMotorsSetSpeed(30, 30);
    //             delay(1000);  
    //             rkMotorsSetSpeed(0, 0);          
    // }

void forward(float distance, int speed) {  // vzdalenost v mm 0 .. 32000, rychlost -100 .. 100, interne po nasobcich 10
    
    // int lastL = 0;
    // int lastR = 0;
    float actualL = 0;
    float actualR = 0;
    float actualRD =0; // chtena hodnota na pravem enkoderu
    float corr = 0; // korekce rychlosti
    float const maxCorr = 5; // maximalni korekce rychlosti
    float err = 0;
    float const PP = 1; // clen P v PID regulatoru ( zatim pouze P regulator :-) )
    //float distEnc = distance / 0.4256; // z experimentu; vzdalenot v ticich enkoderu teoreticky vychazi 0.4375
    float distEncReduced = distance;
    
    rkMotorsSetPositionLeft(); // reset enkoderu 
    rkMotorsSetPositionRight();

    if(state == 1) {
        rkMotorsDriveAsync(350, 350, 50, [&](){printf("rovnì"); state = 2;});
    }
    
    rkMotorsDriveAsync(130, -130, 20);
    delay(2000);
    rkMotorsDriveAsync(1000, 1000, 50);

    delay(10000);
    printf("L: %f, R: %f\n", rkMotorsGetPositionLeft(), rkMotorsGetPositionRight() );


    // while ( abs(distEncReduced) > abs(actualL) ) {
    //     printf("dist: %f, pos: %f\n", distEncReduced, actualL );
    //     actualL = rkMotorsGetPositionLeft();
    //     actualR = rkMotorsGetPositionRight();
    //     actualRD = actualL * 0.99013;
    //     err = actualRD - actualR;
    //     corr = PP*err;
    //     corr = round(corr);
    //     if (corr > maxCorr) corr = maxCorr;  // zabraneni prilis velke korekce rychlosti
    //         if (corr < -maxCorr) corr = -maxCorr;
    //     if (corr >0) {
    //         rkMotorsSetSpeed(speed, speed - corr);
    //     }
    //     else {
    //         rkMotorsSetSpeed(speed + corr, speed);
    //     }
    //     // writeDebugStreamLine("jizda: time1: %i vL: %i vR: %i EncL: %i EncR: %i EncRD: %4.2f err: %4.2f corr: %4.2f", time1[T1], actualL - lastL, actualR - lastR, actualL , actualR, actualRD, err, corr );
    //     // lastL = rkMotorsGetPositionLeft();
    //     // lastR = rkMotorsGetPositionRight();
    //     delay(100);
    // }

    while(true) {
        delay(10);
    }


}