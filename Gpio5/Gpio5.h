#ifndef GPIO5_H
#define GPIO5_H
#undef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

#include <unistd.h>
#include <time.h>

#define GPIO_OUT 1
#define GPIO_IN 0

#define rp1_GPIO_FSEL_INPT 1
#define rp1_GPIO_FSEL_OUTP 2

enum gpio_function_rp1
{
    GPIO_FUNC_I2C = 3,
    GPIO_FUNC_PWM1 = 0,
    GPIO_FUNC_SPI = 0,
    GPIO_FUNC_PWM2 = 3,
    GPIO_FUNC_RIO = 5,
    GPIO_FUNC_NULL = 0x1f
};


enum gpio_slew_rate
{
    GPIO_SLEW_RATE_SLOW = 0, ///< Slew rate limiting enabled
    GPIO_SLEW_RATE_FAST = 1  ///< Slew rate limiting disabled
};

enum gpio_drive_strength
{
    GPIO_DRIVE_STRENGTH_2MA = 0, ///< 2 mA nominal drive strength
    GPIO_DRIVE_STRENGTH_4MA = 1, ///< 4 mA nominal drive strength
    GPIO_DRIVE_STRENGTH_8MA = 2, ///< 8 mA nominal drive strength
    GPIO_DRIVE_STRENGTH_12MA = 3 ///< 12 mA nominal drive strength
};

typedef struct
{
    uint32_t status;
    uint32_t ctrl;
} GPIOregs;
#define GPIO ((GPIOregs *)GPIOBase)

typedef struct
{
    uint32_t Out;
    uint32_t OE;
    uint32_t In;
    uint32_t InSync;
} rioregs;

#define rio ((rioregs *)RIOBase)
#define rioXOR ((rioregs *)(RIOBase + 0x1000 / 4))
#define rioSET ((rioregs *)(RIOBase + 0x2000 / 4))
#define rioCLR ((rioregs *)(RIOBase + 0x3000 / 4))

// CLOCKS
typedef struct
{
    uint32_t PWM0_CTRL;
    uint32_t PWM0_DIV_INT;
    uint32_t PWM0_DIV_FRAC;
    uint32_t PWM0_SEL;
} pwmclockregs;
#define PWMCLK ((pwmclockregs *)(ClockBase + 0x74 / 4))

// PWM
typedef struct
{
    uint32_t Ctrl;
    uint32_t Range;
    uint32_t Phase;
    uint32_t Duty;
} PWMregs;
#define PWM ((PWMregs *)(PWMBase + 0x14 / 4))


enum pwm_mode_rp1
{
    Zero = 0x0,
    TrailingEdge = 0x1,
    PhaseCorrect = 0x2,
    PDE = 0x3,
    MSBSerial = 0x4,
    PPM = 0x5,
    LeadingEdge = 0x6,
    LSBSerial = 0x7
};


#define PWMClock 50000000

// SPI
extern uint32_t *PERIBase;

volatile typedef struct
{
    int32_t CTRLR0; // frame format, clock polarity, phase
    int32_t CTRLR1;
    int32_t SSIENR; // enable/disable
    int32_t MWCR;
    int32_t SER;   // slave CS enable
    int32_t BAUDR; // baud rate - clock divisor
    int32_t TXFTLR;
    int32_t RXFTLR;
    int32_t TXFLR;
    int32_t RXFLR;
    int32_t SR;  // status register
    int32_t IMR; // interrupt mask register
    int32_t ISR;
    int32_t RISR;
    int32_t TXOICR;
    int32_t RXOICR;
    int32_t RXUICR;
    int32_t MSTICR;
    int32_t ICR;
    int32_t DMACR;
    int32_t DMATDLR;
    int32_t DMARDLR;
    int32_t IDR;
    int32_t SSI_VERSION_ID;
    int32_t DR; // data register
    int32_t DRx[35];
    int32_t RX_SAMPLE_DLY;
    int32_t SPI_CTRLR0;
    int32_t TXD_DRIVE_EDGE;
} SPIregs;

typedef SPIregs *SPI;

#define RP1_SPI0_BASE 0x050000
#define RP1_SPI1_BASE 0x054000
#define RP1_SPI2_BASE 0x058000
#define RP1_SPI3_BASE 0x05c000
#define RP1_SPI4_BASE 0x060000
#define RP1_SPI5_BASE 0x064000

