#include "../../include/GLOBAL_DEFINES.h"
#include <Wire.h>

class MarvelTubesMiniExpander
{
public:
    MarvelTubesMiniExpander() {};
    void begin();
    void setAll(bool update_ = true);
    void clear(bool update_ = true);
    void setDigit(uint8_t digit, bool update_ = true);
    void setDim(uint32_t duty);

    void setSecondsOnes() { setDigit(SECONDS_ONES); }
    void setSecondsTens() { setDigit(SECONDS_TENS); }
    void setMinutesOnes() { setDigit(MINUTES_ONES); }
    void setMinutesTens() { setDigit(MINUTES_TENS); }
    void setHoursOnes() { setDigit(HOURS_ONES); }
    void setHoursTens() { setDigit(HOURS_TENS); }

    //void i2cScan();
    //void i2cReplayInitSequence(uint8_t address);
    bool expanderWriteCmd(uint8_t address, uint8_t cmd, uint8_t arg);
    //bool expanderWriteCmdDigit(uint8_t arg);
    //bool expanderWriteCmdDim(uint8_t arg);
    // void i2cReplayInitSequence(uint8_t address);

private:
    void i2cScan();
    void i2cReplayInitSequence(uint8_t address);
    static constexpr uint8_t EXPANDER_ADDR = 0x19;
    static constexpr uint8_t EXPANDER_CMD_DIGIT = 0x00;
    static constexpr uint8_t EXPANDER_CMD_DIM = 0x02;
    const int numLCDs = NUM_DIGITS;

    // TODO or better to create new defines fort digits?
    // const uint8_t cs_masks[NUM_DIGITS] = {0xFE, 0xFD, 0xFB, 0xDF, 0xBF, 0x7F};
    const uint8_t cs_masks[NUM_DIGITS] = {0x7F, 0xBF, 0xDF, 0xFB, 0xFD, 0xFE};
};
