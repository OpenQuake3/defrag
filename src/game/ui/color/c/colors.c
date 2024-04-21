#include "../schemes.h"

//:::::::::::::::::::::::
// Quake3 Defaults
static const vec4_t color_menu_text   = { 1.00f, 1.00f, 1.00f, 1.00f };
static const vec4_t color_menu_dim    = { 0.00f, 0.00f, 0.00f, 0.75f };
static const vec4_t color_black       = { 0.00f, 0.00f, 0.00f, 1.00f };
static const vec4_t color_gray50      = { 0.50f, 0.50f, 0.50f, 1.00f };
static const vec4_t color_white       = { 1.00f, 1.00f, 1.00f, 1.00f };
static const vec4_t color_yellow      = { 1.00f, 1.00f, 0.00f, 1.00f };
static const vec4_t color_blue        = { 0.00f, 0.00f, 1.00f, 1.00f };
static const vec4_t color_orangeLight = { 1.00f, 0.68f, 0.00f, 1.00f };
static const vec4_t color_orange      = { 1.00f, 0.43f, 0.00f, 1.00f };
static const vec4_t color_orangeTrans = { 1.00f, 0.43f, 0.00f, 0.30f };
static const vec4_t color_red         = { 1.00f, 0.00f, 0.00f, 1.00f };
static const vec4_t color_dim         = { 0.00f, 0.00f, 0.00f, 0.25f };
// Quake3 default color scheme
Q3Scheme q3color = {
  .pulse          = &color_white,
  .listbar        = &color_orangeTrans,  // transluscent orange
  .text_disabled  = &color_gray50,       // light gray
  .text_normal    = &color_orange,       // light orange,
  .text_highlight = &color_yellow,       // bright yellow
  .text_status    = &color_white,        // bright white
  .text_menu      = &color_white,
};
// Quake3 Credit screen
ColorScheme q3credits = {
  .bg        = &color_black,
  .neutral   = &color_gray50,
  .fg        = &color_white,
  .highlight = &color_orange,
  .key       = &color_red,
};
//:::::::::::::::::::::::


//:::::::::::::::::::::::
// Quake3 based
ColorScheme mStartQ3 = {
  .bg        = &color_black,
  .neutral   = &color_gray50,
  .fg        = &color_white,
  .highlight = &color_orange,
  .key       = &color_red,
};
//:::::::::::::::::::::::


//:::::::::::::::::::::::
// Color Schemes
//:::::::::::::::::::::::
static const vec4_t k00;  // Black
static const vec4_t k50;  // Gray50
static const vec4_t wht;  // White
static const vec4_t re3;  // Orange
static const vec4_t re6;  // Red
//:::::::::::::::::::::::
ColorScheme mStartColor = {
  .bg        = &k00,
  .neutral   = &k50,
  .fg        = &wht,
  .highlight = &re3,
  .key       = &re6,
};
//:::::::::::::::::::::::


