// calibrate encoders


void calibrate_globe(void)
{ int c, n;
  as5600_0.setOffset(0);
  as5600_1.setOffset(0);
  GlobeSettings.Offset0 = (2048-as5600_0.rawAngle());
  Serial.print("Offset measured 0: ");
  Serial.println(GlobeSettings.Offset0);
  GlobeSettings.Offset1 = (2048-as5600_1.rawAngle());
  Serial.print("Offset measured 1: ");
  Serial.println(GlobeSettings.Offset1);
  as5600_0.setOffset(GlobeSettings.Offset0 * AS5600_RAW_TO_DEGREES);
  as5600_1.setOffset(GlobeSettings.Offset1 * AS5600_RAW_TO_DEGREES);
  EEPROM.put(0x0, GlobeSettings);
  EEPROM.commit();
  Serial.println("Calibration Of The Globe done.");
}