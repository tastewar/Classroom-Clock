
const PROGMEM BellSched NormalDay=
{
  9,
  {
    { 8, 0, 8,50, OUTOFCLUSTER, RED,          OUTOFCLUSTER, YELLOW},
    { 8,52, 9,40, RED,          YELLOW,       YELLOW,       OUTOFCLUSTER},
    { 9,42, 9,52, ASPIRE,       ASPIRE,       ASPIRE,       ASPIRE},
    { 9,54,10,42, YELLOW,       OUTOFCLUSTER, RED,          RED},
    {10,44,11,32, GREEN,        OUTOFCLUSTER, BLUE,         GREEN},
    {11,34,12,23, OUTOFCLUSTER, BLUE,         GREEN,        PURPLE},
    {12,25,12,46, LUNCH,        LUNCH,        LUNCH,        LUNCH},
    {12,48,13,36, BLUE,         PURPLE,       PURPLE,       OUTOFCLUSTER},
    {13,38,14,26, PURPLE,       GREEN,        OUTOFCLUSTER, BLUE},
  }
};

const PROGMEM BellSched ER1115=
{
  7,
  {
    { 8, 0, 8,26, OUTOFCLUSTER, RED,          OUTOFCLUSTER, YELLOW},
    { 8,28, 8,54, RED,          YELLOW,       YELLOW,       OUTOFCLUSTER},
    { 8,56, 9,22, YELLOW,       OUTOFCLUSTER, RED,          RED},
    { 9,24, 9,50, GREEN,        OUTOFCLUSTER, BLUE,         GREEN},
    { 9,52,10,18, OUTOFCLUSTER, BLUE,         GREEN,        PURPLE},
    {10,20,10,46, BLUE,         PURPLE,       PURPLE,       OUTOFCLUSTER},
    {10,48,11,15, PURPLE,       GREEN,        OUTOFCLUSTER, BLUE},
  }
};

const PROGMEM BellSched ER1200=
{
  7,
  {
    { 8, 0, 8,33, OUTOFCLUSTER, RED,          OUTOFCLUSTER, YELLOW},
    { 8,35, 9, 8, RED,          YELLOW,       YELLOW,       OUTOFCLUSTER},
    { 9,10, 9,43, YELLOW,       OUTOFCLUSTER, RED,          RED},
    { 9,45,10,18, GREEN,        OUTOFCLUSTER, BLUE,         GREEN},
    {10,20,10,52, OUTOFCLUSTER, BLUE,         GREEN,        PURPLE},
    {10,54,11,26, BLUE,         PURPLE,       PURPLE,       OUTOFCLUSTER},
    {11,28,12,00, PURPLE,       GREEN,        OUTOFCLUSTER, BLUE},
  }
};

const PROGMEM BellSched ER1300=
{
  8,
  {
    { 8, 0, 8,37, OUTOFCLUSTER, RED,          OUTOFCLUSTER, YELLOW},
    { 8,39, 9,16, RED,          YELLOW,       YELLOW,       OUTOFCLUSTER},
    { 9,18, 9,55, YELLOW,       OUTOFCLUSTER, RED,          RED},
    { 9,57,10,34, GREEN,        OUTOFCLUSTER, BLUE,         GREEN},
    {10,36,11,12, BLUE,         PURPLE,       PURPLE,       OUTOFCLUSTER},
    {11,14,12, 0, OUTOFCLUSTER, BLUE,         GREEN,        PURPLE},
    {12, 2,12,22, LUNCH,        LUNCH,        LUNCH,        LUNCH},
    {12,24,13, 0, PURPLE,       GREEN,        OUTOFCLUSTER, BLUE},
  }
};