//::::::::::::::::::::::::::::::::::::::::
//::      ::  COLOR PALETTES  ::      ::::
//::::::::::::::::::::::::::::::::::::::::
// Colorblind 24 palette
//   From:  http://mkweb.bcgsc.ca/colorblind/
//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// CB24: Pinks
static const vec4_t pi8 = { 0.337, 0.004, 0.200, 1.000 };  // #560133 : mulberry
static const vec4_t pi7 = { 0.475, 0.004, 0.286, 1.000 };  // #790149 : french plum
static const vec4_t pi6 = { 0.624, 0.004, 0.384, 1.000 };  // #9F0162 : jazzberry jam
static const vec4_t pi5 = { 0.780, 0.000, 0.486, 1.000 };  // #C7007C : magenta
static const vec4_t pi4 = { 0.937, 0.000, 0.588, 1.000 };  // #EF0096 : persian rose
static const vec4_t pi3 = { 1.000, 0.353, 0.686, 1.000 };  // #FF5AAF : barbie pink
static const vec4_t pi2 = { 1.000, 0.616, 0.784, 1.000 };  // #FF9DC8 : amaranth pink
static const vec4_t pi1 = { 1.000, 0.812, 0.886, 1.000 };  // #FFCFE2 : azalea
//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// CB24: Purples
static const vec4_t pu8 = { 0.271, 0.008, 0.439, 1.000 };  // #450270 : christalle
static const vec4_t pu7 = { 0.396, 0.004, 0.624, 1.000 };  // #65019F : purple heart
static const vec4_t pu6 = { 0.518, 0.000, 0.804, 1.000 };  // #8400CD : french violet
static const vec4_t pu5 = { 0.655, 0.000, 0.988, 1.000 };  // #A700FC : electric purple
static const vec4_t pu4 = { 0.855, 0.000, 0.992, 1.000 };  // #DA00FD : psychedelic purple
static const vec4_t pu3 = { 1.000, 0.235, 0.996, 1.000 };  // #FF3CFE : fuchsia
static const vec4_t pu2 = { 1.000, 0.573, 0.992, 1.000 };  // #FF92FD : violet
static const vec4_t pu1 = { 1.000, 0.800, 0.996, 1.000 };  // #FFCCFE : pale mauve
//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// CB24: Reds
static const vec4_t re8 = { 0.353, 0.000, 0.059, 1.000 };  // #5A000F : rosewood
static const vec4_t re7 = { 0.494, 0.000, 0.094, 1.000 };  // #7E0018 : hot chile
static const vec4_t re6 = { 0.643, 0.004, 0.133, 1.000 };  // #A40122 : alabama crimson
static const vec4_t re5 = { 0.804, 0.008, 0.176, 1.000 };  // #CD022D : amaranth red
static const vec4_t re4 = { 0.965, 0.008, 0.224, 1.000 };  // #F60239 : carmine
static const vec4_t re3 = { 1.000, 0.431, 0.227, 1.000 };  // #FF6E3A : burning orange
static const vec4_t re2 = { 1.000, 0.675, 0.231, 1.000 };  // #FFAC3B : frenzee
static const vec4_t re1 = { 1.000, 0.863, 0.239, 1.000 };  // #FFDC3D : gargoyle gas
//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// CB24: Teals
static const vec4_t te8 = { 0.000, 0.239, 0.188, 1.000 };  // #003D30 : sherwood green
static const vec4_t te7 = { 0.000, 0.341, 0.271, 1.000 };  // #005745 : deep opal
static const vec4_t te6 = { 0.000, 0.451, 0.361, 1.000 };  // #00735C : robin hood
static const vec4_t te5 = { 0.000, 0.569, 0.459, 1.000 };  // #009175 : elf green
static const vec4_t te4 = { 0.000, 0.686, 0.557, 1.000 };  // #00AF8E : jeepers creepers
static const vec4_t te3 = { 0.000, 0.796, 0.655, 1.000 };  // #00CBA7 : aquamarine
static const vec4_t te2 = { 0.000, 0.922, 0.757, 1.000 };  // #00EBC1 : vivid opal
static const vec4_t te1 = { 0.525, 1.000, 0.871, 1.000 };  // #86FFDE : light turquoise
//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// CB24: Blues
static const vec4_t bl8 = { 0.000, 0.188, 0.435, 1.000 };  // #00306F : madison
static const vec4_t bl7 = { 0.000, 0.282, 0.620, 1.000 };  // #00489E : tory blue
static const vec4_t bl6 = { 0.000, 0.373, 0.800, 1.000 };  // #005FCC : royal blue
static const vec4_t bl5 = { 0.000, 0.475, 0.980, 1.000 };  // #0079FA : azure
static const vec4_t bl4 = { 0.000, 0.624, 0.980, 1.000 };  // #009FFA : bleu de france
static const vec4_t bl3 = { 0.000, 0.761, 0.976, 1.000 };  // #00C2F9 : capri
static const vec4_t bl2 = { 0.000, 0.898, 0.973, 1.000 };  // #00E5F8 : aqua blue
static const vec4_t bl1 = { 0.486, 1.000, 0.980, 1.000 };  // #7CFFFA : electric blue
//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// CB24: Greens
static const vec4_t gr8 = { 0.000, 0.251, 0.008, 1.000 };  // #004002 : british racing green
static const vec4_t gr7 = { 0.000, 0.353, 0.004, 1.000 };  // #005A01 : san felix
static const vec4_t gr6 = { 0.000, 0.467, 0.008, 1.000 };  // #007702 : bilbao
static const vec4_t gr5 = { 0.000, 0.584, 0.012, 1.000 };  // #009503 : india green
static const vec4_t gr4 = { 0.000, 0.706, 0.031, 1.000 };  // #00B408 : kelly green
static const vec4_t gr3 = { 0.000, 0.827, 0.008, 1.000 };  // #00D302 : vivid harlequin
static const vec4_t gr2 = { 0.000, 0.957, 0.027, 1.000 };  // #00F407 : radioactive green
static const vec4_t gr1 = { 0.686, 1.000, 0.165, 1.000 };  // #AFFF2A : lime
//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Grayscale
static const vec4_t k00 = { 0.000, 0.000, 0.000, 1.000 };  // #000000 : black
static const vec4_t k05 = { 0.051, 0.051, 0.051, 1.000 };  // #0D0D0D : gray05
static const vec4_t k10 = { 0.102, 0.102, 0.102, 1.000 };  // #1A1A1A : gray10
static const vec4_t k15 = { 0.149, 0.149, 0.149, 1.000 };  // #262626 : gray15
static const vec4_t k20 = { 0.200, 0.200, 0.200, 1.000 };  // #333333 : gray20
static const vec4_t k25 = { 0.251, 0.251, 0.251, 1.000 };  // #404040 : gray25
static const vec4_t k30 = { 0.302, 0.302, 0.302, 1.000 };  // #4D4D4D : gray30
static const vec4_t k35 = { 0.349, 0.349, 0.349, 1.000 };  // #595959 : gray35
static const vec4_t k40 = { 0.400, 0.400, 0.400, 1.000 };  // #666666 : gray40
static const vec4_t k45 = { 0.451, 0.451, 0.451, 1.000 };  // #737373 : gray45
static const vec4_t k50 = { 0.502, 0.502, 0.502, 1.000 };  // #808080 : gray50
static const vec4_t k55 = { 0.549, 0.549, 0.549, 1.000 };  // #8C8C8C : gray55
static const vec4_t k60 = { 0.600, 0.600, 0.600, 1.000 };  // #999999 : gray60
static const vec4_t k65 = { 0.651, 0.651, 0.651, 1.000 };  // #A6A6A6 : gray65
static const vec4_t k70 = { 0.702, 0.702, 0.702, 1.000 };  // #B3B3B3 : gray70
static const vec4_t k75 = { 0.749, 0.749, 0.749, 1.000 };  // #BFBFBF : gray75
static const vec4_t k80 = { 0.800, 0.800, 0.800, 1.000 };  // #CCCCCC : gray80
static const vec4_t k85 = { 0.851, 0.851, 0.851, 1.000 };  // #D9D9D9 : gray85
static const vec4_t k90 = { 0.902, 0.902, 0.902, 1.000 };  // #E6E6E6 : gray90
static const vec4_t k95 = { 0.949, 0.949, 0.949, 1.000 };  // #F2F2F2 : gray95
static const vec4_t wht = { 1.000, 1.000, 1.000, 1.000 };  // #FFFFFF : white
//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
