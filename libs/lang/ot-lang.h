//==========================================================================
// ObTools::Lang: ot-lang.h
//
// Public definitions for ObTools::Lang
// ISO language codes
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#ifndef __OBTOOLS_LANG_H
#define __OBTOOLS_LANG_H

#include <string>
#include <iostream>

namespace ObTools { namespace Lang {

using namespace std;

//==========================================================================
// Enum class for languages
enum class Language
{
  unknown,
  abkhazian,
  achinese,
  acoli,
  adangme,
  adyghe,
  afar,
  afrihili,
  afrikaans,
  afro_asiatic_languages,
  ainu,
  akan,
  akkadian,
  albanian,
  aleut,
  algonquian_languages,
  altaic_languages,
  amharic,
  angika,
  apache_languages,
  arabic,
  aragonese,
  arapaho,
  arawak,
  armenian,
  aromanian,
  artificial_languages,
  assamese,
  asturian,
  athapascan_languages,
  australian_languages,
  austronesian_languages,
  avaric,
  avestan,
  awadhi,
  aymara,
  azerbaijani,
  balinese,
  baltic_languages,
  baluchi,
  bambara,
  bamileke_languages,
  banda_languages,
  bantu_languages,
  basa,
  bashkir,
  basque,
  batak_languages,
  beja,
  belarusian,
  bemba,
  bengali,
  berber_languages,
  bhojpuri,
  bihari_languages,
  bikol,
  bini,
  bislama,
  blin,
  blissymbols,
  bokmal_norwegian,
  bosnian,
  braj,
  breton,
  buginese,
  bulgarian,
  buriat,
  burmese,
  caddo,
  catalan,
  caucasian_languages,
  cebuano,
  celtic_languages,
  central_american_indian_languages,
  central_khmer,
  chagatai,
  chamic_languages,
  chamorro,
  chechen,
  cherokee,
  cheyenne,
  chibcha,
  chichewa,
  chinese,
  chinook_jargon,
  chipewyan,
  choctaw,
  church_slavic,
  chuukese,
  chuvash,
  classical_newari,
  classical_syriac,
  coptic,
  cornish,
  corsican,
  cree,
  creek,
  creoles_and_pidgins,
  creoles_and_pidgins_english_based,
  creoles_and_pidgins_french_based,
  creoles_and_pidgins_portuguese_based,
  crimean_tatar,
  croatian,
  cushitic_languages,
  czech,
  dakota,
  danish,
  dargwa,
  delaware,
  dinka,
  divehi,
  dogri,
  dogrib,
  dravidian_languages,
  duala,
  dutch,
  dutch_middle,
  dyula,
  dzongkha,
  eastern_frisian,
  efik,
  egyptian_ancient,
  ekajuk,
  elamite,
  english,
  english_middle,
  english_old,
  erzya,
  esperanto,
  estonian,
  ewe,
  ewondo,
  fang,
  fanti,
  faroese,
  fijian,
  filipino,
  finnish,
  finno_ugrian_languages,
  fon,
  french,
  french_middle,
  french_old,
  friulian,
  fulah,
  ga,
  gaelic,
  galibi_carib,
  galician,
  ganda,
  gayo,
  gbaya,
  geez,
  georgian,
  german,
  german_middle_high,
  german_old_high,
  germanic_languages,
  gilbertese,
  gondi,
  gorontalo,
  gothic,
  grebo,
  greek_ancient,
  greek_modern,
  guarani,
  gujarati,
  gwichin,
  haida,
  haitian,
  hausa,
  hawaiian,
  hebrew,
  herero,
  hiligaynon,
  himachali_languages,
  hindi,
  hiri_motu,
  hittite,
  hmong,
  hungarian,
  hupa,
  iban,
  icelandic,
  ido,
  igbo,
  ijo_languages,
  iloko,
  inari_sami,
  indic_languages,
  indo_european_languages,
  indonesian,
  ingush,
  interlingua,
  interlingue,
  inuktitut,
  inupiaq,
  iranian_languages,
  irish,
  irish_middle,
  irish_old,
  iroquoian_languages,
  italian,
  japanese,
  javanese,
  judeo_arabic,
  judeo_persian,
  kabardian,
  kabyle,
  kachin,
  kalaallisut,
  kalmyk,
  kamba,
  kannada,
  kanuri,
  kara_kalpak,
  karachay_balkar,
  karelian,
  karen_languages,
  kashmiri,
  kashubian,
  kawi,
  kazakh,
  khasi,
  khoisan_languages,
  khotanese,
  kikuyu,
  kimbundu,
  kinyarwanda,
  kirghiz,
  klingon,
  komi,
  kongo,
  konkani,
  korean,
  kosraean,
  kpelle,
  kru_languages,
  kuanyama,
  kumyk,
  kurdish,
  kurukh,
  kutenai,
  ladino,
  lahnda,
  lamba,
  land_dayak_languages,
  lao,
  latin,
  latvian,
  lezghian,
  limburgan,
  lingala,
  lithuanian,
  lojban,
  low_german,
  lower_sorbian,
  lozi,
  luba_katanga,
  luba_lulua,
  luiseno,
  lule_sami,
  lunda,
  luo,
  lushai,
  luxembourgish,
  macedonian,
  madurese,
  magahi,
  maithili,
  makasar,
  malagasy,
  malay,
  malayalam,
  maltese,
  manchu,
  mandar,
  mandingo,
  manipuri,
  manobo_languages,
  manx,
  maori,
  mapudungun,
  marathi,
  mari,
  marshallese,
  marwari,
  masai,
  mayan_languages,
  mende,
  mikmaq,
  minangkabau,
  mirandese,
  mohawk,
  moksha,
  mon_khmer_languages,
  mongo,
  mongolian,
  mossi,
  multiple_languages,
  munda_languages,
  nko,
  nahuatl_languages,
  nauru,
  navajo,
  ndebele_north,
  ndebele_south,
  ndonga,
  neapolitan,
  nepal_bhasa,
  nepali,
  nias,
  niger_kordofanian_languages,
  nilo_saharan_languages,
  niuean,
  no_linguistic_content,
  nogai,
  norse_old,
  north_american_indian_languages,
  northern_frisian,
  northern_sami,
  norwegian,
  norwegian_nynorsk,
  nubian_languages,
  nyamwezi,
  nyankole,
  nyoro,
  nzima,
  occitan,
  official_aramaic,
  ojibwa,
  oriya,
  oromo,
  osage,
  ossetian,
  otomian_languages,
  pahlavi,
  palauan,
  pali,
  pampanga,
  pangasinan,
  panjabi,
  papiamento,
  papuan_languages,
  pedi,
  persian,
  persian_old,
  philippine_languages,
  phoenician,
  pohnpeian,
  polish,
  portuguese,
  prakrit_languages,
  provencal_old,
  pushto,
  quechua,
  rajasthani,
  rapanui,
  rarotongan,
  romance_languages,
  romanian,
  romansh,
  romany,
  rundi,
  russian,
  salishan_languages,
  samaritan_aramaic,
  sami_languages,
  samoan,
  sandawe,
  sango,
  sanskrit,
  santali,
  sardinian,
  sasak,
  scots,
  selkup,
  semitic_languages,
  serbian,
  serer,
  shan,
  shona,
  sichuan_yi,
  sicilian,
  sidamo,
  sign_languages,
  siksika,
  sindhi,
  sinhala,
  sino_tibetan_languages,
  siouan_languages,
  skolt_sami,
  slave,
  slavic_languages,
  slovak,
  slovenian,
  sogdian,
  somali,
  songhai_languages,
  soninke,
  sorbian_languages,
  sotho_southern,
  south_american_indian_languages,
  southern_altai,
  southern_sami,
  spanish,
  sranan_tongo,
  standard_moroccan_tamazight,
  sukuma,
  sumerian,
  sundanese,
  susu,
  swahili,
  swati,
  swedish,
  swiss_german,
  syriac,
  tagalog,
  tahitian,
  tai_languages,
  tajik,
  tamashek,
  tamil,
  tatar,
  telugu,
  tereno,
  tetum,
  thai,
  tibetan,
  tigre,
  tigrinya,
  timne,
  tiv,
  tlingit,
  tok_pisin,
  tokelau,
  tonga_nyasa,
  tonga_islands,
  tsimshian,
  tsonga,
  tswana,
  tumbuka,
  tupi_languages,
  turkish,
  turkish_ottoman,
  turkmen,
  tuvalu,
  tuvinian,
  twi,
  udmurt,
  ugaritic,
  uighur,
  ukrainian,
  umbundu,
  uncoded_languages,
  undetermined,
  upper_sorbian,
  urdu,
  uzbek,
  vai,
  venda,
  vietnamese,
  volapuk,
  votic,
  wakashan_languages,
  walloon,
  waray,
  washo,
  welsh,
  western_frisian,
  wolaitta,
  wolof,
  xhosa,
  yakut,
  yao,
  yapese,
  yiddish,
  yoruba,
  yupik_languages,
  zande_languages,
  zapotec,
  zaza,
  zenaga,
  zhuang,
  zulu,
  zuni,
};

//--------------------------------------------------------------------------
// Convert an ISO 629 code to a language
Language iso_639_to_lang(const string& iso);

//--------------------------------------------------------------------------
// Convert an ISO 639-1 (2 character) code to a language
Language iso_639_1_to_lang(const string& iso);

//--------------------------------------------------------------------------
// Convert a language to an ISO 639-1 (2 character)
string lang_to_iso_639_1(Language lang);

//--------------------------------------------------------------------------
// Convert an ISO 639-2 (3 character) code to a language
Language iso_639_2_to_lang(const string& iso);

//--------------------------------------------------------------------------
// Convert a language to an ISO 639-2 (3 character)
string lang_to_iso_639_2(Language lang);

//--------------------------------------------------------------------------
// Convert a language to a string
string lang_to_string(Language lang);

//--------------------------------------------------------------------------
// Write a language to a stream
ostream& operator<<(ostream& s, Language lang);

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_LANG_H