#include "sgtl5000.h"
#include <stdlib.h>
#include <stdio.h>

extern I2C_HandleTypeDef hi2c1;

/**
 * @brief Read a register of SGTL5000 audio codec
 * 
 * @param reg 16-bit register address
 * @param val Pointer to store the read value
 * @return I2C_SUCCESS on success, I2C_FAIL on failure
 */
uint8_t sgtl5000_reg_read(uint16_t reg, uint16_t* val)
{
    uint8_t buf[2];
    if (HAL_I2C_Mem_Read(&hi2c1, SGTL5000_ADDR, reg, I2C_MEMADD_SIZE_16BIT, buf, 2, HAL_MAX_DELAY) != HAL_OK) {
        return I2C_FAIL;
    }
    *val = ((uint16_t)buf[0] << 8) | buf[1]; // SGTL5000 sends MSB first
    return I2C_SUCCESS;
}

/**
 * @brief Write a value to a register of SGTL5000 audio codec
 * 
 * @param reg 16-bit register address
 * @param val 16-bit value to write
 * @return I2C_SUCCESS on success, I2C_FAIL on failure
 */
uint8_t sgtl5000_reg_write(uint16_t reg, uint16_t val)
{
    uint8_t buf[2] = {(uint8_t)(val >> 8), (uint8_t)(val &  0xFF)}; // Prepare data in MSB first format
    if (HAL_I2C_Mem_Write(&hi2c1, SGTL5000_ADDR, reg, I2C_MEMADD_SIZE_16BIT, buf, 2, HAL_MAX_DELAY) != HAL_OK) {
        return I2C_FAIL;
    }
    return I2C_SUCCESS;
}

/**
 * @brief Write a value to a register and verify the write operation
 * 
 * @param reg 16-bit register address
 * @param val 16-bit value to write
 * @return I2C_SUCCESS on success, I2C_FAIL on failure, I2C_MISMATCH if read value does not match written value
 */
uint8_t sgtl5000_reg_write_verify(uint16_t reg, uint16_t val)
{
    uint8_t status;
    status = sgtl5000_reg_write(reg, val);
    if (status != I2C_SUCCESS) {
        return status; // Return if write operation failed
    }
    uint16_t read_val;
    status |= sgtl5000_reg_read(reg, &read_val);
    if (status != I2C_SUCCESS) {
        return status; // Return if write/read operation failed
    }
    if (read_val != val) {
        return I2C_MISMATCH; // Return if read value does not match written value
    }

    return I2C_SUCCESS;
}

/**
 * @brief Modify specific bits in a register of SGTL5000 audio codec
 * 
 * @param reg 16-bit register address
 * @param mask 16-bit mask to update the register
 * @param shift Number of bits to shift the value
 * @param value 16-bit value to apply with the mask
 * @return I2C_SUCCESS on success, I2C_FAIL on failure
 */
uint8_t  sgtl5000_reg_modify(uint16_t reg, uint16_t mask, uint8_t shift, uint16_t value)
{
    uint16_t current_val;
    uint8_t status;
    status = sgtl5000_reg_read(reg, &current_val);
    if (status != I2C_SUCCESS) {
        return status; // Return if read operation failed
    }
    uint16_t new_val = (current_val & ~mask) | ((value << shift) & mask);
    status |= sgtl5000_reg_write(reg, new_val);
    return status; 
}

/**
 * @brief Modify specific bits in a register and verify the modification
 * 
 * @param reg 16-bit register address
 * @param mask 16-bit mask to update the register
 * @param shift Number of bits to shift the value
 * @param value 16-bit value to apply with the mask
 * @return I2C_SUCCESS on success, I2C_FAIL on failure, I2C_MISMATCH if read value does not match expected value
 */
