/* Public-edition staging: local UI additions and/or privacy review; see source manifest. */
/* SPDX-License-Identifier: Apache-2.0 */
#include "muse_pet_art.h"
static const char icons[10][16][17]={
{"                ","     yyyyyy     ","   yyYYYYYYyy   ","  yYYYYYYYYYYy  "," yYYWYYYYYYYYYy "," yYWYYYOOYYYYYy ","yYYWYYOYYOYYYYYy","yYYWYYOYYOYYYYYy","yYYYYYOYYOYYYYYy","yYYYYYOYYOYYYYYy"," yYYYYYOOYYYYYy "," yYYYYYYYYYYYYy ","  yYYYYYYYYYYy  ","   yyYYYYYYyy   ","     yyyyyy     ","                "},
{"                ","                ","    bbbbbbbb    ","  bbWWbWWbWWbb  "," bbbbbbbbbbbbbb "," bbbbbbbbbbbbbb "," GGGGGGGGGGGGGG ","  RRRRRRRRRRRR  "," YYYYYYYYYYYYYY ","  OOOOOOOOOOOO  "," bbbbbbbbbbbbbb "," bbbbbbbbbbbbbb ","  bbbbbbbbbbbb  ","                ","                ","                "},
{"                ","      PPPP      ","    PPPWWPPP    ","   PPWWPPPPPP   ","  PPWPPPPPPPPP  ","  PWPPPPPPPPPP  "," PPWPPPPPPPPPPP "," PPPPPPPPPPPPPP "," PPPPPPPPPPPPPP ","  PPPPPPPPPPPP  ","   PPPPPPPPPP   ","    PPPPPPPP    ","   OOOOOOOOOO   ","  OOOYYYYYYOOO  ","  OOOOOOOOOOOO  ","                "},
{"    W    W      ","     W  W       ","                ","  WWWWWWWWW     ","  WWWWWWWWWWWW  ","  WWOOOOOWW  W  ","  WWWWWWWWW  W  ","  WWWWWWWWWWWW  ","  WWWWWWWWW     ","  WWWWWWWWW     ","   WWWWWWW      ","    WWWWW       ","                "," WWWWWWWWWWWW   ","                ","                "},
{"                ","       R        ","      RRR       ","   WWWWWWWWWW   ","  WWWWWWWWWWWW  ","  bbbbbbbbbbbb  ","  WWWWWWWWWWWW  ","  WWWWWWWWWWWW  ","  bbbbbbbbbbbb  ","  WWWWWWWWWWWW  ","  WWWWWWWWWWWW  ","  OOOOOOOOOOOO  ","  OOOOOOOOOOOO  "," WWWWWWWWWWWWWW ","                ","                "},
{"                ","       W        ","     W   W      ","                ","    WWWWWWWW    ","   WOOOOGOWWW   ","  WOOOGOOOOWWW  "," WWWWOOOWWWWWWW "," OOOOOOOOOOOOOO ","  RRRRRRRRRRRR  ","  RRRRRRRRRRRR  ","   RRRRRRRRRR   ","    RRRRRRRR    ","     RRRRRR     ","                ","                "},
{"                ","   W     W      ","     W W        ","                ","    RRGGWWRR    ","   RGWWRRWWRR   "," OORRRRRRRRRROO "," OORRRRRRRRRROO "," OOOOOOOOOOOOOO "," OOOOOOOOOOOOOO ","  OOOOOOOOOOOO  ","   OOOOOOOOOO   ","    OOOOOOOO    ","   OO      OO   ","                ","                "},
{"                ","     bbbbb      ","   bbbbbbbbb    ","  bbbbWbbbbbb   ","  bbbbbbbbbbbb  "," bbbbbWbbbbbbb  "," bbbbbbbbbbbbbb "," bbbbWbbbbbbbbb "," bbbbbbbbbbbbbb ","  bbbbbbbbbbbb  ","   bbbbbbbbbb   ","    bbbbbbbb    ","     WWWWWW     ","      WWWW      ","                ","                "},
{"       RR       ","       RR       ","       RR       "," WWWWWWWWWWWW   ","  WWWWWWWWWW    ","  bbbbbbbbbb    ","  bbbbbbbbbb    ","   bbbbbbbb     ","   ObbObbOb     ","   bbObbObb     ","   ObbObbOb     ","    bbbbbb      ","    ObObOb      ","    bbbbbb      ","                ","                "},
{"                ","                ","  DDDDDDDDDDDD  "," DCCCCCCCCCCCCD "," DCWWWWWWWWWWCD "," DCWCCCCCCCCWCD "," DCWWWWWWWWWWCD "," DCWCCCCCCCCWCD "," DCWWWWWWWWWWCD "," DCCCCCCCCCCCCD ","  DDDDDDDDDDDD  ","     CCCD       ","    CCD         ","   DD           ","                ","                "},
};
uint32_t muse_pet_art_pixel(unsigned icon,int x,int y) {
 if(icon>=10||x<0||x>=16||y<0||y>=16)return 0;
 switch(icons[icon][y][x]) {
 case 'y':return 0xA76A19;case 'Y':return 0xF6CB4B;case 'W':return 0xFFF2CE;
 case 'O':return 0x644331;case 'b':return 0xCB934F;case 'G':return 0x79AC69;
 case 'R':return 0xBD6655;case 'P':return 0xB5A4E7;case 'C':return 0x7DADCB;case 'D':return 0x51637F;default:return 0;
 }
}
