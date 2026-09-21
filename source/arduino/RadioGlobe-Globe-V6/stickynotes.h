// sticky notes
/*

/**
 * Stelt de afspeelsnelheid van de VS1053 nauwkeurig in via PPM.
 * Bereik: -100000 (10% langzamer) tot 100000 (10% sneller)
 */
void ESP32_VS1053_Stream:: setVs1053SpeedPPM(int32_t ppm) {
    // 1. Hardware-beveiliging: begrenzen op -10% (-100.000) tot +10% (+100.000) PPM
    //if (ppm > 100000)  ppm = 100000;
    //if (ppm < -100000) ppm = -100000;

    // 2. Lees de huidige basissnelheid van de stream (bijv. 44100 of 48000 Hz)
    uint16_t currentAuData = _vs1053->readRegister(0x05); // SCI_AUDATA
    uint16_t baseSampleRate = currentAuData & 0xFFFE;   // Haal de mono/stereo bit weg
    uint16_t monoBit = currentAuData & 0x0001;          // Bewaar de mono/stereo status

    // Veiligheidsklep: als de stream nog niet gestart is, ga uit van 44.1 kHz
    if (baseSampleRate < 8000) {
        baseSampleRate = 44100;
    }

baseSampleRate = 44100;

    // 3. Bereken de nieuwe sample rate met 64-bit precisie om overflow te voorkomen
    // Formule: Nieuwe_Rate = Basis_Rate * (1 + PPM / 1.000.000)
    int64_t offsetHz = ((int64_t)baseSampleRate * ppm) / 1000000;
    int32_t targetSampleRate = (int32_t)baseSampleRate + offsetHz;

    // 4. Extra beveiliging voor de hardware klokgrenzen van de VS1053 DAC
    if (targetSampleRate < 8000)  targetSampleRate = 8000;
    if (targetSampleRate > 50000) targetSampleRate = 50000;

    // 5. Voeg de mono/stereo bit weer toe en schrijf direct naar de klok
    uint16_t finalAuData = (uint16_t)targetSampleRate | monoBit;
   _vs1053->writeRegister(0x05, finalAuData); // Overschrijf SCI_AUDATA
}


21:02:57.159 -> TELL GLOBE: START_THIS_STATION 21 -> AU-https://tcom-s1.tcom.net.au/2ten
21:02:57.159 ->  -----------------  AddStationToQueueForGlobe() station for country <AU>
21:02:57.159 -> TELL GLOBE: GET_TIMEZONE_BY_GPS 27 -> 0 -29.057600 152.018997 101
21:02:57.192 -> TELL GLOBE: GET_GEOLOCATION_BY_GPS 40 -> 0 -29.057600 152.018997 101
21:02:57.192 -> ESPNOW esp_now_send() SERIALNR 509 -> START_THIS_STATION 21 sent to globe = >AU-https://tcom-s1.tcom.net.au/2ten<
21:02:57.192 -> Flag set early by AddStationToQueueForGlobe() = AU
21:02:57.192 -> WORLD FLAG AU SET FROM CACHE
21:02:57.192 -> Town = Tenterfield
21:02:57.192 -> Country = Australia
21:02:57.259 -> ESPNOW esp_now_send() SERIALNR 510 -> GET_TIMEZONE_BY_GPS 27 sent to globe = >0 -29.057600 152.018997 101<
21:02:57.259 -> ESPNOW esp_now_send() SERIALNR 511 -> GET_GEOLOCATION_BY_GPS 40 sent to globe = >0 -29.057600 152.018997 101<
21:02:57.259 -> Station 1 (country Australia )found in 184 mS
21:02:57.292 -> Station 2 (country Australia )found in 185 mS
21:02:57.292 -> 

    // part below added frank
    if (!stream->available())
            return;
    if (_metaDataStart && _musicDataPosition == _metaDataStart  && stream->available())
    {   
        const auto metaLen = stream->read() * 16;
        _bytesLeftInChunk--;

        if (metaLen)
        {
            size_t cnt = 0;

            while (cnt < metaLen)
            {

                if(stream->available())
                {
                _localbuffer[cnt++] = stream->read();
                _bytesLeftInChunk--;
                }
            }

            if (_infoCallback)
                _handleMetadata(reinterpret_cast<char *>(_localbuffer), metaLen);
        }

        _musicDataPosition = 0;
    }






own mac globe E0:5A:1B:E2:F0:68

BOM list Aliexpress

USB Power in connector, white 2x aliexpress.com/item/1005009014483994.html
OR ----> USB Power in connector, black 2x aliexpress.com/item/1005009014483994.html
VS1053B Sound module aliexpress.com/item/1005006307742844.html
female header connector 2x5 pins aliexpress.com/item/33004522737.htm
ESP32-S3-16RN8 with antenna connector aliexpress.com/item/1005008682657771.html
Mini Speaker 2x aliexpress.com/item/1005006119816805.html
Speaker grill 2x aliexpress.com/item/1005003827219821.html
Mini Amplifier 2x aliexpress.com/item/1005005611287848.html
Connector kit JST PH 2.0mm aliexpress.com/item/1005004658650664.html
Volume potentiometer 10K 2x aliexpress.com/item/1005004550780370.html
Led current driver aliexpress.com/item/1005005891873508.html
Encoder 2x aliexpress.com/item/1005007718766639.html
Pogo Connector aliexpress.com/item/1005007457425590.html
Waveshare 2.1 inch round touch display aliexpress.com/item/1005008679530480.html
Lithium battery 10x34x50mm 2500mAh aliexpress.com/item/1005009017357025.html
Bluetooth module aliexpress.com/item/1005006761214266.html
Black screw kit aliexpress.com/item/1005006143451558.html
Lead beads aliexpress.com/item/1005008258670040.html
4mm shaft aliexpress.com/item/1005005041338002.html
2mm shaft counter weight aliexpress.com/item/4000322946057.html
20mm lens aliexpress.com/item/32929649843.html

gedrag intern speaker aan/uit

& bij start - vertel puck volume/bass/treble/intspeaker status, vanuit eeprom (puck neemt dat over)
& bij start - haal status op uit eeprom
& loop - honoreer speaker on/off
& SetVolumeMapped() - bij volume 0 dan mute versterkers anders un-mute mits speakers enabled

todo
touchsensor - op stalen truss rand
portal -> mqtt server instelling
BT module aan/uit


*/