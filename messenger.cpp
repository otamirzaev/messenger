#include <iostream>
#include <string>
#include <bitset>
#include <iomanip>
#include <fstream>
#include "messenger.h"

#ifdef _DEBUG
std::ostream& dbg_out = std::cout;
#else
std::ofstream dev_null("/dev/null");
std::ostream& dbg_out = dev_null;
#endif

#define NAME_MAXLEN            15
#define MSG_MAXLEN             31
#define MAX_BYTES_WITHOUT_NAME 33 // max number of bytes without name bytes: 2 + 31
#define FLAG                   5
#define FLAG_MASK              -96

// https://github.com/mozram/CRC4-ITU
static unsigned char const table_byte[256] = {
    0x0, 0x7, 0xe, 0x9, 0x5, 0x2, 0xb, 0xc, 0xa, 0xd, 0x4, 0x3, 0xf, 0x8, 0x1, 0x6,
    0xd, 0xa, 0x3, 0x4, 0x8, 0xf, 0x6, 0x1, 0x7, 0x0, 0x9, 0xe, 0x2, 0x5, 0xc, 0xb,
    0x3, 0x4, 0xd, 0xa, 0x6, 0x1, 0x8, 0xf, 0x9, 0xe, 0x7, 0x0, 0xc, 0xb, 0x2, 0x5,
    0xe, 0x9, 0x0, 0x7, 0xb, 0xc, 0x5, 0x2, 0x4, 0x3, 0xa, 0xd, 0x1, 0x6, 0xf, 0x8,
    0x6, 0x1, 0x8, 0xf, 0x3, 0x4, 0xd, 0xa, 0xc, 0xb, 0x2, 0x5, 0x9, 0xe, 0x7, 0x0,
    0xb, 0xc, 0x5, 0x2, 0xe, 0x9, 0x0, 0x7, 0x1, 0x6, 0xf, 0x8, 0x4, 0x3, 0xa, 0xd,
    0x5, 0x2, 0xb, 0xc, 0x0, 0x7, 0xe, 0x9, 0xf, 0x8, 0x1, 0x6, 0xa, 0xd, 0x4, 0x3,
    0x8, 0xf, 0x6, 0x1, 0xd, 0xa, 0x3, 0x4, 0x2, 0x5, 0xc, 0xb, 0x7, 0x0, 0x9, 0xe,
    0xc, 0xb, 0x2, 0x5, 0x9, 0xe, 0x7, 0x0, 0x6, 0x1, 0x8, 0xf, 0x3, 0x4, 0xd, 0xa,
    0x1, 0x6, 0xf, 0x8, 0x4, 0x3, 0xa, 0xd, 0xb, 0xc, 0x5, 0x2, 0xe, 0x9, 0x0, 0x7,
    0xf, 0x8, 0x1, 0x6, 0xa, 0xd, 0x4, 0x3, 0x5, 0x2, 0xb, 0xc, 0x0, 0x7, 0xe, 0x9,
    0x2, 0x5, 0xc, 0xb, 0x7, 0x0, 0x9, 0xe, 0x8, 0xf, 0x6, 0x1, 0xd, 0xa, 0x3, 0x4,
    0xa, 0xd, 0x4, 0x3, 0xf, 0x8, 0x1, 0x6, 0x0, 0x7, 0xe, 0x9, 0x5, 0x2, 0xb, 0xc,
    0x7, 0x0, 0x9, 0xe, 0x2, 0x5, 0xc, 0xb, 0xd, 0xa, 0x3, 0x4, 0x8, 0xf, 0x6, 0x1,
    0x9, 0xe, 0x7, 0x0, 0xc, 0xb, 0x2, 0x5, 0x3, 0x4, 0xd, 0xa, 0x6, 0x1, 0x8, 0xf,
    0x4, 0x3, 0xa, 0xd, 0x1, 0x6, 0xf, 0x8, 0xe, 0x9, 0x0, 0x7, 0xb, 0xc, 0x5, 0x2
};

