//-----------------------------------------------------------------------------
// libfic2.h 
// nyacom <kzh@nyacom.net> (C) 2018.09
//-----------------------------------------------------------------------------
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/file.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

//-----------------------------------------------------------------------------
// Board definition
// Note: Uncomment if target board is FiC Mark2
// #define FICMK2
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
#define GPIO_LOCK_TIMEOUT (30)      // Lockfile timeout
#define COMM_TIMEOUT (10)           // Communication timeout 

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
// PIN Alias Number
//-----------------------------------------------------------------------------

#ifndef FICMK2
//-----------------------------------------------------------------------------
// PIN Numbers (Functional aliases for mk1 board)
//-----------------------------------------------------------------------------
#define RP_PWOK RP_CD16
#define RP_G_CKSEL RP_CD17
#define RP_CFSEL RP_CD17        // for mk2 board

#define RP_RREQ RP_CD15
#define RP_RSTB RP_CD14
#define RP_FREQ RP_CD13
#define RP_FACK RP_CD12

#define RP_DATA0 RP_CD8
#define RP_DATA1 RP_CD9
#define RP_DATA2 RP_CD10
#define RP_DATA3 RP_CD11

//#define RP_DATA0 RP_CD4
//#define RP_DATA1 RP_CD5
//#define RP_DATA2 RP_CD6
//#define RP_DATA3 RP_CD7
//#define RP_DATA4 RP_CD8
//#define RP_DATA5 RP_CD9
//#define RP_DATA6 RP_CD10
//#define RP_DATA7 RP_CD11

#define RP_DATA_LOW  RP_DATA0
#define RP_DATA_TOP  RP_DATA3

//-----------------------------------------------------------------------------
// PIN bits (Funtional aliases for mk1 board)
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
// BIT MASK
//-----------------------------------------------------------------------------
#define COMM_DATABUS_MASK                                                      \
  (RP_PIN_RSTB | RP_PIN_DATA0 | RP_PIN_DATA1 | RP_PIN_DATA2 | RP_PIN_DATA3 |   \
   RP_PIN_DATA4 | RP_PIN_DATA5 | RP_PIN_DATA6 | RP_PIN_DATA7)

#else // For fic mark2 board

//   bit       Conf_IO_IN : Comment
//   //   31-28        x  x  0 :
//   //      27    FAK  I  0 :
//   //      26    FRQ  I  0 :
//   //      25     CFSL  O  0 :
//   //      24        x  I  x :
//   //      23   DB[15]  B  0 :
//   //      22   DB[14]  B  0 :
//   //      21   DB[13]  B  0 :
//   //      20   DB[12]  B  0 :
//   //      19   DB[11]  B  0 :
//   //      18   DB[10]  B  0 :
//   //      17   DB[ 9]  B  0 :
//   //      16   DB[ 8]  B  0 :
//   //      15   DB[ 7]  B  0 :
//   //      14   DB[ 6]  B  0 :
//   //      13   DB[ 5]  B  0 :
//   //      12   DB[ 4]  B  0 :
//   //      11   DB[ 3]  B  0 :
//   //      10   DB[ 2]  B  0 :
//   //       9   DB[ 1]  B  0 :
//   //       8   DB[ 0]  B  0 :
//   //       7        x  x  0 :
//   //       6   FREQ    O  1 :
//   //       5       x   I  0 :
//   //       4   FSTB    O  1 :
//   //       3   I2C_SC  B  x :
//   //       2   I2C_SD  B  x :
//   //    1- 0        x  x  x :

//-----------------------------------------------------------------------------
// PIN Numbers (Functional aliases for mk2 board)
//-----------------------------------------------------------------------------
#define RP_PWOK  RP_CD16
#define RP_CFSEL RP_CD17        // for mk2 board

#define RP_RREQ  RP_DONE
#define RP_RSTB  RP_INIT
#define RP_FREQ  RP_CSI_B
#define RP_FACK  RP_RDWR_B

#define RP_DATA0 RP_CD0
#define RP_DATA1 RP_CD1
#define RP_DATA2 RP_CD2
#define RP_DATA3 RP_CD3
#define RP_DATA4 RP_CD4
#define RP_DATA5 RP_CD5
#define RP_DATA6 RP_CD6
#define RP_DATA7 RP_CD7
#define RP_DATA8 RP_CD8
#define RP_DATA9 RP_CD9
#define RP_DATA10 RP_CD10
#define RP_DATA11 RP_CD11
#define RP_DATA12 RP_CD12
#define RP_DATA13 RP_CD13
#define RP_DATA14 RP_CD14
#define RP_DATA15 RP_CD15

#define RP_DATA_LOW  RP_DATA0
#define RP_DATA_TOP  RP_DATA15
//-----------------------------------------------------------------------------
// PIN bits (Funtional aliases for mk2 board)
//-----------------------------------------------------------------------------
#define RP_PIN_PWOK RP_PIN_CD16
#define RP_PIN_CFSEL RP_PIN_CD17

