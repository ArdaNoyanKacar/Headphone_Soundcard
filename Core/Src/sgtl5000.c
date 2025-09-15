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
        printf("Expected 0x%04X, but read 0x%04X from register 0x%04X\r\n", val, read_val, reg);
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
 * @brief Read the ID register of SGTL5000 audio codec
 */
uint8_t sgtl5000_read_id()
{
    uint16_t id_combined = 0;

    uint8_t status = sgtl5000_reg_read(SGTL5000_CHIP_ID, &id_combined);
    uint8_t part_id = (id_combined >> 8) & 0xFF; // Extract Part ID
    uint8_t revision_id = id_combined & 0xFF; // Extract Revision ID

    if (status != I2C_SUCCESS) {
        printf("Failed to read SGTL5000 ID\r\n");
        printf("SGTL5000 status code : %2x\r\n", status);
        return status;
    }
    printf("SGTL5000 ID: Part ID = 0x%02X, Revision ID = 0x%02X\r\n", part_id, revision_id);
    return I2C_SUCCESS;
}

/**
 * @brief Print all main registers of the SGTL5000 auidio codec
 */
uint8_t  sgtl5000_print_all_regs()
{
    uint16_t reg_value;
    for (uint16_t reg = 0; reg <= 0x013A; reg += 2) {
        sgtl5000_reg_read(reg, &reg_value);
        printf("SGTL5000 Register 0x%04X: 0x%04X\r\n", reg, reg_value);
    }
    return I2C_SUCCESS;
}

// Helper functions for initialization steps

/**
 * @brief Power up the analog blocks of SGTL5000 audio codec
 * @return I2C_SUCCESS on success, I2C_FAIL on failure
 */
uint8_t sgtl5000_powerup_analog()
{
    uint8_t status;

    // Turn of startup power supplies (VDDD is externally driven)
    status = sgtl5000_reg_write_verify(SGTL5000_CHIP_ANA_POWER, 0x4060);
    if (status != I2C_SUCCESS) {
        printf("Failed to write to SGTL5000_CHIP_ANA_POWER 1\r\n");
        return status;
    }

    // Configure reference voltage and bias current
    status = sgtl5000_reg_write_verify(SGTL5000_CHIP_REF_CTRL, 0x004E); // Set bias current
    if (status != I2C_SUCCESS) {
        printf("Failed to write to SGTL5000_CHIP_REF_CTRL 2\r\n");
        return status;
    }

    status = sgtl5000_reg_write_verify(SGTL5000_CHIP_LINE_OUT_CTRL, 0x0F22); // Set lineout reference voltage to VDDIO / 2 (1.65V)
    if (status != I2C_SUCCESS) {
        printf("Failed to write to SGTL5000_CHIP_LINE_OUT_CTRL 3\r\n");
        return status;
    }

    // Short detect configuration
    status = sgtl5000_reg_write_verify(SGTL5000_CHIP_SHORT_CTRL, 0x1106);
    if (status != I2C_SUCCESS) {
        printf("Failed to write to SGTL5000_CHIP_SHORT_CTRL 4\r\n");
        return status;
    }

    // Final analog power up after limits are set
    status = sgtl5000_reg_write_verify(SGTL5000_CHIP_ANA_POWER, 0x40FB);
    if (status != I2C_SUCCESS) {
        printf("Failed to write to SGTL5000_CHIP_ANA_POWER 5\r\n");
        return status;
    }

    return I2C_SUCCESS;
}

/**
 * @brief Power up the digital blocks of SGTL5000 audio codec
 * @return I2C_SUCCESS on success, I2C_FAIL on failure
 */
uint8_t sgtl5000_powerup_digital()
{
    uint8_t status = sgtl5000_reg_write_verify(SGTL5000_CHIP_DIG_POWER, 0x0073);
    if (status != I2C_SUCCESS) {
        printf("Failed to write to SGTL5000_CHIP_DIG_POWER 6\r\n");
        return status;
    }
    return I2C_SUCCESS;
}

/**
 * @brief Configure the clocks of SGTL5000 audio codec
 * @return I2C_SUCCESS on success, I2C_FAIL on failure
 */
