#include <VoltageReference.h>

VoltageReference vRef;

void setup() {
	Serial.begin(9600);
	while (!Serial);
	Serial.println("Calibrating voltage reference");
	vRef.begin();
}


#define DETECT_PIN A0

// set this to true for voltages below 1V
#define LOW_VOLTAGE false

void loop() {

#if (LOW_VOLTAGE)
	analogReference(INTERNAL);
#endif
	int analog = analogRead(DETECT_PIN);
	int vcc = vRef.readVcc();
	Serial.print("Board voltage is ");
	Serial.print(vcc);
	Serial.print("mV, analog pin voltage is ");
#if (LOW_VOLTAGE)
	Serial.print(vRef.internalValue() * analog / 1023);
#else
	Serial.print(vcc * analog / 1023);
#endif
	Serial.print("mV");
	delay(50);
}
