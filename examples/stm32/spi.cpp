
#include "spi.h"
#include "MCP3911.h"
#include <string.h>


class myADC : public MCP3911::C {
    public:
        virtual void begin(void) {

            Vref = 1.2023f;

            reset();
        }
        virtual void SPI_read(uint8_t addr, uint8_t *buffer, size_t count) {

            uint8_t TxBuff[256];
            uint8_t RxBuff[256];
            uint8_t cmd = addr << 1 | 1;
            HAL_StatusTypeDef status;

            TxBuff[0] = cmd;

            adc_ss_low();
            if (HAL_SPI_TransmitReceive(&hspi1, TxBuff, RxBuff, count+1, 100) != HAL_OK)
                Error_Handler();
            memcpy(buffer,&RxBuff[1],count);
            adc_ss_high();
        }
        virtual void SPI_write(uint8_t addr, uint8_t *buffer, size_t count) {
        
            HAL_StatusTypeDef status;
            uint8_t TxBuffer[256];
            uint8_t RxBuffer[256];

            uint8_t cmd = addr << 1 | 0;

            adc_ss_low();

            TxBuffer[0] = cmd;
            memcpy(&TxBuffer[1], buffer, count);

            if (HAL_SPI_TransmitReceive(&hspi1, TxBuffer, RxBuffer, count+1, 100) != HAL_OK)
                Error_Handler();

            adc_ss_high();

        }
};

MCP3911::_ConfRegMap c;
myADC adc;

void mcp3911_setup(void) {


    adc.begin();

    {   using namespace MCP3911;

        // set register map ptr
        adc._c = &c;

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

        // vref adjustment
        c.vrefcal += 0x11;


        // write out all regs
        adc.reg_write(REG_MOD, TYPE);
    }
}

void loop(void) {

    double v = adc.get_value(0);

}