uint8_t sgtl5000_configure_clocks()
{
    // MCLK = 12.288MHz, Fs = 48kHz
    uint8_t status = sgtl5000_reg_write_verify(SGTL5000_CHIP_CLK_CTRL, 0x0008);
    if (status != I2C_SUCCESS) {
        printf("Failed to write to SGTL5000_CHIP_CLK_CTRL 8\r\n");
        return status;
    }
    return I2C_SUCCESS;
}

/**
 * @brief Configure the I2S interface of SGTL5000 audio codec
 * @return I2C_SUCCESS on success, I2C_FAIL on failure
 */
uint8_t sgtl5000_configure_i2s()
{
    // SCLKFREQ=64Fs, MS=Slave, SCLK_INV=0, DLEN=16, I2S mode via LRALIGN=0, LRPOL=0
    uint8_t status = sgtl5000_reg_write_verify(SGTL5000_CHIP_I2S_CTRL, 0X0080);
    if (status != I2C_SUCCESS) {
        printf("Failed to write to SGTL5000_CHIP_I2S_CTRL 9\r\n");
        return status;
    }
    return I2C_SUCCESS;
}

/**
 * @brief Configure the routing of SGTL5000 audio codec
 * @return I2C_SUCCESS on success, I2C_FAIL on failure
 */
uint8_t sgtl5000_configure_routing()
{
    uint8_t status = sgtl5000_reg_write_verify(SGTL5000_CHIP_SSS_CTRL, 0x0030);
    if (status != I2C_SUCCESS) {
        printf("Failed to write to SGTL5000_CHIP_SSS_CTRL 10\r\n");
        return status;
    }
    return I2C_SUCCESS;
}

/**
 * @brief Change the volume of SGTL5000 audio code
 * @param volume_percent Volume percentage (0-100)
 * @return I2C_SUCCESS on success, I2C_FAIL on failure
 */
uint8_t sgtl5000_change_dac_volume(uint8_t volume_percent)
{
    if (volume_percent > 100) {
        volume_percent = 100; // Clamp to 100%
    }

    // 0% -> 0xFC (min)
    // 100% -> 0x00 (max)
    int32_t volume_span = DAC_VOL_MAX - DAC_VOL_MIN; // Negative
    int32_t volume_value = DAC_VOL_MIN + (volume_span * volume_percent) / 100;
    uint16_t lr_volume = (uint16_t)(((uint16_t)volume_value << 8) | (uint16_t)volume_value);
    uint8_t status = sgtl5000_reg_write_verify(SGTL5000_CHIP_DAC_VOL, lr_volume);
    if (status != I2C_SUCCESS) {
        printf("Failed to write to SGTL5000_CHIP_DAC_VOL\r\n");
        return status;
    }

    return I2C_SUCCESS;
}

/**
 * @brief Mute or unmute the DAC output of SGTL5000 audio codec
 * @param on true to mute, false to unmute
 */
uint8_t sgtl5000_dac_mute(bool mute)
{
    uint8_t status;
    status = sgtl5000_reg_modify(SGTL5000_CHIP_ADCDAC_CTRL, ADCDAC_CTRL_DAC_MUTE_MASK, ADCDAC_CTRL_DAC_MUTE_SHIFT, mute ? ADCDAC_CTRL_DAC_MUTE_ON : ADCDAC_CTRL_DAC_MUTE_OFF);
    if (status != I2C_SUCCESS) {
        printf("Failed to modify SGTL5000_CHIP_ADCDAC_CTRL for DAC mute\r\n");
    }
    return status;
}

/**
 * @brief Set the surround sound mode of SGTL5000 audio codec
 * @param mode Surround sound mode (SGTL_SURROUND_OFF, SGTL_SURROUND_MONO, SGTL_SURROUND_STEREO)
 * @param width Width of the surround eff   ect (0-7)
 * @return I2C_SUCCESS on success, I2C_FAIL on failure
 */
