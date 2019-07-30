#ifndef PACKAGE_H
#define PACKAGE_H

#include <inttypes.h>
#include <QByteArray>
#include <vector>

namespace PckNS {

using byte_t = unsigned char;

union PackageType {
    struct {
        byte_t start;
        byte_t bytes[4];
        byte_t checksum;
    } pck;

    struct {
        byte_t bytes[6];
    } byteArray;
};

/*
struct PackageType {
    const uint8_t start{ 0x0A };
    uint8_t bytes[4]{ 0x00 };
    uint8_t checksum{ 0xFF };
};
*/
class Package
{
public:
    Package();
    Package(byte_t byte1, byte_t byte2, byte_t byte3, byte_t byte4);

    ~Package();

    void setByte(size_t byteNum, uint8_t byteData) { m_packageData.pck.bytes[byteNum] = byteData; }

    std::vector<byte_t> makePackage();
    std::vector<byte_t> makeEncodedPackage();

private:
    void calcChecksum();

    PackageType m_packageData;
};

} // PckNS
#endif // PACKAGE_H
