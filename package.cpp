#include "package.h"

using namespace PckNS;

Package::Package()
{
    m_packageData.pck.start = 0x0A;
    for (int i = 0; i < 4; ++i) {
        m_packageData.pck.bytes[i] = 0x00;
    }
}

Package::Package(byte_t byte1, byte_t byte2, byte_t byte3, byte_t byte4)
{
    m_packageData.pck.bytes[0] = byte1;
    m_packageData.pck.bytes[1] = byte2;
    m_packageData.pck.bytes[2] = byte3;
    m_packageData.pck.bytes[3] = byte4;
}

Package::~Package()
{
    // Do nothing
}

std::vector<byte_t> Package::makePackage()
{
    calcChecksum();

    std::vector<byte_t> tmpVec;
    for (std::size_t i = 0; i < 6; ++i) {
        tmpVec.push_back(m_packageData.byteArray.bytes[i]);
    }

    return tmpVec;
}

std::vector<byte_t> Package::makeEncodedPackage()
{
    std::vector<byte_t> encPck;
    std::vector<byte_t> section;
    std::vector<byte_t> nonEncPck{ this->makePackage() };

    byte_t sectionElementCounter{ 1u };

    for (std::size_t i = 0; i < nonEncPck.size(); ++i) {
        if (nonEncPck[i] == 0x00) {
            encPck.push_back(sectionElementCounter);
            for (auto it = section.begin(); it != section.end(); ++it) {
                encPck.push_back(*it);
            }
            section.clear();
            sectionElementCounter = 1u;
        } else {
            ++sectionElementCounter;
            section.push_back(nonEncPck[i]);
        }
    }

    encPck.push_back(sectionElementCounter);
    for (auto it = section.begin(); it != section.end(); ++it) {
        encPck.push_back(*it);
    }
    encPck.push_back(0x00);

    return encPck;
}

// Private:
void Package::calcChecksum()
{
    byte_t tmpByte{ 0xFF };

    for (uint8_t i = 0; i < 5; ++i) {
        tmpByte -= m_packageData.byteArray.bytes[i];
    }

    m_packageData.pck.checksum = tmpByte;
}