uint8_t sgtl5000_dap_surround_set(sgtl_surround_mode_t mode, uint8_t width)
{
    if (width > 7) {
        width = 7;  // Max width is 7
    }
    
    sgtl5000_dac_mute(true); // Mute DAC during configuration
    uint16_t surround_val = ((uint16_t)width << 4) | (mode & 0x3);
    uint8_t status;
    status = sgtl5000_reg_write_verify(SGTL5000_DAP_SURROUND, surround_val); // 0x010A
    if (status != I2C_SUCCESS) {
        printf("Failed to write to SGTL5000_DAP_SURROUND\r\n");
        return status;
    }
    sgtl5000_dac_mute(false); // Unmute DAC after configuration
    return I2C_SUCCESS;
}

/**
 * @brief Enable or disable bass enhancement
 * @param enable true to enable, false to disable
 * @param bass_level Bass enhancement level (0-127, where 0 is max boost and 127 is min boost)
 */
uint8_t sgtl5000_dap_bass_enhance_set(bool enable, uint8_t lr_level, uint8_t bass_level)
{
    uint8_t status = I2C_SUCCESS;
    if (bass_level > 0x7F) bass_level = 0x7F;  // clamp to field

    sgtl5000_dac_mute(true); // Mute DAC during configuration

    if (enable) {

        // Adjust the LR value
        status = sgtl5000_reg_modify_verify(SGTL5000_DAP_BASS_ENHANCE_CTRL, 0x3F00, 8, lr_level & 0x3F);
        if (status != I2C_SUCCESS) {
            printf("Failed to set LR level\r\n");
            sgtl5000_dac_mute(false);
            return status;
        }

        uint16_t ctrl16;
        status = sgtl5000_reg_read(SGTL5000_DAP_BASS_ENHANCE_CTRL, &ctrl16);
        if (status != I2C_SUCCESS) {
            printf("Failed to read SGTL5000_DAP_BASS_ENHANCE_CTRL\r\n");
            sgtl5000_dac_mute(false);
            return status;
        }

        // current 7-bit BASS_LEVEL (0x00 = most boost, 0x7F = least)
        uint8_t curr = (uint8_t)(ctrl16 & 0x007F);
        uint8_t next = curr;

        // ramp up to least boost (0x7F) BEFORE enabling
        uint8_t steps = (uint8_t)(0x7F - curr);
        for (uint8_t i = 0; i < steps; i++) {
            next++;
            status = sgtl5000_reg_modify_verify(SGTL5000_DAP_BASS_ENHANCE_CTRL, 0x007F, 0, next);
            if (status != I2C_SUCCESS) { sgtl5000_dac_mute(false); return status; }
        }

        // enable (preserves cutoff/HPF/etc.)
        status = sgtl5000_reg_modify_verify(SGTL5000_DAP_BASS_ENHANCE, 0x0001, 0, 1);
        if (status != I2C_SUCCESS) {
            printf("Failed to enable bass enhance\r\n"); 
            sgtl5000_dac_mute(false); 
            return status;
        }

        // ramp down from 0x7F to target (decreasing code = more boost)
        next  = 0x7F;
        steps = (uint8_t)(0x7F - bass_level);
        for (uint8_t i = 0; i < steps; i++) {
            next--;
            status = sgtl5000_reg_modify_verify(SGTL5000_DAP_BASS_ENHANCE_CTRL, 0x007F, 0, next);
            if (status != I2C_SUCCESS) {
                printf("Failed to set bass level\r\n");
                sgtl5000_dac_mute(false); 
                return status; 
            }
        }
    } else {
        // disable, leave CTRL as-is
        status = sgtl5000_reg_modify_verify(SGTL5000_DAP_BASS_ENHANCE, 0x0001, 0, 0);
        if (status != I2C_SUCCESS) {
            printf("Failed to disable bass enhance\r\n");
            sgtl5000_dac_mute(false);
            return status;
        }
    }

    sgtl5000_dac_mute(false);
    return status;
}


/**
 * @brief Bypass the EQ of SGTL5000 audio codec
 * @return I2C_SUCCESS on success, I2C_FAIL on failure
 */