static unsigned char crc4itu(unsigned char crc, std::vector<unsigned char> data, unsigned char len) {

    if (data.empty()) {
        return 0;
    }

    crc &= 0xf;
    int i = 0;
    while (len--) {
        crc = table_byte[crc ^ data[i++]];
    }

    return crc;
}

std::vector<std::string> parse(std::vector<uint8_t> param) {

    uint8_t firstByte = param.at(0);
    // Check FLAG
    if (((((firstByte & 0xE0) >> 5)) & 7) != 5) {
        throw std::runtime_error("FLAG is not set to '0b101'!");
    } else {
        dbg_out << "FLAG is set to '0b101'!" << std::endl;
    }

    uint8_t mask = (1 << 5) - 1;
    // The last 5 bits + 1 right shift = length of NAME_LEN
    uint8_t NameLen = (firstByte & mask) >> 1;
    uint8_t lastBitOfFirstByte = firstByte & 1;
    uint8_t secondByte = param.at(1);
    uint8_t halfSecondByte = (secondByte >> 4) & 15;
    uint8_t MsgLen;

    if (lastBitOfFirstByte) {
        MsgLen = halfSecondByte |= 1UL << 4;
    } else {
        MsgLen = halfSecondByte;
    }

    // CRC value from packet
    uint8_t CRC4 = secondByte & ((1 << 4) - 1);
    std::vector<unsigned char> name;
    for (int i = 0; i < NameLen; ++i) {
        name.push_back(param.at(2 + i));
    }

    name.push_back('\0');

    dbg_out << "NAME is '" << name.data() << "'!" << std::endl;

    std::vector<unsigned char> message;
    for (int i = 0; i < MsgLen; ++i) {
        message.push_back(param.at(2 + NameLen + i));
    }

    message.push_back('\0');

    dbg_out << "MESSAGE is '" << message.data() << "'!" << std::endl;

    // String to check CRC4
    std::vector<unsigned char> controlledString;
    controlledString.push_back(5);
    controlledString.push_back(NameLen);
    controlledString.push_back(MsgLen);
    controlledString.insert(controlledString.end(), name.begin(),    name.end()    - 1);
    controlledString.insert(controlledString.end(), message.begin(), message.end() - 1);

    // Calculated CRC value
    std::uint8_t iCRC4 = crc4itu(0x00, controlledString, controlledString.size());

    // Check CRC4 value
    if (CRC4 != iCRC4) {
        throw std::runtime_error("CRC4 hash values not matching! Packet is corrupted!\n");
    } else {
        dbg_out << "Packet is not modified!" << std::endl << std::endl;
    }

    std::string nmStr(name.begin(),     name.end());
    std::string msgStr(message.begin(), message.end() - 1);
    std::vector<std::string> retVec;
    retVec.push_back(nmStr);
    retVec.push_back(msgStr);
    return retVec;
}

void packData(const std::string_view& name, const std::string_view& text, std::vector<uint8_t>& buff) {

    uint8_t firstByte = FLAG_MASK;
    firstByte = firstByte + name.length() * 2;

    std::bitset<8> fB1{ firstByte };
    dbg_out << "\nFirst  byte: " << std::setw(25) << fB1.to_string() << std::endl;

    if (text.length() > 15) {
        firstByte = firstByte + 1;
        std::bitset<8> fB2{ firstByte };
        dbg_out << "The first byte was changed: " << std::setw(10) << fB2.to_string() << std::endl;
    }

    buff.push_back(firstByte);
    uint8_t secondByte = (text.length()) << 4;
    std::bitset<8> sB1{ secondByte };
    dbg_out << "Second byte: " << std::setw(25) << sB1.to_string() << std::endl;

    std::vector<unsigned char> controlledString;

    controlledString.push_back(FLAG);
    controlledString.push_back(name.length());
    controlledString.push_back(text.length());
    controlledString.insert(controlledString.end(), name.begin(), name.end());
    controlledString.insert(controlledString.end(), text.begin(), text.end());

    std::uint8_t CRC4 = crc4itu(0x00, controlledString, controlledString.size());

    std::bitset<8> crc4{ CRC4 };
    dbg_out << "CRC4   value: " << std::setw(24) << crc4.to_string() << std::endl;

    secondByte = secondByte + CRC4;

    std::bitset<8> sB2{ secondByte };
    dbg_out << "Second byte with CRC4 value: " << std::setw(9) << sB2.to_string() << std::endl;

    buff.push_back(secondByte);

    for (int i = 0; i < name.length(); ++i) {
        buff.push_back(name[i]);
    }

    for (int i = 0; i < text.length(); ++i) {
        buff.push_back(text[i]);
    }

    dbg_out << "\nBuffer size: " << buff.size() << std::endl << std::endl;
}

