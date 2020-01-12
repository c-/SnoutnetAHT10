#include <Arduino.h>
#include <SnoutnetAHT10.h>

// cribbed from
// https://github.com/liutyi/arduino-humidity-sensors-test/blob/master/arduino-humidity-sensors-test.ino

#define DEFAULT_TIMEOUT 300

#define AHT1X_CMD_SIZE 3
#define AHT1X_DATA_SIZE 6
#define AHT1X_INIT_DELAY 75
#define AHT1X_T_MEASUREMENT_DELAY 60
#define AHT1X_RH_MEASUREMENT_DELAY 20
#define AHT1X_TOTAL_MEASUREMENT_DELAY 80
#define AHT1X_RESET_DURATION 20
#define AHT1X_READ 0xAC /* t and RH Calibrated Measurement 1010 1100 */
#define AHT1X_READ_DATA0 0x33
#define AHT1X_READ_DATA1 0x00
#define AHT1X_INIT 0xE1 /* Init: 1110 0001 */
#define AHT1X_INIT_DATA0 0x08
#define AHT1X_INIT_DATA1 0x00
#define AHT1X_RESET 0xBA /* Reset: 1011 1010 */

SnoutnetAHT10::SnoutnetAHT10(uint8_t saddr) {
	addr = saddr;
}

SnoutnetAHT10::~SnoutnetAHT10() {
}

bool SnoutnetAHT10::begin() {
	Wire.beginTransmission(addr);
	Wire.write(AHT1X_RESET);
	if( Wire.endTransmission() ) return false;

	delay(AHT1X_RESET_DURATION);

	Wire.beginTransmission(addr);
	Wire.write( AHT1X_INIT );
	Wire.write( AHT1X_INIT_DATA0 );
	Wire.write( AHT1X_INIT_DATA1 );
	Wire.endTransmission();

	// wait and check for status. We technically should also look
	// for the "calibrated" bit since that's the command we just sent.
	int tries = 5;
	do {
		delay(AHT1X_INIT_DELAY);
	} while( tries-- && getStatus()&0x80 );

	return tries ? true : false;
}

bool SnoutnetAHT10::getReading(double* ptt, double* prh) {
	float tt, rh;
	if( getReading(&tt,&rh) ) {
		if( ptt ) *ptt = tt;
		if( prh ) *prh = rh;
		return true;
	}
	return false;
}

uint8_t SnoutnetAHT10::getStatus() {
	Wire.requestFrom(addr,1);
	return Wire.read();
}

bool SnoutnetAHT10::getReading(float* tt, float* rh) {
	unsigned char readBuffer[AHT1X_DATA_SIZE];

	Wire.beginTransmission(addr);
	Wire.write( AHT1X_READ );
	Wire.write( AHT1X_READ_DATA0 );
	Wire.write( AHT1X_READ_DATA1 );
	Wire.endTransmission();

	long timeout = millis() + DEFAULT_TIMEOUT;

	// wait until it's not busy
	while( millis() < timeout && getStatus()&0x80 ) {
		delay(AHT1X_TOTAL_MEASUREMENT_DELAY / 4);
	}

	// grab the readings
	Wire.requestFrom((uint8_t)addr, (uint8_t)AHT1X_DATA_SIZE);
	while ( millis() < timeout) {
		if (Wire.available() < AHT1X_DATA_SIZE) {
			delay(AHT1X_TOTAL_MEASUREMENT_DELAY / 4);
		} else {
			for (int i = 0; i < AHT1X_DATA_SIZE; i++) {
				readBuffer[i] = Wire.read();
			}

			if( rh ) {
				uint32_t xresult = (((uint32_t)readBuffer[1] << 16)
					| ((uint32_t)readBuffer[2] << 8)
					| (uint32_t)readBuffer[3]) >> 4;
				*rh = (float)xresult;
				*rh *= 100;
				*rh /= 1048576;
			}

			if( tt ) {
				uint32_t xresult = (((uint32_t)readBuffer[3] & 0x0F) << 16)
					| ((uint32_t)readBuffer[4] << 8) | (uint32_t)readBuffer[5];
				*tt = (float)xresult;
				*tt *= 200;
				*tt /= 1048576;
				*tt -= 50;
			}

			break;
		}
	}
	return true;
}