uint8_t sgtl5000_reg_modify_verify(uint16_t reg, uint16_t mask, uint8_t shift, uint16_t value)
{
    uint16_t current_val;
    uint8_t status;
    status = sgtl5000_reg_read(reg, &current_val);
    if (status != I2C_SUCCESS) {
        return status; // Return if read operation failed
    }
    uint16_t new_val = (current_val & ~mask) | ((value << shift) & mask);
    status = sgtl5000_reg_write_verify(reg, new_val);
    if (status != I2C_SUCCESS) {
        return status; // Return if verify operation failed
    }
    return I2C_SUCCESS; 
}

/**
 * @brief Power up the SGTL5000 audio codec
 * 
 */
uint8_t sgtl5000_powerup()
{   
    uint8_t status;
    // Turn of startup power supplies (VDDD is externally driven)
    status = sgtl5000_reg_write(SGTL5000_CHIP_ANA_POWER, 0X4260);
    if (status != I2C_SUCCESS) {
        return status; // Return if write failed
    }

    // Reference voltage and bias current configuration
    // VDDA = 1.8V, VDDA / 2 = 0.9V
    status = sgtl5000_reg_write(SGTL5000_CHIP_REF_CTRL, 0x004E); // Set bias current
    if (status != I2C_SUCCESS) {
        return status; // Return if write failed
    }
    status = sgtl5000_reg_write(SGTL5000_CHIP_LINE_OUT_CTRL, 0x0322); // Set lineout reference voltage to VDDIO / 2 (1.65V)
    if (status != I2C_SUCCESS) {
        return status; // Return if write failed
    }

    // Other analog block configurations
    status = sgtl5000_reg_write(SGTL5000_CHIP_REF_CTRL, 0x004F); // Configure slow ramp up rate
    if (status != I2C_SUCCESS) {
        return status; // Return if write failed
    }
    status = sgtl5000_reg_write(SGTL5000_CHIP_SHORT_CTRL, 0x1106); // Enable short circuit protection
    if (status != I2C_SUCCESS) {
        return status; // Return if write failed
    }

    //sgtl5000_reg_write(SGTL5000_CHIP_ANA_CTRL, 0x0133); // Enable zero-cross detect

    // Power up Inputs/Outputs/Digital Blocks
    status = sgtl5000_reg_write(SGTL5000_CHIP_ANA_POWER, 0x6AFF); // Power up LINEOUT, HP, ADC, DAC,
    if (status != I2C_SUCCESS) {
        return status; // Return if write failed
    }

    // Power up digital blocks
    // I2S_IN (bit 0), I2S_OUT (bit 1), DAP (bit 4), DAC (bit 5), // ADC (bit 6) are powered on
    status = sgtl5000_reg_write(SGTL5000_CHIP_DIG_POWER, 0x0073);
    if (status != I2C_SUCCESS) {
        return status; // Return if write failed
    }

    // Set LINEOUT volume level
    status = sgtl5000_reg_write(SGTL5000_CHIP_LINE_OUT_VOL, 0x0505);
    if (status != I2C_SUCCESS) {
        return status; // Return if write failed
    }

    return I2C_SUCCESS; 
}

/**
 * @brief Configure the clock settings of SGTL5000 audio codec
 */
uint8_t sgtl5000_clock_config()
{
    uint8_t status;
    // Set SYS_FS (bits 3:2) to 0x2 (48kHz), 
    status = sgtl5000_reg_modify(SGTL5000_CHIP_CLK_CTRL, CHIP_CLK_CTRL_SYS_FS_MASK,  CHIP_CLK_CTRL_SYS_FS_SHIFT, CHIP_CLK_CTRL_SYS_FS_48K);
    if (status != I2C_SUCCESS) {
        return status; // Return if write failed
    }

    // Set MCLK_FREQ (bits 1:0) to  0x0 (256*Fs)
    status = sgtl5000_reg_modify(SGTL5000_CHIP_CLK_CTRL, CHIP_CLK_CTRL_MCLK_FREQ_MASK, CHIP_CLK_CTRL_MCLK_FREQ_SHIFT, CHIP_CLK_CTRL_MCLK_256FS);
    if (status != I2C_SUCCESS) {
        return status; // Return if write failed
    }
    
    return I2C_SUCCESS;
}




