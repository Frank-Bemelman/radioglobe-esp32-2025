// todo - check what country code belongs to lat/lon pair
// https://gist.github.com/inside-code-yt/7064d1d1553a2ee117e60217cfd1d099
// https://github.com/rapideditor/country-coder/blob/main/src/data/borders.json?short_path=7afbd5e
// https://developers.google.com/maps/documentation/geocoding/places-geocoding
// country valuta
// https://en.wikipedia.org/wiki/ISO_4217#List_of_ISO_4217_currency_codes

typedef struct country_info {
        char* code; // ISO 3166-1 two letter country code
        char* name; // country name
        char* valutaname; // valuta name i.e. Euro
        char* valutacode; // ISO 4217 three letter valuta code i.e. EUR
        int16_t  valutanumber; // ISO 4217-1 three digit code i.e. EUR
        float exchangerate;
    };


country_info CountryList[] =
{{ "AD", "Andorra", "Euro", "EUR",978, 0 },
 { "AE", "United Arab Emirates", "UAE Dirham", "AED", 784, 0 },
 { "AF", "Afghanistan", "Afghani", "AFN", 971, 0 },
 { "AG", "Antigua And Barbuda", "East Caribbean Dollar", "XCD",	951, 0 },
 { "AI", "Anguilla", "East Caribbean Dollar", "XCD", 951, 0 },
 { "AL", "Albania", "Lek", "ALL", 8, 0 },
 { "AM", "Armenia", "Armenian Dram", "AMD",	51, 0 },
 { "AN", "Netherlands Antilles", "Carribean Guilder", "XCG", 0, 0 },
 { "AO", "Angola", "Kwanza", "AOA", 973, 0 },
 { "AQ", "Antarctica", "Euro", "EUR", 978, 0 }, // why not
 { "AR", "Argentina", "Argentine Peso",	"ARS", 32, 0 },
 { "AS", "American Samoa", "US Dollar", "USD", 840, 0 },
 { "AT", "Austria", "Euro", "EUR", 978, 0 },
 { "AU", "Australia", "Australian Dollar", "AUD",	36, 0 },
 { "AW", "Aruba", "Aruban Florin", "AWG", 533, 0 },
 { "AX", "Aland Islands", "Euro", "EUR", 978, 0 },
 { "AZ", "Azerbaijan", "Azerbaijanian Manat", "AZN", 944, 0 },

 { "BA", "Bosnia And Herzegovina", "Convertible Mark", "BAM", 977, 0 },
 { "BB", "Barbados", "Barbados Dollar",	"BBD", 52, 0 },
 { "BD", "Bangladesh", "Taka", "BDT", 50, 0 },
 { "BE", "Belgium", "Euro", "EUR", 978, 0 },
 { "BF", "Burkina Faso", "CFA Franc", "XOF", 952, 0 },
 { "BG", "Bulgaria", "Bulgarian Lev", "BGN", 975, 0 },
 { "BH", "Bahrain", "Bahraini Dinar", "BHD", 48, 0 },
 { "BI", "Burundi", "Burundi Franc", "BIF",	108, 0 },
 { "BJ", "Benin", "CFA Franc", "XOF", 952, 0 },
 { "BL", "Saint Barthelemy", "Euro", "EUR", 978, 0 },
 { "BM", "Bermuda", "Bermudian Dollar",	"BMD", 60, 0 },
 { "BN", "Brunei Darussalam", "Brunei Dollar", "BND", 96, 0 },
 { "BO", "Bolivia", "Boliviano", "BOB",	68, 0 },
 { "BQ", "Dutch Caribbean", "", "", 0},
 { "BR", "Brazil", "Brazilian Real", "BRL",	986, 0 },
 { "BS", "Bahamas", "Bahamian Dollar", "BSD", 44, 0 },
 { "BT", "Bhutan", "Ngultrum", "BTN", 64, 0 },
 { "BV", "Bouvet Island", "Norwegian Krone", "NOK",	578, 0 },
 { "BW", "Botswana", "Pula", "BWP",	72, 0 },
 { "BY", "Belarus", "Belarussian Ruble", "BYN",	933, 0 },
 { "BZ", "Belize", "Belize Dollar",	"BZD", 84, 0 },

 { "CA", "Canada", "Canadian Dollar", "CAD", 124, 0 },
 { "CC", "Cocos (Keeling) Islands", "Australian Dollar", "AUD", 36, 0 },
 { "CD", "Congo, Democratic Republic", "Congolese Franc", "CDF",	976, 0 },
 { "CF", "Central African Republic", "CFA Franc", "XAF", 950, 0 },
 { "CG", "Congo", "CFA Franc", "XAF", 950, 0 },
 { "CH", "Switzerland", "Swiss Franc", "CHF", 756, 0 },
 { "CI", "Ivory Coast", "CFA Franc", "XAF", 950, 0 },
 { "CK", "Cook Islands", "New Zealand Dollar", "NZD", 554, 0 },
 { "CL", "Chile", "Chilean Peso", "CLP", 152, 0 },
 { "CM", "Cameroon", "CFA Franc", "XAF", 950, 0 },
 { "CN", "China", "Yuan Renminbi", "CNY", 156, 0 },
 { "CO", "Colombia", "Colombian Peso", "COP", 170, 0 },
 { "CR", "Costa Rica", "Costa Rican Colon", "CRC", 188, 0 },
 { "CU", "Cuba", "Cuban Peso", "CUP", 192, 0 },
 { "CV", "Cape Verde", "Cabo Verde Escudo", "CVE", 132, 0 },
 { "CW", "Curacao", "Caribbean Guilder", "XCG", 532, 0 },
 { "CX", "Christmas Island", "", "", 0, 0 },
 { "CY", "Cyprus", "Euro", "EUR", 978, 0 },
 { "CZ", "Czech Republic", "Czech Koruna", "CZK", 203, 0 },
 


 { "DE", "Germany", "Euro", "EUR", 978, 0 },
 { "DJ", "Djibouti", "Djibouti Franc", "DJF", 262, 0 },
 { "DK", "Denmark", "Danish Krone", "DKK", 208, 0 },
 { "DM", "Dominica", "East Caribbean Dollar", "XCD", 951, 0 },
 { "DO", "Dominican Republic", "Dominican Peso", "DOP",	214, 0 },
 { "DZ", "Algeria", "Algerian Dinar","DZD",	12, 0 },

 { "EC", "Ecuador", "US Dollar", "USD", 840, 0 },
 { "EE", "Estonia", "Euro", "EUR", 978, 0 },
 { "EG", "Egypt",  "Egyptian Pound", "EGP",	818, 0 },
 { "EH", "Western Sahara", "Moroccan Dirham", "MAD", 504, 0 },
 { "ER", "Eritrea", "Nakfa", "ERN", 232, 0 },
 { "ES", "Spain", "Euro", "EUR", 978, 0 },
 { "ET", "Ethiopia", "Ethiopian Birr", "ETB", 230, 0 },
 
 { "FI", "Finland", "Euro", "EUR", 978, 0 },
 { "FJ", "Fiji", "Fiji Dollar", "FJD", 242, 0 },
 { "FK", "Falkland Islands (Malvinas)", "Falkland Islands Pound", "FKP", 238, 0 },
 { "FM", "Micronesia, Federated States Of", "", "", 0, 0 },
 { "FO", "Faroe Islands", "Danish Krone", "DKK", 208, 0 },
 { "FR", "France", "Euro", "EUR", 978, 0 },

 { "GA", "Gabon", "CFA Franc", "XAF", 950, 0 },
 { "GB", "United Kingdom", "Pound Sterling", "GBP", 826, 0 },
 { "GD", "Grenada", "East Caribbean Dollar", "XCD", 951, 0 },
 { "GE", "Georgia", "Lari", "GEL", 981, 0 },
 { "GF", "French Guiana", "Euro", "EUR", 978, 0 },
 { "GG", "Guernsey", "Pound Sterling", "GBP", 826, 0 },
 { "GH", "Ghana", "Cedi", "GHS", 936, 0 },
 { "GI", "Gibraltar", "Gibraltar Pound", "GIP", 292, 0 },
 { "GL", "Greenland", "Danish Krone", "DKK", 208, 0 },
 { "GM", "Gambia", "Dalasi", "GMD",	270, 0 },
 { "GN", "Guinea", "Guinea Franc", "GNF", 324, 0 },
 { "GP", "Guadeloupe", "Euro", "EUR", 978, 0 },
 { "GQ", "Equatorial Guinea", "CFA Franc", "XAF", 950, 0 },
 { "GR", "Greece", "Euro", "EUR", 978, 0 },
 { "GS", "South Georgia And Sandwich Isl.", "Falkland Islands Pound", "FKP", 238, 0 },
 { "GT", "Guatemala", "Quetzal", "GTQ",	320, 0 },
 { "GU", "Guam", "US Dollar", "USD", 840, 0 },
 { "GW", "Guinea-Bissau", "CFA Franc", "XOF", 952, 0 },
 { "GY", "Guyana", "Guyana Dollar", "GYD", 328, 0 },

 { "HK", "Hong Kong", "Hong Kong Dollar", "HKD", 344, 0 },
 { "HM", "Heard Island & Mcdonald Islands", "", "", 0, 0 },
 { "HN", "Honduras", "Lempira", "HNL", 340, 0 },
 { "HR", "Croatia", "Euro", "EUR", 978, 0 },
 { "HT", "Haiti", "Gourde", "HTG", 332, 0 },
 { "HU", "Hungary", "Forint", "HUF", 348, 0 },

 { "ID", "Indonesia", "Rupiah", "IDR", 360, 0 },
 { "IE", "Ireland", "Euro", "EUR", 978, 0 },
 { "IL", "Israel", "Sheqel", "ILS",	376, 0 }, // "New Israeli Sheqel"
 { "IM", "Isle Of Man", "Pound Sterling", "GBP",	826, 0 },
 { "IN", "India", "Indian Rupee",	"INR", 356, 0 },
 { "IO", "British Indian Ocean Territory", "", "", 0, 0 },
 { "IQ", "Iraq", "Iraqi Dinar", "IQD", 368, 0 },
 { "IR", "Iran, Islamic Republic Of", "Iranian Rial", "IRR", 364, 0 },
 { "IS", "Iceland", "Iceland Krona", "ISK", 352, 0 },
 { "IT", "Italy", "Euro", "EUR", 978, 0 },

 { "JE", "Jersey", "Pound Sterling", "GBP",	826, 0 },
 { "JM", "Jamaica", "Jamaican Dollar", "JMD", 388, 0 },
 { "JO", "Jordan", "Jordanian Dinar", "JOD", 400, 0 },
 { "JP", "Japan", "Yen", "JPY",	392, 0 },
 
 { "KE", "Kenya", "Kenyan Shilling", "KES",	404, 0 },
 { "KG", "Kyrgyzstan", "Som", "KGS", 417, 0 },
 { "KH", "Cambodia", "Riel", "KHR",	116, 0 },
 { "KI", "Kiribati", "Australian Dollar", "AUD", 36, 0 },
 { "KM", "Comoros", "Comoro Franc", "KMF", 174, 0 },
 { "KN", "Saint Kitts And Nevis", "East Caribbean Dollar", "XCD", 951, 0 },
 { "KP", "North Korea", "North Korean Won", "KPW", 408, 0 },
 { "KR", "South Korea", "Won", "KRW", 410, 0 },
 { "KW", "Kuwait", "Kuwaiti Dinar", "KWD", 414, 0 },
 { "KY", "Cayman Islands", "Cayman Islands Dollar", "KYD", 136, 0 },
 { "KZ", "Kazakhstan", "Tenge", "KZT", 398, 0 },

 { "LA", "Lao People\"s Democratic Republic", "", "", 0, 0 },
 { "LB", "Lebanon", "Lebanese Pound", "LBP", 422, 0 },
 { "LC", "Saint Lucia", "East Caribbean Dollar", "XCD", 951, 0 },
 { "LI", "Liechtenstein", "Swiss Franc", "CHF", 756, 0 },
 { "LK", "Sri Lanka", "Sri Lanka Rupee", "LKR", 144, 0 },
 { "LR", "Liberia", "Liberian Dollar", "LRD", 430, 0 },
 { "LS", "Lesotho", "Loti", "LSL", 426, 0 },
 { "LT", "Lithuania", "Euro", "EUR", 978, 0 },
 { "LU", "Luxembourg", "Euro", "EUR", 978, 0 },
 { "LV", "Latvia", "Euro", "EUR", 978, 0 },
 { "LY", "Libyan Arab Jamahiriya", "", "", 0, 0 },

 { "MA", "Morocco", "Moroccan Dirham", "MAD", 504, 0 },
 { "MC", "Monaco", "Euro", "EUR", 978, 0 },
 { "MD", "Moldova", "Moldovan Leu", "MDL", 498, 0 },
 { "ME", "Montenegro", "Euro", "EUR", 978, 0 },
 { "MF", "Saint Martin", "Euro", "EUR", 978, 0 },
 { "MG", "Madagascar", "Malagasy Ariary", "MGA", 969, 0 },
 { "MH", "Marshall Islands", "US Dollar", "USD", 0 },
 { "MK", "Macedonia", "Denar", "MKD", 807, 0 },
 { "ML", "Mali", "CFA Franc", "XOF", 952, 0 },
 { "MM", "Myanmar", "Kyat", "MMK", 104, 0 },
 { "MN", "Mongolia", "Tugrik", "MNT", 496, 0 },
 { "MO", "Macao", "Pataca", "MOP", 446, 0 },
 { "MP", "Northern Mariana Islands", "US Dollar", "USD", 840, 0 },
 { "MQ", "Martinique", "Euro", "EUR", 978, 0 },
 { "MR", "Mauritania", "Ouguiya", "MRU", 929, 0 },
 { "MS", "Montserrat", "East Caribbean Dollar", "XCD", 951, 0 },
 { "MT", "Malta", "Euro", "EUR", 978, 0 },
 { "MU", "Mauritius", "Mauritius Rupee", "MUR", 480, 0 },
 { "MV", "Maldives", "Rufiyaa", "MVR", 462, 0 },
 { "MW", "Malawi", "Kwacha", "MWK",	454, 0 },
 { "MX", "Mexico", "Mexican Peso", "MXN", 484, 0 },
 { "MY", "Malaysia", "Malaysian Ringgit", "MYR", 458, 0 },
 { "MZ", "Mozambique", "Mozambique Metical", "MZN", 943, 0 },

 { "NA", "Namibia", "Namibia Dollar", "NAD", 516, 0 },
 { "NC", "New Caledonia", "CFP Franc", "XPF", 953, 0 },
 { "NE", "Niger", "CFA Franc", "XOF", 952, 0 },
 { "NF", "Norfolk Island", "Australian Dollar", "AUD", 36, 0 },
 { "NG", "Nigeria", "Naira", "NGN", 566, 0 },
 { "NI", "Nicaragua", "Cordoba Oro", "NIO", 558, 0 },
 { "NL", "Netherlands", "Euro", "EUR", 978, 0 },
 { "NO", "Norway", "Norwegian Krone", "NOK", 578, 0 },
 { "NP", "Nepal", "Nepalese Rupee", "NPR", 524, 0 },
 { "NR", "Nauru", "Australian Dollar", "AUD", 36, 0 },
 { "NU", "Niue", "New Zealand Dollar", "NZD", 554, 0 },
 { "NZ", "New Zealand", "New Zealand Dollar", "NZD", 554, 0 },
 
 { "OM", "Oman", "Rial Omani", "OMR", 512, 0 },

 { "PA", "Panama", "Balboa", "PAB",	590, 0 },
 { "PE", "Peru", "Nuevo Sol", "PEN", 604, 0 },
 { "PF", "French Polynesia", "CFP Franc", "XPF", 953, 0 },
 { "PG", "Papua New Guinea", "Kina", "PGK", 598, 0 },
 { "PH", "Philippines", "Philippine Peso", "PHP", 608, 0 },
 { "PK", "Pakistan", "Pakistan Rupee", "PKR", 586, 0 },
 { "PL", "Poland", "Zloty", "PLN", 985, 0 },
 { "PM", "Saint Pierre And Miquelon", "", "", 0, 0 },
 { "PN", "Pitcairn", "New Zealand Dollar", "NZD", 554, 0 },
 { "PR", "Puerto Rico", "US Dollar", "USD", 840, 0 },
 { "PS", "Palestinian Territory, Occupied", "", "", 0, 0 },
 { "PT", "Portugal", "Euro", "EUR", 978, 0 },
 { "PW", "Palau", "US Dollar", "USD", 840, 0 },
 { "PY", "Paraguay", "Guarani", "PYG", 600, 0 },

 { "QA", "Qatar", "Qatari Rial", "QAR",	634, 0 },

 { "RE", "Reunion", "Euro", "EUR", 978, 0 },
 { "RO", "Romania", "Romanian Leu", "RON", 946, 0 },
 { "RS", "Serbia", "Serbian Dinar", "RSD", 941, 0 },
 { "RU", "Russian Federation", "Russian Ruble",	"RUB", 643, 0 },
 { "RW", "Rwanda", "Rwanda Franc", "RWF", 646, 0 },

 { "SA", "Saudi Arabia", "Saudi Riyal", "SAR", 682, 0 },
 { "SB", "Solomon Islands", "Solomon Islands Dollar", "SBD", 90, 0 },
 { "SC", "Seychelles", "Seychelles Rupee", "SCR", 690, 0 },
 { "SD", "Sudan", "Sudanese Pound",	"SDG", 938, 0 },
 { "SE", "Sweden", "Swedish Krona", "SEK", 752, 0 },
 { "SG", "Singapore", "Singapore Dollar", "SGD", 702, 0 },
 { "SH", "Saint Helena", "Saint Helena Pound", "SHP", 654, 0 },
 { "SI", "Slovenia", "Euro", "EUR", 978, 0 },
 { "SJ", "Svalbard And Jan Mayen", "", "", 0, 0 },
 { "SK", "Slovakia", "Euro", "EUR", 978, 0 },
 { "SL", "Sierra Leone", "Leone", "SLE", 925, 0 },
 { "SM", "San Marino", "Euro", "EUR", 978, 0 },
 { "SN", "Senegal", "CFA Franc", "XOF",	952, 0 },
 { "SO", "Somalia", "Somali Shilling", "SOS", 706, 0 },
 { "SR", "Suriname", "Surinam Dollar", "SRD", 968, 0 },
 { "SS", "South Sudan", "South Sudanese Pound", "SSP", 728, 0 },
 { "ST", "Sao Tome And Principe", "Dobra", "STN", 930, 0 },
 { "SV", "El Salvador", "El Salvador Colon", "SVC",	222, 0 },
 { "SX", "Sint Maarten", "US Dollar", "USD", 840, 0 },
 { "SY", "Syrian Arab Republic", "Syrian Pound", "SYP", 760, 0 },
 { "SZ", "Swaziland", "Lilangeni", "SZL", 748, 0 },

 { "TC", "Turks And Caicos Islands", "US Dollar", "USD", 840, 0 },
 { "TD", "Chad", "CFA Franc", "XOF", 952, 0 },
 { "TF", "French Southern Territories", "Euro", "EUR", 978, 0 },
 { "TG", "Togo", "CFA Franc", "XOF", 952, 0 }, 	
 { "TH", "Thailand", "Baht", "THB", 764, 0 },
 { "TJ", "Tajikistan", "Somoni", "TJS", 972, 0 },
 { "TK", "Tokelau", "New Zealand Dollar", "NZD", 554, 0 },
 { "TL", "Timor-Leste", "US Dollar", "USD",	840, 0 },
 { "TM", "Turkmenistan", "Turkmenistan New Manat", "TMT", 934, 0 },
 { "TN", "Tunisia", "Tunisian Dinar", "TND", 788, 0 },
 { "TO", "Tonga", "Pa’anga", "TOP",	776, 0 },
 { "TR", "Turkey", "Turkish Lira","TRY", 949, 0 },
 { "TT", "Trinidad And Tobago", "Trinidad and Tobago Dollar", "TTD", 780, 0 },
 { "TV", "Tuvalu", "Australian Dollar", "AUD", 36, 0 },
 { "TW", "Taiwan", "New Taiwan Dollar", "TWD", 901, 0 },
 { "TZ", "Tanzania", "Tanzanian Shilling", "TZS", 834, 0 },
 
 { "UA", "Ukraine", "Hryvnia", "UAH",	980, 0 },
 { "UG", "Uganda", "Uganda Shilling", "UGX", 800, 0 },
 { "UM", "United States Outlying Islands", "US Dollar",	"USD",	840, 0 },
 { "US", "United States", "US Dollar",	"USD",	840, 0 },
 { "UY", "Uruguay", "Peso Uruguayo", "UYU",	858, 0 },
 { "UZ", "Uzbekistan", "Uzbekistan Sum", "UZS",	860, 0 },

 { "VA", "Holy See (Vatican City State)", "Euro", "EUR", 978, 0 },
 { "VC", "Saint Vincent And Grenadines", "", "", 0, 0 },
 { "VE", "Venezuela", "Bolivar", "VEF", 937, 0 },
 { "VG", "Virgin Islands, British", "", "", 0, 0 },
 { "VI", "Virgin Islands, U.S.", "", "", 0, 0 },
 { "VN", "Vietnam", "Dong", "VND", 704, 0 },
 { "VU", "Vanuatu", "Vatu", "VUV", 548, 0 },

 { "WF", "Wallis And Futuna", "CFP Franc", "XPF", 953, 0 },
 { "WS", "Samoa", "Tala", "WST", 882, 0 },

 { "YE", "Yemen", "Yemeni Rial", "YER",	886, 0 },
 { "YT", "Mayotte", "Euro", "EUR", 978, 0 },

 { "ZA", "South Africa", "Rand", "ZAR",	710, 0 },
 { "ZM", "Zambia", "Zambian Kwacha", "ZMW",	967, 0 },
 { "ZW", "Zimbabwe", "Zimbabwe Dollar", "ZWL", 932, 0 },

 { "XX", "XX-???", "Sardines", "", 0, 1}
};


