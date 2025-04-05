#include "Gpio5.h"

uint32_t *PERIBase;
uint32_t *GPIOBase;
uint32_t *RIOBase;
uint32_t *PADBase;
uint32_t *pad;

uint32_t *ClockBase;
uint32_t *PWMBase;

int sleep_ms(int ms)
{
    struct timespec delay = {0, ms * 1000 * 1000};
    return nanosleep(&delay, NULL);
}

void sleep_us(int us){ 
    if (us < 100)
    {       
         volatile int i;
        for (i = 0; i < 403*us-5;)        {
            i++;
        } 
  }
    else
    {
        struct timespec delay = {0, us * 1000};
        nanosleep(&delay, NULL);
    }
    return;
}

uint32_t time_us_32()
{
    struct timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    return t.tv_sec * 1000 * 1000 + t.tv_nsec / 1000;
}

int rp1_Init()
{
    int memfd = open("/dev/mem", O_RDWR | O_SYNC);
    uint32_t *map = (uint32_t *)mmap(
        NULL,
        64 * 1024 * 1024,
        (PROT_READ | PROT_WRITE),
        MAP_SHARED,
        memfd,
        0x1f00000000);
    close(memfd);
    PERIBase = map;
    if (map == MAP_FAILED)
    {
        int memfd = open("/dev/gpiomem0", O_RDWR | O_SYNC);
        uint32_t *map = (uint32_t *)mmap(
            NULL,
            576 * 1024,
            (PROT_READ | PROT_WRITE),
            MAP_SHARED,
            memfd,
            0x0);
        if (map == MAP_FAILED)
        {
            printf("mmap failed: %s\n", strerror(errno));
            return (-1);
        };
        close(memfd);
        PERIBase = map - 0xD0000 / 4;
    };

    GPIOBase = PERIBase + 0xD0000 / 4;
    RIOBase = PERIBase + 0xe0000 / 4;
    PADBase = PERIBase + 0xf0000 / 4;
    pad = PADBase + 1;
    ClockBase = PERIBase + 0x18000 / 4;
    PWMBase = PERIBase + 0x98000 / 4;
    return 0;
}

// Initialize GPIO lines
void gpio_init(uint32_t gpio)
{
    gpio_set_dir(gpio, GPIO_IN);
    gpio_put(gpio, 0);
    gpio_set_function(gpio, GPIO_FUNC_RIO);
}

void gpio_deinit(uint32_t gpio)
{
    gpio_set_function(gpio, GPIO_FUNC_NULL);
}

void gpio_init_mask(uint32_t gpio_mask)
{
    for (int i = 0; i < 29; i++)
    {
        if (gpio_mask & 1)
        {
            gpio_init(i);
        }
        gpio_mask >>= 1;
    }
}
void gpio_set_function(uint32_t gpio, enum gpio_function_rp1 fn)
{
    pad[gpio] = 0x50;
    GPIO[gpio].ctrl = fn;
}

// Set Direction
void gpio_set_dir(uint32_t gpio, bool out)
{
    uint32_t mask = 1ul << gpio;
    if (out)
        gpio_set_dir_out_masked(mask);
    else
        gpio_set_dir_in_masked(mask);
}
void gpio_set_dir_in_masked(uint32_t mask)
{
    rioCLR->OE = mask;
}
void gpio_set_dir_out_masked(uint32_t mask)
{
    rioSET->OE = mask;
}
void gpio_set_dir_masked(uint32_t mask, uint32_t value)
{
    rioXOR->OE = (rio->OE ^ value) & mask;
}

// SETGET GPIO
void gpio_set_mask(uint32_t mask)
{
    rioSET->Out = mask;
}

void gpio_clr_mask(uint32_t mask)
{
    rioCLR->Out = mask;
}
void gpio_xor_mask(uint32_t mask)
{

    rioXOR->Out = mask;
}

void gpio_put(uint32_t gpio, bool value)
{
    uint32_t mask = 1ul << gpio;
    if (value)
        gpio_set_mask(mask);
    else
        gpio_clr_mask(mask);
}

void gpio_put_masked(uint32_t mask, uint32_t value)
{
    rioXOR->Out = (rio->Out ^ value) & mask;
}

