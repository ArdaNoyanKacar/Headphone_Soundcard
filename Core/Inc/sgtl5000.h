#ifndef SGTL5000_H
#define SGTL5000_H

#include "stm32f4xx_hal.h"
#include "stdint.h"
#include "stdbool.h"

// Success & Fail
#define I2C_SUCCESS 0
#define I2C_FAIL 1
#define I2C_MISMATCH 2

// I2C Address
#define SGTL5000_ADDR 0x0A << 1 // 7-bit address shifted left for HAL I2C

// Volume
#define HP_VOL_MIN 0x7F
#define HP_VOL_MAX 0x00 // +12dB

// I2S
#define I2S_USE_DEFAULT 0xFF

// Register Address
#define SGTL5000_CHIP_ID				0x0000
#define SGTL5000_CHIP_DIG_POWER			0x0002
#define SGTL5000_CHIP_CLK_CTRL			0x0004
#define SGTL5000_CHIP_I2S_CTRL			0x0006
#define SGTL5000_CHIP_SSS_CTRL			0x000A
#define SGTL5000_CHIP_ADCDAC_CTRL		0x000E
#define SGTL5000_CHIP_DAC_VOL			0x0010
#define SGTL5000_CHIP_PAD_STRENGTH		0x0014
#define SGTL5000_CHIP_ANA_ADC_CTRL		0x0020
#define SGTL5000_CHIP_ANA_HP_CTRL		0x0022
#define SGTL5000_CHIP_ANA_CTRL			0x0024
#define SGTL5000_CHIP_LINREG_CTRL		0x0026
#define SGTL5000_CHIP_REF_CTRL			0x0028
#define SGTL5000_CHIP_MIC_CTRL			0x002A
#define SGTL5000_CHIP_LINE_OUT_CTRL		0x002C
#define SGTL5000_CHIP_LINE_OUT_VOL		0x002E
#define SGTL5000_CHIP_ANA_POWER			0x0030
#define SGTL5000_CHIP_PLL_CTRL			0x0032
#define SGTL5000_CHIP_CLK_TOP_CTRL		0x0034
#define SGTL5000_CHIP_ANA_STATUS		0x0036
#define SGTL5000_CHIP_SHORT_CTRL		0x003C
#define SGTL5000_CHIP_ANA_TEST2			0x003A
#define SGTL5000_DAP_CTRL				0x0100
#define SGTL5000_DAP_PEQ				0x0102
#define SGTL5000_DAP_BASS_ENHANCE		0x0104
#define SGTL5000_DAP_BASS_ENHANCE_CTRL	0x0106
#define SGTL5000_DAP_AUDIO_EQ			0x0108
#define SGTL5000_DAP_SURROUND			0x010A
#define SGTL5000_DAP_FLT_COEF_ACCESS	0x010C
#define SGTL5000_DAP_COEF_WR_B0_MSB		0x010E
#define SGTL5000_DAP_COEF_WR_B0_LSB		0x0110
#define SGTL5000_DAP_EQ_BASS_BAND0		0x0116
#define SGTL5000_DAP_EQ_BASS_BAND1		0x0118
#define SGTL5000_DAP_EQ_BASS_BAND2		0x011A
#define SGTL5000_DAP_EQ_BASS_BAND3		0x011C
#define SGTL5000_DAP_EQ_BASS_BAND4		0x011E
#define SGTL5000_DAP_MAIN_CHAN			0x0120
#define SGTL5000_DAP_MIX_CHAN			0x0122
#define SGTL5000_DAP_AVC_CTRL			0x0124
#define SGTL5000_DAP_AVC_THRESHOLD		0x0126
#define SGTL5000_DAP_AVC_ATTACK			0x0128
#define SGTL5000_DAP_AVC_DECAY			0x012A
#define SGTL5000_DAP_COEF_WR_B1_MSB		0x012C
#define SGTL5000_DAP_COEF_WR_B1_LSB		0x012E
#define SGTL5000_DAP_COEF_WR_B2_MSB		0x0130
#define SGTL5000_DAP_COEF_WR_B2_LSB		0x0132
#define SGTL5000_DAP_COEF_WR_A1_MSB		0x0134
#define SGTL5000_DAP_COEF_WR_A1_LSB		0x0136
#define SGTL5000_DAP_COEF_WR_A2_MSB		0x0138
#define SGTL5000_DAP_COEF_WR_A2_LSB		0x013A