uint8_t  sgtl5000_configure_dsp()
{
    // Enable DSP
    uint8_t status = sgtl5000_reg_modify(SGTL5000_DAP_CTRL, DAP_CTRL_DAP_EN_MASK, DAP_CTRL_DAP_EN_SHIFT, DAP_CTRL_DAP_EN);
    if (status != I2C_SUCCESS) {
        return status; // Return if write failed
    }
    // .. Add more stuff here later
    return I2C_SUCCESS;
}

/**
 * @brief Configure the input and output routing of SGTL5000 audio codec
 * 
 * @param source Audio source to route (LINEIN or I2S)
 * @param output Audio output to route (LINEOUT, HP, or BOTH)
 * @param dsp_enable Enable or disable DSP processing
 */
uint8_t sgtl5000_input_output_route(audio_source_t source, audio_output_t output, bool dsp_enable)
{
    uint8_t status = I2C_SUCCESS;

    if (dsp_enable) {
        // DAP will be enabled in the init function before the routing is performed 

        // Choose audio source for DSP processing
        if (source == AUDIO_SOURCE_I2S) {
            // I2S -> DAP
            status |= sgtl5000_reg_modify(SGTL5000_CHIP_SSS_CTRL, SSS_CTRL_DAP_SEL_MASK, SSS_CTRL_DAP_SEL_SHIFT, SSS_CTRL_DAP_SEL_I2S);
            // DAP -> DAC
            status |= sgtl5000_reg_modify(SGTL5000_CHIP_SSS_CTRL, SSS_CTRL_DAC_SEL_MASK, SSS_CTRL_DAC_SEL_SHIFT, SSS_CTRL_DAC_SEL_DAP);
        }
        else if (source == AUDIO_SOURCE_LINEIN) {
            // LINEIN -> ADC
            status |= sgtl5000_reg_modify(SGTL5000_CHIP_ANA_CTRL, CHIP_ANA_CTRL_ADC_SEL_MASK,
                                            CHIP_ANA_CTRL_ADC_SEL_SHIFT, CHIP_ANA_CTRL_ADC_SEL_LINEIN);
            // ADC -> DAP
            status |= sgtl5000_reg_modify(SGTL5000_CHIP_SSS_CTRL, SSS_CTRL_DAP_SEL_MASK, 
                                            SSS_CTRL_DAP_SEL_SHIFT, SSS_CTRL_DAP_SEL_ADC);
            // DAP -> DAC
            status |= sgtl5000_reg_modify(SGTL5000_CHIP_SSS_CTRL, SSS_CTRL_DAC_SEL_MASK, 
                                            SSS_CTRL_DAC_SEL_SHIFT, SSS_CTRL_DAC_SEL_DAP);
            // Unmute ADC
            status |= sgtl5000_reg_modify(SGTL5000_CHIP_ANA_CTRL, CHIP_ANA_CTRL_ADC_MUTE_MASK, 
                CHIP_ANA_CTRL_ADC_MUTE_SHIFT, CHIP_ANA_CTRL_ADC_MUTE_OFF);
        }
        // Choose audio output for the DAC
        if (output == AUDIO_OUTPUT_HP) {
            // DAC -> HP
            status |= sgtl5000_reg_modify(SGTL5000_CHIP_ANA_CTRL, CHIP_ANA_CTRL_HP_SEL_MASK, 
                                            CHIP_ANA_CTRL_HP_SEL_SHIFT, CHIP_ANA_CTRL_HP_SEL_DAC);
        }
        else if (output == AUDIO_OUTPUT_LINEOUT) {  
            ; // LINEOUT is always connected to DAC
        }
        else if (output == AUDIO_OUTPUT_BOTH) {
            // DAC -> HP
            status |= sgtl5000_reg_modify(SGTL5000_CHIP_ANA_CTRL, CHIP_ANA_CTRL_HP_SEL_MASK, 
                                            CHIP_ANA_CTRL_HP_SEL_SHIFT, CHIP_ANA_CTRL_HP_SEL_DAC);
        }
    }
    else {
        if (source == AUDIO_SOURCE_LINEIN && output == AUDIO_OUTPUT_HP) {
            // If LINEIN -> HP, route LINEIN directly to HP
            status |= sgtl5000_reg_modify(SGTL5000_CHIP_ANA_CTRL, CHIP_ANA_CTRL_HP_SEL_MASK, 
                                            CHIP_ANA_CTRL_HP_SEL_SHIFT, CHIP_ANA_CTRL_HP_SEL_LINEIN);
        }
        else {
            // Choose audio source for direct output
            if (source == AUDIO_SOURCE_I2S) {
                // I2S -> DAC
                status |= sgtl5000_reg_modify(SGTL5000_CHIP_SSS_CTRL, SSS_CTRL_DAC_SEL_MASK, 
                                                SSS_CTRL_DAC_SEL_SHIFT, SSS_CTRL_DAC_SEL_I2S);
            }
            else if (source == AUDIO_SOURCE_LINEIN) {
                // LINEIN -> ADC
                status |= sgtl5000_reg_modify(SGTL5000_CHIP_ANA_CTRL, CHIP_ANA_CTRL_ADC_SEL_MASK, 
                                                CHIP_ANA_CTRL_ADC_SEL_SHIFT, CHIP_ANA_CTRL_ADC_SEL_LINEIN);
                
                // ADC -> DAC
                status |= sgtl5000_reg_modify(SGTL5000_CHIP_SSS_CTRL, SSS_CTRL_DAC_SEL_MASK, 
                                                SSS_CTRL_DAC_SEL_SHIFT, SSS_CTRL_DAC_SEL_ADC);

                // Unmute ADC
                status |= sgtl5000_reg_modify(SGTL5000_CHIP_ANA_CTRL, CHIP_ANA_CTRL_ADC_MUTE_MASK, 
                    CHIP_ANA_CTRL_ADC_MUTE_SHIFT, CHIP_ANA_CTRL_ADC_MUTE_OFF);
            }
            // Choose audio output for the DAC
            if (output == AUDIO_OUTPUT_LINEOUT) {
                ; // LINEOUT is always connected to DAC 
            }
            else if (output == AUDIO_OUTPUT_HP) {
                // DAC -> HP
                status |= sgtl5000_reg_modify(SGTL5000_CHIP_ANA_CTRL, CHIP_ANA_CTRL_HP_SEL_MASK, 
                                                CHIP_ANA_CTRL_HP_SEL_SHIFT, CHIP_ANA_CTRL_HP_SEL_DAC);
            }
            else if (output == AUDIO_OUTPUT_BOTH) {
                // DAC -> HP
                status |= sgtl5000_reg_modify(SGTL5000_CHIP_ANA_CTRL, CHIP_ANA_CTRL_HP_SEL_MASK, 
                                                CHIP_ANA_CTRL_HP_SEL_SHIFT, CHIP_ANA_CTRL_HP_SEL_DAC);
            }
        }  
    }

    return status;
}

