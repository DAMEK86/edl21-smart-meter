#ifndef ENCODE_H
#define ENCODE_H

#include <stdint.h>

union SmlValue {
    int i;
    unsigned char data[8];
};

class Edl21
{
private:
    unsigned char StartSequence[8] = { 0x1b, 0x1b, 0x1b, 0x1b, 0x01, 0x01, 0x01, 0x01 };
    unsigned char EscapeSequence[5] = {0x1b, 0x1b, 0x1b, 0x1b, 0x1a};

    int findIndexInSequence(unsigned char *stream, unsigned short size, unsigned char *sequence, unsigned char sizeOfSequence);
public:
    int FindStartIndexInSequence(unsigned char *stream, unsigned short size);
    int FindEscapeIndexInSequence(unsigned char *stream, unsigned short size);
    int FindPowerInSequence(unsigned char *msg, unsigned short msgSize, unsigned char *sequence, unsigned char sequenceSize);
    int FindConsumptionInSequence(unsigned char *msg, unsigned short msgSize, unsigned char *sequence, unsigned char sequenceSize);

    unsigned char ConsumptionOverallSequence[19] = { 0x77, 0x07, 0x01, 0x00, 0x01, 0x08, 0x00, 0xFF, 0x65, 0x00, 0x01, 0x01, 0x82, 0x01, 0x62, 0x1E, 0x52, 0xFF, 0x59 };
    unsigned char ConsumptionHTSequence[15] = { 0x77, 0x07, 0x01, 0x00, 0x01, 0x08, 0x01, 0xFF, 0x01, 0x01, 0x62, 0x1E, 0x52, 0xFF, 0x59 };
    unsigned char ConsumptionNTSequence[15] = { 0x77, 0x07, 0x01, 0x00, 0x01, 0x08, 0x02, 0xFF, 0x01, 0x01, 0x62, 0x1E, 0x52, 0xFF, 0x59 };
    unsigned char DeliveredOverallSequence[19] = { 0x77, 0x07, 0x01, 0x00, 0x02, 0x08, 0x00, 0xFF, 0x65, 0x00, 0x01, 0x01, 0x82, 0x01, 0x62, 0x1E, 0x52, 0xFF, 0x59 };
    unsigned char DeliveredHTSequence[15] = { 0x77, 0x07, 0x01, 0x00, 0x02, 0x08, 0x01, 0xff, 0x01, 0x01, 0x62, 0x1e, 0x52, 0xff, 0x59};
    unsigned char DeliveredNTSequence[15] = { 0x77, 0x07, 0x01, 0x00, 0x02, 0x08, 0x02, 0xff, 0x01, 0x01, 0x62, 0x1e, 0x52, 0xff, 0x59};
    unsigned char PowerAllSequence[15] = { 0x77, 0x07, 0x01, 0x00, 0x10, 0x07, 0x00, 0xFF, 0x01, 0x01, 0x62, 0x1B, 0x52, 0x00, 0x55};
    unsigned char PowerL1Sequence[15] = { 0x77, 0x07, 0x01, 0x00, 0x24, 0x07, 0x00, 0xFF, 0x01, 0x01, 0x62, 0x1B, 0x52, 0x00, 0x55};
    unsigned char PowerL2Sequence[15] = { 0x77, 0x07, 0x01, 0x00, 0x38, 0x07, 0x00, 0xFF, 0x01, 0x01, 0x62, 0x1B, 0x52, 0x00, 0x55};
    unsigned char PowerL3Sequence[15] = { 0x77, 0x07, 0x01, 0x00, 0x4C, 0x07, 0x00, 0xFF, 0x01, 0x01, 0x62, 0x1B, 0x52, 0x00, 0x55};
};

extern Edl21 Edl21encoder;

#endif