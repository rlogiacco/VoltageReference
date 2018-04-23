#include <EEPROM.h>
#include <VoltageReference.h>

// sets the storage area to the very end of the EEPROM
#define VREF_EEPROM_ADDR (E2END - 2)

int address = VREF_EEPROM_ADDR;

/**
 * Loads the calibration value from EEPROM
 */
uint32_t load(uint16_t address) {
	uint32_t calibration = 0;
	byte msb = EEPROM.read(address);
	byte mid = EEPROM.read(address + 1);
	byte lsb = EEPROM.read(address + 2);
	calibration = (((long) msb) << 16) | ((mid << 8) | ((lsb & 0xFF) & 0xFFFF));
	if (calibration == 16777215L) {
		Serial.println("No calibration value stored into EEPROM, using default");
		calibration = DEFAULT_REFERENCE_CALIBRATION;
	} else {
		Serial.print(F("Read from EEPROM address "));
		Serial.print(address);
		Serial.print(F(" calibration value "));
		Serial.println(calibration);
	}
	return calibration;
}

/**
 * Saves the calibration value into EEPROM
 */
void save(uint16_t address, uint32_t calibration) {
	EEPROM.write(address, calibration >> 16);
	EEPROM.write(address + 1, calibration >> 8);
	EEPROM.write(address + 2, calibration & 0xFF);
	Serial.println(F("Saved calibration value into EEPROM"));
}

/**
 * Clears the calibration value from EEPROM
 */
void clear(uint16_t address) {
	EEPROM.write(address, 0xFF);
	EEPROM.write(address + 1, 0xFF);
	EEPROM.write(address + 2, 0xFF);
	Serial.println(F("Cleared calibration value from EEPROM"));
}

void menu() {
	Serial.println(F("--- MENU ---"));
	Serial.println(F("\tR to read Vcc"));
	Serial.println(F("\tL to load calibration from EEPROM"));
	Serial.println(F("\tS to store calibration into EEPROM"));
	Serial.println(F("\tC to clear calibration from EEPROM"));
	Serial.println(F("\tdddd (4 digits) to calibrate for mV"));
	Serial.println(F("\tA to print EEPROM calibration start address (length 3)"));
	Serial.println(F("\tAdddd (4 digits) to set EEPROM calibration start address (length 3)"));
	Serial.println(F("\tH prints this help"));
}

VoltageReference vRef;

void setup() {
	Serial.begin(9600);
	while (!Serial);
	vRef.begin();
	menu();
}

void parse() {
	char c = Serial.read();
	if (isalpha(c)) {
		if (c == 's' || c == 'S') {
			save(address, vRef.calibrate(vRef.readVcc()));
		} else if (c == 'c' || c == 'C') {
			clear(address);
		} else if (c == 'l' || c == 'L') {
			vRef.begin(load(address));
		} else if (c == 'r' || c == 'R') {
			Serial.print(F("Calculated input voltage is "));
			Serial.print(vRef.readVcc());
			Serial.println(F("mV"));
			Serial.print(F("Real bandgap voltage is "));
			Serial.print(vRef.internalValue());
			Serial.println(F("mV"));
		}  else if (c == 'a' || c == 'A') {
			if (Serial.available() > 0) {
				c = Serial.read();
				char* buffer = new char[5];
				uint8_t i = 0;
				while (isdigit(c)) {
					buffer[i++] = c;
					c = Serial.read();
				}
				buffer[i++] = '\0';
				address = atol(buffer);
				Serial.print(F("Setting EEPROM start address to "));
				Serial.println(address);
			} else {
				Serial.print(F("EEPROM start address is "));
				Serial.println(address);
			}
		} else if (c == 'h' || c == 'H') {
			menu();
		}
	} else if(isdigit(c)){
		char* buffer = new char[5];
		uint8_t i = 0;
		while (isdigit(c)) {
			buffer[i++] = c;
			c = Serial.read();
		}
		buffer[i++] = '\0';
		long voltage = atol(buffer);
		Serial.print(F("Calibrating for Vcc "));
		Serial.print(voltage);
		Serial.println("mV");
		uint32_t calibration = vRef.calibrate(voltage);
		Serial.print(F("Calibration value is "));
		Serial.println(calibration);
		Serial.print(F("Real bandgap voltage is "));
		Serial.print((calibration + (ANALOG_MAX_VALUE / 2)) / ANALOG_MAX_VALUE);
		Serial.println(F("mV"));
		vRef.begin(calibration);
	}
}

void loop() {
	digitalWrite(13, HIGH);
	delay(1000);
	digitalWrite(13, LOW);
	delay(1000);
	if (Serial.available() > 0) {
		parse();
	}
}