/**
 * @brief Configure the I2S interface of SGTL5000 audio codec
 * 
 * @param i2s_config Pointer to I2S configuration structure. If NULL, default settings will be used.
 */
uint8_t  sgtl5000_configure_i2s(i2s_config_t* i2s_config)
{   
    uint8_t status;
    // Use default I2S configuration if i2s_config is NULL
    if (i2s_config == NULL) {
        //SCLKFREQ=64Fs, MS=Slave, SCLK_INV=0, DLEN=16, I2S mode via LRALIGN=0, LRPOL=0
        status = sgtl5000_reg_write(SGTL5000_CHIP_I2S_CTRL, CHIP_I2S_CTRL_DEFAULT);
        
    }
    else {
        uint16_t i2s_ctrl_cfg_mask = i2s_config -> sclk_freq << 8 | 
                                     (i2s_config -> ms_mode << 7) |
                                     (i2s_config -> sclk_inv << 6) |
                                     (i2s_config -> dlen << 4) |
                                     (i2s_config -> i2s_mode << 2) |
                                     (i2s_config -> lr_align << 1) |
                                     i2s_config -> lr_pol;

        status = sgtl5000_reg_write(SGTL5000_CHIP_I2S_CTRL, i2s_ctrl_cfg_mask);
    } 
    return status; 
}

