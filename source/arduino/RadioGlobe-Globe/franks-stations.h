// just some random stations urls for testing 


struct station_data {
    int ns;
    int ew;
    char* url;
};

const station_data entrys[] PROGMEM = {
  4581, 2012, "https://streaming.karolina.rs/karolina.mp3",
  5704,    992, "http://live-icy.gss.dr.dk:8000/A/A10L.mp3",
  5226,    476, "https://icecast.omroep.nl/radio1-sb-mp3",
  3113,  -8342, "https://playerservices.streamtheworld.com/api/livestream-redirect/RAC_1.mp3",
  1632,  12035, "https://vocenustrale.ice.infomaniak.ch/vocenustrale-128.mp3",
  4191,    873, "https://altacanzonacorsa.ice.infomaniak.ch/altacanzonacorsa-128.mp3",
  0,         0, "http://stream.srg-ssr.ch/m/rsj/mp3_128",
  0,0, ""
};