// CHIP_CLK_CTRL (0x0004) Clock Control
#define CHIP_CLK_CTRL_SYS_FS_MASK      0x000C // Bits 3:2
#define CHIP_CLK_CTRL_SYS_FS_SHIFT     2
#define CHIP_CLK_CTRL_SYS_FS_96K       0x3 // 0x3 = 96kHz
#define CHIP_CLK_CTRL_SYS_FS_48K       0x2 // 0x2 = 48kHz

#define CHIP_CLK_CTRL_MCLK_FREQ_MASK   0x0003 // Bits 1:0
#define CHIP_CLK_CTRL_MCLK_FREQ_SHIFT  0
#define CHIP_CLK_CTRL_MCLK_USE_PLL     0x3 // 0x3 = MCLK from PLL
#define CHIP_CLK_CTRL_MCLK_512FS       0x2 // 0x2 = MCLK from 512*FS
#define CHIP_CLK_CTRL_MCLK_384FS       0x1 // 0x1 = MCLK from 384*FS
#define CHIP_CLK_CTRL_MCLK_256FS       0x0 // 0x0 = MCLK from 256*FS

// CHIP_I2S_CTRL (0x0006) I2S Control
// Combined I2S Config
#define CHIP_I2S_CTRL_DEFAULT       0x0030 //SCLKFREQ=64Fs, MS=Slave, SCLK_INV=0, DLEN=16, I2S mode via LRALIGN=0, LRPOL=0

// SCLKFREQ 
#define CHIP_I2S_CTRL_SCLK_FREQ_MASK  0x0100 // Bit 8
#define CHIP_I2S_CTRL_SCLK_FREQ_SHIFT 8
#define CHIP_I2S_CTRL_SCLK_FREQ_32FS  0x1 // 0x1 = SCLK frequency is 32*Fs
#define CHIP_I2S_CTRL_SCLK_FREQ_64FS  0x0 // 0x0 = SCLK frequency is 64*Fs

// DLEN
#define CHIP_I2S_CTRL_DLEN_MASK       0x0030 // Bits 5:4
#define CHIP_I2S_CTRL_DLEN_SHIFT      4
#define CHIP_I2S_CTRL_DLEN_16BITS     0x3 // 0x3 = 16 bits per sample
#define CHIP_I2S_CTRL_DLEN_20BITS     0x2 // 0x2 = 20 bits per sample 
#define CHIP_I2S_CTRL_DLEN_24BITS     0x1 // 0x1 = 24 bits per sample (Only valid when SCLK frequency is 64*Fs)
#define CHIP_I2S_CTRL_DLEN_32BITS     0x0 // 0x0 = 32 bits per sample (Only valid when SCLK frequency is 64*Fs)

// CHIP_SSS_CTRL (0x000A) System Signal Routing
// DAP_SELECT
#define SSS_CTRL_DAP_SEL_MASK   0x00C0  // Bits 7:6
#define SSS_CTRL_DAP_SEL_SHIFT  6
#define SSS_CTRL_DAP_SEL_I2S    0x1 // 0x1 = I2S to DAP
#define SSS_CTRL_DAP_SEL_ADC    0x0 // 0x0 = ADC to DAP

