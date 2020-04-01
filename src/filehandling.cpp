#include "filehandling.h"
#include <sstream>
#include <codecvt>
#include <locale>
#include <array>


Byte ReadByte(std::ifstream &file){
    Byte value;
    file.read((char*)(&value),1);
    return value;
}
Short ReadShort(std::ifstream &file){
    Short value;
    std::array<Byte,2> bytes;
    file.read((char*)(bytes.data()),2);
    value = bytes[0] | (bytes[1]<<8);
    return value;
}
Int ReadInt(std::ifstream &file){
    Int value;
    std::array<Byte,4> bytes;
    file.read((char*)(bytes.data()),4);
    value = bytes[0] | (bytes[1]<<8) | (bytes[2]<<16) | (bytes[3]<<24);
    return value;
}
Long ReadLong(std::ifstream &file){
    Long value;
    std::array<Byte,8> bytes;
    file.read((char*)(bytes.data()),8);
    value = bytes[0] | (bytes[1]<<8) | (bytes[2]<<16) | (bytes[3]<<24) | (bytes[4]<<32) | (bytes[5]<<40) | (bytes[6]<<48) | (bytes[7]<<56);
    return value;
}

std::u16string ReadString(std::ifstream &file){
    std::u16string string;
    bool stringTerminated=false;
    while(!stringTerminated){
        Short currShort=ReadShort(file);
        if (currShort==0){
            stringTerminated=true;
        }else{
            string+=(char16_t)currShort;
        }
    }
    return string;
}

std::string ReadByteString(std::ifstream &file){
    std::string string;
    bool stringTerminated=false;
    while(!stringTerminated){
        Byte currByte=ReadByte(file);
        if (currByte==0){
            stringTerminated=true;
        }else{
            string+=(char)currByte;
        }
    }
    return string;
}

void WriteByte(std::ofstream &file, Byte value){
    file.write((char*)(&value),1);
}
void WriteShort(std::ofstream &file, Short value){
    std::array<Byte,2> bytes;
    bytes[0]= value & 0xFF;
    bytes[1]= (value>>8) & 0xFF;
    file.write((char*)(bytes.data()),2);
}

void WriteInt(std::ofstream &file, Int value){
    std::array<Byte,4> bytes;
    bytes[0]= value & 0xFF;
    bytes[1]= (value>>8) & 0xFF;
    bytes[2]= (value>>16) & 0xFF;
    bytes[3]= (value>>24) & 0xFF;
    file.write((char*)(bytes.data()),4);
}
void WriteLong(std::ofstream &file, Long value){
    std::array<Byte,8> bytes;
    bytes[0]= value & 0xFF;
    bytes[1]= (value>>8) & 0xFF;
    bytes[2]= (value>>16) & 0xFF;
    bytes[3]= (value>>24) & 0xFF;
    bytes[4]= (value>>32) & 0xFF;
    bytes[5]= (value>>40) & 0xFF;
    bytes[6]= (value>>48) & 0xFF;
    bytes[7]= (value>>56) & 0xFF;
    file.write((char*)(bytes.data()),8);
}

void WriteString(std::ofstream &file, const std::u16string &string){
    for (std::size_t i=0; i<string.size();i++){
        WriteShort(file,(Short)(string[i]));
    }
    WriteShort(file,0);
}

void WriteByteString(std::ofstream &file, const std::string &string){
    for (std::size_t i=0; i<string.size();i++){
        WriteByte(file,(Byte)(string[i]));
    }
    WriteByte(file,0);
}

void ClearFileData(FILE_DATA &fileData){
    fileData.field1.clear();
    fileData.field2.clear();
    fileData.field3.clear();
    fileData.field4.clear();
    fileData.field5.clear();
    fileData.field6.clear();
    fileData.field7.clear();
    fileData.field8.clear();
    fileData.field9.clear();
    fileData.field10.clear();
    fileData.field11.clear();
    fileData.pointer1.clear();
    fileData.pointer2.clear();
    fileData.pointer3.clear();
    fileData.pointer4.clear();
    fileData.pointer5.clear();
    fileData.pointer6.clear();
    fileData.string1.clear();
    fileData.string2.clear();
    fileData.string3.clear();
    fileData.string4.clear();
    fileData.string5.clear();
    fileData.string6.clear();
}

