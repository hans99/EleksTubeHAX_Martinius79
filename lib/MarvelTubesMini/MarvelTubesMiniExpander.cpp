#include "MarvelTubesMiniExpander.h"

static bool expander_present = false;

void MarvelTubesMiniExpander::begin()
{
    Serial.println("MarvelTubesMiniExpander::begin");
    Wire.begin(3, 4);
    Wire.setTimeOut(50);
    i2cScan();

    // Check for I/O expander presence by attempting to read a register (e.g., input reg 0x00)
    Wire.beginTransmission(EXPANDER_ADDR);
    expander_present = (Wire.endTransmission() == 0);
    Serial.printf("Expander present at 0x%02X: %s\n", EXPANDER_ADDR, expander_present ? "yes" : "no");

    if (expander_present)
    {
        // Polarity register + init sequence from captured original firmware
        // i2cReplayInitSequence writes reg 0x01=0xFE which likely enables TFT power!
        //expanderWriteCmd(EXPANDER_ADDR, 0x01, 0x11);

        //expanderWriteCmd(EXPANDER_ADDR, 0x02, 0x11);
        i2cReplayInitSequence(EXPANDER_ADDR);

        setDim(150);
        //expanderWriteCmd(EXPANDER_ADDR, 0x03, 0x11);

        // Init all displays once via TFT_eSPI (all I/O expander pins low = broadcast)
        // INITR_GREENTAB160x80 = 0x06 → correct offsets colstart=26, rowstart=1 for 80x160 panel
        Serial.println("TFT init (with all I/O expander pins low)...");
        //expanderWriteCmd(EXPANDER_ADDR, 0x00, 0x00);
        //delay(5);
        //test_tft.init(INITR_GREENTAB160x80);
        // test_tft.writecommand(TFT_INVOFF); // Library sends INVON for this tab type, but panel needs INVOFF
        //test_tft.setRotation(0);
        //test_tft.fillScreen(TFT_BLACK);
        //expanderWriteCmd(EXPANDER_ADDR, 0x00, 0xFF); // all I/O expander outputs high
        Serial.println("TFT init done.");
    }
}

static bool i2cReadReg(uint8_t address, uint8_t reg, uint8_t &value)
{
    Wire.beginTransmission(address);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0)
    {
        return false;
    }
    if (Wire.requestFrom(address, (uint8_t)1) != 1)
    {
        return false;
    }
    value = Wire.read();
    return true;
}

static bool i2cReadDirect(uint8_t address, uint8_t &value)
{
    if (Wire.requestFrom(address, (uint8_t)1) != 1)
    {
        return false;
    }
    value = Wire.read();
    return true;
}

static bool i2cWriteReg(uint8_t address, uint8_t reg, uint8_t value)
{
    Serial.printf("expanderWriteCmd at 0x%02X: %d %d\n", address, reg, value);
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

void MarvelTubesMiniExpander::setAll(bool update_)
{
    //expander.expanderWriteCmdDigit(0x00);
    i2cWriteReg(EXPANDER_ADDR, EXPANDER_CMD_DIGIT, 0x00);
}

void MarvelTubesMiniExpander::clear(bool update_)
{
    //expander.expanderWriteCmdDigit(0xFF);
    i2cWriteReg(EXPANDER_ADDR, EXPANDER_CMD_DIGIT, 0xFF);
}

void MarvelTubesMiniExpander::setDigit(uint8_t digit, bool update_)
{
    //expander.expanderWriteCmdDigit(cs_masks[digit]);
    i2cWriteReg(EXPANDER_ADDR, EXPANDER_CMD_DIGIT, cs_masks[digit]);
}

void MarvelTubesMiniExpander::setDim(uint32_t duty)
{
    i2cWriteReg(EXPANDER_ADDR, EXPANDER_CMD_DIM, 255 - duty);
}

bool MarvelTubesMiniExpander::expanderWriteCmd(uint8_t address, uint8_t cmd, uint8_t arg)
{
    Serial.printf("expanderWriteCmd at 0x%02X: %d %d\n", address, cmd, arg);
    return i2cWriteReg(address, cmd, arg);
    delay(2);
}
/*
bool expanderWriteCmdDigit(uint8_t arg)
{
    Serial.printf("expanderWriteCmdDigit: %d\n", arg);
    return i2cWriteReg(EXPANDER_ADDR, EXPANDER_CMD_DIGIT, arg);
    delay(2);
}

bool expanderWriteCmdDim(uint8_t arg)
{
    Serial.printf("expanderWriteCmdDim: %d\n", arg);
    return i2cWriteReg(EXPANDER_ADDR, EXPANDER_CMD_DIM, arg);
    delay(2);
}
*/
void MarvelTubesMiniExpander::i2cReplayInitSequence(uint8_t address)
{
    Serial.println("MarvelTubesMiniExpander::i2cReplayInitSequence");
    static bool replay_done = false;
    if (replay_done)
    {
        return;
    }
    replay_done = true;

    Serial.println("  Replay: init sequence 01 FE -> 01 FC -> 01 FE");
    i2cWriteReg(address, 0x01, 0xFE);
    delay(90);
    i2cWriteReg(address, 0x01, 0xFC);
    delay(90);
    i2cWriteReg(address, 0x01, 0xFE);
    delay(100);
}

void MarvelTubesMiniExpander::i2cScan()
{
    uint8_t found = 0;
    Serial.println("I2C scan on SDA=GPIO4, SCL=GPIO3");
    for (uint8_t address = 1; address < 127; address++)
    {
        Wire.beginTransmission(address);
        uint8_t error = Wire.endTransmission();
        if (error == 0)
        {
            Serial.printf("I2C device found at 0x%02X\n", address);
            found++;

            uint8_t value = 0;
            if (i2cReadReg(address, 0x00, value))
            {
                Serial.printf("  Reg 0x00 (Input): 0x%02X\n", value);
            }
            if (i2cReadReg(address, 0x01, value))
            {
                Serial.printf("  Reg 0x01 (Output): 0x%02X\n", value);
            }
            if (i2cReadReg(address, 0x02, value))
            {
                Serial.printf("  Reg 0x02 (Polarity): 0x%02X\n", value);
            }
            if (i2cReadReg(address, 0x03, value))
            {
                Serial.printf("  Reg 0x03 (Config): 0x%02X\n", value);
            }
            if (i2cReadReg(address, 0x04, value))
            {
                Serial.printf("  Reg 0x04 (Test): 0x%02X\n", value);
            }
            if (i2cReadDirect(address, value))
            {
                Serial.printf("  Direct read: 0x%02X\n", value);
            }
        }
        // else
        // {
        //   // print errors
        //   if (error == 4)
        //     Serial.printf("Unknown error at address 0x%02X\n", address);
        //   else if (error == 2)
        //     Serial.printf("NACK on transmit of address 0x%02X\n", address);
        //   else if (error == 3)
        //     Serial.printf("NACK on transmit of data at address 0x%02X\n", address);
        //   else if (error == 1)
        //     Serial.printf("Other error at address 0x%02X\n", address);
        //     else
        //     Serial.printf("Error %d at address 0x%02X\n", error, address);

        // }

        delay(1);
    }
    if (found == 0)
    {
        Serial.println("No I2C devices found.");
    }
}
