// calibrate encoders at NS 0 and EW 0
void calibrate_globe(void)
{ int c, n;
  as5600_lat.setOffset(0);
  as5600_lon.setOffset(0);
  GlobeSettings.OffsetLat = (2048-as5600_lat.rawAngle());
  Serial.print("Offset Lat measured: ");
  Serial.println(GlobeSettings.OffsetLat);
  GlobeSettings.OffsetLon = (2048-as5600_lon.rawAngle());
  Serial.print("Offset Lon measured: ");
  Serial.println(GlobeSettings.OffsetLon);
  as5600_lat.setOffset(GlobeSettings.OffsetLat * AS5600_RAW_TO_DEGREES);
  as5600_lon.setOffset(GlobeSettings.OffsetLon * AS5600_RAW_TO_DEGREES);
  EEPROM.put(0x0, GlobeSettings);
  EEPROM.commit();
  Serial.println("Calibration Of The Globe done.");
}