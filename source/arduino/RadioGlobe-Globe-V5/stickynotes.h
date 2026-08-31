// sticky notes
/*

/**
 * Stelt de afspeelsnelheid van de VS1053 nauwkeurig in via PPM.
 * Bereik: -100000 (10% langzamer) tot 100000 (10% sneller)
 */
void setVs1053SpeedPPM(int32_t ppm) {
    // 1. Hardware-beveiliging: zorg dat we niet buiten de -10% of +10% schieten
    if (ppm > 100000)  ppm = 100000;
    if (ppm < -100000) ppm = -100000;

    // 2. Richt de interne geheugenpointer van de VS1053 op het Finetune-adres
    player.writeRegister(SCI_WRAMADDR, 0x1E07);
    
    // 3. Wiskundig splitsen van het 32-bit getal in twee 16-bit delen:
    // Schuif de bovenste 16 bits naar rechts
    uint16_t highWord = (uint16_t)((ppm >> 16) & 0xFFFF); 
    // Filter alleen de onderste 16 bits eruit
    uint16_t lowWord  = (uint16_t)(ppm & 0xFFFF);          

    // 4. Stuur eerst het hoge deel, de VS1053 schuift dit intern door
    player.writeRegister(SCI_WRAM, highWord);
    
    // 5. Stuur direct daarna het lage deel. De chip voegt ze nu samen tot één 32-bit waarde.
    player.writeRegister(SCI_WRAM, lowWord);
}



// 1. Richt de interne geheugenpointer op het Finetune-adres
player.writeRegister(SCI_WRAMADDR, 0x1E07);

// 2. Schrijf de PPM-snelheidswaarde (Signed 16-bit)
// Voorbeeld: +20000 ppm verhoogt de snelheid direct met exact 2%
player.writeRegister(SCI_WRAM, 20000); 

Let op: Omdat het register via SCI_WRAM 16-bits breed is, gebruiken sommige oudere versies van de patch de volledige 16-bit range (-32768 tot 32767), 
wat je een maximaal bereik geeft van ca. ±3,2%. 
Nieuwere patch-versies combineren twee registers achter de schermen om tot de ±100.000 ppm (±10%) te komen.




#define SAMPLE_RATE_FINETUNE_ADDR 0x1E07 

void setPlaybackSpeedFinetune(int16_t speedModifier) {
    // 1. Tell the VS1053 which RAM address we want to change
    Mp3WriteRegister(SCI_WRAMADDR, SAMPLE_RATE_FINETUNE_ADDR);
    
    // 2. Write the speed modifier value to that address
    Mp3WriteRegister(SCI_WRAM, speedModifier);
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