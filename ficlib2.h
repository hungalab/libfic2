//-----------------------------------------------------------------------------
// libfic2.h 
// nyacom <kzh@nyacom.net> (C) 2018.09
//-----------------------------------------------------------------------------
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/file.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

//-----------------------------------------------------------------------------
#define GPIO_LOCK_TIMEOUT (5)       // Lockfile timeout
#define COMM_TIMEOUT (5)            // Communication timeout 

#define GPIO_DEV "/dev/gpiomem"
#define LOCK_FILE "/tmp/gpio.lock"

//#define BCM2708_PERI_BASE        0x20000000
#define BCM2708_PERI_BASE        0x3F000000	// for RPi3
#define GPIO_BASE                (BCM2708_PERI_BASE + 0x200000) /* GPIO controller */

#define PAGE_SIZE (4*1024)
#define BLOCK_SIZE (4*1024)
 
//-----------------------------------------------------------------------------
// GPIO setup macros.
// Always use INP_GPIO(x) before using OUT_GPIO(x) or SET_GPIO_ALT(x,y)
//-----------------------------------------------------------------------------
// I/O access
volatile unsigned *gpio;

// Change GPIO mode
#define SET_INPUT(g) *(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))
#define SET_OUTPUT(g) *(gpio+((g)/10)) |=  (1<<(((g)%10)*3))
#define SET_ALT(g, a) *(gpio+(((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))
#define SET_ALL_INPUT *(gpio+0) = 0x00; *(gpio+1) = 0x00; *(gpio+2) = 0x00;
//#define SET_GPIO_PULL *(gpio+37) // Pull up/pull down
//#define SET_GPIO_PULLCLK0 *(gpio+38) // Pull up/pull down clock

 // Change GPIO value
#define SET_GPIO *(gpio+7)  // sets   bits which are 1 ignores bits which are 0
#define CLR_GPIO *(gpio+10) // clears bits which are 1 ignores bits which are 0
#define GET_GPIO *(gpio+13) // 0 if LOW, (1<<g) if HIGH
#define GET_GPIO_PIN(g) ((*(gpio+13)&(1<<g)) >> g) // 0 if LOW, (1<<g) if HIGH
 
//-----------------------------------------------------------------------------
// PIN Numbers
//-----------------------------------------------------------------------------
#define RP_INIT 4
#define RP_PROG_B 5
#define RP_DONE 6
#define RP_CCLK 7

#define RP_CD0 8
#define RP_CD1 9
#define RP_CD2 10 
#define RP_CD3 11
#define RP_CD4 12
#define RP_CD5 13
#define RP_CD6 14
#define RP_CD7 15
#define RP_CD8 16
#define RP_CD9 17
#define RP_CD10 18
#define RP_CD11 19
#define RP_CD12 20
#define RP_CD13 21
#define RP_CD14 22
#define RP_CD15 23
#define RP_CD16 24
#define RP_CD17 25
#define RP_CSI_B 26
#define RP_RDWR_B 27 

#define GPIO_PIN_MAX RP_RDWR_B

//-----------------------------------------------------------------------------
// PIN Numbers (Functional aliases)
//-----------------------------------------------------------------------------
#define RP_PWOK RP_CD16
#define RP_G_CKSEL RP_CD17

#define RP_RREQ RP_CD15
#define RP_RSTB RP_CD14
#define RP_FREQ RP_CD13
#define RP_FACK RP_CD12

#define RP_DATA0 RP_CD4
#define RP_DATA1 RP_CD5
#define RP_DATA2 RP_CD6
#define RP_DATA3 RP_CD7
#define RP_DATA4 RP_CD8
#define RP_DATA5 RP_CD9
#define RP_DATA6 RP_CD10
#define RP_DATA7 RP_CD11

//-----------------------------------------------------------------------------
// PIN bits
//-----------------------------------------------------------------------------
#define RP_PIN_INIT (1<<RP_INIT)
#define RP_PIN_PROG_B (1<<RP_PROG_B)
#define RP_PIN_DONE (1<<RP_DONE)
#define RP_PIN_CCLK (1<<RP_CCLK)