#define SPI0 ((SPI)(PERIBase + RP1_SPI0_BASE / 4))
#define SPI1 ((SPI)(PERIBase + RP1_SPI1_BASE / 4))
#define SPI2 ((SPI)(PERIBase + RP1_SPI2_BASE / 4))
#define SPI3 ((SPI)(PERIBase + RP1_SPI3_BASE / 4))
#define SPI4 ((SPI)(PERIBase + RP1_SPI4_BASE / 4))
#define SPI5 ((SPI)(PERIBase + RP1_SPI5_BASE / 4))

#define SPIClock 200000000

typedef enum
{
    SPI_OK = 0,
    SPI_ERROR = 1,
    SPI_BUSY = 2,
    SPI_TIMEOUT = 3,
    SPI_INVALID = 4
} spi_status_t;
typedef enum
{
    SPI_CPHA_0 = 0,
    SPI_CPHA_1 = 1
} spi_cpha_t;

typedef enum
{
    SPI_CPOL_0 = 0,
    SPI_CPOL_1 = 1
} spi_cpol_t;

typedef enum
{
    SPI_LSB_FIRST = 0,
    SPI_MSB_FIRST = 1
} spi_order_t;

// I2C

volatile typedef struct
{
    int32_t con;
    int32_t tar;
    int32_t sar;
    uint32_t _pad0;
    int32_t data_cmd;
    int32_t ss_scl_hcnt;
    int32_t ss_scl_lcnt;
    int32_t fs_scl_hcnt;
    int32_t fs_scl_lcnt;
    uint32_t _pad1[2];
    uint32_t intr_stat;
    int32_t intr_mask;
    int32_t raw_intr_stat;
    int32_t rx_tl;
    int32_t tx_tl;
    int32_t clr_intr;
    int32_t clr_rx_under;
    int32_t clr_rx_over;
    int32_t clr_tx_over;
    int32_t clr_rd_req;
    int32_t clr_tx_abrt;
    int32_t clr_rx_done;
    int32_t clr_activity;
    int32_t clr_stop_det;
    int32_t clr_start_det;
    int32_t clr_gen_call;
    int32_t enable;
    int32_t status;
    int32_t txflr;
    int32_t rxflr;
    int32_t sda_hold;
    int32_t tx_abrt_source;
    int32_t slv_data_nack_only;
    int32_t dma_cr;
    int32_t dma_tdlr;
    int32_t dma_rdlr;
    int32_t sda_setup;
    int32_t ack_general_call;
    int32_t enable_status;
    int32_t fs_spklen;
    uint32_t _pad2;
    int32_t clr_restart_det;
    int32_t SCL_STUCK_AT_LOW_TIMEOUT;
    int32_t IC_SDA_STUCK_AT_LOW_TIMEOUT;
    uint32_t _pad3[16];
    uint32_t comp_param_1;
    uint32_t comp_version;
    uint32_t comp_type;
} i2cregs;

typedef i2cregs *I2C;
#define RP1_I2C0_BASE 0x070000
#define RP1_I2C1_BASE 0x074000
#define RP1_I2C2_BASE 0x078000
#define RP1_I2C3_BASE 0x07c000
#define RP1_I2C4_BASE 0x080000
#define RP1_I2C5_BASE 0x084000
#define RP1_I2C6_BASE 0x088000

#define I2C0 ((I2C)(PERIBase + RP1_I2C0_BASE / 4))
#define I2C1 ((I2C)(PERIBase + RP1_I2C1_BASE / 4))
#define I2C2 ((I2C)(PERIBase + RP1_I2C2_BASE / 4))
#define I2C3 ((I2C)(PERIBase + RP1_I2C3_BASE / 4))
#define I2C4 ((I2C)(PERIBase + RP1_I2C4_BASE / 4))
#define I2C5 ((I2C)(PERIBase + RP1_I2C5_BASE / 4))

#define I2CClock 200000000
#define I2C_TX_BUFFER_DEPTH 32;

// Function declarations
int rp1_Init();

int sleep_ms(int ms);
uint32_t time_us_32();
void sleep_us(int us);

