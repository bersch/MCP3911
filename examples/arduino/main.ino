#include <Arduino.h>
#include <SPI.h>
#include <MCP3911.h>

#define ADC_CS_PIN 9
#define CLOCK_PIN  6

class myADC : public MCP3911::C {
  public:
            virtual void begin(void) {

                Vref = 1.2023f;

                pinMode(ADC_CS_PIN, OUTPUT);
                digitalWrite(ADC_CS_PIN, HIGH);

                SPI.begin();
                SPI.setClockDivider(SPI_CLOCK_DIV2);
                SPI.setBitOrder(MSBFIRST);
                SPI.setDataMode(SPI_MODE0);
                reset();
            }
            virtual void SPI_read(uint8_t addr, uint8_t *buffer, size_t count) {
                digitalWrite(ADC_CS_PIN, LOW);
                SPI.transfer(addr << 1 | 1);
                while (count--)
                  *buffer++ = SPI.transfer(0xff);
                digitalWrite(ADC_CS_PIN, HIGH);
            }
            virtual void SPI_write(uint8_t addr, uint8_t *buffer, size_t count) {
                digitalWrite(ADC_CS_PIN, LOW);
                SPI.transfer(addr << 1 | 0);
                while (count--)
                  SPI.transfer(*buffer++);
                digitalWrite(ADC_CS_PIN, HIGH);
            }
};

MCP3911::_ConfRegMap c;
myADC adc;

void setup_timer(void) {

    pinMode(CLOCK_PIN, OUTPUT);
    TIMSK0 = 0; 

    // Timer 0, Mode 2, toggle a on compare, toggle b on compare
    TCCR0A = bit (WGM01) | bit (COM0A0) | bit (COM0B0);
    TCCR0B = 1;

    OCR0A = 0;
    OCR0B = 0;
}

void setup(void) {

    Serial.begin(115200);

    setup_timer();

    adc.begin();

    {   using namespace MCP3911;

        // set register map ptr
        adc._c = &c;

        adc.reg_read(REG_STATUSCOM, REGISTER, 2);
        c.status.read_reg_incr  = ALL;
        c.status.write_reg_incr = ALL;
        adc.reg_write(REG_STATUSCOM, REGISTER, 2);

        // read default values
        adc.reg_read(REG_CHANNEL_0, ALL);

        c.gain.ch0              = GAINX1;
        c.gain.boost            = BOOSTX2;

        c.status.ch0_modout     = 0;
        c.status.dr_pull_up     = 1;

        c.config.az_on_hi_speed = 1;
        c.config.clkext         = 1;
        c.config.vrefext        = 0;
        c.config.osr            = O4096;
        c.config.prescale       = MCLK4;
        c.config.dither         = MAX;

        adc.status.read_reg_incr = ALL;
        adc.status.write_reg_incr = TYPE;

        // vref adjustment
        c.vrefcal += 0x11;

        // write out all regs
        adc.reg_write(REG_MOD, TYPE);
    }
}

void loop(void) {

    double v = adc.get_value(0);
    Serial.print("CH0: ");Serial.println(v, DEC);

}

