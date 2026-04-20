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
        int16_t valutanumber; // ISO 4217-1 three digit code i.e. EUR is 978
    };


country_info CountryList[] =
{{ "AD", "Andorra", "", "", 0 },
 { "AE", "United Arab Emirates", "", "", 0 },
 { "AF", "Afghanistan", "", "", 0 },
 { "AG", "Antigua And Barbuda", "", "", 0 },
 { "AI", "Anguilla", "", "", 0 },
 { "AL", "Albania", "", "", 0 },
 { "AM", "Armenia", "", "", 0 },
 { "AN", "Netherlands Antilles", "", "", 0 },
 { "AO", "Angola", "", "", 0 },
 { "AQ", "Antarctica", "", "", 0 },
 { "AR", "Argentina", "", "", 0 },
 { "AS", "American Samoa", "", "", 0 },
 { "AT", "Austria", "", "", 0 },
 { "AU", "Australia", "", "", 0 },
 { "AW", "Aruba", "", "", 0 },
 { "AX", "Aland Islands", "", "", 0 },
 { "AZ", "Azerbaijan", "", "", 0 },

 { "BA", "Bosnia And Herzegovina", "", "", 0 },
 { "BB", "Barbados", "", "", 0 },
 { "BD", "Bangladesh", "", "", 0 },
 { "BE", "Belgium", "", "", 0 },
 { "BF", "Burkina Faso", "", "", 0 },
 { "BG", "Bulgaria", "", "", 0 },
 { "BH", "Bahrain", "", "", 0 },
 { "BI", "Burundi", "", "", 0 },
 { "BJ", "Benin", "", "", 0 },
 { "BL", "Saint Barthelemy", "", "", 0 },
 { "BM", "Bermuda", "", "", 0 },
 { "BN", "Brunei Darussalam", "", "", 0 },
 { "BO", "Bolivia", "", "", 0 },
 { "BQ", "Dutch Caribbean"},
 { "BR", "Brazil", "", "", 0 },
 { "BS", "Bahamas", "", "", 0 },
 { "BT", "Bhutan", "", "", 0 },
 { "BV", "Bouvet Island", "", "", 0 },
 { "BW", "Botswana", "", "", 0 },
 { "BY", "Belarus", "", "", 0 },
 { "BZ", "Belize", "", "", 0 },

 { "CA", "Canada", "", "", 0 },
 { "CC", "Cocos (Keeling) Islands", "", "", 0 },
 { "CD", "Congo, Democratic Republic", "", "", 0 },
 { "CF", "Central African Republic", "", "", 0 },
 { "CG", "Congo", "", "", 0 },
 { "CH", "Switzerland", "", "", 0 },
 { "CI", "Ivory Coast", "", "", 0 },
 { "CK", "Cook Islands", "", "", 0 },
 { "CL", "Chile", "", "", 0 },
 { "CM", "Cameroon", "", "", 0 },
 { "CN", "China", "", "", 0 },
 { "CO", "Colombia", "", "", 0 },
 { "CR", "Costa Rica", "", "", 0 },
 { "CU", "Cuba", "", "", 0 },
 { "CV", "Cape Verde", "", "", 0 },
 { "CW", "Curacao", "", "", 0 },
 { "CX", "Christmas Island", "", "", 0 },
 { "CY", "Cyprus", "", "", 0 },
 { "CZ", "Czech Republic", "", "", 0 },
 


 { "DE", "Germany", "", "", 0 },
 { "DJ", "Djibouti", "", "", 0 },
 { "DK", "Denmark", "", "", 0 },
 { "DM", "Dominica", "", "", 0 },
 { "DO", "Dominican Republic", "", "", 0 },
 { "DZ", "Algeria", "", "", 0 },

 { "EC", "Ecuador", "", "", 0 },
 { "EE", "Estonia", "", "", 0 },
 { "EG", "Egypt", "", "", 0 },
 { "EH", "Western Sahara", "", "", 0 },
 { "ER", "Eritrea", "", "", 0 },
 { "ES", "Spain", "", "", 0 },
 { "ET", "Ethiopia", "", "", 0 },
 
 { "FI", "Finland", "", "", 0 },
 { "FJ", "Fiji", "", "", 0 },
 { "FK", "Falkland Islands (Malvinas)", "", "", 0 },
 { "FM", "Micronesia, Federated States Of", "", "", 0 },
 { "FO", "Faroe Islands", "", "", 0 },
 { "FR", "France", "", "", 0 },

 { "GA", "Gabon", "", "", 0 },
 { "GB","United Kingdom", "Pound Sterling", "GBP", 826 },
 { "GD", "Grenada", "", "", 0 },
 { "GE", "Georgia", "", "", 0 },
 { "GF", "French Guiana", "", "", 0 },
 { "GG", "Guernsey", "", "", 0 },
 { "GH", "Ghana", "", "", 0 },
 { "GI", "Gibraltar", "", "", 0 },
 { "GL", "Greenland", "", "", 0 },
 { "GM", "Gambia", "", "", 0 },
 { "GN", "Guinea", "", "", 0 },
 { "GP", "Guadeloupe", "", "", 0 },
 { "GQ", "Equatorial Guinea", "", "", 0 },
 { "GR", "Greece", "", "", 0 },
 { "GS", "South Georgia And Sandwich Isl.", "", "", 0 },
 { "GT", "Guatemala", "", "", 0 },
 { "GU", "Guam", "", "", 0 },
 { "GW", "Guinea-Bissau", "", "", 0 },
 { "GY", "Guyana", "", "", 0 },

 { "HK", "Hong Kong", "", "", 0 },
 { "HM", "Heard Island & Mcdonald Islands", "", "", 0 },
 { "HN", "Honduras", "", "", 0 },
 { "HR", "Croatia", "", "", 0 },
 { "HT", "Haiti", "", "", 0 },
 { "HU", "Hungary", "", "", 0 },

 { "ID", "Indonesia", "", "", 0 },
 { "IE", "Ireland", "", "", 0 },
 { "IL", "Israel", "", "", 0 },
 { "IM", "Isle Of Man", "", "", 0 },
 { "IN", "India", "Rupee", "INR", 356 },
 { "IO", "British Indian Ocean Territory", "", "", 0 },
 { "IQ", "Iraq", "", "", 0 },
 { "IR", "Iran, Islamic Republic Of", "", "", 0 },
 { "IS", "Iceland", "", "", 0 },
 { "IT", "Italy", "", "", 0 },

 { "JE", "Jersey", "", "", 0 },
 { "JM", "Jamaica", "", "", 0 },
 { "JO", "Jordan", "", "", 0 },
 { "JP", "Japan", "", "", 0 },
 
 { "KE", "Kenya", "", "", 0 },
 { "KG", "Kyrgyzstan", "", "", 0 },
 { "KH", "Cambodia", "", "", 0 },
 { "KI", "Kiribati", "", "", 0 },
 { "KM", "Comoros", "", "", 0 },
 { "KN", "Saint Kitts And Nevis", "", "", 0 },
 { "KP", "North Korea", "", "", 0 },
 { "KR", "South Korea", "", "", 0 },
 { "KW", "Kuwait", "", "", 0 },
 { "KY", "Cayman Islands", "", "", 0 },
 { "KZ", "Kazakhstan", "", "", 0 },
 
 { "LA", "Lao People\"s Democratic Republic", "", "", 0 },
 { "LB", "Lebanon", "", "", 0 },
 { "LC", "Saint Lucia", "", "", 0 },
 { "LI", "Liechtenstein", "", "", 0 },
 { "LK", "Sri Lanka", "", "", 0 },
 { "LR", "Liberia", "", "", 0 },
 { "LS", "Lesotho", "", "", 0 },
 { "LT", "Lithuania", "", "", 0 },
 { "LU", "Luxembourg", "", "", 0 },
 { "LV", "Latvia", "", "", 0 },
 { "LY", "Libyan Arab Jamahiriya", "", "", 0 },

 { "MA", "Morocco", "", "", 0 },
 { "MC", "Monaco", "", "", 0 },
 { "MD", "Moldova", "", "", 0 },
 { "ME", "Montenegro", "", "", 0 },
 { "MF", "Saint Martin", "", "", 0 },
 { "MG", "Madagascar", "", "", 0 },
 { "MH", "Marshall Islands", "", "", 0 },
 { "MK", "Macedonia", "", "", 0 },
 { "ML", "Mali", "", "", 0 },
 { "MM", "Myanmar", "", "", 0 },
 { "MN", "Mongolia", "", "", 0 },
 { "MO", "Macao", "", "", 0 },
 { "MP", "Northern Mariana Islands", "", "", 0 },
 { "MQ", "Martinique", "", "", 0 },
 { "MR", "Mauritania", "", "", 0 },
 { "MS", "Montserrat", "", "", 0 },
 { "MT", "Malta", "", "", 0 },
 { "MU", "Mauritius", "", "", 0 },
 { "MV", "Maldives", "", "", 0 },
 { "MW", "Malawi", "", "", 0 },
 { "MX", "Mexico", "", "", 0 },
 { "MY", "Malaysia", "", "", 0 },
 { "MZ", "Mozambique", "", "", 0 },

 { "NA", "Namibia", "", "", 0 },
 { "NC", "New Caledonia", "", "", 0 },
 { "NE", "Niger", "", "", 0 },
 { "NF", "Norfolk Island", "", "", 0 },
 { "NG", "Nigeria", "", "", 0 },
 { "NI", "Nicaragua", "", "", 0 },
 { "NL", "Netherlands", "Euro", "EUR",978 },
 { "NO", "Norway", "", "", 0 },
 { "NP", "Nepal", "", "", 0 },
 { "NR", "Nauru", "", "", 0 },
 { "NU", "Niue", "", "", 0 },
 { "NZ", "New Zealand", "", "", 0 },
 
 { "OM", "Oman", "", "", 0 },

 { "PA", "Panama", "", "", 0 },
 { "PE", "Peru", "", "", 0 },
 { "PF", "French Polynesia", "", "", 0 },
 { "PG", "Papua New Guinea", "", "", 0 },
 { "PH", "Philippines", "", "", 0 },
 { "PK", "Pakistan", "", "", 0 },
 { "PL", "Poland", "", "", 0 },
 { "PM", "Saint Pierre And Miquelon", "", "", 0 },
 { "PN", "Pitcairn", "", "", 0 },
 { "PR", "Puerto Rico", "", "", 0 },
 { "PS", "Palestinian Territory, Occupied", "", "", 0 },
 { "PT", "Portugal", "", "", 0 },
 { "PW", "Palau", "", "", 0 },
 { "PY", "Paraguay", "", "", 0 },

 { "QA", "Qatar", "", "", 0 },

 { "RE", "Reunion", "", "", 0 },
 { "RO", "Romania", "", "", 0 },
 { "RS", "Serbia", "", "", 0 },
 { "RU", "Russia", "", "", 0 },
 { "RW", "Rwanda", "", "", 0 },

 { "SA", "Saudi Arabia", "", "", 0 },
 { "SB", "Solomon Islands", "", "", 0 },
 { "SC", "Seychelles", "", "", 0 },
 { "SD", "Sudan", "", "", 0 },
 { "SE", "Sweden", "", "", 0 },
 { "SG", "Singapore", "", "", 0 },
 { "SH", "Saint Helena", "", "", 0 },
 { "SI", "Slovenia", "", "", 0 },
 { "SJ", "Svalbard And Jan Mayen", "", "", 0 },
 { "SK", "Slovakia", "", "", 0 },
 { "SL", "Sierra Leone", "", "", 0 },
 { "SM", "San Marino", "", "", 0 },
 { "SN", "Senegal", "", "", 0 },
 { "SO", "Somalia", "", "", 0 },
 { "SR", "Suriname", "", "", 0 },
 { "SS", "South Sudan", "", "", 0 },
 { "ST", "Sao Tome And Principe", "", "", 0 },
 { "SV", "El Salvador", "", "", 0 },
 { "SX", "Sint Maarten", "", "", 0 },
 { "SY", "Syrian Arab Republic", "", "", 0 },
 { "SZ", "Swaziland", "", "", 0 },

 { "TC", "Turks And Caicos Islands", "", "", 0 },
 { "TD", "Chad", "", "", 0 },
 { "TF", "French Southern Territories", "", "", 0 },
 { "TG", "Togo", "", "", 0 },
 { "TH", "Thailand", "", "", 0 },
 { "TJ", "Tajikistan", "", "", 0 },
 { "TK", "Tokelau", "", "", 0 },
 { "TL", "Timor-Leste", "", "", 0 },
 { "TM", "Turkmenistan", "", "", 0 },
 { "TN", "Tunisia", "", "", 0 },
 { "TO", "Tonga", "", "", 0 },
 { "TR", "Turkey", "", "", 0 },
 { "TT", "Trinidad And Tobago", "", "", 0 },
 { "TV", "Tuvalu", "", "", 0 },
 { "TW", "Taiwan", "", "", 0 },
 { "TZ", "Tanzania", "", "", 0 },
 
 { "UA", "Ukraine", "", "", 0 },
 { "UG", "Uganda", "", "", 0 },
 { "UM", "United States Outlying Islands", "US Dollar",	"USD",	840 },
 { "US", "United States", "US Dollar",	"USD",	840 },
 { "UY", "Uruguay", "", "", 0 },
 { "UZ", "Uzbekistan", "", "", 0 },

 { "VA", "Holy See (Vatican City State)", "", "", 0 },
 { "VC", "Saint Vincent And Grenadines", "", "", 0 },
 { "VE", "Venezuela", "", "", 0 },
 { "VG", "Virgin Islands, British", "", "", 0 },
 { "VI", "Virgin Islands, U.S.", "", "", 0 },
 { "VN", "Vietnam", "", "", 0 },
 { "VU", "Vanuatu", "", "", 0 },

 { "WF", "Wallis And Futuna", "", "", 0 },
 { "WS", "Samoa", "", "", 0 },

 { "YE", "Yemen", "", "", 0 },
 { "YT", "Mayotte", "", "", 0 },

 { "ZA", "South Africa", "", "", 0 },
 { "ZM", "Zambia", "", "", 0 },
 { "ZW", "Zimbabwe", "", "", 0 },

 { "XK", "Kosovo", "", "", 0},
 { "XX", "At Sea", "", "", 0}
};