bool gpio_get(uint32_t gpio)
{
    return rio->In & (1u << gpio);
}

uint32_t gpio_get_all(void)
{
    return rio->In;
}

// PAD Configure
void gpio_set_pulls(uint32_t gpio, bool up, bool down)
{
    pad[gpio] = pad[gpio] & ~0xC;
    if (up)
    {
        pad[gpio] = pad[gpio] | 0x8;
    };
    if (down)
    {
        pad[gpio] = pad[gpio] | 0x4;
    }
}
void gpio_pull_down(uint32_t gpio)
{
    gpio_set_pulls(gpio, false, true);
}
void gpio_pull_up(uint32_t gpio)
{
    gpio_set_pulls(gpio, true, false);
}
void gpio_disable_pulls(uint32_t gpio)
{
    gpio_set_pulls(gpio, false, false);
}
bool gpio_is_pulled_up(uint32_t gpio)
{
    return (pad[gpio] & 0x8) != 0;
}
bool gpio_is_pulled_down(uint32_t gpio)
{
    return (pad[gpio] & 0x4) != 0;
}
void gpio_set_input_hysteresis_enabled(uint32_t gpio, bool enabled)
{
    if (enabled)
        pad[gpio] = pad[gpio] | 0x2;
    else
        pad[gpio] = pad[gpio] & !0x2;
}
bool gpio_is_input_hysteresis_enabled(uint32_t gpio)
{
    return (pad[gpio] & 0x2) != 0;
}
void gpio_set_slew_rate(uint32_t gpio, enum gpio_slew_rate slew)
{
    if (slew == GPIO_SLEW_RATE_FAST)
    {
        pad[gpio] = pad[gpio] | 1;
    }
    else
    {
        pad[gpio] = pad[gpio] & ~1;
    }
}
enum gpio_slew_rate gpio_get_slew_rate(uint32_t gpio)
{
    return (enum gpio_slew_rate)(pad[gpio] & 1);
}

void gpio_set_drive_strength(uint32_t gpio, enum gpio_drive_strength drive)
{
    pad[gpio] = (pad[gpio] & ~0x30) | (0x30 & (uint32_t)drive << 4);
}
enum gpio_drive_strength gpio_get_drive_strength(uint32_t gpio)
{
    return (enum gpio_drive_strength)((pad[gpio] & 0x30) >> 4);
}

// PWM

int getPWM(uint32_t gpio)
{
    switch (gpio)
    {
    case 12:
        return 0;
    case 13:
        return 1;
    case 14:
        return 2;
    case 15:
        return 3;
    case 18:
        return 2;
    case 19:
        return 3;
    }
    return -1;
}

int pwm_enable(uint32_t gpio)
{
    int pwm = getPWM(gpio);
    if (pwm < 0)
        return -1;
    uint32_t temp = 1 << pwm;
    *PWMBase = *PWMBase | temp | 0x80000000;
    return 0;
}
int pwm_disable(uint32_t gpio)
{
    int pwm = getPWM(gpio);
    if (pwm < 0)
        return -1;
    uint32_t temp = 1 << pwm;
    temp = ~temp & 0xf;
    *PWMBase = (*PWMBase & temp) | 0x80000000;
    return 0;
}

void pwm_init_clock(void)
{
    PWMCLK->PWM0_CTRL = 0x11000840;
    PWMCLK->PWM0_SEL = 1;
}
void pwm_set_clock(uint32_t div, uint32_t frac)
{
    PWMCLK->PWM0_DIV_INT = div;
    PWMCLK->PWM0_DIV_FRAC = frac;
}
int pwm_setup(uint32_t gpio, enum pwm_mode_rp1 mode)
{
    int pwm = getPWM(gpio);
    if (pwm < 0)
        return -1;
    pwm_init_clock();
    if (gpio == 18 | gpio == 19)
    {
        gpio_set_function(gpio, GPIO_FUNC_PWM2);
    }
    else
    {
        gpio_set_function(gpio, GPIO_FUNC_PWM1);
    }
    PWM[pwm].Ctrl = mode;
    pwm_disable(gpio);
    return 0;
}

