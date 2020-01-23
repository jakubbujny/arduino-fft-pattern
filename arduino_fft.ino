#define LIN_OUT8 1 // use the log output function
#define FFT_N 128    // set to 128 point fft

#include <FFT.h> // include the library
#define F_CPU 10000000UL  // 1 MHz
#include <util/delay.h>

uint8_t matrix[20][40];
uint16_t sum[40] = {0};


void setup() {
    Serial.begin(115200); // use the serial port
    TIMSK0 = 0; // turn off timer0 for lower jitter
    analogRead(A5);
    DIDR0 = 0x01; // turn off the digital input for adc0
    pinMode(13, OUTPUT);
    pinMode(12, INPUT);
    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 40; j++) {
            matrix[i][j] = 0;
        }
    }
}

int passed = 0;
void loop() {
    int record = digitalRead(12);
    if(record == 1 && passed == 0) {
        digitalWrite(13, 1);
        cli();
        for (int rowInMatrix = 0; rowInMatrix < 20; rowInMatrix += 1) {
            for (int i = 0; i < FFT_N * 2; i += 2) {
                while (!(ADCSRA & 0x10)); // wait for adc to be ready
                ADCSRA = 0xf5; // restart adc
                byte m = ADCL; // fetch adc data
                byte j = ADCH;
                int k = (j << 8) | m; // form into an int
                k -= 0x0200; // form into a signed int
                k <<= 6; // form into a 16b signed int
                fft_input[i] = k; // put real data into even bins
                fft_input[i + 1] = 0; // set odd bins to 0
            }
            fft_window(); // window the data for better frequency response
            fft_reorder(); // reorder the data before doing the fft
            fft_run(); // process the data in the fft
            fft_mag_lin8(); // take the output of the fft
            sei();
            for (int columnInMatrix = 0; columnInMatrix < 40; columnInMatrix++) {
                matrix[rowInMatrix][columnInMatrix] = fft_lin_out8[columnInMatrix];
                sum[columnInMatrix] += fft_lin_out8[columnInMatrix];
            }
            _delay_ms(50);
        }
        for (int sumPosition = 0; sumPosition < 40; sumPosition++) {
            Serial.print(sum[sumPosition]);
            Serial.print(",");
        }
        Serial.println();
        digitalWrite(13, 0);
        passed = 1;
    }
    if(passed == 1) {
        _delay_ms(5000);
        for (int i = 0; i < 20; i++) {
            for (int j = 0; j < 40; j++) {
                matrix[i][j] = 0;
                sum[j] = 0;
            }
        }
        passed = 0;
    }
}