bool SaveFile(const std::string &filename, FILE_DATA &fileData){
    std::ofstream file(filename,std::ios::out|std::ios::binary);
    if (file.is_open()){
        WriteLong(file,fileData.duelCount);
        Long stringOffset=8+fileData.duelCount*(11*4+6*8);
        // temporary containers to hold u16 strings
        std::vector<std::u16string> string4u16;
        std::vector<std::u16string> string6u16;
        for (Long i=0; i<fileData.duelCount;i++){
            WriteInt(file,fileData.field1[i]);
            WriteInt(file,fileData.field2[i]);
            WriteInt(file,fileData.field3[i]);
            WriteInt(file,fileData.field4[i]);
            WriteInt(file,fileData.field5[i]);
            WriteInt(file,fileData.field6[i]);
            WriteInt(file,fileData.field7[i]);
            WriteInt(file,fileData.field8[i]);
            WriteInt(file,fileData.field9[i]);
            WriteInt(file,fileData.field10[i]);
            WriteInt(file,fileData.field11[i]);
            WriteLong(file,stringOffset);
            stringOffset+=fileData.string1[i].size()+1;
            WriteLong(file,stringOffset);
            stringOffset+=fileData.string2[i].size()+1;
            WriteLong(file,stringOffset);
            stringOffset+=fileData.string3[i].size()+1;
            WriteLong(file,stringOffset);
            // temporarily convert u8 strings back to u16 so we can write them to a file
            string4u16.push_back(std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(SimplifyString(fileData.string4[i])));
            stringOffset+=string4u16[i].size()*2+2;
            WriteLong(file,stringOffset);
            stringOffset+=fileData.string5[i].size()*2+2;
            WriteLong(file,stringOffset);
            string6u16.push_back(std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(SimplifyString(fileData.string6[i])));
            stringOffset+=string6u16[i].size()*2+2;
        }
        for (Long i=0; i<fileData.duelCount;i++){
            WriteByteString(file,fileData.string1[i]);
            WriteByteString(file,fileData.string2[i]);
            WriteByteString(file,fileData.string3[i]);
            WriteString(file,string4u16[i]);
            WriteString(file,fileData.string5[i]);
            WriteString(file,string6u16[i]);
        }
        bool success = file.good();
        file.close();
        return success;
    }
    return false;
}

bool ReadFile(const std::string &filename, FILE_DATA &fileData){
    std::ifstream file(filename,std::ios::in|std::ios::binary);
    if (file.is_open()){
        ClearFileData(fileData);
        fileData.duelCount=ReadLong(file);
        for (Long i=0; i< fileData.duelCount;i++){
            fileData.field1.push_back(ReadInt(file));
            fileData.field2.push_back(ReadInt(file));
            fileData.field3.push_back(ReadInt(file));
            fileData.field4.push_back(ReadInt(file));
            fileData.field5.push_back(ReadInt(file));
            fileData.field6.push_back(ReadInt(file));
            fileData.field7.push_back(ReadInt(file));
            fileData.field8.push_back(ReadInt(file));
            fileData.field9.push_back(ReadInt(file));
            fileData.field10.push_back(ReadInt(file));
            fileData.field11.push_back(ReadInt(file));
            fileData.pointer1.push_back(ReadLong(file));
            fileData.pointer2.push_back(ReadLong(file));
            fileData.pointer3.push_back(ReadLong(file));
            fileData.pointer4.push_back(ReadLong(file));
            fileData.pointer5.push_back(ReadLong(file));
            fileData.pointer6.push_back(ReadLong(file));
        }
        for (Long i=0; i< fileData.duelCount;i++){
            file.seekg(fileData.pointer1[i],file.beg);
            fileData.string1.push_back(ReadByteString(file));
            file.seekg(fileData.pointer2[i],file.beg);
            fileData.string2.push_back(ReadByteString(file));
            file.seekg(fileData.pointer3[i],file.beg);
            fileData.string3.push_back(ReadByteString(file));

            file.seekg(fileData.pointer4[i],file.beg);
            fileData.string4.push_back(std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(ReadString(file)));
            file.seekg(fileData.pointer5[i],file.beg);
            fileData.string5.push_back(ReadString(file));
            file.seekg(fileData.pointer6[i],file.beg);
            fileData.string6.push_back(std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(ReadString(file)));
        }
        bool success = file.good();
        file.close();
        return success;
    }
    return false;
    
}

std::string IntToHexString(const Int &value){
    std::stringstream stream;
    stream << std::hex << value;
    std::string result=stream.str();
    for (auto & c: result) c = toupper(c);
    return result;
}


std::string SimplifyString(const std::string &string){
    std::size_t firstPos=string.find_first_not_of("\t\n\v\f\r ");
    std::size_t lastPos=string.find_last_not_of("\t\n\v\f\r ");
    if (firstPos==std::string::npos){ // string only contains whitespace
        return "";
    }else{
        // trim string first
        std::string trimmedString=string.substr(firstPos,lastPos-firstPos+1);
        // now remove extra whitespaces
        std::string result;
        bool whitespaceEncountered=false;
        for (std::size_t i=0; i<trimmedString.size();i++){
            if (trimmedString[i]=='\t' || trimmedString[i]=='\n' || trimmedString[i]=='\v' || trimmedString[i]=='\f' || trimmedString[i]=='\r' || trimmedString[i]==' '){
                if (!whitespaceEncountered){
                    result+=' ';
                    whitespaceEncountered=true;
                }
            }else{
                whitespaceEncountered=false;
                result+=trimmedString[i];
            }
        }
        return result;
    }
}