/*
found here https://www.iban.com/currency-codes

//AFGHANISTAN	Afghani	AFN	971
//ALBANIA	Lek	ALL	008
//ALGERIA	Algerian Dinar	DZD	012
//AMERICAN SAMOA	US Dollar	USD	840
//ANDORRA	Euro	EUR	978
//ANGOLA	Kwanza	AOA	973
//ANGUILLA	East Caribbean Dollar	XCD	951 (landcode AI)
  ANTARCTICA	No universal currency		
//ANTIGUA AND BARBUDA	East Caribbean Dollar	XCD	951
//ARGENTINA	Argentine Peso	ARS	032
//ARMENIA	Armenian Dram	AMD	051
//ARUBA	Aruban Florin	AWG	533
//AUSTRALIA	Australian Dollar	AUD	036
//AUSTRIA	Euro	EUR	978
//AZERBAIJAN	Azerbaijanian Manat	AZN	944
//BAHAMAS (THE)	Bahamian Dollar	BSD	044
//BAHRAIN	Bahraini Dinar	BHD	048
//BANGLADESH	Taka	BDT	050
//BARBADOS	Barbados Dollar	BBD	052
//BELARUS	Belarussian Ruble	BYN	933
//BELGIUM	Euro	EUR	978
//BELIZE	Belize Dollar	BZD	084
BENIN	CFA Franc BCEAO	XOF	952
//BERMUDA	Bermudian Dollar	BMD	060
//BHUTAN	Ngultrum	BTN	064
  BHUTAN	Indian Rupee	INR	356
//BOLIVIA (PLURINATIONAL STATE OF)	Boliviano	BOB	068
  BOLIVIA (PLURINATIONAL STATE OF)	Mvdol	BOV	984
BONAIRE, SINT EUSTATIUS AND SABA	US Dollar	USD	840
//BOSNIA AND HERZEGOVINA	Convertible Mark	BAM	977
//BOTSWANA	Pula	BWP	072
BOUVET ISLAND	Norwegian Krone	NOK	578
//BRAZIL	Brazilian Real	BRL	986
BRITISH INDIAN OCEAN TERRITORY (THE)	US Dollar	USD	840
//BRUNEI DARUSSALAM	Brunei Dollar	BND	096
//BULGARIA	Bulgarian Lev	BGN	975
BURKINA FASO	CFA Franc BCEAO	XOF	952
//BURUNDI	Burundi Franc	BIF	108
//CABO VERDE	Cabo Verde Escudo	CVE	132
//CAMBODIA	Riel	KHR	116
//CAMEROON	CFA Franc BEAC	XAF	950
//CANADA	Canadian Dollar	CAD	124
//CAYMAN ISLANDS (THE)	Cayman Islands Dollar	KYD	136
CENTRAL AFRICAN REPUBLIC (THE)	CFA Franc BEAC	XAF	950
CHAD	CFA Franc BEAC	XAF	950
  CHILE	Unidad de Fomento	CLF	990
//CHILE	Chilean Peso	CLP	152
//CHINA	Yuan Renminbi	CNY	156
CHRISTMAS ISLAND	Australian Dollar	AUD	036
//COCOS (KEELING) ISLANDS (THE)	Australian Dollar	AUD	036
//COLOMBIA	Colombian Peso	COP	170
COLOMBIA	Unidad de Valor Real	COU	970
//COMOROS (THE)	Comoro Franc	KMF	174
//CONGO (THE DEMOCRATIC REPUBLIC OF THE)	Congolese Franc	CDF	976
  CONGO (THE)	CFA Franc BEAC	XAF	950
//COOK ISLANDS (THE)	New Zealand Dollar	NZD	554
//COSTA RICA	Costa Rican Colon	CRC	188
//CROATIA	Euro	EUR	978
  CUBA	Peso Convertible	CUC	931
//CUBA	Cuban Peso	CUP	192
//CURAÇAO	Caribbean Guilder	XCG	532
//CYPRUS	Euro	EUR	978
//CZECH REPUBLIC (THE)	Czech Koruna	CZK	203
CÔTE D'IVOIRE	CFA Franc BCEAO	XOF	952
//DENMARK	Danish Krone	DKK	208
//DJIBOUTI	Djibouti Franc	DJF	262
//DOMINICA	East Caribbean Dollar	XCD	951
//DOMINICAN REPUBLIC (THE)	Dominican Peso	DOP	214
ECUADOR	US Dollar	USD	840
//EGYPT	Egyptian Pound	EGP	818
//EL SALVADOR	El Salvador Colon	SVC	222
  EL SALVADOR	US Dollar	USD	840
EQUATORIAL GUINEA	CFA Franc BEAC	XAF	950
//ERITREA	Nakfa	ERN	232
//ESTONIA	Euro	EUR	978
//ETHIOPIA	Ethiopian Birr	ETB	230
EUROPEAN UNION	Euro	EUR	978
//FALKLAND ISLANDS (THE) [MALVINAS]	Falkland Islands Pound	FKP	238
//FAROE ISLANDS (THE)	Danish Krone	DKK	208
//FIJI	Fiji Dollar	FJD	242
FINLAND	Euro	EUR	978
FRANCE	Euro	EUR	978
FRENCH GUIANA	Euro	EUR	978
//FRENCH POLYNESIA	CFP Franc	XPF	953
FRENCH SOUTHERN TERRITORIES (THE)	Euro	EUR	978
//GABON	CFA Franc BEAC	XAF	950
//GAMBIA (THE)	Dalasi	GMD	270
//GEORGIA	Lari	GEL	981
GERMANY	Euro	EUR	978
//GHANA	Ghana Cedi	GHS	936
//GIBRALTAR	Gibraltar Pound	GIP	292
//GREECE	Euro	EUR	978
//GREENLAND	Danish Krone	DKK	208
//GRENADA	East Caribbean Dollar	XCD	951
GUADELOUPE	Euro	EUR	978
//GUAM	US Dollar	USD	840
//GUATEMALA	Quetzal	GTQ	320
//GUERNSEY	Pound Sterling	GBP	826
//GUINEA	Guinea Franc	GNF	324
//GUINEA-BISSAU	CFA Franc BCEAO	XOF	952
//GUYANA	Guyana Dollar	GYD	328
//HAITI	Gourde	HTG	332
HAITI	US Dollar	USD	840
HEARD ISLAND AND McDONALD ISLANDS	Australian Dollar	AUD	036
HOLY SEE (THE)	Euro	EUR	978
//HONDURAS	Lempira	HNL	340
//HONG KONG	Hong Kong Dollar	HKD	344
//HUNGARY	Forint	HUF	348
//ICELAND	Iceland Krona	ISK	352
//INDIA	Indian Rupee	INR	356
//INDONESIA	Rupiah	IDR	360
INTERNATIONAL MONETARY FUND (IMF) 	SDR (Special Drawing Right)	XDR	960
//IRAN (ISLAMIC REPUBLIC OF)	Iranian Rial	IRR	364
//IRAQ	Iraqi Dinar	IQD	368
//IRELAND	Euro	EUR	978
//ISLE OF MAN	Pound Sterling	GBP	826
//ISRAEL	New Israeli Sheqel	ILS	376
  ITALY	Euro	EUR	978
//JAMAICA	Jamaican Dollar	JMD	388
//JAPAN	Yen	JPY	392
//JERSEY	Pound Sterling	GBP	826
//JORDAN	Jordanian Dinar	JOD	400
//KAZAKHSTAN	Tenge	KZT	398
//KENYA	Kenyan Shilling	KES	404
//KIRIBATI	Australian Dollar	AUD	036
//KOREA (THE DEMOCRATIC PEOPLE’S REPUBLIC OF)	North Korean Won	KPW	408
//KOREA (THE REPUBLIC OF SOUTH)	Won	KRW	410
//KUWAIT	Kuwaiti Dinar	KWD	414
//KYRGYZSTAN	Som	KGS	417
LAO PEOPLE’S DEMOCRATIC REPUBLIC (THE)	Kip	LAK	418
LATVIA	Euro	EUR	978
//LEBANON	Lebanese Pound	LBP	422
//LESOTHO	Loti	LSL	426
  LESOTHO	Rand	ZAR	710
//LIBERIA	Liberian Dollar	LRD	430
LIBYA	Libyan Dinar	LYD	434
//LIECHTENSTEIN	Swiss Franc	CHF	756
LITHUANIA	Euro	EUR	978
LUXEMBOURG	Euro	EUR	978
//MACAO	Pataca	MOP	446
//MADAGASCAR	Malagasy Ariary	MGA	969
//MALAWI	Kwacha	MWK	454
//MALAYSIA	Malaysian Ringgit	MYR	458
//MALDIVES	Rufiyaa	MVR	462
//MALI	CFA Franc BCEAO	XOF	952
//MALTA	Euro	EUR	978
//MARSHALL ISLANDS (THE)	US Dollar	USD	840
//MARTINIQUE	Euro	EUR	978
//MAURITANIA	Ouguiya	MRU	929
//MAURITIUS	Mauritius Rupee	MUR	480
MAYOTTE	Euro	EUR	978
MEMBER COUNTRIES OF THE AFRICAN DEVELOPMENT BANK GROUP	ADB Unit of Account	XUA	965
//MEXICO	Mexican Peso	MXN	484
MEXICO	Mexican Unidad de Inversion (UDI)	MXV	979
MICRONESIA (FEDERATED STATES OF)	US Dollar	USD	840
//MOLDOVA (THE REPUBLIC OF)	Moldovan Leu	MDL	498
//MONACO	Euro	EUR	978
//MONGOLIA	Tugrik	MNT	496
MONTENEGRO	Euro	EUR	978
//MONTSERRAT	East Caribbean Dollar	XCD	951
//MOROCCO	Moroccan Dirham	MAD	504
//MOZAMBIQUE	Mozambique Metical	MZN	943
//MYANMAR	Kyat	MMK	104
//NAMIBIA	Namibia Dollar	NAD	516
  NAMIBIA	Rand	ZAR	710
//NAURU	Australian Dollar	AUD	036
//NEPAL	Nepalese Rupee	NPR	524
//NETHERLANDS (THE)	Euro	EUR	978
//NEW CALEDONIA	CFP Franc	XPF	953
//NEW ZEALAND	New Zealand Dollar	NZD	554
//NICARAGUA	Cordoba Oro	NIO	558
//NIGER (THE)	CFA Franc BCEAO	XOF	952
//NIGERIA	Naira	NGN	566
//NIUE	New Zealand Dollar	NZD	554
NORFOLK ISLAND	Australian Dollar	AUD	036
NORTHERN MARIANA ISLANDS (THE)	US Dollar	USD	840
//NORWAY	Norwegian Krone	NOK	578
//OMAN	Rial Omani	OMR	512
//PAKISTAN	Pakistan Rupee	PKR	586
//PALAU	US Dollar	USD	840
PALESTINE, STATE OF	No universal currency		
//PANAMA	Balboa	PAB	590
  PANAMA	US Dollar	USD	840
//PAPUA NEW GUINEA	Kina	PGK	598
//PARAGUAY	Guarani	PYG	600
//PERU	Nuevo Sol	PEN	604
//PHILIPPINES (THE)	Philippine Peso	PHP	608
//PITCAIRN	New Zealand Dollar	NZD	554
//POLAND	Zloty	PLN	985
PORTUGAL	Euro	EUR	978
PUERTO RICO	US Dollar	USD	840
//QATAR	Qatari Rial	QAR	634
//REPUBLIC OF NORTH MACEDONIA	Denar	MKD	807
//ROMANIA	Romanian Leu	RON	946
//RUSSIAN FEDERATION (THE)	Russian Ruble	RUB	643
//RWANDA	Rwanda Franc	RWF	646
RÉUNION	Euro	EUR	978
SAINT BARTHÉLEMY	Euro	EUR	978
//SAINT HELENA, ASCENSION AND TRISTAN DA CUNHA	Saint Helena Pound	SHP	654
//SAINT KITTS AND NEVIS	East Caribbean Dollar	XCD	951
//SAINT LUCIA	East Caribbean Dollar	XCD	951
  SAINT MARTIN (FRENCH PART)	Euro	EUR	978
SAINT PIERRE AND MIQUELON	Euro	EUR	978
SAINT VINCENT AND THE GRENADINES	East Caribbean Dollar	XCD	951
//SAMOA	Tala	WST	882
SAN MARINO	Euro	EUR	978
//SAO TOME AND PRINCIPE	Dobra	STN	930
//SAUDI ARABIA	Saudi Riyal	SAR	682
//SENEGAL	CFA Franc BCEAO	XOF	952
//SERBIA	Serbian Dinar	RSD	941
//SEYCHELLES	Seychelles Rupee	SCR	690
//SIERRA LEONE	Leone	SLE	925
//SINGAPORE	Singapore Dollar	SGD	702
SINT MAARTEN (DUTCH PART)	Caribbean guilder	XCG	532
SISTEMA UNITARIO DE COMPENSACION REGIONAL DE PAGOS "SUCRE"	Sucre	XSU	994
//SLOVAKIA	Euro	EUR	978
SLOVENIA	Euro	EUR	978
//SOLOMON ISLANDS	Solomon Islands Dollar	SBD	090
//SOMALIA	Somali Shilling	SOS	706
//SOUTH AFRICA	Rand	ZAR	710
SOUTH GEORGIA AND THE SOUTH SANDWICH ISLANDS	No universal currency		
//SOUTH SUDAN	South Sudanese Pound	SSP	728
SPAIN	Euro	EUR	978
//SRI LANKA	Sri Lanka Rupee	LKR	144
SUDAN (THE)	Sudanese Pound	SDG	938
//SURINAME	Surinam Dollar	SRD	968
SVALBARD AND JAN MAYEN	Norwegian Krone	NOK	578
//SWAZILAND	Lilangeni	SZL	748
//SWEDEN	Swedish Krona	SEK	752
  SWITZERLAND	WIR Euro	CHE	947
//SWITZERLAND	Swiss Franc	CHF	756
  SWITZERLAND	WIR Franc	CHW	948
//SYRIAN ARAB REPUBLIC	Syrian Pound	SYP	760
//TAIWAN (PROVINCE OF CHINA)	New Taiwan Dollar	TWD	901
//TAJIKISTAN	Somoni	TJS	972
//TANZANIA, UNITED REPUBLIC OF	Tanzanian Shilling	TZS	834
//THAILAND	Baht	THB	764
//TIMOR-LESTE	US Dollar	USD	840
//TOGO	CFA Franc BCEAO	XOF	952
//TOKELAU	New Zealand Dollar	NZD	554
//TONGA	Pa’anga	TOP	776
//TRINIDAD AND TOBAGO	Trinidad and Tobago Dollar	TTD	780
//TUNISIA	Tunisian Dinar	TND	788
//TURKEY	Turkish Lira	TRY	949
//TURKMENISTAN	Turkmenistan New Manat	TMT	934
  TURKS AND CAICOS ISLANDS (THE)	US Dollar	USD	840
//TUVALU	Australian Dollar	AUD	036
//UGANDA	Uganda Shilling	UGX	800
//UKRAINE	Hryvnia	UAH	980
//UNITED ARAB EMIRATES (THE)	UAE Dirham	AED	784
//UNITED KINGDOM OF GREAT BRITAIN AND NORTHERN IRELAND (THE)	Pound Sterling	GBP	826
  UNITED STATES MINOR OUTLYING ISLANDS (THE)	US Dollar	USD	840
  UNITED STATES OF AMERICA (THE)	US Dollar	USD	840
  URUGUAY	Uruguay Peso en Unidades Indexadas (URUIURUI)	UYI	940
//URUGUAY	Peso Uruguayo	UYU	858
//UZBEKISTAN	Uzbekistan Sum	UZS	860
//VANUATU	Vatu	VUV	548
//VENEZUELA (BOLIVARIAN REPUBLIC OF)	Bolivar	VEF	937
//VIETNAM	Dong	VND	704
  VIRGIN ISLANDS (BRITISH)	US Dollar	USD	840
  VIRGIN ISLANDS (U.S.)	US Dollar	USD	840
//WALLIS AND FUTUNA	CFP Franc	XPF	953
//WESTERN SAHARA	Moroccan Dirham	MAD	504
//YEMEN	Yemeni Rial	YER	886
//ZAMBIA	Zambian Kwacha	ZMW	967
//ZIMBABWE	Zimbabwe Dollar	ZWL	932
  ÅLAND ISLANDS	Euro	EUR	978

*/