void packMultStrData(const std::string_view& name, const std::string_view& txt, std::vector<uint8_t>& buff) {

    std::vector<uint8_t> vtemp;
    for (int i = 0; i < txt.length(); ++i) {
        vtemp.emplace_back(txt[i]);
        if (vtemp.size() == MSG_MAXLEN) {
            packData(name, std::string(vtemp.begin(), vtemp.end()), buff);
            vtemp.clear();
        }

        if (txt.length() - i == 1) {
            packData(name, std::string(vtemp.begin(), vtemp.end()), buff);
        }
    }
}

namespace messenger {

    std::vector<uint8_t> make_buff(const msg_t& msg) {
        
        dbg_out << "Initial Name ("    << msg.name.length() << " chars.): " << "'" << msg.name  << "'\n";
        dbg_out << "Initial Message (" << msg.text.length() << " chars.): " << "'" << msg.text << "'\n\n";

        if (msg.name.empty()) {
            throw std::length_error("Name can not be empty!\n");
        }

        if (msg.text.empty()) {
            throw std::length_error("Message can not be empty!\n");
        }

        if (msg.name.size() > NAME_MAXLEN) {
            throw std::length_error("Name can not be longer than NAME_MAXLEN");
        }

        std::vector<uint8_t> buff;

        if (msg.text.size() > MSG_MAXLEN) {
            packMultStrData(msg.name, msg.text, buff);
            return buff;

        } else {
            packData(msg.name, msg.text, buff);
            return buff;
        }
    }

    msg_t parse_buff(std::vector<uint8_t>& buff) {

        int mod          = 0;
        int numOfPackets = 0;

        uint8_t fstByte = buff.at(0);
        uint8_t msk     = (1 << 5) - 1;
        // The last 5 bits + 1 right shift = length of NAME_LEN
        uint8_t NmLen = (fstByte & msk) >> 1;

        std::string concStr = "";
        std::string retName = "";
        std::vector<uint8_t> vconcStr;

        if (buff.size() > (MAX_BYTES_WITHOUT_NAME + NmLen)) {
            mod = buff.size() % (MAX_BYTES_WITHOUT_NAME + NmLen);
            numOfPackets = ((buff.size() - mod)/(MAX_BYTES_WITHOUT_NAME + NmLen)) + 1;
            std::vector<std::vector<uint8_t>> vecPacks(numOfPackets, std::vector<uint8_t>());

            int idx = 0;

            for (int k = 0; k < numOfPackets; k++) {
                
                if (k != ((buff.size() - mod)/(MAX_BYTES_WITHOUT_NAME + NmLen))) {
                    for (int j = idx; j < (idx + (MAX_BYTES_WITHOUT_NAME + NmLen)); ++j) {
                        vecPacks[k].push_back(buff[j]);
                    }
                    idx += (MAX_BYTES_WITHOUT_NAME + NmLen);

                } else {
                    for (int j = idx; j < idx + mod; ++j) {
                        vecPacks[k].push_back(buff[j]);
                    }
                    idx += mod;
                }
            }

            for (int k = 0; k < numOfPackets; ++k) {
                std::vector<std::string> nameAndMessage = parse(vecPacks[k]);
                retName = nameAndMessage[0];
                concStr.append(nameAndMessage[1]);
            }

            msg_t retMsg_t(retName, concStr);
            return retMsg_t;

        } else {
            std::vector<std::string> nameAndMessage = parse(buff);
            msg_t retMsg_t(nameAndMessage[0], nameAndMessage[1]);
            return retMsg_t;
        }
    }
}