/**
 * @brief Adjust the volume of SGTL5000 audio codec
 * @param volume Volume level to set (0-255)
 * @param output Audio output type (LINEOUT, HP, BOTH)
 * @param init Initialization flag
 */
uint8_t  sgtl5000_adjust_volume(uint8_t volume, audio_output_t output, bool init)
{
    uint8_t status = I2C_SUCCESS;
    uint16_t ana_ctrl = 0;

    // Read current analog rouiting to know if HP is fed from DAC or LINEIN (bypass)
    status |= sgtl5000_reg_read(SGTL5000_CHIP_ANA_CTRL, &ana_ctrl);
    if (status != I2C_SUCCESS) {
        return status; // Return if read operation failed
    }
    uint8_t hp_sel = (ana_ctrl & CHIP_ANA_CTRL_HP_SEL_MASK) >> CHIP_ANA_CTRL_HP_SEL_SHIFT;
    bool hp_from_dac = (hp_sel == CHIP_ANA_CTRL_HP_SEL_DAC);


    // Volume and Mute Control
    if (output == AUDIO_OUTPUT_HP || output == AUDIO_OUTPUT_BOTH) {
        if (init) {
            // Configure HP_OUT left and right volume to minimum , unmute
            status |= sgtl5000_reg_write(SGTL5000_CHIP_ANA_HP_CTRL, 0x7F7F);
            status |= sgtl5000_reg_modify(SGTL5000_CHIP_ANA_CTRL, CHIP_ANA_CTRL_HP_MUTE_MASK, 
                CHIP_ANA_CTRL_HP_MUTE_SHIFT, CHIP_ANA_CTRL_HP_MUTE_OFF);

            if (hp_from_dac) {
                // Unmute DAC
                status |= sgtl5000_reg_write(SGTL5000_CHIP_DAC_VOL, 0x3C3C); // 0dB 
                status |= sgtl5000_reg_modify(SGTL5000_CHIP_ADCDAC_CTRL, ADCDAC_CTRL_DAC_MUTE_MASK, 
                ADCDAC_CTRL_DAC_MUTE_SHIFT, ADCDAC_CTRL_DAC_MUTE_OFF);
            }
        }

        // Left and Right volumes are assumed to be the same
        uint8_t cur_vol_left;
        if (init) {
            cur_vol_left = HP_VOL_MIN; // 0x7F
        }
        else {
            // Read current HP volume
            uint16_t cur_vol;
            status |= sgtl5000_reg_read(SGTL5000_CHIP_ANA_HP_CTRL, &cur_vol);
            if (status != I2C_SUCCESS) {
                return status; // Return if read operation failed
            }
            cur_vol_left = (cur_vol >> 8) & 0xFF; // Extract left channel volume
        }
        uint8_t new_vol_left = (volume > HP_VOL_MIN) ? HP_VOL_MIN : volume; // Clamp the volume to 0x7F
        uint8_t num_steps = abs(new_vol_left - cur_vol_left);
        uint16_t cur_vol;

        // Gradual change
        for (uint8_t i = 0; i < num_steps; i++) {
            if (new_vol_left > cur_vol_left) {
                cur_vol_left++;
            } else {
                cur_vol_left--;
            }

            cur_vol = (cur_vol_left << 8) | cur_vol_left; // Combine left and right volumes
            // Write the new volume to both left and right channels
            status |= sgtl5000_reg_write(SGTL5000_CHIP_ANA_HP_CTRL, cur_vol);
        }
    }

    

    // LINEOUT and DAC volume control
    if (output == AUDIO_OUTPUT_LINEOUT || output == AUDIO_OUTPUT_BOTH) {
        // Unmute LINEOUT
        status |= sgtl5000_reg_modify(SGTL5000_CHIP_ANA_CTRL, CHIP_ANA_CTRL_LINOUT_MUTE_MASK, 
            CHIP_ANA_CTRL_LINOUT_MUTE_SHIFT, CHIP_ANA_CTRL_LINOUT_MUTE_OFF);
        
        // Unmute DAC
        status |= sgtl5000_reg_write(SGTL5000_CHIP_DAC_VOL, 0x3C3C); // 0dB
        status |= sgtl5000_reg_modify(SGTL5000_CHIP_ADCDAC_CTRL, ADCDAC_CTRL_DAC_MUTE_MASK, 
            ADCDAC_CTRL_DAC_MUTE_SHIFT, ADCDAC_CTRL_DAC_MUTE_OFF);
    }

    return status;
}