/*
found here https://www.iban.com/currency-codes

AFGHANISTAN	Afghani	AFN	971
ALBANIA	Lek	ALL	008
ALGERIA	Algerian Dinar	DZD	012
AMERICAN SAMOA	US Dollar	USD	840
ANDORRA	Euro	EUR	978
ANGOLA	Kwanza	AOA	973
ANGUILLA	East Caribbean Dollar	XCD	951
ANTARCTICA	No universal currency		
ANTIGUA AND BARBUDA	East Caribbean Dollar	XCD	951
ARGENTINA	Argentine Peso	ARS	032
ARMENIA	Armenian Dram	AMD	051
ARUBA	Aruban Florin	AWG	533
AUSTRALIA	Australian Dollar	AUD	036
AUSTRIA	Euro	EUR	978
AZERBAIJAN	Azerbaijanian Manat	AZN	944
BAHAMAS (THE)	Bahamian Dollar	BSD	044
BAHRAIN	Bahraini Dinar	BHD	048
BANGLADESH	Taka	BDT	050
BARBADOS	Barbados Dollar	BBD	052
BELARUS	Belarussian Ruble	BYN	933
BELGIUM	Euro	EUR	978
BELIZE	Belize Dollar	BZD	084
BENIN	CFA Franc BCEAO	XOF	952
BERMUDA	Bermudian Dollar	BMD	060
BHUTAN	Ngultrum	BTN	064
BHUTAN	Indian Rupee	INR	356
BOLIVIA (PLURINATIONAL STATE OF)	Boliviano	BOB	068
BOLIVIA (PLURINATIONAL STATE OF)	Mvdol	BOV	984
BONAIRE, SINT EUSTATIUS AND SABA	US Dollar	USD	840
BOSNIA AND HERZEGOVINA	Convertible Mark	BAM	977
BOTSWANA	Pula	BWP	072
BOUVET ISLAND	Norwegian Krone	NOK	578
BRAZIL	Brazilian Real	BRL	986
BRITISH INDIAN OCEAN TERRITORY (THE)	US Dollar	USD	840
BRUNEI DARUSSALAM	Brunei Dollar	BND	096
BULGARIA	Bulgarian Lev	BGN	975
BURKINA FASO	CFA Franc BCEAO	XOF	952
BURUNDI	Burundi Franc	BIF	108
CABO VERDE	Cabo Verde Escudo	CVE	132
CAMBODIA	Riel	KHR	116
CAMEROON	CFA Franc BEAC	XAF	950
CANADA	Canadian Dollar	CAD	124
CAYMAN ISLANDS (THE)	Cayman Islands Dollar	KYD	136
CENTRAL AFRICAN REPUBLIC (THE)	CFA Franc BEAC	XAF	950
CHAD	CFA Franc BEAC	XAF	950
CHILE	Unidad de Fomento	CLF	990
CHILE	Chilean Peso	CLP	152
CHINA	Yuan Renminbi	CNY	156
CHRISTMAS ISLAND	Australian Dollar	AUD	036
COCOS (KEELING) ISLANDS (THE)	Australian Dollar	AUD	036
COLOMBIA	Colombian Peso	COP	170
COLOMBIA	Unidad de Valor Real	COU	970
COMOROS (THE)	Comoro Franc	KMF	174
CONGO (THE DEMOCRATIC REPUBLIC OF THE)	Congolese Franc	CDF	976
CONGO (THE)	CFA Franc BEAC	XAF	950
COOK ISLANDS (THE)	New Zealand Dollar	NZD	554
COSTA RICA	Costa Rican Colon	CRC	188
CROATIA	Euro	EUR	978
CUBA	Peso Convertible	CUC	931
CUBA	Cuban Peso	CUP	192
CURAÇAO	Caribbean Guilder	XCG	532
CYPRUS	Euro	EUR	978
CZECH REPUBLIC (THE)	Czech Koruna	CZK	203
CÔTE D'IVOIRE	CFA Franc BCEAO	XOF	952
DENMARK	Danish Krone	DKK	208
DJIBOUTI	Djibouti Franc	DJF	262
DOMINICA	East Caribbean Dollar	XCD	951
DOMINICAN REPUBLIC (THE)	Dominican Peso	DOP	214
ECUADOR	US Dollar	USD	840
EGYPT	Egyptian Pound	EGP	818
EL SALVADOR	El Salvador Colon	SVC	222
EL SALVADOR	US Dollar	USD	840
EQUATORIAL GUINEA	CFA Franc BEAC	XAF	950
ERITREA	Nakfa	ERN	232
ESTONIA	Euro	EUR	978
ETHIOPIA	Ethiopian Birr	ETB	230
EUROPEAN UNION	Euro	EUR	978
FALKLAND ISLANDS (THE) [MALVINAS]	Falkland Islands Pound	FKP	238
FAROE ISLANDS (THE)	Danish Krone	DKK	208
FIJI	Fiji Dollar	FJD	242
FINLAND	Euro	EUR	978
FRANCE	Euro	EUR	978
FRENCH GUIANA	Euro	EUR	978
FRENCH POLYNESIA	CFP Franc	XPF	953
FRENCH SOUTHERN TERRITORIES (THE)	Euro	EUR	978
GABON	CFA Franc BEAC	XAF	950
GAMBIA (THE)	Dalasi	GMD	270
GEORGIA	Lari	GEL	981
GERMANY	Euro	EUR	978
GHANA	Ghana Cedi	GHS	936
GIBRALTAR	Gibraltar Pound	GIP	292
GREECE	Euro	EUR	978
GREENLAND	Danish Krone	DKK	208
GRENADA	East Caribbean Dollar	XCD	951
GUADELOUPE	Euro	EUR	978
GUAM	US Dollar	USD	840
GUATEMALA	Quetzal	GTQ	320
GUERNSEY	Pound Sterling	GBP	826
GUINEA	Guinea Franc	GNF	324
GUINEA-BISSAU	CFA Franc BCEAO	XOF	952
GUYANA	Guyana Dollar	GYD	328
HAITI	Gourde	HTG	332
HAITI	US Dollar	USD	840
HEARD ISLAND AND McDONALD ISLANDS	Australian Dollar	AUD	036
HOLY SEE (THE)	Euro	EUR	978
HONDURAS	Lempira	HNL	340
HONG KONG	Hong Kong Dollar	HKD	344
HUNGARY	Forint	HUF	348
ICELAND	Iceland Krona	ISK	352
//INDIA	Indian Rupee	INR	356
INDONESIA	Rupiah	IDR	360
INTERNATIONAL MONETARY FUND (IMF) 	SDR (Special Drawing Right)	XDR	960
IRAN (ISLAMIC REPUBLIC OF)	Iranian Rial	IRR	364
IRAQ	Iraqi Dinar	IQD	368
IRELAND	Euro	EUR	978
ISLE OF MAN	Pound Sterling	GBP	826
ISRAEL	New Israeli Sheqel	ILS	376
ITALY	Euro	EUR	978
JAMAICA	Jamaican Dollar	JMD	388
JAPAN	Yen	JPY	392
JERSEY	Pound Sterling	GBP	826
JORDAN	Jordanian Dinar	JOD	400
KAZAKHSTAN	Tenge	KZT	398
KENYA	Kenyan Shilling	KES	404
KIRIBATI	Australian Dollar	AUD	036
KOREA (THE DEMOCRATIC PEOPLE’S REPUBLIC OF)	North Korean Won	KPW	408
KOREA (THE REPUBLIC OF)	Won	KRW	410
KUWAIT	Kuwaiti Dinar	KWD	414
KYRGYZSTAN	Som	KGS	417
LAO PEOPLE’S DEMOCRATIC REPUBLIC (THE)	Kip	LAK	418
LATVIA	Euro	EUR	978
LEBANON	Lebanese Pound	LBP	422
LESOTHO	Loti	LSL	426
LESOTHO	Rand	ZAR	710
LIBERIA	Liberian Dollar	LRD	430
LIBYA	Libyan Dinar	LYD	434
LIECHTENSTEIN	Swiss Franc	CHF	756
LITHUANIA	Euro	EUR	978
LUXEMBOURG	Euro	EUR	978
MACAO	Pataca	MOP	446
MADAGASCAR	Malagasy Ariary	MGA	969
MALAWI	Kwacha	MWK	454
MALAYSIA	Malaysian Ringgit	MYR	458
MALDIVES	Rufiyaa	MVR	462
MALI	CFA Franc BCEAO	XOF	952
MALTA	Euro	EUR	978
MARSHALL ISLANDS (THE)	US Dollar	USD	840
MARTINIQUE	Euro	EUR	978
MAURITANIA	Ouguiya	MRU	929
MAURITIUS	Mauritius Rupee	MUR	480
MAYOTTE	Euro	EUR	978
MEMBER COUNTRIES OF THE AFRICAN DEVELOPMENT BANK GROUP	ADB Unit of Account	XUA	965
MEXICO	Mexican Peso	MXN	484
MEXICO	Mexican Unidad de Inversion (UDI)	MXV	979
MICRONESIA (FEDERATED STATES OF)	US Dollar	USD	840
MOLDOVA (THE REPUBLIC OF)	Moldovan Leu	MDL	498
MONACO	Euro	EUR	978
MONGOLIA	Tugrik	MNT	496
MONTENEGRO	Euro	EUR	978
MONTSERRAT	East Caribbean Dollar	XCD	951
MOROCCO	Moroccan Dirham	MAD	504
MOZAMBIQUE	Mozambique Metical	MZN	943
MYANMAR	Kyat	MMK	104
NAMIBIA	Namibia Dollar	NAD	516
NAMIBIA	Rand	ZAR	710
NAURU	Australian Dollar	AUD	036
NEPAL	Nepalese Rupee	NPR	524
//NETHERLANDS (THE)	Euro	EUR	978
NEW CALEDONIA	CFP Franc	XPF	953
NEW ZEALAND	New Zealand Dollar	NZD	554
NICARAGUA	Cordoba Oro	NIO	558
NIGER (THE)	CFA Franc BCEAO	XOF	952
NIGERIA	Naira	NGN	566
NIUE	New Zealand Dollar	NZD	554
NORFOLK ISLAND	Australian Dollar	AUD	036
NORTHERN MARIANA ISLANDS (THE)	US Dollar	USD	840
NORWAY	Norwegian Krone	NOK	578
OMAN	Rial Omani	OMR	512
PAKISTAN	Pakistan Rupee	PKR	586
PALAU	US Dollar	USD	840
PALESTINE, STATE OF	No universal currency		
PANAMA	Balboa	PAB	590
PANAMA	US Dollar	USD	840
PAPUA NEW GUINEA	Kina	PGK	598
PARAGUAY	Guarani	PYG	600
PERU	Nuevo Sol	PEN	604
PHILIPPINES (THE)	Philippine Peso	PHP	608
PITCAIRN	New Zealand Dollar	NZD	554
POLAND	Zloty	PLN	985
PORTUGAL	Euro	EUR	978
PUERTO RICO	US Dollar	USD	840
QATAR	Qatari Rial	QAR	634
REPUBLIC OF NORTH MACEDONIA	Denar	MKD	807
ROMANIA	Romanian Leu	RON	946
RUSSIAN FEDERATION (THE)	Russian Ruble	RUB	643
RWANDA	Rwanda Franc	RWF	646
RÉUNION	Euro	EUR	978
SAINT BARTHÉLEMY	Euro	EUR	978
SAINT HELENA, ASCENSION AND TRISTAN DA CUNHA	Saint Helena Pound	SHP	654
SAINT KITTS AND NEVIS	East Caribbean Dollar	XCD	951
SAINT LUCIA	East Caribbean Dollar	XCD	951
SAINT MARTIN (FRENCH PART)	Euro	EUR	978
SAINT PIERRE AND MIQUELON	Euro	EUR	978
SAINT VINCENT AND THE GRENADINES	East Caribbean Dollar	XCD	951
SAMOA	Tala	WST	882
SAN MARINO	Euro	EUR	978
SAO TOME AND PRINCIPE	Dobra	STN	930
SAUDI ARABIA	Saudi Riyal	SAR	682
SENEGAL	CFA Franc BCEAO	XOF	952
SERBIA	Serbian Dinar	RSD	941
SEYCHELLES	Seychelles Rupee	SCR	690
SIERRA LEONE	Leone	SLE	925
SINGAPORE	Singapore Dollar	SGD	702
SINT MAARTEN (DUTCH PART)	Caribbean guilder	XCG	532
SISTEMA UNITARIO DE COMPENSACION REGIONAL DE PAGOS "SUCRE"	Sucre	XSU	994
SLOVAKIA	Euro	EUR	978
SLOVENIA	Euro	EUR	978
SOLOMON ISLANDS	Solomon Islands Dollar	SBD	090
SOMALIA	Somali Shilling	SOS	706
SOUTH AFRICA	Rand	ZAR	710
SOUTH GEORGIA AND THE SOUTH SANDWICH ISLANDS	No universal currency		
SOUTH SUDAN	South Sudanese Pound	SSP	728
SPAIN	Euro	EUR	978
SRI LANKA	Sri Lanka Rupee	LKR	144
SUDAN (THE)	Sudanese Pound	SDG	938
SURINAME	Surinam Dollar	SRD	968
SVALBARD AND JAN MAYEN	Norwegian Krone	NOK	578
SWAZILAND	Lilangeni	SZL	748
SWEDEN	Swedish Krona	SEK	752
SWITZERLAND	WIR Euro	CHE	947
SWITZERLAND	Swiss Franc	CHF	756
SWITZERLAND	WIR Franc	CHW	948
SYRIAN ARAB REPUBLIC	Syrian Pound	SYP	760
TAIWAN (PROVINCE OF CHINA)	New Taiwan Dollar	TWD	901
TAJIKISTAN	Somoni	TJS	972
TANZANIA, UNITED REPUBLIC OF	Tanzanian Shilling	TZS	834
THAILAND	Baht	THB	764
TIMOR-LESTE	US Dollar	USD	840
TOGO	CFA Franc BCEAO	XOF	952
TOKELAU	New Zealand Dollar	NZD	554
TONGA	Pa’anga	TOP	776
TRINIDAD AND TOBAGO	Trinidad and Tobago Dollar	TTD	780
TUNISIA	Tunisian Dinar	TND	788
TURKEY	Turkish Lira	TRY	949
TURKMENISTAN	Turkmenistan New Manat	TMT	934
TURKS AND CAICOS ISLANDS (THE)	US Dollar	USD	840
TUVALU	Australian Dollar	AUD	036
UGANDA	Uganda Shilling	UGX	800
UKRAINE	Hryvnia	UAH	980
UNITED ARAB EMIRATES (THE)	UAE Dirham	AED	784
//UNITED KINGDOM OF GREAT BRITAIN AND NORTHERN IRELAND (THE)	Pound Sterling	GBP	826
UNITED STATES MINOR OUTLYING ISLANDS (THE)	US Dollar	USD	840
UNITED STATES OF AMERICA (THE)	US Dollar	USD	840
URUGUAY	Uruguay Peso en Unidades Indexadas (URUIURUI)	UYI	940
URUGUAY	Peso Uruguayo	UYU	858
UZBEKISTAN	Uzbekistan Sum	UZS	860
VANUATU	Vatu	VUV	548
VENEZUELA (BOLIVARIAN REPUBLIC OF)	Bolivar	VEF	937
VIET NAM	Dong	VND	704
VIRGIN ISLANDS (BRITISH)	US Dollar	USD	840
VIRGIN ISLANDS (U.S.)	US Dollar	USD	840
WALLIS AND FUTUNA	CFP Franc	XPF	953
WESTERN SAHARA	Moroccan Dirham	MAD	504
YEMEN	Yemeni Rial	YER	886
ZAMBIA	Zambian Kwacha	ZMW	967
ZIMBABWE	Zimbabwe Dollar	ZWL	932
ÅLAND ISLANDS	Euro	EUR	978

*/
