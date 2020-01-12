#ifndef SNOUTNET_AHT10_H
#define SNOUTNET_AHT10_H

#include <Wire.h>

#define SNOUTNET_AHT10_ADDRLOW 0x38
#define SNOUTNET_AHT10_ADDRHIGH 0x39

class SnoutnetAHT10 {
private:
	uint8_t addr;

public:
	SnoutnetAHT10(uint8_t saddr = SNOUTNET_AHT10_ADDRLOW );
	~SnoutnetAHT10();
	bool begin();

	bool getReading(float* tt, float* rh);
	bool getReading(double* tt, double* rh);
	uint8_t getStatus();
};

#endif