uint8_t sgtl5000_dap_eq_bypass(void)
{
    sgtl5000_dac_mute(true); // Mute DAC during configuration
    uint8_t status;
    status = sgtl5000_reg_write_verify(SGTL5000_DAP_AUDIO_EQ, 0x0000); // Bypass EQ
    if (status != I2C_SUCCESS) {
        printf("Failed to write to SGTL5000_DAP_AUDIO_EQ for EQ bypass\r\n");
    }
    sgtl5000_dac_mute(false); // Unmute DAC after configuration
    return I2C_SUCCESS;
}

uint8_t sgtl5000_dap_geq_enable(void)
{
    sgtl5000_dac_mute(true); // Mute DAC during configuration
    uint8_t status;
    status = sgtl5000_reg_write_verify(SGTL5000_DAP_AUDIO_EQ, 0x0003); // Enable EQ
    if (status != I2C_SUCCESS) {
        printf("Failed to write to SGTL5000_DAP_AUDIO_EQ for EQ enable\r\n");
    }
    sgtl5000_dac_mute(false); // Unmute DAC after configuration
    return I2C_SUCCESS;
}

uint16_t sgtl5000_geq_code_from_db(int8_t db)
{
    // clamp to chip range (±12 dB)
    if (db < -12) {
         db = -12; 
    }
    if (db > +12) {
         db = +12; 
    }

    int16_t code = 0x2F + (int16_t)db * 4;  // 0 dB = 0x2F; 1 dB = 4 steps
    if (code < 0x00) {
         code = 0x00; 
    }
    if (code > 0x5F) {
         code = 0x5F; 
    }
    return (uint16_t)code;
}



uint8_t sgtl5000_dap_geq_ramp_band(uint16_t band_reg, uint16_t target)
{
    uint8_t  status;
    uint16_t value;

    status = sgtl5000_reg_read(band_reg, &value);
    if (status != I2C_SUCCESS) { 
        return status; 
    }

    uint16_t curr = value & 0x007F;
    uint16_t goal = target;
    if (goal > 0x5F) {
         goal = 0x5F; 
    } // Clamp to max

    if (curr == goal) {
         return I2C_SUCCESS;
    }

    uint16_t num_steps;
    int8_t   dir;
    if (goal > curr) {
        num_steps = (uint16_t)(goal - curr);
        dir   = +1;
    } else {
        num_steps = (uint16_t)(curr - goal);
        dir   = -1;
    }

    uint16_t next = curr;
    for (uint16_t i = 0; i < num_steps; i++) {
        next = (uint16_t)(next + dir);
        status = sgtl5000_reg_modify_verify(band_reg, 0x007F, 0, next);
        if (status != I2C_SUCCESS) {
             return status; 
        }
        HAL_Delay(1); // Small delay 
    }
    return I2C_SUCCESS;
}



/**
 * @brief Set the GEQ bands of SGTL5000 audio codec
 * @param b0_db Gain for Band 0 in dB (-12 to +12)
 * @param b1_db Gain for Band 1 in dB (-12 to +12)
 * @param b2_db Gain for Band 2 in dB (-12 to +12)
 * @param b3_db Gain for Band 3 in dB (-12 to +12)
 * @param b4_db Gain for Band 4 in dB (-12 to +12)
 */