#define RP_PIN_CD0 (1<<RP_CD0)
#define RP_PIN_CD1 (1<<RP_CD1)
#define RP_PIN_CD2 (1<<RP_CD2)
#define RP_PIN_CD3 (1<<RP_CD3)
#define RP_PIN_CD4 (1<<RP_CD4)
#define RP_PIN_CD5 (1<<RP_CD5)
#define RP_PIN_CD6 (1<<RP_CD6)
#define RP_PIN_CD7 (1<<RP_CD7)
#define RP_PIN_CD8 (1<<RP_CD8)
#define RP_PIN_CD9 (1<<RP_CD9)
#define RP_PIN_CD10 (1<<RP_CD10)
#define RP_PIN_CD11 (1<<RP_CD11)
#define RP_PIN_CD12 (1<<RP_CD12)
#define RP_PIN_CD13 (1<<RP_CD13)
#define RP_PIN_CD14 (1<<RP_CD14)
#define RP_PIN_CD15 (1<<RP_CD15)
#define RP_PIN_CD16 (1<<RP_CD16)
#define RP_PIN_CD17 (1<<RP_CD17)
#define RP_PIN_CSI_B (1<<RP_CSI_B)
#define RP_PIN_RDWR_B (1<<RP_RDWR_B) 

//-----------------------------------------------------------------------------
// PIN bits (Funtional aliases)
//-----------------------------------------------------------------------------
#define RP_PIN_PWOK RP_PIN_CD16
#define RP_PIN_G_CKSEL RP_PIN_CD17

#define RP_PIN_RREQ RP_PIN_CD15
#define RP_PIN_RSTB RP_PIN_CD14
#define RP_PIN_FREQ RP_PIN_CD13
#define RP_PIN_FACK RP_PIN_CD12

#define RP_PIN_DATA0 RP_PIN_CD4
#define RP_PIN_DATA1 RP_PIN_CD5
#define RP_PIN_DATA2 RP_PIN_CD6
#define RP_PIN_DATA3 RP_PIN_CD7
#define RP_PIN_DATA4 RP_PIN_CD8
#define RP_PIN_DATA5 RP_PIN_CD9
#define RP_PIN_DATA6 RP_PIN_CD10
#define RP_PIN_DATA7 RP_PIN_CD11
//-----------------------------------------------------------------------------
//#define COMM_MASK 0x00cfff00
#define COMM_DATABUS_MASK                                                      \
  (RP_PIN_RSTB | RP_PIN_DATA0 | RP_PIN_DATA1 | RP_PIN_DATA2 | RP_PIN_DATA3 |   \
   RP_PIN_DATA4 | RP_PIN_DATA5 | RP_PIN_DATA6 | RP_PIN_DATA7)

#define COMM_CMD_HLS_START 0x01
#define COMM_CMD_HLS_RESET 0x04
#define COMM_CMD_WRITE 0x02
#define COMM_CMD_READ 0x03

#define COMM_ADDR_HLS 0x1000

//-----------------------------------------------------------------------------
enum PROG_MODE {
    PM_NORMAL = 0,  // Normal mode
    PM_PR = 1,      // for partial-reconfigurtaion
};

enum COMM_PORT_DIR {
    COMM_PORT_SND = 0,
    COMM_PORT_RCV = 1,
};

//-----------------------------------------------------------------------------
#define __DEBUG__

#ifdef __DEBUG__
#define DEBUGTHRU printf("DEBUGTHRU: %s:%d\n", __FILE__, __LINE__)
#define DEBUGOUT printf
#define DEBUGCOMM fic_comm_busdebug(__LINE__)

#else
#define DEBUGOUT //
#endif
 
//-----------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------
extern int fic_comm_setup8();
extern int fic_comm_setup4();

extern int fic_done();
extern int fic_power();

extern void fic_comm_portdir8(enum COMM_PORT_DIR dir);
extern void fic_comm_portdir4(enum COMM_PORT_DIR dir);

extern int fic_comm_wait_fack_down();
extern int fic_comm_wait_fack_up();
extern int fic_comm_wait_freq_down();
extern int fic_comm_wait_freq_up();

extern int fic_comm_send8(uint32_t bus);
extern int fic_comm_send4(uint32_t bus);
extern int fic_comm_receive();

extern int fic_comm_setaddr8(uint16_t addr);
extern int fic_comm_setaddr4(uint16_t addr);

extern int fic_wb8(uint16_t addr, uint8_t data);
extern int fic_rb8(uint16_t addr);
extern int fic_wb4(uint16_t addr, uint8_t data);
extern int fic_rb4(uint16_t addr);
extern int fic_hls_send4(uint8_t *data, size_t size);
extern int fic_hls_receive4(size_t size, uint8_t *buf);

extern int fic_prog_init_sm16();
extern int fic_prog_init_sm8();
extern int fic_prog_init();

extern size_t fic_prog_sm16(uint8_t *data, size_t size, enum PROG_MODE pm, size_t *tx_byte);
extern size_t fic_prog_sm8(uint8_t *data, size_t size, enum PROG_MODE pm, size_t *tx_byte);

extern int fic_hls_start8();
extern int fic_hls_start4();
extern int fic_hls_reset8();
extern int fic_hls_reset4();

extern int fic_gpio_open();
extern int fic_gpio_close();

extern int gpio_unlock();
extern int gpio_lock();