int pwm_set_invert(int32_t gpio)
{
    int pwm = getPWM(gpio);
    if (pwm < 0)
        return -1;
    PWM[pwm].Ctrl = PWM[pwm].Ctrl | 0x8;
    *PWMBase = *PWMBase | 0x80000000;
    return 0;
}
int pwm_clr_invert(int32_t gpio)
{
    int pwm = getPWM(gpio);
    if (pwm < 0)
        return -1;
    PWM[pwm].Ctrl = PWM[pwm].Ctrl & ~0x8ul;
    *PWMBase = *PWMBase | 0x80000000;
    return 0;
}

int pwm_set_range_duty_phase(int32_t gpio, uint32_t range, uint32_t duty, uint32_t phase)
{
    int pwm = getPWM(gpio);
    if (pwm < 0)
        return -1;
    PWM[pwm].Range = range;
    PWM[pwm].Duty = duty;
    PWM[pwm].Phase = phase;
    *PWMBase = *PWMBase | 0x80000000;
    return 0;
}

int pwm_set_frequency_duty(int32_t gpio, int32_t freq, int dutyPercent)
{
    int32_t div = PWMCLK->PWM0_DIV_INT;
    int32_t frac = PWMCLK->PWM0_DIV_FRAC;
    int32_t pwmf = PWMClock / div;
    int32_t range = pwmf / freq-1;
    int32_t duty = range * dutyPercent / 1000+1;
    pwm_set_range_duty_phase(gpio, range, duty, 0);
    return 0;
}

// SPI

void dump_all_spi_regs(SPI spi, const char *msg)
{

    printf("\nSPI register dump:%s\n", msg);
    uint32_t *i;
    int j = 1;
    for (i = (uint32_t *)spi; i <= (uint32_t *)&(spi->SSI_VERSION_ID); i++)
    {
        printf("spi reg %x @  %x %p: %x\n", j++, (char *)i - (char *)spi, i, *i);
    }
}

void spi_enable(SPI spi, bool enable)
{
    if (enable)
    {
        spi->SSIENR = 0x1;
    }
    else
    {
        spi->SSIENR = 0x0;
    }
}

