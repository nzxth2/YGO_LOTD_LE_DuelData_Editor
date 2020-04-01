#pragma once

#include <fstream>
#include <vector>
#include <string>

using Byte=uint8_t;
using Short=uint16_t;
using Int=uint32_t;
using Long=uint64_t;

struct FILE_DATA{
    // number of characters in this file
    Long duelCount;    

    // duel id (001 to duelCount)
    std::vector<Int> field1;
    // series id (0=Yu-Gi-Oh!, 5=Vrains)
    std::vector<Int> field2;
    // duel number in series
    std::vector<Int> field3;
    // left character id
    std::vector<Int> field4;
    // right character id
    std::vector<Int> field5;
    // left character deck id
    std::vector<Int> field6;
    // right character deck id
    std::vector<Int> field7;
    // arena id
    std::vector<Int> field8;
    // reward pack
    std::vector<Int> field9;
    // ??? (always -1)
    std::vector<Int> field10;
    // ??? (always 0, except for 2 duels)
    std::vector<Int> field11;
    
    // pointer to duel id name
    std::vector<Long> pointer1;
    // pointer to left character alt ver id
    std::vector<Long> pointer2;
    // pointer to right character alt ver id
    std::vector<Long> pointer3;
    // pointer to duel name
    std::vector<Long> pointer4;
    // pointer to duel description
    std::vector<Long> pointer5;
    // pointer to duel tip
    std::vector<Long> pointer6;

    // duel id name
    std::vector<std::string> string1;
    // left character alt ver id
    std::vector<std::string> string2;
    // right character alt ver id
    std::vector<std::string> string3;
    // duel name
    std::vector<std::string> string4;
    // duel description (unused)
    std::vector<std::u16string> string5;
    // duel tip
    std::vector<std::string> string6;
};

Byte ReadByte(std::ifstream &file);
Short ReadShort(std::ifstream &file);
Int ReadInt(std::ifstream &file);
Long ReadLong(std::ifstream &file);
std::u16string ReadString(std::ifstream &file);
std::string ReadByteString(std::ifstream &file);

void WriteByte(std::ofstream &file, Byte value);
void WriteShort(std::ofstream &file, Short value);
void WriteInt(std::ofstream &file, Int value);
void WriteLong(std::ofstream &file, Long value);
void WriteString(std::ofstream &file, const std::u16string &string);
void WriteByteString(std::ofstream &file, const std::string &string);

bool ReadFile(const std::string &filename, FILE_DATA &fileData);
bool SaveFile(const std::string &filename, FILE_DATA &fileData);

std::string IntToHexString(const Int &value);

void ClearFileData(FILE_DATA &fileData);
std::string SimplifyString(const std::string &string);