// DAC_SELECT
#define SSS_CTRL_DAC_SEL_MASK   0x0030  // bits 5:4
#define SSS_CTRL_DAC_SEL_SHIFT  4
#define SSS_CTRL_DAC_SEL_DAP    0x3  // 0x3 = DAP to DAC
#define SSS_CTRL_DAC_SEL_I2S    0x1  // 0x1 = I2S to DAC
#define SSS_CTRL_DAC_SEL_ADC    0x0  // 0x0 = ADC to DAC

// CHIP_ANA_CTRL (0x0024) Analog Output Routing
// MUTE_LO
#define CHIP_ANA_CTRL_LINOUT_MUTE_MASK  0x0100 // Bit 8
#define CHIP_ANA_CTRL_LINOUT_MUTE_SHIFT 8
#define CHIP_ANA_CTRL_LINOUT_MUTE_ON    0x1 // 0x1 = Mute Lineout
#define CHIP_ANA_CTRL_LINOUT_MUTE_OFF   0x0 // 0x0 = Unmute Lineout

// SELECT_HP
#define CHIP_ANA_CTRL_HP_SEL_MASK       0x0040 // Bit 6
#define CHIP_ANA_CTRL_HP_SEL_SHIFT      6
#define CHIP_ANA_CTRL_HP_SEL_LINEIN     0x1 // 0X1 = LINEINE to HP bypass
#define CHIP_ANA_CTRL_HP_SEL_DAC        0x0 // 0x0 = DAC to HP

// MUTE_HP
#define CHIP_ANA_CTRL_HP_MUTE_MASK      0x0010 // Bit 4
#define CHIP_ANA_CTRL_HP_MUTE_SHIFT     4
#define CHIP_ANA_CTRL_HP_MUTE_ON        0x1 // 0x1 = Mute HP
#define CHIP_ANA_CTRL_HP_MUTE_OFF       0x0 // 0x0 = Unmute HP

// SELECT_ADC
#define CHIP_ANA_CTRL_ADC_SEL_MASK       0x0004  // Bit 2
#define CHIP_ANA_CTRL_ADC_SEL_SHIFT      2
#define CHIP_ANA_CTRL_ADC_SEL_LINEIN     0x1  // 0x1 = LINEIN to ADC
#define CHIP_ANA_CTRL_ADC_SEL_MIC        0x0  // 0x0 = MIC to ADC

// MUTE_ADC 
#define CHIP_ANA_CTRL_ADC_MUTE_MASK      0x0001  // Bit 0
#define CHIP_ANA_CTRL_ADC_MUTE_SHIFT     0
#define CHIP_ANA_CTRL_ADC_MUTE_ON        0x1  // 0x1 = Mute ADC
#define CHIP_ANA_CTRL_ADC_MUTE_OFF       0x0  // 0x0 = Unmute ADC
 
// CHIP_DIG_POWER (0x0002) Digital Block Power
#define DIG_POWER_ADC_EN           0x0040
#define DIG_POWER_DAC_EN           0x0020
#define DIG_POWER_DAP_EN           0x0010
#define DIG_POWER_I2S_IN_EN        0x0001

// DAP_CTRL (0x0100)
#define DAP_CTRL_DAP_EN_MASK       0x0001
#define DAP_CTRL_DAP_EN_SHIFT      0
#define DAP_CTRL_DAP_EN            0x1  // 0x1 = Enable DAP
#define DAP_CTRL_DAP_DIS           0x0  // 0x0 = Disable DAP

// ADCDAC_CTRL (0x000E)
#define ADCDAC_CTRL_DAC_MUTE_MASK  0x000C // Bits 3:2
#define ADCDAC_CTRL_DAC_MUTE_SHIFT 2
#define ADCDAC_CTRL_DAC_MUTE_ON    0x3 // 0x3 = Mute DAC
#define ADCDAC_CTRL_DAC_MUTE_OFF   0x0 // 0x0 = Unmute DAC