uint8_t sgtl5000_dap_geq_set_bands_db(int8_t b0_db, int8_t b1_db, int8_t b2_db, int8_t b3_db, int8_t b4_db)
{
    uint8_t  status;
    uint16_t b0_code;
    uint16_t b1_code;
    uint16_t b2_code;
    uint16_t b3_code;
    uint16_t b4_code;

    // Convert dB values to register codes
    b0_code = sgtl5000_geq_code_from_db(b0_db);
    b1_code = sgtl5000_geq_code_from_db(b1_db);
    b2_code = sgtl5000_geq_code_from_db(b2_db);
    b3_code = sgtl5000_geq_code_from_db(b3_db);
    b4_code = sgtl5000_geq_code_from_db(b4_db);

    // Mute DAC during configuration
    sgtl5000_dac_mute(true);

    // Enable GEQ if not already enabled
    status = sgtl5000_dap_geq_enable();
    if (status != I2C_SUCCESS) {
        sgtl5000_dac_mute(false);
        return status;
    }

    status = sgtl5000_dap_geq_ramp_band(SGTL5000_DAP_EQ_BAND0, b0_code);
    if (status != I2C_SUCCESS) { 
        sgtl5000_dac_mute(false); 
        return status; 
    }

    status = sgtl5000_dap_geq_ramp_band(SGTL5000_DAP_EQ_BAND1, b1_code);
    if (status != I2C_SUCCESS) { 
        sgtl5000_dac_mute(false); 
        return status; 
    }

    status = sgtl5000_dap_geq_ramp_band(SGTL5000_DAP_EQ_BAND2, b2_code);
    if (status != I2C_SUCCESS) { 
        sgtl5000_dac_mute(false); 
        return status; 
    }

    status = sgtl5000_dap_geq_ramp_band(SGTL5000_DAP_EQ_BAND3, b3_code);
    if (status != I2C_SUCCESS) { 
        sgtl5000_dac_mute(false); 
        return status; 
    }

    status = sgtl5000_dap_geq_ramp_band(SGTL5000_DAP_EQ_BAND4, b4_code);
    if (status != I2C_SUCCESS) {
        sgtl5000_dac_mute(false);
        return status;
    }

    // Unmute DAC after configuration
    sgtl5000_dac_mute(false);
    return I2C_SUCCESS;
}


 

/**
 * @brief Configure the DSP of SGTL5000 audio codec
 * @return I2C_SUCCESS on success, I2C_FAIL on failure
 */
uint8_t sgtl5000_configure_dsp()
{
    uint8_t status;
    status = sgtl5000_reg_write_verify(SGTL5000_DAP_CTRL, 0x0001);
    if (status != I2C_SUCCESS) {
        printf("Failed to write to SGTL5000_DAP_CTRL 11\r\n");
        return status;
    }
    HAL_Delay(50);
    status = sgtl5000_reg_write_verify(SGTL5000_DAP_AUDIO_EQ, 0x0003);
    if (status != I2C_SUCCESS) {
        printf("Failed to write to SGTL5000_DAP_AUDIO_EQ 12\r\n");
        return status;
    }
    HAL_Delay(50);
    status = sgtl5000_reg_write_verify(SGTL5000_DAP_AVC_THRESHOLD, 0x0A40);
    if (status != I2C_SUCCESS) {
        printf("Failed to write to SGTL5000_DAP_AVC_THRESHOLD 13\r\n");
        return status;
    }
    HAL_Delay(50);
    status = sgtl5000_reg_write_verify(SGTL5000_DAP_AVC_ATTACK, 0x0014);
    if (status != I2C_SUCCESS) {
        printf("Failed to write to SGTL5000_DAP_AVC_ATTACK 14\r\n");
        return status;
    }
    HAL_Delay(50);
    status = sgtl5000_reg_write_verify(SGTL5000_DAP_AVC_DECAY, 0x0028);
    if (status != I2C_SUCCESS) {
        printf("Failed to write to SGTL5000_DAP_AVC_DECAY 15\r\n");
        return status;
    }
    HAL_Delay(50);
    status = sgtl5000_reg_write_verify(SGTL5000_DAP_AVC_CTRL, 0x0001);
    
    HAL_Delay(50);
    return I2C_SUCCESS;
}

/**
 * @brief Set initial volume and levels of SGTL5000 audio codec
 * @return I2C_SUCCESS on success, I2C_FAIL on failure
 */
