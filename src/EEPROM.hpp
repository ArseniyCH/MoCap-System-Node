/**
 * @brief 
 * 
 * @file EEPROM.hpp
 * @author Arseniy Churin
 * @date 2018-05-18
 */

#ifndef EEPROM_HPP
#define EEPROM_HPP

#define COLOR_ADDRESS 100

#include <EEPROM.h>
#include <Arduino.h>
#include <WString.h>

const uint8_t color_size = 3;

/**
 * @brief Write string in memory
 * 
 * Write string in EEPROM memory by address
 * First byte is lenght of string
 * Other - content bytes
 * 
 * @param startAt Start address
 * @param buf uint8_t array content
 * @param len length of content
 */
void SaveString(int startAt, const uint8_t *buf, uint8_t len)
{
    EEPROM.begin(512);
    EEPROM.write(startAt, len);
    for (byte i = 0; i < len; i++)
    {
        EEPROM.write(i + startAt + 1, buf[i]);
    }
    EEPROM.write(startAt + len + 1, 'o');
    EEPROM.write(startAt + len + 2, 'k');

    EEPROM.end();
}

/**
 * @brief Read string from memory
 * 
 * Read string from EEPROM memory by address
 * First byte is lenght of string
 * Other - content bytes
 * 
 * @param startAt Start address
 */
String ReadString(uint8_t startAt)
{
    String read = "";
    EEPROM.begin(512);
    uint8_t len = EEPROM.read(startAt);
    for (uint8_t i = 0; i < len; i++)
    {
        read += (char)EEPROM.read(i + startAt + 1);
    }
    String check = "";
    check += (char)EEPROM.read(startAt + len + 1);
    check += (char)EEPROM.read(startAt + len + 2);

    EEPROM.end();

    Serial.print("ssid from eeprom: ");
    Serial.println(read);

    Serial.print("check from eeprom: ");
    Serial.println(check);

    if (!check.equals("ok"))
        return "";

    return read;
}

/**
 * @brief Save byte in memory
 * 
 * Write byte in EEPROM memory by address
 * 
 * @param startAt Address
 * @param val Value to write
 */
void SaveByte(int startAt, uint8_t val)
{
    EEPROM.begin(512);
    EEPROM.write(startAt, val);
    EEPROM.commit();
}

/**
 * @brief Read byte from memory
 * 
 * Read byte from EEPROM memory by address
 * 
 * @param startAt Address
 * @return byte 
 */
uint8_t ReadByte(byte startAt)
{
    EEPROM.begin(512);
    byte Read = -1;
    Read = EEPROM.read(startAt);
    return Read;
}

/**
 * @brief Write 2-byte int to memory
 * 
 * Write 2-byte int in EEPROM memory by address
 * Convert to number to 2 bytes and write in memory
 * 
 * @param addr Start address
 * @param num Value to write
 */
void SaveInt(int addr, int num)
{
    EEPROM.begin(512);
    byte raw[2];
    (int &)raw = num;
    for (byte i = 0; i < 2; i++)
        EEPROM.write(addr + i, raw[i]);
    EEPROM.commit();
}

/**
 * @brief Reed 2-byte int from memory
 * 
 * Read 2-byte int from EEPROM memory by address
 * Read two bytes from memory and converts to int
 * 
 * @param addr Start address
 * @return int 
 */
uint16_t ReadInt(uint16_t addr)
{
    EEPROM.begin(512);
    uint16_t number;

    number = EEPROM.read(addr++);
    number = number << 8;
    number |= EEPROM.read(addr);

    Serial.print("OK: ");
    Serial.print(EEPROM.read(addr - 1));
    Serial.println(EEPROM.read(addr));

    Serial.print("Number: ");
    Serial.println(number);

    return number;
}

void WriteRGB(uint16_t *buf, uint16_t address, bool is8bit = false)
{
    EEPROM.begin(512);

    for (int i = 0; i < color_size * 2; ++i)
    {
        if (!is8bit)
            EEPROM.write(address++, buf[i] / 4);
        else
            EEPROM.write(address++, buf[i]);
    }

    EEPROM.write(address + color_size * 2, 'o');
    EEPROM.write(address + color_size * 2 + 1, 'k');

    EEPROM.commit();
}

void ReadMappedRGB(uint8_t *buf, uint16_t address)
{
    EEPROM.begin(512);

    int16_t check = ReadInt(address + color_size * 2);
    Serial.print("check: ");
    Serial.println(check);
    if (check != 28523)
    {
        memset(buf, 0, color_size * 2);
        buf[2] = 0xFF;
        WriteRGB((uint16_t *)buf, address, true);
        EEPROM.end();
        return;
    }

    for (int i = 0; i < color_size * 2; ++i)
    {
        buf[i] = EEPROM.read(address++);
    }

    EEPROM.end();

    return;
}

void ReadRGB(uint16_t *buf, uint16_t address)
{
    uint8_t *pre = new uint8_t[color_size * 2];
    ReadMappedRGB(pre, address);
    for (int i = 0; i < color_size * 2; ++i)
    {
        buf[i] = pre[i] * 4;
    }

    delete[] pre;
}

bool CompareArrays(uint16_t *arr1, uint16_t *arr2, uint16_t length)
{
    for (int i = 0; i < length; ++i)
    {
        if (arr1[i] != arr2[i])
            return false;
    }
    return true;
}

#endif