// CHIP_ANA_POWER (0x0030) Analog Power Control
#define CHIP_ANA_POWER_PLL_EN_MASK  0x0400 // Bit 10
#define CHIP_ANA_POWER_PLL_EN_SHIFT 10
#define CHIP_ANA_POWER_PLL_EN       0x1 // 0x1 = Enable PLL
#define CHIP_ANA_POWER_PLL_DIS      0x0 // 0x0 = Disable

#define CHIP_ANA_POWER_VCOMP_POWERUP_MASK 0x0100 // Bit 8
#define CHIP_ANA_POWER_VCOMP_POWERUP_SHIFT 8
#define CHIP_ANA_POWER_VCOMP_POWERUP 0x1 // 0x1 = Power up VCOMP
#define CHIP_ANA_POWER_VCOMP_POWERDOWN 0x0 // 0x0 = Power down VCOMP

// CHIP_CLK_TOP_CTRL (0x0034) Clock Top Control
#define CHIP_CLK_TOP_CTRL_INPUT_FREQ_DIV2_MASK 0x0008 // Bit 3
#define CHIP_CLK_TOP_CTRL_INPUT_FREQ_DIV2_SHIFT 3
#define CHIP_CLK_TOP_CTRL_INPUT_FREQ_DIV2 0x1 // 0x1 = Divide input frequency by 2
#define CHIP_CLK_TOP_CTRL_INPUT_FREQ_DIV1 0x0 // 0x0 = Do not divide input frequency




// Audio Source Options
typedef enum {
    AUDIO_SOURCE_LINEIN = 0,
    AUDIO_SOURCE_I2S
} audio_source_t;

// Audio Output Options
typedef enum {
    AUDIO_OUTPUT_LINEOUT = 0,
    AUDIO_OUTPUT_HP,
    AUDIO_OUTPUT_BOTH, // Both Lineout and HP
} audio_output_t;

// I2S Configuration
typedef struct {
    uint8_t sclk_freq; // SCLK frequency 
    uint8_t ms_mode; // Master/Slave mode 
    uint8_t sclk_inv; // SCLK inversion 
    uint8_t dlen; // Data length 
    uint8_t i2s_mode; // I2S mode   
    uint8_t lr_align; // LR alignment 
    uint8_t lr_pol; // LR polarity  
} i2s_config_t;

// Configuration Structure for SGTL5000
typedef struct {
    audio_source_t audio_source; // Source of audio input
    audio_output_t audio_output; // Destination of audio output
    bool dsp_enable; // Enable Digital Signal Processing
    uint32_t sys_mclk; // System Master Clock frequency in MHz
    uint32_t sys_fs; // System Sampling Frequency in Hz
    i2s_config_t *i2s_config; // I2S configuration
    uint8_t volume; // Volume level
} sgtl5000_config_t;



// Function Prototypes

// SGTL5000 Register Operations
uint8_t  sgtl5000_reg_read(uint16_t reg, uint16_t* val);
uint8_t  sgtl5000_reg_write(uint16_t reg, uint16_t val);
uint8_t  sgtl5000_reg_write_verify(uint16_t reg, uint16_t val);
uint8_t  sgtl5000_reg_modify(uint16_t reg, uint16_t mask, uint8_t shift, uint16_t value);
uint8_t  sgtl5000_reg_modify_verify(uint16_t reg, uint16_t mask, uint8_t shift, uint16_t value);

// SGTL5000 Initializaition and Confgiuration
uint8_t  sgtl5000_read_id();
uint8_t  sgtl5000_print_all_regs();
uint8_t  sgtl5000_powerup();
uint8_t  sgtl5000_clock_config();
uint8_t  sgtl5000_input_output_route(audio_source_t source, audio_output_t output, bool dsp_enable);
uint8_t  sgtl5000_configure_i2s(i2s_config_t* i2s_config_t);
uint8_t  sgtl5000_adjust_volume(uint8_t volume, audio_output_t output, bool init);
uint8_t  sgtl5000_configure_dsp();
uint8_t  sgtl5000_init(sgtl5000_config_t* config);
#endif