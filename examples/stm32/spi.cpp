
#include "spi.h"
#include "MCP3911.h"
#include <string.h>


class myADC : public MCP3911::C {
  private:


            uint8_t SPI1_send(uint8_t data){
                while(((SPI1->SR) & SPI_FLAG_TXE) != SPI_FLAG_TXE); // wait while tx-flag not empty
                *(uint8_t *)&(SPI1->DR) = data; // write data to be transmitted to the SPI data register
                while ((SPI1->SR & SPI_FLAG_RXNE) != SPI_FLAG_RXNE); // wait while rx-buffer not empty
                /* Wait until the bus is ready before releasing Chip select */
                while(((SPI1->SR) & SPI_FLAG_BSY) == SPI_FLAG_BSY);
                return *(uint8_t *)&(SPI1->DR); // return received data from SPI data register
            }

  public:
            virtual void begin(void) {

                Vref = 1.2023f;

                reset();
            }

            virtual void SPI_read(uint8_t addr, uint8_t *buffer, size_t count) {
                SPI1_send(addr << 1 | 1);
                while (count--) 
                   *buffer++ = SPI1_send(0x00);
            }
            virtual void SPI_write(uint8_t addr, uint8_t *buffer, size_t count) {
                SPI1_send(addr << 1 | 0);
                while (count--) 
                   SPI1_send(*buffer++);
            }
};

MCP3911::_ConfRegMap c;
myADC adc;

void mcp3911_setup(void) {


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

        c.status.read_reg_incr = ALL;
        c.status.write_reg_incr = TYPE;

        // vref adjustment
        c.vrefcal += 0x11;

        // write out all regs
        adc.reg_write(REG_MOD, TYPE);

        // set grouping for TYPE
        c.status.read_reg_incr  = TYPE;
        adc.reg_write(REG_STATUSCOM, REGISTER, 2);
    }
}

void loop(void) {

    double v0,v1;
    
    adc.reg_read(REG_CHANNEL0, TYPE); // read 6 regs
    v0 = adc.get_value(0);
    v1 = adc.get_value(1);

}