void gpio_set_dir(uint32_t gpio, bool out);
void gpio_set_dir_in_masked(uint32_t mask);
void gpio_set_dir_out_masked(uint32_t mask);
void gpio_set_dir_masked(uint32_t mask, uint32_t value);

void gpio_set_mask(uint32_t mask);
void gpio_clr_mask(uint32_t mask);
void gpio_xor_mask(uint32_t mask);

void gpio_put(uint32_t gpio, bool value);
void gpio_put_masked(uint32_t mask, uint32_t value);
bool gpio_get(uint32_t gpio);
uint32_t gpio_get_all(void);
void gpio_set_function(uint32_t gpio, enum gpio_function_rp1 fn);

void gpio_init(uint32_t gpio);
void gpio_init_mask(uint32_t gpio_mask);
// PAD
// Pull
void gpio_set_pulls(uint32_t gpio, bool up, bool down);
void gpio_pull_down(uint32_t gpio);
void gpio_pull_up(uint32_t gpio);
void gpio_disable_pulls(uint32_t gpio);
bool gpio_is_pulled_up(uint32_t gpio);
bool gpio_is_pulled_down(uint32_t gpio);
// Other
void gpio_set_input_hysteresis_enabled(uint32_t gpio, bool enabled);
bool gpio_is_input_hysteresis_enabled(uint32_t gpio);
void gpio_set_slew_rate(uint32_t gpio, enum gpio_slew_rate slew);
enum gpio_slew_rate gpio_get_slew_rate(uint32_t gpio);
void gpio_set_drive_strength(uint32_t gpio, enum gpio_drive_strength drive);
enum gpio_drive_strength gpio_get_drive_strength(uint32_t gpio);

// PWM
int pwm_setup(uint32_t gpio, enum pwm_mode_rp1 mode);
int pwm_enable(uint32_t gpio);
int pwm_disable(uint32_t gpio);
void pwm_set_clock(uint32_t div, uint32_t frac);
int pwm_set_invert(int32_t gpio);
int pwm_clr_invert(int32_t gpio);
int pwm_set_range_duty_phase(int32_t gpio, uint32_t range, uint32_t duty, uint32_t phase);
int pwm_set_frequency_duty(int32_t gpio, int32_t freq, int dutyPercent);
// SPI
void spi_init(SPI spi, int32_t baudrate);
void spi_set_format(SPI spi, uint32_t data_bits, spi_cpol_t cpol, spi_cpha_t cpha, spi_order_t order);
void spi_set_baudrate(SPI spi, int32_t baudrate);
int32_t spi_get_baudrate(SPI spi);

void spi_set_slave(SPI spi, int slave);
void spi_set_CS_toggle(SPI spi, bool enable);

int spi_write_read_blocking(SPI spi, const uint8_t *src, uint8_t *dst, size_t len);
int spi_write_blocking(SPI spi, const uint8_t *src, size_t len);
int spi_read_blocking(SPI spi, uint8_t repeated_tx_data, uint8_t *dst, size_t len);

int spi_write16_read16_blocking(SPI spi, const uint16_t *src, uint16_t *dst, size_t len);
int spi_write16_blocking(SPI spi, const uint16_t *src, size_t len);
int spi_read16_blocking(SPI spi, uint16_t repeated_tx_data, uint16_t *dst, size_t len);

int spi_write32_read32_blocking(SPI spi, const uint32_t *src, uint32_t *dst, size_t len);
int spi_write32_blocking(SPI spi, const uint32_t *src, size_t len);
int spi_read32_blocking(SPI spi, uint32_t repeated_tx_data, uint32_t *dst, size_t len);

uint32_t i2c_init(I2C i2c, uint32_t baudrate);
int32_t i2c_set_baudrate(I2C i2c, int32_t baudrate);
void i2c_reset(I2C i2c);

int i2c_read_blocking(I2C i2c, uint8_t addr, uint8_t *dst, size_t len, bool nostop);
int i2c_write_blocking(I2C i2c, uint8_t addr, const uint8_t *src, size_t len, bool nostop);

int i2c_write_timeout_per_char_us(I2C i2c, uint8_t addr, const uint8_t *src, size_t len, bool nostop, uint32_t timeout_per_char_us);
int i2c_read_timeout_per_char_us(I2C i2c, uint8_t addr, uint8_t *dst, size_t len, bool nostop, uint32_t timeout_per_char_us);

#endif