/**
 * @brief Initialize the SGTL5000 audio codec with the provided configuration
 * 
 * @param config Pointer to the SGTL5000 configuration structure
 */
uint8_t  sgtl5000_init(sgtl5000_config_t* config)
{
    
    uint8_t status;

    // Power up the codec
    status = sgtl5000_powerup();
    if (status != I2C_SUCCESS) {
        printf("SGTL5000 power up failed with status code: %d\n", status);
        return status; // Return if power up failed
    }
    printf("SGTL5000 powered up successfully.\n");

    HAL_Delay(500); // Wait for power up 

    // Configure Clocks
    status = sgtl5000_clock_config();
    if (status != I2C_SUCCESS) {
        printf("SGTL5000 clock configuration failed with status code: %d\n", status);
        return status; // Return if clock configuration failed
    }
    printf("SGTL5000 clock configured successfully.\n");

    // Configure DSP engine
    if (config->dsp_enable) {
        status = sgtl5000_configure_dsp();
        if (status != I2C_SUCCESS) {
            printf("SGTL5000 DSP configuration failed with status code: %d\n", status);
            return status; // Return if DSP configuration failed
        }
        printf("SGTL5000 DSP configured successfully.\n");
    } else {
        printf("SGTL5000 DSP is disabled.\n");
    }
    
    // Setup Input & Output Routing
    status = sgtl5000_input_output_route(config->audio_source, config->audio_output, config->dsp_enable);
    if (status != I2C_SUCCESS) {
        printf("SGTL5000 input/output routing failed with status code: %d\n", status);
        return status; // Return if routing configuration failed
    }
    printf("SGTL5000 input/output routing configured successfully.\n");

    // Configure I2S settings for SGTL5000
    status = sgtl5000_configure_i2s(&config->i2s_config);
    if (status != I2C_SUCCESS) {
        printf("SGTL5000 I2S configuration failed with status code: %d\n", status);
        return status; // Return if I2S configuration failed
    }
    printf("SGTL5000 I2S configured successfully.\n");

    // Volume Control
    status = sgtl5000_adjust_volume(config->volume, config->audio_output, true);
    if (status != I2C_SUCCESS) {
        printf("SGTL5000 volume adjustment failed with status code: %d\n", status);
        return status; // Return if volume adjustment failed
    }
    printf("SGTL5000 volume adjusted successfully.\n");

    return I2C_SUCCESS;
}


