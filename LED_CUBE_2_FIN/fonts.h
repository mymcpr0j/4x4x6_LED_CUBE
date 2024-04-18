#define SYMBOLS_COUNT 67
#define BYTES_IN_SYMBOL 6
#define BITS_FILLED 4

//https://fontstruct.com/fontstructions/show/2392018/bitach-4x6
//https://fontstruct.com/fontstructions/show/2133937/4x6-font-5
byte my_font[SYMBOLS_COUNT][BYTES_IN_SYMBOL]{
  { //0 - 0
    0b0110,
    0b1001,
    0b1011,
    0b1101,
    0b1001,
    0b0110
  },
  { //1 - 1
    0b0001,
    0b0011,
    0b0101,
    0b0001,
    0b0001,
    0b0001
  },
  { //2 - 2
    0b0110,
    0b1001,
    0b0010,
    0b0100,
    0b1000,
    0b1111
  },
  { //3 - 3
    0b0110,
    0b1001,
    0b0010,
    0b0001,
    0b1001,
    0b0110
  },
  { //4 - 4
    0b0011,
    0b0101,
    0b1001,
    0b1111,
    0b0001,
    0b0001
  },
  { //5 - 5
    0b1111,
    0b1000,
    0b1110,
    0b0001,
    0b1001,
    0b0110
  },
  { //6 - 6
    0b0110,
    0b1000,
    0b1110,
    0b1001,
    0b1001,
    0b0110
  },
  { //7 - 7
    0b1111,
    0b0001,
    0b0010,
    0b0100,
    0b0100,
    0b0100
  },
  { //8 - 8
    0b0110,
    0b1001,
    0b0110,
    0b1001,
    0b1001,
    0b0110
  },
  { //9 - 9
    0b0110,
    0b1001,
    0b1001,
    0b0111,
    0b0001,
    0b0110
  },
  { //10 - space
    0b0000,
    0b0000,
    0b0000,
    0b0000,
    0b0000,
    0b0000
  },
  { //11 - A
    0b0111,
    0b1001,
    0b1001,
    0b1111,
    0b1001,
    0b1001
  },
  { //12 - B
    0b1110,
    0b1001,
    0b1110,
    0b1001,
    0b1001,
    0b1110
  },
  { //13 - C
    0b0111,
    0b1000,
    0b1000,
    0b1000,
    0b1000,
    0b0111
  },
  { //14 - D
    0b1110,
    0b1001,
    0b1001,
    0b1001,
    0b1001,
    0b1110
  },
  { //15 - E
    0b1111,
    0b1000,
    0b1110,
    0b1000,
    0b1000,
    0b1111
  },
    { //16 - F
    0b1111,
    0b1000,
    0b1110,
    0b1000,
    0b1000,
    0b1000
  },
  { //17 - G
    0b0110,
    0b1001,
    0b1000,
    0b1011,
    0b1001,
    0b0110
  },
  { //18 - H
    0b1001,
    0b1001,
    0b1111,
    0b1001,
    0b1001,
    0b1001
  },
  { //19 - I
    0b0111,
    0b0010,
    0b0010,
    0b0010,
    0b0010,
    0b0111
  },
  { //20 - J
    0b0011,
    0b0001,
    0b0001,
    0b0001,
    0b1001,
    0b0110
  },
  { //21 - K
    0b1001,
    0b1010,
    0b1100,
    0b1010,
    0b1001,
    0b1001
  },
  { //22 - L
    0b1000,
    0b1000,
    0b1000,
    0b1000,
    0b1000,
    0b1111
  },
  { //23 - M
    0b1001,
    0b1111,
    0b1111,
    0b1001,
    0b1001,
    0b1001
  },
  { //24 - N
    0b1001,
    0b1001,
    0b1101,
    0b1011,
    0b1001,
    0b1001
  },
  { //25 - O
    0b0110,
    0b1001,
    0b1001,
    0b1001,
    0b1001,
    0b0110
  },
  { //26 - P
    0b1110,
    0b1001,
    0b1001,
    0b1110,
    0b1000,
    0b1000
  },
  { //27 - Q
    0b0110,
    0b1001,
    0b1001,
    0b1001,
    0b1010,
    0b0101
  },
  { //28 - R
    0b1110,
    0b1001,
    0b1001,
    0b1110,
    0b1001,
    0b1001
  },
  { //29 - S
    0b0110,
    0b1001,
    0b0100,
    0b0010,
    0b1001,
    0b0110
  },
  { //30 - T
    0b0111,
    0b0010,
    0b0010,
    0b0010,
    0b0010,
    0b0010,
  },
  { //31 - U
    0b1001,
    0b1001,
    0b1001,
    0b1001,
    0b1001,
    0b0110
  },
  { //32 - V
    0b1001,
    0b1001,
    0b1001,
    0b1001,
    0b0110,
    0b0110
  },
  { //33 - W
    0b1001,
    0b1001,
    0b1001,
    0b1111,
    0b1111,
    0b1001
  },
  { //34 - X
    0b1001,
    0b1001,
    0b0110,
    0b0110,
    0b1001,
    0b1001
  },
  { //35 - Y
    0b0101,
    0b0101,
    0b0101,
    0b0010,
    0b0010,
    0b0010
  },
  { //36 - Z
    0b1111,
    0b0001,
    0b0010,
    0b0100,
    0b1000,
    0b1111
  },
  { //37 - ?
    0b1111,
    0b1001,
    0b0010,
    0b0100,
    0b0000,
    0b0100
  },
  { //38 - !
    0b0001,
    0b0001,
    0b0001,
    0b0001,
    0b0000,
    0b0001
  },
  { //39 - _
    0b0000,
    0b0000,
    0b0000,
    0b0000,
    0b0000,
    0b1111
  },
  { //40 - -
    0b0000,
    0b0000,
    0b0000,
    0b1111,
    0b0000,
    0b0000
  },
  { //41 - *
    0b0101,
    0b0010,
    0b0101,
    0b0000,
    0b0000,
    0b0000
  },
  { // 42 - #
    0b0101,
    0b1111,
    0b0101,
    0b0101,
    0b1111,
    0b0101
  },
  { // 43 - $
    0b0010,
    0b0111,
    0b0110,
    0b0011,
    0b0111,
    0b0010
  },
  { // 44 - %
    0b0101,
    0b0001,
    0b0010,
    0b0010,
    0b0100,
    0b0101
  },
  { //45 - &
    0b0110,
    0b1001,
    0b0110,
    0b1010,
    0b1001,
    0b0111
  },
  { //46 - |
    0b0001,
    0b0001,
    0b0001,
    0b0001,
    0b0001,
    0b0001
  },
  { //47 - ~
    0b0000,
    0b0000,
    0b0101,
    0b1010,
    0b0000,
    0b0000
  },
  { //48 - +
    0b0000,
    0b0000,
    0b0010,
    0b0111,
    0b0010,
    0b0000
  },
  { //49 - (
    0b0100,
    0b1000,
    0b1000,
    0b1000,
    0b1000,
    0b0100
  },
  { //50 - )
    0b0010,
    0b0001,
    0b0001,
    0b0001,
    0b0001,
    0b0010
  },
  { //51 - /
    0b0001,
    0b0001,
    0b0010,
    0b0010,
    0b0100,
    0b0100
  },
  { //52 - :
    0b0000,
    0b0001,
    0b0000,
    0b0000,
    0b0001,
    0b0000
  },
  { //53 - .
    0b0000,
    0b0000,
    0b0000,
    0b0000,
    0b0001,
    0b0000
  },
    { //54 - ,
    0b0000,
    0b0000,
    0b0000,
    0b0000,
    0b0001,
    0b0010
  },
  { //55 - ;
    0b0000,
    0b0001,
    0b0000,
    0b0000,
    0b0001,
    0b0010
  },
  { //56 - <
    0b000,
    0b0010,
    0b0100,
    0b1000,
    0b0100,
    0b0010
  },
  { //57 - >
    0b0000,
    001000,
    0b0010,
    0b0001,
    0b0010,
    0b0100
  },
  { // 58 - =
    0b0000,
    0b0000,
    0b0111,
    0b0000,
    0b0111,
    0b0000
  },
  { //59 - [
    0b1100,
    0b1000,
    0b1000,
    0b1000,
    0b1000,
    0b1100
  },
  { //60 - ]
    0b0011,
    0b0001,
    0b0001,
    0b0001,
    0b0001,
    0b0011
  },
  { //61 - backslash
    0b0100,
    0b0100,
    0b0010,
    0b0010,
    0b0001,
    0b0001
  },
  { //62 - ^
    0b0010,
    0b0101,
    0b0000,
    0b0000,
    0b0000,
    0b0000
  },
  { //63 - '
    0b0010,
    0b0001,
    0b0000,
    0b0000,
    0b0000,
    0b0000
  },
  { //64 - {
    0b0010,
    0b0100,
    0b1100,
    0b0100,
    0b0100,
    0b0010
  },
  { //65 - }
    0b0100,
    0b0010,
    0b0011,
    0b0010,
    0b0010,
    0b0100
  },
  { //68 - null
  0,0,0,0,0,0
  }
};

char my_font_symbols[SYMBOLS_COUNT] {
  '0',
  '1',
  '2',
  '3',
  '4',
  '5',
  '6',
  '7',
  '8',
  '9',
  ' ',
  'A',
  'B',
  'C',
  'D',
  'E',
  'F',
  'G',
  'H',
  'I',
  'J',
  'K',
  'L',
  'M',
  'N',
  'O',
  'P',
  'Q',
  'R',
  'S',
  'T',
  'U',
  'V',
  'W',
  'X',
  'Y',
  'Z',
  '?',
  '!',
  '_',
  '-',
  '*',
  '#',
  '$',
  '%',
  '&',
  '|',
  '~',
  '+',
  '(',
  ')',
  '/',
  ':',
  '.',
  ',',
  ';',
  '<',
  '>',
  '=',
  '[',
  ']',
  '\\',
  '^',
  '\'',
  '{',
  '}',
  '\0' //end of string for tests
};

int my_font_symbol_pos(char c) {
  for(int i = 0; i<SYMBOLS_COUNT;i++) {
    if(c == my_font_symbols[i]) return i;
  }
  return -1;
}