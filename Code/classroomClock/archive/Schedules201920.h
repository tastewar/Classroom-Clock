const PROGMEM BellSched FirstDay=
{
  10,
  {
    { 8, 0, 9,40, ASPIRE,       OUTOFCLUSTER, RED,          OUTOFCLUSTER},
    { 9,43,10,12, RED,          RED,          OUTOFCLUSTER, YELLOW},
    {10,15,10,44, YELLOW,       ASPIRE,       ASPIRE,       ASPIRE},
    {10,47,11,16, OUTOFCLUSTER, YELLOW,       YELLOW,       RED},
    {11,19,12,10, GREEN,        GREEN,        GREEN,        GREEN},
    {12,13,12,37, LUNCH,        PURPLE,       OUTOFCLUSTER, LUNCH},
    {12,40,13, 9, BLUE,         LUNCH,        OUTOFCLUSTER, BLUE},
    {13,12,13,41, PURPLE,       OUTOFCLUSTER, BLUE,         OUTOFCLUSTER},
    {13,44,14,13, OUTOFCLUSTER, BLUE,         PURPLE,       PURPLE},
    {14,16,14,26, ASPIRE,       BLUE,         PURPLE,       PURPLE},
  }
};

const PROGMEM BellSched NormalDay=
{
  10,
  {
    { 8, 0, 8,50, RED,          OUTOFCLUSTER, RED,          OUTOFCLUSTER},
    { 8,52, 9,40, YELLOW,       RED,          OUTOFCLUSTER, YELLOW},
    { 9,42, 9,52, ASPIRE,       ASPIRE,       ASPIRE,       ASPIRE},
    { 9,54,10,42, OUTOFCLUSTER, YELLOW,       YELLOW,       RED},
    {10,44,11,32, GREEN,        GREEN,        GREEN,        GREEN},
    {11,34,11,57, LUNCH,        PURPLE,       OUTOFCLUSTER, BLUE},
    {11,59,12,21, BLUE,         PURPLE,       OUTOFCLUSTER, LUNCH},
    {12,23,12,46, BLUE,         LUNCH,        OUTOFCLUSTER, BLUE},
    {12,48,13,36, PURPLE,       OUTOFCLUSTER, BLUE,         OUTOFCLUSTER},
    {13,38,14,26, OUTOFCLUSTER, BLUE,         PURPLE,       PURPLE},
  }
};

const PROGMEM BellSched ER1115=
{
  7,
  {
    { 8, 0, 8,26, RED,          OUTOFCLUSTER, RED,          OUTOFCLUSTER},
    { 8,28, 8,54, YELLOW,       RED,          OUTOFCLUSTER, YELLOW},
    { 8,56, 9,22, OUTOFCLUSTER, YELLOW,       YELLOW,       RED},
    { 9,24, 9,50, GREEN,        GREEN,        GREEN,        GREEN},
    { 9,52,10,18, BLUE,         PURPLE,       OUTOFCLUSTER, BLUE},
    {10,20,10,46, PURPLE,       OUTOFCLUSTER, BLUE,         OUTOFCLUSTER},
    {10,48,11,15, OUTOFCLUSTER, BLUE,         PURPLE,       PURPLE},
  }
};

const PROGMEM BellSched ER1200=
{
  7,
  {
    { 8, 0, 8,33, RED,          OUTOFCLUSTER, RED,          OUTOFCLUSTER},
    { 8,35, 9, 8, YELLOW,       RED,          OUTOFCLUSTER, YELLOW},
    { 9,10, 9,43, OUTOFCLUSTER, YELLOW,       YELLOW,       RED},
    { 9,45,10,18, GREEN,        GREEN,        GREEN,        GREEN},
    {10,20,10,52, BLUE,         PURPLE,       OUTOFCLUSTER, BLUE},
    {10,54,11,26, PURPLE,       OUTOFCLUSTER, BLUE,         OUTOFCLUSTER},
    {11,28,12,00, OUTOFCLUSTER, BLUE,         PURPLE,       PURPLE},
  }
};

const PROGMEM BellSched ER1300=
{
  9,
  {
    { 8, 0, 8,37, RED,          OUTOFCLUSTER, RED,          OUTOFCLUSTER},
    { 8,39, 9,16, YELLOW,       RED,          OUTOFCLUSTER, YELLOW},
    { 9,18, 9,55, OUTOFCLUSTER, YELLOW,       YELLOW,       RED},
    { 9,57,10,34, GREEN,        GREEN,        GREEN,        GREEN},
    {10,36,11,12, PURPLE,       OUTOFCLUSTER, BLUE,         OUTOFCLUSTER},
    {11,14,11,36, LUNCH,        PURPLE,       OUTOFCLUSTER, BLUE},
    {11,38,12, 0, BLUE,         PURPLE,       OUTOFCLUSTER, LUNCH},
    {12, 2,12,22, BLUE,         LUNCH,        OUTOFCLUSTER, BLUE},
    {12,24,13, 0, OUTOFCLUSTER, BLUE,         PURPLE,       PURPLE},
  }
};
