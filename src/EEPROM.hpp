/**
 * @brief 
 * 
 * @file EEPROM.hpp
 * @author Arseniy Churin
 * @date 2018-05-18
 */

#ifndef EEPROM_HPP
#define EEPROM_HPP

#include <EEPROM.h>
#include <Arduino.h>
#include <WString.h>

const char *command;
char eRead[25];
byte len;
bool byteRead;
byte tray = 0;
bool success = true;

/**
 * @brief Write string in memory
 * 
 * Write string in EEPROM memory by address
 * First byte is lenght of string
 * Other - content bytes
 * 
 * @param startAt Start address
 * @param id Char array content
 */
void SaveString(int startAt, const char *id)
{
    EEPROM.begin(512);
    EEPROM.write(startAt, strlen(id));
    for (byte i = 0; i <= strlen(id); i++)
    {
        EEPROM.write(i + startAt + 1, (uint8_t)id[i]);
    }
    EEPROM.commit();
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
void ReadString(byte startAt)
{
    EEPROM.begin(512);
    byte bufor = EEPROM.read(startAt);
    for (byte i = 0; i <= bufor; i++)
    {
        eRead[i] = (char)EEPROM.read(i + startAt + 1);
    }
    len = bufor;
}

/**
 * @brief Save byte in memory
 * 
 * Write byte in EEPROM memory by address
 * 
 * @param startAt Address
 * @param val Value to write
 */
void SaveByte(int startAt, byte val)
{
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
byte ReadByte(byte startAt)
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
int ReadInt(int addr)
{
    EEPROM.begin(512);
    byte raw[2];
    for (byte i = 0; i < 2; i++)
        raw[i] = EEPROM.read(addr + i);
    int &num = (int &)raw;
    return num;
}

#endif