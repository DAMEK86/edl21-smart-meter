#include <encode.h>

static union SmlValue currentValue;

int Edl21::FindStartIndexInSequence(unsigned char *stream, unsigned short size) {
    return findIndexInSequence(stream, size, StartSequence, sizeof(StartSequence) / sizeof(unsigned char));
};

int Edl21::FindEscapeIndexInSequence(unsigned char *stream, unsigned short size) {
    return findIndexInSequence(stream, size, EscapeSequence, sizeof(EscapeSequence) / sizeof(unsigned char));
};

typedef void copySequenceData(unsigned short offset, unsigned char *msg);

inline void copyPowerData(unsigned short offset, unsigned char *msg) {
  for(int y = 0; y< 4; y++){
    currentValue.data[3-y] = msg[offset+y+1];
  }
}

inline void copyConsumptionData(unsigned short offset, unsigned char *msg) {
  for(int y = 0; y< 8; y++){
    currentValue.data[7-y] = msg[offset+y+1];
  }
}

int extractValueFromSequence(unsigned char *msg, unsigned short msgSize, unsigned char *sequence, unsigned char sequenceSize, copySequenceData *callback) {
  int startIndex = 0;
  currentValue.i = 0;
  for(unsigned short x = 0; x < msgSize; x++){
    if (msg[x] == sequence[startIndex])
    {
      startIndex++;
      if (startIndex == sequenceSize)
      {
        callback(x, msg);
        startIndex = 0;
      }
    }
    else {
      startIndex = 0;
    }
  }

   return currentValue.i;
}

int Edl21::FindPowerInSequence(unsigned char *msg, unsigned short msgSize, unsigned char *sequence, unsigned char sequenceSize) { 
  return extractValueFromSequence(msg, msgSize, sequence, sequenceSize, copyPowerData);
}

int Edl21::FindConsumptionInSequence(unsigned char *msg, unsigned short msgSize, unsigned char *sequence, unsigned char sequenceSize) { 
  return extractValueFromSequence(msg, msgSize, sequence, sequenceSize, copyConsumptionData);
}

int Edl21::findIndexInSequence(unsigned char *stream, unsigned short size, unsigned char *sequence, unsigned char sizeOfSequence) {
    for (unsigned int i = 0; i < size; i++)
    {
        if (stream[i] == sequence[0])
        {
            bool match = true;
            for (unsigned char j = 0; j < sizeOfSequence; j++)
            {
                if (stream[i + j] == sequence[j])
                {
                    continue;
                }
                match = false;
                break;
            }

            if (match)
            {
                return i;
            }
        }
    }

    return -1;
}