#define RP_PIN_RREQ  RP_PIN_DONE
#define RP_PIN_RSTB  RP_PIN_INIT
#define RP_PIN_FREQ  RP_PIN_CSI_B
#define RP_PIN_FACK  RP_PIN_RDWR_B

#define RP_PIN_DATA0 RP_PIN_CD0
#define RP_PIN_DATA1 RP_PIN_CD1
#define RP_PIN_DATA2 RP_PIN_CD2
#define RP_PIN_DATA3 RP_PIN_CD3
#define RP_PIN_DATA4 RP_PIN_CD4
#define RP_PIN_DATA5 RP_PIN_CD5
#define RP_PIN_DATA6 RP_PIN_CD6
#define RP_PIN_DATA7 RP_PIN_CD7
#define RP_PIN_DATA8 RP_PIN_CD8
#define RP_PIN_DATA9 RP_PIN_CD9
#define RP_PIN_DATA10 RP_PIN_CD10
#define RP_PIN_DATA11 RP_PIN_CD11
#define RP_PIN_DATA12 RP_PIN_CD12
#define RP_PIN_DATA13 RP_PIN_CD13
#define RP_PIN_DATA14 RP_PIN_CD14
#define RP_PIN_DATA15 RP_PIN_CD15

#define COMM_DATABUS_MASK (RP_PIN_RSTB |                                                 \
                           RP_PIN_DATA0 | RP_PIN_DATA1 | RP_PIN_DATA2 | RP_PIN_DATA3 |   \
                           RP_PIN_DATA4 | RP_PIN_DATA5 | RP_PIN_DATA6 | RP_PIN_DATA7 |   \
                           RP_PIN_DATA8 | RP_PIN_DATA9 | RP_PIN_DATA10 | RP_PIN_DATA11 | \
                           RP_PIN_DATA12 | RP_PIN_DATA13 | RP_PIN_DATA14 | RP_PIN_DATA15)
#endif

//-----------------------------------------------------------------------------
#define COMM_CMD_HLS_START 0x01
#define COMM_CMD_HLS_RESET 0x04
#define COMM_CMD_WRITE 0x02
#define COMM_CMD_READ 0x03

//-----------------------------------------------------------------------------
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

enum PROG_SMAP_MODE {
    PM_SMAP_8 = 8,
    PM_SMAP_16 = 16,
};

enum PROG_STATUS {
    PM_STAT_INIT = 0,
    PM_STAT_PROG = 1,
    PM_STAT_DONE = 2,
    PM_STAT_FAIL = 3,
};


//-----------------------------------------------------------------------------
typedef struct _prog_async_status {
    pthread_t prog_th;

    enum PROG_STATUS    stat;           // Configuration status
    enum PROG_SMAP_MODE smap_mode;      // SMAP mode
    enum PROG_MODE      prog_mode;      // Programing mode (Normal/PR)
    time_t              prog_st_time;   // Configuration start ts
    time_t              prog_ed_time;   // Configuration done ts
    uint8_t*            prog_data;
    size_t              prog_size;      // Configuration size
    size_t              tx_size;        // Transfered size
} prog_async_status;

extern struct _prog_async_status PROG_ASYNC_STATUS;

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
// extern inline int fic_set_gpio(uint32_t set);
// extern inline int fic_clr_gpio(uint32_t set);

extern int fic_comm_setup();

extern int fic_done();
extern int fic_power();

extern void fic_comm_portdir(enum COMM_PORT_DIR dir);

extern int fic_comm_wait_fack_down();
extern int fic_comm_wait_fack_up();
extern int fic_comm_wait_freq_down();
extern int fic_comm_wait_freq_up();

extern int fic_comm_send(uint32_t bus);
extern int fic_comm_receive();

extern int fic_comm_setaddr(uint16_t addr);

extern int fic_write(uint16_t addr, uint16_t data);
extern int fic_read(uint16_t addr);
extern int fic_hls_send(uint8_t *data, size_t size);
extern int fic_hls_receive(uint8_t *buf, size_t size);

extern int fic_prog_init_sm16();
extern int fic_prog_init_sm8();
extern int fic_prog_init(enum PROG_MODE pm);

extern size_t fic_prog_sm16(uint8_t *data, size_t size, enum PROG_MODE pm);
extern size_t fic_prog_sm8(uint8_t *data, size_t size, enum PROG_MODE pm);

extern int fic_prog_sm16_async(uint8_t *data, size_t size, enum PROG_MODE pm);
extern int fic_prog_sm8_async(uint8_t *data, size_t size, enum PROG_MODE pm);
extern int fic_prog_async_status();

extern int fic_hls_start();
extern int fic_hls_reset();

extern int fic_gpio_open();
extern int fic_gpio_close(int fd_lock);

extern int gpio_unlock();
extern int gpio_lock();