void spi_init(SPI spi, int32_t baudrate)
{

    //  dump_all_spi_regs(spi, "Just after spi created");
    spi_set_baudrate(spi, baudrate);
    spi->BAUDR = SPIClock / baudrate;
    spi_enable(spi, false);
    // set CPOL and CHPA
    spi_set_format(spi, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    spi_enable(spi, false);
    // set CE to stay active (bit 24 set low)
    spi->CTRLR0 = (spi->CTRLR0) & 0xFFFFFFFFFEFFFFFF;
    // set CE0 as default
    spi->SER = 1;
    //  clear interrupts
    uint32_t reg_icr = spi->ICR;
    spi_enable(spi, true);

    //  dump_all_spi_regs(spi, "Just after spi setting");
}

void spi_set_format(SPI spi, uint32_t data_bits, spi_cpol_t cpol, spi_cpha_t cpha, spi_order_t order)
{
    spi_enable(spi, false);
    // Bit 16 to 20 databits
    //  6 CPHA
    //  7 CPOL
    uint32_t mask = 0x1F00C0;
    data_bits = (data_bits - 1) & 0x1F;
    uint32_t value = data_bits << 16 | cpol << 7 | cpha << 6;
    int32_t data = spi->CTRLR0;
    spi->CTRLR0 = (data & ~mask) | (value & mask);
    spi_enable(spi, true);
}

bool spi_is_writable(SPI spi)
{
    return (spi->SR) & 0x2;
}

bool spi_is_readable(SPI spi)
{
    return spi->SR & 0x8;
}

bool spi_is_busy(SPI spi)
{
    return spi->SR & 0x1;
}
bool spi_rx_full(SPI spi)
{
    return spi->SR & 0x10;
}

int spi_rx_num(SPI spi)
{
    return spi->RXFLR;
}
int spi_tx_num(SPI spi)
{
    return spi->TXFLR;
}

void spi_set_baudrate(SPI spi, int32_t baudrate)
{
    int32_t div = SPIClock / baudrate;
    if (div % 2 != 0)
        div = div - 1;
    spi_enable(spi, false);
    spi->BAUDR = div;
    spi_enable(spi, true);
}

int32_t spi_get_baudrate(SPI spi)
{
    return SPIClock / (spi->BAUDR);
}

int spi_write_read_blocking(SPI spi, const uint8_t *src, uint8_t *dst, size_t len)
{
    const size_t fifo_depth = 64;
    size_t rx_remaining = len, tx_remaining = len;
    for (int i = 0; i < (fifo_depth < len ? fifo_depth : len) - 1; i++)
    {
        spi->DR = (uint32_t)*src++;
        --tx_remaining;
    }
    while (rx_remaining || tx_remaining)
    {
        if (tx_remaining && spi_is_writable(spi) && spi_rx_num(spi) < fifo_depth)
        {
            spi->DR = (uint32_t)*src++;
            --tx_remaining;
        }
        if (rx_remaining && spi_is_readable(spi))
        {
            *dst++ = (uint8_t)spi->DR;
            --rx_remaining;
        }
    }
    return (int)len;
}

int spi_write16_read16_blocking(SPI spi, const uint16_t *src, uint16_t *dst, size_t len)
{
    const size_t fifo_depth = 64;
    size_t rx_remaining = len, tx_remaining = len;
    for (int i = 0; i < (fifo_depth < len ? fifo_depth : len) - 1; i++)
    {
        spi->DR = (uint32_t)*src++;
        --tx_remaining;
    }
    while (rx_remaining || tx_remaining)
    {
        if (tx_remaining && spi_is_writable(spi) && spi_rx_num(spi) < fifo_depth)
        {
            spi->DR = (uint32_t)*src++;
            --tx_remaining;
        }
        if (rx_remaining && spi_is_readable(spi))
        {
            *dst++ = (uint16_t)spi->DR;
            --rx_remaining;
        }
    }
    return (int)len;
}

int spi_write32_read32_blocking(SPI spi, const uint32_t *src, uint32_t *dst, size_t len)
{
    const size_t fifo_depth = 64;
    size_t rx_remaining = len, tx_remaining = len;
    for (int i = 0; i < (fifo_depth < len ? fifo_depth : len) - 1; i++)
    {
        spi->DR = (uint32_t)*src++;
        --tx_remaining;
    }
    while (rx_remaining || tx_remaining)
    {
        if (tx_remaining && spi_is_writable(spi) && spi_rx_num(spi) < fifo_depth)
        {
            spi->DR = (uint32_t)*src++;
            --tx_remaining;
        }
        if (rx_remaining && spi_is_readable(spi))
        {
            *dst++ = (uint32_t)spi->DR;
            --rx_remaining;
        }
    }
    return (int)len;
}

int spi_write_blocking(SPI spi, const uint8_t *src, size_t len)
{
    uint8_t dst[len];
    return spi_write_read_blocking(spi, src, dst, len);
}

int spi_read_blocking(SPI spi, uint8_t repeated_tx_data, uint8_t *dst, size_t len)
{
    uint8_t src[len];
    for (int i = 0; i < len; i++)
    {
        src[i] = repeated_tx_data;
    }
    return spi_write_read_blocking(spi, src, dst, len);
}
int spi_write16_blocking(SPI spi, const uint16_t *src, size_t len)
{
    uint16_t dst[len];
    return spi_write16_read16_blocking(spi, src, dst, len);
}

int spi_read16_blocking(SPI spi, uint16_t repeated_tx_data, uint16_t *dst, size_t len)
{
    uint16_t src[len];
    for (int i = 0; i < len; i++)
    {
        src[i] = repeated_tx_data;
    }
    return spi_write16_read16_blocking(spi, src, dst, len);
}
int spi_write32_blocking(SPI spi, const uint32_t *src, size_t len)
{
    uint32_t dst[len];
    return spi_write32_read32_blocking(spi, src, dst, len);
}

int spi_read32_blocking(SPI spi, uint32_t repeated_tx_data, uint32_t *dst, size_t len)
{
    uint32_t src[len];
    for (int i = 0; i < len; i++)
    {
        src[i] = repeated_tx_data;
    }
    return spi_write32_read32_blocking(spi, src, dst, len);
}

void spi_set_slave(SPI spi, int slave)
{
    spi_enable(spi, false);
    spi->SER = 1ul << slave;
    spi_enable(spi, true);
}
void spi_set_CS_toggle(SPI spi, bool enable)
{
    spi_enable(spi, false);
    if (enable)
    {
        spi->CTRLR0 = (spi->CTRLR0) | 0x1000000;
    }
    else
    {
        spi->CTRLR0 = (spi->CTRLR0) & 0xFFFFFFFFFEFFFFFF;
    }
    spi_enable(spi, true);
}

// I2C
void dump_all_i2c_regs(I2C i2c, const char *msg)
{
    printf("\nI2C register dump:%s\n", msg);
    uint32_t *i;
    int j = 1;
    for (i = (uint32_t *)i2c; i <= (uint32_t *)&(i2c->comp_type); i++)
    {
        printf("i2c reg %x @  %x %p: %x\n", j++, (char *)i - (char *)i2c, i, *i);
    }
}

void i2c_enable(I2C i2c, bool enable)
{
    i2c->enable = enable ? 1 : 0;
}

uint32_t i2c_init(I2C i2c, uint32_t baudrate)
{
    // dump_all_i2c_regs(i2c, "Before init");
    i2c_enable(i2c, false);
    // Configure as a fast-mode master with RepStart support, 7-bit addresses
    i2c->con = (0x2ul << 1) | 0x01 | 0x040 | 0x20 | 0x100;
    // Set FIFO watermarks to 1
    i2c->tx_tl = 0;
    i2c->rx_tl = 0;
    return i2c_set_baudrate(i2c, baudrate);
}

int32_t i2c_set_baudrate(I2C i2c, int32_t baudrate)
{
    i2c_enable(i2c, false);
    // use "fast" mode
    i2c->con = (i2c->con & ~0x06ul) | (0x02 << 1 & 0x06ul);
    // set frequency and duty
    uint32_t period = (I2CClock + baudrate / 2) / baudrate;
    i2c->fs_scl_lcnt = period * 3 / 5; // 40% duty cycle
    i2c->fs_scl_hcnt = period - i2c->fs_scl_lcnt;

    // set spike suppression
    i2c->fs_spklen = i2c->fs_scl_lcnt < 16 ? 1 : i2c->fs_scl_lcnt / 16;

    // set hold time
    uint32_t sda_tx_hold_count = (baudrate < 1000000) ? ((I2CClock * 3) / 10000000) + 1 : ((I2CClock * 3) / 25000000) + 1;
    i2c->sda_hold = (i2c->sda_hold & ~0x0000ffff) | (sda_tx_hold_count & 0x0000ffff);

    i2c_enable(i2c, true);
    // dump_all_i2c_regs(i2c, "after init");
    return I2CClock / period;
}

void i2c_reset(I2C spi)
{
    uint32_t *resetreg = PERIBase + 14000 / 4;
    uint32_t *resetdonereg = PERIBase + 14004 / 4;
    volatile int32_t temp = *resetreg;
    *resetreg = *resetreg | 0x00020000;
    temp = *resetreg;
    /*    do
       {
   volatile int32_t temp=(*resetdonereg & 0x00020000);
       } while (!(*resetdonereg & 0x00020000));
        */
}
bool restart_on_next = false;

uint64_t micros()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    uint64_t us = ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
    return us;
}