uint8_t sgtl5000_set_levels()
{
    uint8_t status;
    status = sgtl5000_reg_write_verify(SGTL5000_CHIP_ANA_ADC_CTRL, 0x0000);
    if (status != I2C_SUCCESS) {
        printf("Failed to write to SGTL5000_CHIP_ANA_ADC_CTRL 17\r\n");
        return status;
    }
    HAL_Delay(50);
    status = sgtl5000_reg_write_verify(SGTL5000_CHIP_DAC_VOL, 0x3C3C);
    if (status != I2C_SUCCESS) {
        printf("Failed to write to SGTL5000_CHIP_DAC_VOL 18\r\n");
        return status;
    }
    HAL_Delay(50);
    status = sgtl5000_reg_write_verify(SGTL5000_CHIP_LINE_OUT_VOL, 0x0606);
    if (status != I2C_SUCCESS) {
        printf("Failed to write to SGTL5000_CHIP_LINE_OUT_VOL 19\r\n");
        return status;
    }
    HAL_Delay(50);
    status = sgtl5000_reg_write_verify(SGTL5000_CHIP_ANA_HP_CTRL, 0x1818);
    if (status != I2C_SUCCESS) {
        printf("Failed to write to SGTL5000_CHIP_ANA_HP_CTRL 20\r\n");
        return status;
    }
    HAL_Delay(50);
    status = sgtl5000_reg_write_verify(SGTL5000_CHIP_ADCDAC_CTRL, 0x0000);
    if (status != I2C_SUCCESS) {
        printf("Failed to write to SGTL5000_CHIP_ADCDAC_CTRL 21\r\n");
        return status;
    }
    HAL_Delay(50);
    status = sgtl5000_reg_write_verify(SGTL5000_CHIP_ANA_CTRL, 0x0004);
    if (status != I2C_SUCCESS) {
        printf("Failed to write to SGTL5000_CHIP_ANA_CTRL 22\r\n");
        return status;
    }

    return I2C_SUCCESS;
}

/**
 * @brief Initialize the SGTL5000 audio codec with the provided configuration
 * 
 * @param config Pointer to the SGTL5000 configuration structure
 */
uint8_t  sgtl5000_init()
{
    uint8_t status;
    
    // Analog Power Up
    status = sgtl5000_powerup_analog();
    if (status != I2C_SUCCESS) {
        printf("Failed to power up analog blocks\r\n");
        return status;
    }
    // Digital Power Up
    status = sgtl5000_powerup_digital();
    if (status != I2C_SUCCESS) {
        printf("Failed to power up digital blocks\r\n");
        return status;
    }
    // Start LINEOUT volume at 0dB
    status = sgtl5000_reg_write_verify(SGTL5000_CHIP_LINE_OUT_VOL, 0x0606);
    if (status != I2C_SUCCESS) {
        printf("Failed to write to SGTL5000_CHIP_LINE_OUT_VOL 7\r\n");
        return status;
    }
    // Clock Configuration
    status = sgtl5000_configure_clocks();
    if (status != I2C_SUCCESS) {
        printf("Failed to configure clocks\r\n");
        return status;
    }
    // I2S Configuration
    status = sgtl5000_configure_i2s();
    if (status != I2C_SUCCESS) {
        printf("Failed to configure I2S\r\n");
        return status;
    }
    // Routing Configuration
    status = sgtl5000_configure_routing();
    if (status != I2C_SUCCESS) {
        printf("Failed to configure routing\r\n");
        return status;
    }
    // DSP Configuration
    status = sgtl5000_configure_dsp();
    if (status != I2C_SUCCESS) {
        printf("Failed to configure DSP\r\n");
        return status;
    }
    // Set initial levels
    status = sgtl5000_set_levels();
    if (status != I2C_SUCCESS) {
        printf("Failed to set initial levels\r\n");
        return status;
    }
    /*
    status = sgtl5000_dap_surround_set(SGTL_SURROUND_STEREO, 7); // Enable surround sound w
    if (status != I2C_SUCCESS) {
        printf("Failed to set surround sound\r\n");
        return status;
    }
    printf("Surround sound enabled\r\n");

    
    status = sgtl5000_dap_bass_enhance_set(true, 0x00, 0x00); // Enable bass enhancement
    if (status != I2C_SUCCESS) {
        printf("Failed to set bass enhancement\r\n");
        return status;
    }
    */
    sgtl5000_dap_geq_set_bands_db(-12, +12, +12, +12, -12); 
    if (status != I2C_SUCCESS) {
        printf("Failed to set GEQ bands\r\n");
        return status;
    }
    printf("GEQ bands set\r\n");
    
    return I2C_SUCCESS;
}
