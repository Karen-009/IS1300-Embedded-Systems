#ifndef __SSD1306_TEST_H__
#define __SSD1306_TEST_H__

#include <_ansi.h>

_BEGIN_STD_C

/* Constants */
#define NUM_COLUMNS 8   // Number of columns in the animation

/* Existing SSD1306 test functions */
void ssd1306_TestBorder(void);
void ssd1306_TestFonts1(void);
void ssd1306_TestFonts2(void);
void ssd1306_TestFPS(void);
void ssd1306_TestAll(void);
void ssd1306_TestLine(void);
void ssd1306_TestRectangle(void);
void ssd1306_TestRectangleFill(void);
void ssd1306_TestRectangleInvert(void);
void ssd1306_TestCircle(void);
void ssd1306_TestArc(void);
void ssd1306_TestPolyline(void);
void ssd1306_TestDrawBitmap(void);

/* Animation functions */
void ssd1306_AnimInit(void);
void ssd1306_AnimStep(void);
void ssd1306_AniFrame(int frames[NUM_COLUMNS]);  // New: handles full animation frame

_END_STD_C

#endif // __SSD1306_TEST_H__