int32_t i2c_handleAbort(I2C i2c, bool timeout, int32_t abortreason)
{
    if (timeout)
        return 1 << 31 | 1 << 30; // bit 30 set for timout
    return abortreason | 1 << 31;
}

int i2c_write_blocking_internal(I2C i2c, uint8_t addr, const uint8_t *src, size_t len, bool nostop, uint32_t timeout_per_char_us)
{
    i2c_enable(i2c, false);
    i2c->tar = addr;
    i2c_enable(i2c, true);

    bool abort = false;
    bool timeout = false;
    uint32_t abort_reason = 0;

    int byte_ctr;

    int ilen = (int)len;
    for (byte_ctr = 0; byte_ctr < ilen; ++byte_ctr)
    {
        bool first = byte_ctr == 0;
        bool last = byte_ctr == ilen - 1;

        uint32_t startbitnext = ((uint32_t)!!(first && restart_on_next)) << 10;
        uint32_t stopbit = ((uint32_t)!!(last && !nostop)) << 9;
        uint64_t tm = micros() + timeout_per_char_us;
        i2c->data_cmd = startbitnext | stopbit | *src++;
        do
        {
            if (micros() > tm)
                timeout = true;
        } while (!timeout && !(i2c->raw_intr_stat & 0x10));

        if (timeout)
            break;

        // check for non-timeout abort
        abort_reason = i2c->tx_abrt_source;
        if (abort_reason)
        {
            int32_t temp = i2c->clr_tx_abrt;
            abort = true;
        }
        /*  if (abort || (last && !nostop))
         {
             do
             {
                 if (micros() > tm)
                     timeout = true;
             } while (!timeout && !(i2c->raw_intr_stat & 0x10));
             i2c->clr_stop_det;
             if (timeout || abort)
                 break;
         } */
    }
    restart_on_next = nostop;
    if (abort || timeout)
        return i2c_handleAbort(i2c, timeout, abort_reason);
    return byte_ctr;
}

