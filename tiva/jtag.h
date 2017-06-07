#define RESET   0
#define IDLE    1
#define DRSELECT 2
#define DRCAPTURE 3
#define DRSHIFT 4
#define DREXIT1 5
#define DRPAUSE 6
#define DREXIT2 7
#define DRUPDATE 8
#define IRSELEST 9
#define IRCAPTURE 10
#define IRSHIFT 11
#define IRPAUSE 12
#define IREXIT1 13
#define IREXIT2 14
#define IRUPDATE 15

#define TMS 0x02        //Corresponds to a 1 at pin B1
#define TDI 0x01        //Corresponds to a 1 at pin B0
#define TRST 0x10       //Corresponds to a 1 at pin B4

void jtag_set_state(uint8_t state);
void jtag_endir_state(uint8_t state);
void jtag_enddr_state(uint8_t state);


void jtag_ir_write(uint8_t ir_len, uint32_t instruction);

uint8_t JTAG_clock(uint32_t val);
uint8_t JTAG_read();
uint8_t JTAG_clock(uint32_t val);
uint8_t JTAG_read();
void jtag_change_state(uint8_t curr_state, uint8_t next_state);