size_t i2c_get_write_available(I2C i2c)
{
    return I2C_TX_BUFFER_DEPTH - (i2c->txflr);
}
size_t i2c_get_read_available(I2C i2c)
{
    return i2c->rxflr;
}

int i2c_read_blocking_internal(I2C i2c, uint8_t addr, uint8_t *dst, size_t len, bool nostop, uint32_t timeout_per_char_us)
{

    i2c_enable(i2c, false);
    i2c->tar = addr;
    i2c_enable(i2c, true);

    bool abort = false;
    bool timeout = false;
    uint32_t abort_reason;

    int byte_ctr;
    int ilen = (int)len;

    for (byte_ctr = 0; byte_ctr < ilen; ++byte_ctr)
    {
        bool first = byte_ctr == 0;
        bool last = byte_ctr == ilen - 1;

        while (!i2c_get_write_available(i2c))
        {
        };

        uint32_t startbitnext = (uint32_t)!!(first && restart_on_next) << 10;
        uint32_t stopbit = (uint32_t)!!(last && !nostop) << 9;
        uint64_t tm = micros() + timeout_per_char_us;
        i2c->data_cmd = startbitnext | stopbit | 0x100;

        do
        {
            if (micros() > tm)
            {
                timeout = true;
                abort = true;
            }
            abort_reason = i2c->tx_abrt_source;
            // check tx abort bits
            if (i2c->raw_intr_stat & 0x40)
            {
                abort = true;
                i2c->clr_tx_abrt;
            }

        } while (!abort && !i2c_get_read_available(i2c));

        if (abort)
            break;

        *dst++ = (uint8_t)i2c->data_cmd;
    }
    restart_on_next = nostop;
    if (abort)
        return i2c_handleAbort(i2c, timeout, abort_reason);
    return byte_ctr;
}

int i2c_read_blocking(I2C i2c, uint8_t addr, uint8_t *dst, size_t len, bool nostop)
{
    return i2c_read_blocking_internal(i2c, addr, dst, len, nostop, 0xFFFFFFFF);
}
int i2c_write_blocking(I2C i2c, uint8_t addr, const uint8_t *src, size_t len, bool nostop)
{
    return i2c_write_blocking_internal(i2c, addr, src, len, nostop, 0xFFFFFFFF);
}

int i2c_write_timeout_per_char_us(I2C i2c, uint8_t addr, const uint8_t *src, size_t len, bool nostop, uint32_t timeout_per_char_us)
{
    return i2c_write_blocking_internal(i2c, addr, src, len, nostop, timeout_per_char_us);
}

int i2c_read_timeout_per_char_us(I2C i2c, uint8_t addr, uint8_t *dst, size_t len, bool nostop, uint32_t timeout_per_char_us)
{

    return i2c_read_blocking_internal(i2c, addr, dst, len, nostop, timeout_per_char_us);
}

// spi->CTRLR0 = (data & ~mask) | (value & mask);
