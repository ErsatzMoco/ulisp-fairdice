/*
  Fair dice uLisp extension - Version 1.0 - Dec 2025
  Hartmut Grawe - github.com/ersatzmoco - Dec 2025

  Licensed under the MIT license: https://opensource.org/licenses/MIT
*/

#include <Adafruit_GFX.h>
#include <Adafruit_IS31FL3731.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>

Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);

typedef struct ulmatrix {
      int mnum = -1;
      int adr;
      Adafruit_IS31FL3731 matrix;
    };

static ulmatrix matrixlist[4];

static int compmatrix[16][18];

static float sinarr[360];
static float cosarr[360];

/*
  (random-seed [seed])
  Initialize pseudo-random algorithm using number seed. If seed is omitted,
  analogRead(27) sets the seed using noise from uninitialized ADC pin (RP2040 only).
*/
object *fn_RandomSeed (object *args, object *env) {
  (void) env;

  unsigned long mys = micros() * analogRead(27);
  if (args != NULL) {
   mys = checkinteger(first(args));
  } 
  randomSeed(mys);

  return nil;
}



/*
  3-axis sensor functions (ADXL345)
*/
/*
  (accel-begin)
  Initialize ADXL345 using I2C and standard address (#x53).
*/
object *fn_AccelBegin (object *args, object *env) {
  (void) args, (void) env;

  if(!accel.begin()) {
    return nil;
  }
  return tee;
}

/*
  (accel-set-range g)
  Set g range for the accelerometer (2, 4, 8 or 16).
*/
object *fn_AccelSetRange (object *args, object *env) {
  (void) env;

  int range = checkinteger(first(args));
  if (range != 2 && range != 4 && range != 8 && range != 16) {
    pfstring("Range must be 2, 4, 8 or 16", (pfun_t)pserial);
    return nil;
  }

  switch (range) {
    case 2:
      accel.setRange(ADXL345_RANGE_2_G);
      break;
    case 4:
      accel.setRange(ADXL345_RANGE_4_G);
      break;
    case 8:
      accel.setRange(ADXL345_RANGE_8_G);
      break;
    case 16:
      accel.setRange(ADXL345_RANGE_16_G);
      break;
  }
  return nil;
}

/*
  (accel-get-event)
  Return list with most recent sensor event data.
*/
object *fn_AccelGetEvent (object *args, object *env) {
  (void) args, (void) env;

  sensors_event_t event;
  accel.getEvent(&event);

  object* ax = makefloat(event.acceleration.x);
  object* ay = makefloat(event.acceleration.y);
  object* az = makefloat(event.acceleration.z);

  return cons(ax, cons(ay, cons(az, NULL)));
}



/*
  Charlieplexed LED-Matrix functions
  Up to 4 matrices, optionally with different I2C addresses, may be stored and addressed using integer numbers.
*/
/*
  (led-matrix-begin [mnum = 0] [adr = #x74])
  Initialize the LED matrix driver. Optionally assign it number mnum, optionally use I2C address adr.
*/
object *fn_LEDMatrixBegin (object *args, object *env) {
  (void) env;

  int mnum = 0;
  if (args != NULL) {
    mnum = checkinteger(first(args));   // zero based index!;
    mnum = constrain(mnum, 0, 3);
    args = cdr(args);
  }

  int adr = ISSI_ADDR_DEFAULT;
  if (args != NULL) {
    adr = checkinteger(first(args));
  }

  if (matrixlist[mnum].mnum == -1) {
    matrixlist[mnum].mnum = mnum;
    matrixlist[mnum].adr = adr;
    matrixlist[mnum].matrix = Adafruit_IS31FL3731();
  }
  else {
    pfstring("matrix number already in use!", (pfun_t)pserial);
    return nil;
  }

  if (!matrixlist[mnum].matrix.begin(adr)) {
    pfstring("matrix module not found!", (pfun_t)pserial);
    matrixlist[mnum].mnum = -1;
    return nil;
  }

  return tee;
}

/*
  (led-matrix-draw-pixel x y b [mnum = 0])
  Set LED at position x, y to brightness b. Optionally use matrix module number mnum.
*/
object *fn_LEDMatrixDrawPixel (object *args, object *env) {
  (void) env;

  uint16_t params[3];
  for (int i=0; i<3; i++) { params[i] = checkinteger(car(args)); args = cdr(args); }

  int mnum = 0;
  if (args != NULL) {
    mnum = checkinteger(first(args));   // zero based index!;
    mnum = constrain(mnum, 0, 3);
  }

  if (matrixlist[mnum].mnum != -1) {
    matrixlist[mnum].matrix.drawPixel(params[0], params[1], params[2]);
    return tee;
  }
  else {
    pfstring("Matrix not found", (pfun_t)pserial);
    return nil;
  }
}

/*
  (led-matrix-set-frame f [mnum = 0])
  Direct all graphics commands to frame buffer f. Optionally use matrix module number mnum.
*/
object *fn_LEDMatrixSetFrame (object *args, object *env) {
  (void) env;
  int f = checkinteger(first(args));
  args = cdr(args);
  f = constrain(f, 0, 7);

  int mnum = 0;
  if (args != NULL) {
    mnum = checkinteger(first(args));   // zero based index!;
    mnum = constrain(mnum, 0, 3);
  }

  if (matrixlist[mnum].mnum != -1) {
    matrixlist[mnum].matrix.setFrame(f);
    return tee;
  }
  else {
    pfstring("Matrix not found", (pfun_t)pserial);
    return nil;
  }
}

/*
  (led-matrix-display-frame f [mnum = 0])
  Display frame buffer f. Optionally use matrix module number mnum.
*/
object *fn_LEDMatrixDisplayFrame (object *args, object *env) {
  (void) env;
  int f = checkinteger(first(args));
  args = cdr(args);
  f = constrain(f, 0, 7);

  int mnum = 0;
  if (args != NULL) {
    mnum = checkinteger(first(args));   // zero based index!;
    mnum = constrain(mnum, 0, 3);
  }

  if (matrixlist[mnum].mnum != -1) {
    matrixlist[mnum].matrix.displayFrame(f);
    return tee;
  }
  else {
    pfstring("Matrix not found", (pfun_t)pserial);
    return nil;
  }
}

/*
  (led-matrix-clear [mnum = 0])
  Clear LED matrix. Optionally use matrix module number mnum.
*/
object *fn_LEDMatrixClear (object *args, object *env) {
  (void) env;

  int mnum = 0;
  if (args != NULL) {
    mnum = checkinteger(first(args));   // zero based index!;
    mnum = constrain(mnum, 0, 3);
  }

  if (matrixlist[mnum].mnum != -1) {
    matrixlist[mnum].matrix.clear();
    return tee;
  }
  else {
    pfstring("Matrix not found", (pfun_t)pserial);
    return nil;
  }
}

/*
  (led-matrix-set-rotation rot [mnum = 0])
  Set rotation of matrix.
  0 = no rotation, 1 = 90 degrees, 2 = 180 degrees, 3 = 270 degrees,
  4 = hor. mirrored, 5 = vert. mirrored.
  Optionally use matrix module number mnum.
*/
object *fn_LEDMatrixSetRotation (object *args, object *env) {
  (void) env;
  int r = checkinteger(first(args));
  args = cdr(args);
  r = constrain(r, 0, 5);

  int mnum = 0;
  if (args != NULL) {
    mnum = checkinteger(first(args));   // zero based index!;
  }

  if (matrixlist[mnum].mnum != -1) {
    matrixlist[mnum].matrix.setRotation(r);
    mnum = constrain(mnum, 0, 3);
    return tee;
  }
  else {
    pfstring("Matrix not found", (pfun_t)pserial);
    return nil;
  }
}

/*
  (led-matrix-set-text-color br [mnum = 0])
  Set brightness br for text (range 0-255). Optionally use matrix module number mnum.
*/
object *fn_LEDMatrixSetTextColor (object *args, object *env) {
  (void) env;
  int c = checkinteger(first(args));
  args = cdr(args);
  c = constrain(c, 0, 255);

  int mnum = 0;
  if (args != NULL) {
    mnum = checkinteger(first(args));   // zero based index!;
    mnum = constrain(mnum, 0, 3);
  }

  if (matrixlist[mnum].mnum != -1) {
    matrixlist[mnum].matrix.setTextColor(c);
    return tee;
  }
  else {
    pfstring("Matrix not found", (pfun_t)pserial);
    return nil;
  }
}

/*
  (led-matrix-set-text-size mf [mnum = 0])
  Set text magnification factor. Optionally use matrix module number mnum.
*/
object *fn_LEDMatrixSetTextSize (object *args, object *env) {
  (void) env;
  int mf = checkinteger(first(args));
  args = cdr(args);
  mf = constrain(mf, 0, 4);

  int mnum = 0;
  if (args != NULL) {
    mnum = checkinteger(first(args));   // zero based index!;
    mnum = constrain(mnum, 0, 3);
  }

  if (matrixlist[mnum].mnum != -1) {
    matrixlist[mnum].matrix.setTextSize(mf);
    return tee;
  }
  else {
    pfstring("Matrix not found", (pfun_t)pserial);
    return nil;
  }
}

/*
  (led-matrix-set-text-wrap w [mnum = 0])
  Set text wrap false/true. Optionally use matrix module number mnum.
*/
object *fn_LEDMatrixSetTextWrap (object *args, object *env) {
  (void) env;

  bool w = (first(args) == nil) ? false : true;

  int mnum = 0;
  if (args != NULL) {
    mnum = checkinteger(first(args));   // zero based index!;
    mnum = constrain(mnum, 0, 3);
  }

  if (matrixlist[mnum].mnum != -1) {
    matrixlist[mnum].matrix.setTextWrap(w);
    return tee;
  }
  else {
    pfstring("Matrix not found", (pfun_t)pserial);
    return nil;
  }
}

/*
  (led-matrix-set-cursor x y [mnum = 0])
  Set text cursor to position x y. Optionally use matrix module number mnum.
*/
object *fn_LEDMatrixSetCursor (object *args, object *env) {
  (void) env;

  uint16_t params[2];
  for (int i=0; i<2; i++) { params[i] = checkinteger(car(args)); args = cdr(args); }

  int mnum = 0;
  if (args != NULL) {
    mnum = checkinteger(first(args));   // zero based index!;
    mnum = constrain(mnum, 0, 3);
  }

  if (matrixlist[mnum].mnum != -1) {
    matrixlist[mnum].matrix.setCursor(params[0], params[1]);
    return tee;
  }
  else {
    pfstring("Matrix not found", (pfun_t)pserial);
    return nil;
  }
}

/*
  (led-matrix-draw-char x y c fg bg mf [mnum = 0])
  Draw char c to screen at location x y with foreground/background brightness fg/bg and size mf.
  Optionally use matrix module number mnum.
*/
object *fn_LEDMatrixDrawChar (object *args, object *env) {
  (void) env;

  uint16_t params[6];
  for (int i=0; i<6; i++) { params[i] = checkinteger(car(args)); args = cdr(args); }

  int mnum = 0;
  if (args != NULL) {
    mnum = checkinteger(first(args));   // zero based index!;
    mnum = constrain(mnum, 0, 3);
  }

  if (matrixlist[mnum].mnum != -1) {
    matrixlist[mnum].matrix.drawChar(params[0], params[1], params[2], params[3], params[4], params[5]);
    return tee;
  }
  else {
    pfstring("Matrix not found", (pfun_t)pserial);
    return nil;
  }
}

/*
  (led-matrix-print str [mnum = 0])
  Print str to screen at location x y using text brightness.
  Optionally use matrix module number mnum.
*/
object *fn_LEDMatrixPrint (object *args, object *env) {
  (void) env;
  
  int slength = stringlength(checkstring(first(args)))+1;
  char *matrixbuf = (char*)malloc(slength);
  cstring(first(args), matrixbuf, slength);
  args = cdr(args);

int mnum = 0;
  if (args != NULL) {
    mnum = checkinteger(first(args));   // zero based index!;
    mnum = constrain(mnum, 0, 3);
  }

  if (matrixlist[mnum].mnum != -1) {
    matrixlist[mnum].matrix.print(matrixbuf);
    free(matrixbuf);
    return tee;
  }
  else {
    pfstring("Matrix not found", (pfun_t)pserial);
    free(matrixbuf);
    return nil;
  }
}

/*
  (led-matrix-draw-line x0 y0 x1 y1 br [mnum = 0])
  Draw a line between positions x0/y0 and x1/y1 with brightness br (range 0-255).
  Optionally use matrix module number mnum.
*/
object *fn_LEDMatrixDrawLine (object *args, object *env) {
  (void) env;

  uint16_t params[5];
  for (int i=0; i<5; i++) { params[i] = checkinteger(car(args)); args = cdr(args); }

int mnum = 0;
  if (args != NULL) {
    mnum = checkinteger(first(args));   // zero based index!;
    mnum = constrain(mnum, 0, 3);
  }

  if (matrixlist[mnum].mnum != -1) {
    matrixlist[mnum].matrix.drawLine(params[0], params[1], params[2], params[3], params[4]);
    return tee;
  }
  else {
    pfstring("Matrix not found", (pfun_t)pserial);
    return nil;
  }
}

/*
  (led-matrix-draw-rect x y w h br [mnum = 0])
  Draw empty rectangle at x y with width w, height h and brightness br (range 0-255).
  Optionally use matrix module number mnum.
*/
object *fn_LEDMatrixDrawRect (object *args, object *env) {
  (void) env;

  uint16_t params[5];
  for (int i=0; i<5; i++) { params[i] = checkinteger(car(args)); args = cdr(args); }

  int mnum = 0;
  if (args != NULL) {
    mnum = checkinteger(first(args));   // zero based index!;
    mnum = constrain(mnum, 0, 3);
  }

  if (matrixlist[mnum].mnum != -1) {
    matrixlist[mnum].matrix.drawRect(params[0], params[1], params[2], params[3], params[4]);
    return tee;
  }
  else {
    pfstring("Matrix not found", (pfun_t)pserial);
    return nil;
  }
}

/*
  (led-matrix-fill-rect x y w h br [mnum = 0])
  Draw filled rectangle at x y with width w, height h and brightness br (range 0-255).
  Optionally use matrix module number mnum.
*/
object *fn_LEDMatrixFillRect (object *args, object *env) {
  (void) env;
  
  uint16_t params[5];
  for (int i=0; i<5; i++) { params[i] = checkinteger(car(args)); args = cdr(args); }

  int mnum = 0;
  if (args != NULL) {
    mnum = checkinteger(first(args));   // zero based index!;
    mnum = constrain(mnum, 0, 3);
  }

  if (matrixlist[mnum].mnum != -1) {
    matrixlist[mnum].matrix.fillRect(params[0], params[1], params[2], params[3], params[4]);
    return tee;
  }
  else {
    pfstring("Matrix not found", (pfun_t)pserial);
    return nil;
  }
}

/*
  (led-matrix-draw-circle x y r br [mnum = 0])
  Draw empty circle at position x y with radius r and brightness br (range 0-255).
  Optionally use matrix module number mnum.
*/
object *fn_LEDMatrixDrawCircle (object *args, object *env) {
  (void) env;
  
  uint16_t params[4];
  for (int i=0; i<4; i++) { params[i] = checkinteger(car(args)); args = cdr(args); }

  int mnum = 0;
  if (args != NULL) {
    mnum = checkinteger(first(args));   // zero based index!;
    mnum = constrain(mnum, 0, 3);
  }

  if (matrixlist[mnum].mnum != -1) {
    matrixlist[mnum].matrix.drawCircle(params[0], params[1], params[2], params[3]);
    return tee;
  }
  else {
    pfstring("Matrix not found", (pfun_t)pserial);
    return nil;
  }
}

/*
  (led-matrix-fill-circle x y r br [mnum = 0])
  Draw filled circle at position x y with radius r and brightness br (range 0-255).
  Optionally use matrix module number mnum.
*/
object *fn_LEDMatrixFillCircle (object *args, object *env) {
  (void) env;
  
  uint16_t params[4];
  for (int i=0; i<4; i++) { params[i] = checkinteger(car(args)); args = cdr(args); }

  int mnum = 0;
  if (args != NULL) {
    mnum = checkinteger(first(args));   // zero based index!;
    mnum = constrain(mnum, 0, 3);
  }

  if (matrixlist[mnum].mnum != -1) {
    matrixlist[mnum].matrix.fillCircle(params[0], params[1], params[2], params[3]);
    return tee;
  }
  else {
    pfstring("Matrix not found", (pfun_t)pserial);
    return nil;
  }
}

/*
  (led-matrix-draw-round-rect x y w h r br [mnum = 0])
  Draw empty rectangle at x y with width w, height h and brightness br (range 0-255).
  Edges are rounded with radius r.
  Optionally use matrix module number mnum.
*/
object *fn_LEDMatrixDrawRoundRect (object *args, object *env) {
  (void) env;
  
  uint16_t params[6];
  for (int i=0; i<6; i++) { params[i] = checkinteger(car(args)); args = cdr(args); }

  int mnum = 0;
  if (args != NULL) {
    mnum = checkinteger(first(args));   // zero based index!;
    mnum = constrain(mnum, 0, 3);
  }

  if (matrixlist[mnum].mnum != -1) {
    matrixlist[mnum].matrix.drawRoundRect(params[0], params[1], params[2], params[3], params[4], params[5]);
    return tee;
  }
  else {
    pfstring("Matrix not found", (pfun_t)pserial);
    return nil;
  }
}

/*
  (led-matrix-fill-round-rect x y w h r br [mnum = 0])
  Draw filled rectangle at x y with width w, height h and brightness br (range 0-255).
  Edges are rounded with radius r.
  Optionally use matrix module number mnum.
*/
object *fn_LEDMatrixFillRoundRect (object *args, object *env) {
  (void) env;
  
  uint16_t params[6];
  for (int i=0; i<6; i++) { params[i] = checkinteger(car(args)); args = cdr(args); }

  int mnum = 0;
  if (args != NULL) {
    mnum = checkinteger(first(args));   // zero based index!;
    mnum = constrain(mnum, 0, 3);
  }

  if (matrixlist[mnum].mnum != -1) {
    matrixlist[mnum].matrix.fillRoundRect(params[0], params[1], params[2], params[3], params[4], params[5]);
    return tee;
  }
  else {
    pfstring("Matrix not found", (pfun_t)pserial);
    return nil;
  }
}

/*
  (led-matrix-draw-triangle x0 y0 x1 y1 x2 y2 br [mnum = 0])
  Draw empty triangle with corners at x0/y0, x1/y1, x2/y2 and brightness br (range 0-255).
  Optionally use matrix module number mnum.
*/
object *fn_LEDMatrixDrawTriangle (object *args, object *env) {
  (void) env;
  
  uint16_t params[7];
  for (int i=0; i<7; i++) { params[i] = checkinteger(car(args)); args = cdr(args); }

  int mnum = 0;
  if (args != NULL) {
    mnum = checkinteger(first(args));   // zero based index!;
    mnum = constrain(mnum, 0, 3);
  }

  if (matrixlist[mnum].mnum != -1) {
    matrixlist[mnum].matrix.drawTriangle(params[0], params[1], params[2], params[3], params[4], params[5], params[6]);
    return tee;
  }
  else {
    pfstring("Matrix not found", (pfun_t)pserial);
    return nil;
  }
}

/*
  (led-matrix-fill-triangle x0 y0 x1 y1 x2 y2 br [mnum = 0])
  Draw filled triangle with corners at x0/y0, x1/y1, x2/y2 and brightness br (range 0-255).
  Optionally use matrix module number mnum.
*/
object *fn_LEDMatrixFillTriangle (object *args, object *env) {
  (void) env;
  
  uint16_t params[7];
  for (int i=0; i<7; i++) { params[i] = checkinteger(car(args)); args = cdr(args); }

  int mnum = 0;
  if (args != NULL) {
    mnum = checkinteger(first(args));   // zero based index!;
    mnum = constrain(mnum, 0, 3);
  }

  if (matrixlist[mnum].mnum != -1) {
    matrixlist[mnum].matrix.fillTriangle(params[0], params[1], params[2], params[3], params[4], params[5], params[6]);
    return tee;
  }
  else {
    pfstring("Matrix not found", (pfun_t)pserial);
    return nil;
  }
}


/*
  (load-mono fname arr [offx] [offy])
  Open monochrome BMP file fname from SD if it exits and copy it into the two-dimensional uLisp bit array provided.
  When the image is larger than the array, only the upper leftmost area of the bitmap fitting into the array is loaded.
  Providing offx and offy you may move the "window" of the array to other parts of the bitmap (useful e.g. for tiling).
*/

object *fn_LoadMono (object *args, object *env) {

  SD.begin(SDCARD_SS_PIN);

  int slength = stringlength(checkstring(first(args)))+1;
  char* fnbuf = (char*)malloc(slength);
  cstring(first(args), fnbuf, slength);
  File file;

  if (!SD.exists(fnbuf)) {
    pfstring("File not found", (pfun_t)pserial);
    free(fnbuf);
    return nil;
  }
  object* array = second(args);
  if (!arrayp(array)) {
    pfstring("Argument is not an array", (pfun_t)pserial);
    free(fnbuf);
    return nil;
  }
  object* dimensions = cddr(array);
  if (listp(dimensions)) {
    if (listlength(dimensions) != 2) {
      pfstring("Array must be two-dimensional", (pfun_t)pserial);
      free(fnbuf);
      return nil;
    }
  }
  else {
    pfstring("Array must be two-dimensional", (pfun_t)pserial);
      free(fnbuf);
      return nil;
  }
  args = cddr(args);
  int offx = 0;
  int offy = 0;

  if (args != NULL) {
    offx = checkinteger(car(args));
    args = cdr(args);
    if (args != NULL) {
      offy = checkinteger(car(args));
    }
  }
  (void) args;

  int aw = abs(first(dimensions)->integer);
  int ah = abs(second(dimensions)->integer);
  int bit;
  object* subscripts;
  object* ox;
  object* oy;
  object* oyy;
  object** element;

  char buffer[BUFFERSIZE];
  file = SD.open(fnbuf);
  if (!file) { 
    pfstring("Problem reading from SD card", (pfun_t)pserial);
    free(fnbuf);
    return nil;
  }

  char b = file.read();
  char m = file.read();
  if ((m != 77) || (b != 66)) {
    pfstring("No BMP file", (pfun_t)pserial);
    free(fnbuf);
    return nil;
  }

  file.seek(10);
  uint32_t offset = SDRead32(file);
  SDRead32(file);
  int32_t width = SDRead32(file);
  int32_t height = SDRead32(file);
  int linebytes = floor(width / 8);
  int restbits = width % 8;
  if (restbits > 0) linebytes++;
  int zpad = 0;
  if ((linebytes % 4) > 0) {
    zpad = (4 - (linebytes % 4));
  }

  file.seek(28);
  uint16_t depth = file.read();
  if (depth > 1) { 
    pfstring("No monochrome bitmap file", (pfun_t)pserial);
    free(fnbuf);
    return nil;
  }

  file.seek(offset);

  int lx = 0;
  int bmpbyte = 0;
  int bmpbit = 0;

  for (int ly = (height - 1); ly >= 0; ly--) {
    for (int bx = 0; bx < linebytes; bx++) {
      bmpbyte = file.read();
      for (int bix = 0; bix < 8; bix++) {
        lx = (bx * 8) + bix;
        if ((lx < (aw+offx)) && (ly < (ah+offy)) && (lx >= offx) && (ly >= offy)) {
          ox = number(lx-offx);
          oy = number(ly-offy);
          oyy = cons(oy, NULL);
          subscripts = cons(ox, oyy);
          element = getarray(array, subscripts, env, &bit);

          bmpbit = bmpbyte & (1 << (7-bix));
          if (bmpbit > 0) {
            bmpbit = 1;
          }
          else {
            bmpbit = 0;
          }
          *element = number((checkinteger(*element) & ~(1<<bit)) | bmpbit<<bit);

          myfree(subscripts);
          myfree(oyy);
          myfree(oy);
          myfree(ox);
        }
      }
    }
    //ignore trailing zero bytes
    if (zpad > 0) {
      for (int i = 0; i < zpad; i++) {
        file.read();
      }
    }
  }

  file.close();
  free(fnbuf);
  return nil;
}


/*
  Combined LED matrix commands
  Matrices must be numbered as 0 and 1. Use "canvas-begin" to ensure creation of valid canvas and brightness compensation.
*/

/* 
  (canvas-begin adr0 adr1)
  Initialize two LED matrices at I2C adresses adr0 and adr1 and assign them to matrix numbers 0 and 1.
  Create brightness compensation array for tilted matrices.
*/
object *fn_CanvasBegin (object *args, object *env) {
  (void) env;

  int adr[2] = {checkinteger(first(args)), checkinteger(second(args))};

  for (int m = 0; m < 2; m++) {
    if (matrixlist[m].mnum == -1) {
      matrixlist[m].mnum = 0;
      matrixlist[m].adr = adr[m];
      matrixlist[m].matrix = Adafruit_IS31FL3731();
    }
    else {
      pfstring("matrix number already in use!", (pfun_t)pserial);
      return nil;
    }

    if (!matrixlist[m].matrix.begin(adr[m])) {
      pfstring("matrix module not found!", (pfun_t)pserial);
      matrixlist[m].mnum = -1;
      return nil;
    }
  }

  //fill brightness compensation array
  for (int x = 0; x < 16; x++) {
    for (int y = 0; y < 9; y++) {
      compmatrix[x][y] = 10 + round(y * 1.2);
      compmatrix[x][17-y] = 10 + round(y * 1.2);
    }
  }

  //fill cos and sin arrays
  for (int a = 0; a < 360; a++) {
    sinarr[a] = sin(a * DEG_TO_RAD);
    cosarr[a] = cos(a * DEG_TO_RAD);
  }

  return tee;
}

/*
  (canvas-draw-pixel x y b)
  Draw pixel on two combined LED matrices at position x y with brightness b.
*/
object *fn_CanvasDrawPixel (object *args, object *env) {
  (void) env;

  int x = 0, y = 0, b = 0;
  x = checkinteger(first(args));
  y = checkinteger(second(args));
  b = checkinteger(third(args));

  if (y < 9) {
    matrixlist[0].matrix.drawPixel(x, y, b);
  }
  else {
    matrixlist[1].matrix.drawPixel(x, (y % 9), b);
  }

  return nil;
}

/*
  (canvas-clear)
  Clear current frames of combined LED matrices.
*/
object *fn_CanvasClear (object *args, object *env) {
  (void) args, (void) env;

  matrixlist[0].matrix.clear();
  matrixlist[1].matrix.clear();

  return nil;
}

/*
  (canvas-clear-all)
  Clear all frames of combined LED matrices.
*/
object *fn_CanvasClearAll (object *args, object *env) {
  (void) args, (void) env;

  for (int f = 0; f < 8; f++) {
    matrixlist[0].matrix.setFrame(f);
    matrixlist[0].matrix.clear();
    matrixlist[1].matrix.setFrame(f);
    matrixlist[1].matrix.clear();
  }

  matrixlist[0].matrix.setFrame(0);
  matrixlist[1].matrix.setFrame(0);
  matrixlist[0].matrix.displayFrame(0);
  matrixlist[1].matrix.displayFrame(0);
  return nil;
}

/*
  (canvas-set-frame f)
  Set current frame of combined LED matrices to f.
*/
object *fn_CanvasSetFrame (object *args, object *env) {
  (void) env;

  int f = checkinteger(first(args));
  matrixlist[0].matrix.setFrame(f);
  matrixlist[1].matrix.setFrame(f);

  return nil;
}

/*
  (canvas-display-frame f)
  Display frame f of combined LED matrices.
*/
object *fn_CanvasDisplayFrame (object *args, object *env) {
  (void) env;

  int f = checkinteger(first(args));
  matrixlist[0].matrix.displayFrame(f);
  matrixlist[1].matrix.displayFrame(f);

  return nil;
}

/*
  (canvas-draw-array arr comp [bright] [offx] [offy])
  Draw uLisp array arr on two combined LED matrices. If comp is true, use brightness compensation.
  Optionally draw array using offset offx and offy. The function automatically distinguishes between
  integer and bit arrays. If a bit array is provided, the pixels are drawn using brightness bright.
  Otherwise the integer value at an array position determines the brightness of the pixel.
*/
object *fn_CanvasDrawArray (object *args, object *env) {

  object* array = first(args);
  if (!arrayp(array)) error2("argument is not an array");

  object *dimensions = cddr(array);
  if (listp(dimensions)) {
    if (listlength(dimensions) != 2) error2("array must be two-dimensional");
  }
  else error2("array must be two-dimensional");
  
  bool comp = false;
  args = cdr(args);
  if (args != NULL) {
    comp = (first(args) == nil) ? false : true;
    args = cdr(args);
  }

  int bright = 0;
  if (args != NULL) {
    bright = checkinteger(first(args));
    args = cdr(args);
  } 

  int x = 0, y = 0, offx = 0, offy = 0;
  if (args != NULL) {
    offx = checkinteger(first(args));
    args = cdr(args);
  } 
  if (args != NULL) {
    offy = checkinteger(first(args));
  }

  (void) args;

  int bmpbit = 0;
  int aw = abs(first(dimensions)->integer);
  int ah = abs(second(dimensions)->integer);
  int bit;
  object* subscripts;
  object* ox;
  object* oy;
  object* oyy;
  object** element;

  int pixel;

  for (int ay = 0; ay < ah; ay++) {
    for (int ax = 0; ax < aw; ax++) {
      ox = number(ax);
      oy = number(ay);
      oyy = cons(oy, NULL);
      subscripts = cons(ox, oyy);
      element = getarray(array, subscripts, env, &bit);
      if (bit < 0) {
        pixel = bright + checkinteger(*element);
      }
      else {
        bmpbit = abs(checkinteger(*element) & (1<<bit));
        if (bmpbit > 0) {
          pixel = bright;
        }
        else {
          pixel = 0;
        }
      }
      myfree(subscripts);
      myfree(oyy);
      myfree(oy);
      myfree(ox);
    
      x = ax + offx;
      y = ay + offy;

      if ((x < 16) && (y < 18)) {

        if (comp && (pixel > 0)) {
          pixel = pixel + compmatrix[x][y];
        }

        if (y < 9) {
          matrixlist[0].matrix.drawPixel(x, y, pixel);
        }
        else {
          matrixlist[1].matrix.drawPixel(x, (y % 9), pixel);
        }
      }
    }
  }

  return nil;
}

/*
  (canvas-transform source target angles vect cz)
  Rotate 2D image array source around all three axes using list angles, 
  translate it using list vect and project it to target area using observer z position cz.
  Angles given in degrees (int), result written into target array (omitting points not set,
  so target array needs to be empty).
*/
object *fn_CanvasTransform (object *args, object *env) {

  object* source = first(args);
  if (!arrayp(source)) error2("argument is not an array");
  object *sdim = cddr(source);
  if (listp(sdim)) {
    if (listlength(sdim) != 2) error2("array must be two-dimensional");
  }
  else error2("array must be two-dimensional");

  object* target = second(args);
  if (!arrayp(target)) error2("argument is not an array");
  object *tdim = cddr(target);
  if (listp(tdim)) {
    if (listlength(tdim) != 2) error2("array must be two-dimensional");
  }
  else error2("array must be two-dimensional");
  
  args = cdr(args);
  args = cdr(args);

  //retrieve rotation angles around x, y, z axes
  int ra, rb, rc = 0;
  ra = checkinteger(car(first(args))) % 360;
  rb = checkinteger(car(cdr(first(args)))) % 360;
  rc = checkinteger(car(cddr(first(args)))) % 360;

  args = cdr(args);

  //retrieve translation values
  int ta, tb, tc = 0;
  ta = checkinteger(car(first(args)));
  tb = checkinteger(car(cdr(first(args))));
  tc = checkinteger(car(cddr(first(args))));

  args = cdr(args);

  //store sin and cos values for all three angles
  float sina = sinarr[ra];
  float cosa = cosarr[ra];

  float sinb = sinarr[rb];
  float cosb = cosarr[rb];

  float sinc = sinarr[rc];
  float cosc = cosarr[rc];

  //calculate matrix factors
  //z component of 2D source is always 0
  float A = cosb * cosc;
  float B = cosb * sinc;
  //float C = -sinb;
  float D = sina * sinb * cosc - cosa * sinc;
  float E = sina * sinb * sinc + cosa * cosc;
  //float F = sina * cosb;
  float G = cosa * sinb * cosc + sina * sinc;
  float H = cosa * sinb * sinc - sina * cosc;
  //float I = cosa * cosb;

  //set position of camera/observer
  int cx = 0;
  int cy = 0;
  int cz = checkinteger(first(args));

  //projected point position
  int px = 0, py = 0;

  //retrieve points from source array, dimensions of target array
  int sw = abs(first(sdim)->integer);
  int sh = abs(second(sdim)->integer);
  int swh = (int)(sw/2);
  int shh = (int)(sh/2);
  int sx = 0, sy = 0;

  int tw = abs(first(tdim)->integer);
  int th = abs(second(tdim)->integer);
  int twh = (int)(tw/2);
  int thh = (int)(th/2);

  int bmpbit = 0;
  int bit;
  int pixel;
  int rx = 0, ry = 0, rz = 0;
  object* subscripts;
  object* ox;
  object* oy;
  object* oyy;
  object** r_element;
  object** w_element;

  for (int ay = 0; ay < sh; ay++) {
    for (int ax = 0; ax < sw; ax++) {
      ox = number(ax);
      oy = number(ay);
      oyy = cons(oy, NULL);
      subscripts = cons(ox, oyy);
      r_element = getarray(source, subscripts, env, &bit);
      if (bit < 0) {
        pixel = checkinteger(*r_element);
      }
      else {
        bmpbit = abs(checkinteger(*r_element) & (1<<bit));
        if (bmpbit > 0) {
          pixel = 1;
        }
        else {
          pixel = 0;
        }
      }
      myfree(subscripts);
      myfree(oyy);
      myfree(oy);
      myfree(ox);

      //rotate and poject point only if it is set (> 0)
      if (pixel < 1) continue; // pixel not set -> resume loops

      //transform 2D graphics to origin before rotation
      sx = ax - swh;
      sy = ay - shh;

      //rotate point
      //z component of 2D source is always 0
      rx = sx * A + sy * B; // + z*C;
      ry = sx * D + sy * E; // + z*F;
      rz = sx * G + sy * H; // + z*I;

      //translate point
      rx = rx + ta;
      ry = ry + tb;
      rz = rz + tc;

      //project point (and move center to center of screen)
      px = ((cx*rz-rx*cz)/(rz-cz)) + twh;
      py = ((cy*rz-ry*cz)/(rz-cz)) + thh;

      //clip point: Write pixel to target array if it's visible
      if ((px >= 0) && (px < tw) && (py >= 0) && (py < th)) {
        ox = number(px);
        oy = number(py);
        oyy = cons(oy, NULL);
        subscripts = cons(ox, oyy);
        w_element = getarray(target, subscripts, env, &bit);
        if (bit < 0) {
          *w_element = number(pixel);
        }
        else {
          *w_element = number((checkinteger(*w_element) & ~(1<<bit)) | 1<<bit);
        }
        myfree(subscripts);
        myfree(oyy);
        myfree(oy);
        myfree(ox);
      }
    }
  }
  
  return nil;

}

/*
  (canvas-transform-draw source angles vect cz [comp = nil])
  Rotate 2D image array source around all three axes using list angles, 
  translate it using list vect and project it to canvas using observer z position cz.
  Angles given in degrees (int), result directly drawn to canvas. 
  Optionally use compensation matrix if comp is t.
*/
object *fn_CanvasTransformDraw (object *args, object *env) {

  object* source = first(args);
  if (!arrayp(source)) error2("argument is not an array");
  object *sdim = cddr(source);
  if (listp(sdim)) {
    if (listlength(sdim) != 2) error2("array must be two-dimensional");
  }
  else error2("array must be two-dimensional");
  
  args = cdr(args);

  //retrieve rotation angles around x, y, z axes
  int ra, rb, rc = 0;
  ra = checkinteger(car(first(args))) % 360;
  rb = checkinteger(car(cdr(first(args)))) % 360;
  rc = checkinteger(car(cddr(first(args)))) % 360;

  args = cdr(args);

  //retrieve translation values
  int ta, tb, tc = 0;
  ta = checkinteger(car(first(args)));
  tb = checkinteger(car(cdr(first(args))));
  tc = checkinteger(car(cddr(first(args))));

  args = cdr(args);

  //store sin and cos values for all three angles
  float sina = sinarr[ra];
  float cosa = cosarr[ra];

  float sinb = sinarr[rb];
  float cosb = cosarr[rb];

  float sinc = sinarr[rc];
  float cosc = cosarr[rc];

  //calculate matrix factors
  //z component of 2D source is always 0
  float A = cosb * cosc;
  float B = cosb * sinc;
  //float C = -sinb;
  float D = sina * sinb * cosc - cosa * sinc;
  float E = sina * sinb * sinc + cosa * cosc;
  //float F = sina * cosb;
  float G = cosa * sinb * cosc + sina * sinc;
  float H = cosa * sinb * sinc - sina * cosc;
  //float I = cosa * cosb;

  //set position of camera/observer
  int cx = 0;
  int cy = 0;
  int cz = checkinteger(first(args));

  args = cdr(args);

  bool comp = false;
  if (args != NULL) {
    comp = (first(args) == nil) ? false : true;
  }

  //projected point position
  int px = 0, py = 0;

  //retrieve points from source array
  int sw = abs(first(sdim)->integer);
  int sh = abs(second(sdim)->integer);
  int swh = (int)(sw/2);
  int shh = (int)(sh/2);
  int sx = 0, sy = 0;

  //dimension of canvas
  int tw = 16;
  int th = 18;
  int twh = 8;
  int thh = 9;

  int bmpbit = 0;
  int bit;
  int pixel;
  int rx = 0, ry = 0, rz = 0;
  object* subscripts;
  object* ox;
  object* oy;
  object* oyy;
  object** r_element;
  object** w_element;

  for (int ay = 0; ay < sh; ay++) {
    for (int ax = 0; ax < sw; ax++) {
      ox = number(ax);
      oy = number(ay);
      oyy = cons(oy, NULL);
      subscripts = cons(ox, oyy);
      r_element = getarray(source, subscripts, env, &bit);
      if (bit < 0) {
        pixel = checkinteger(*r_element);
      }
      else {
        bmpbit = abs(checkinteger(*r_element) & (1<<bit));
        if (bmpbit > 0) {
          pixel = 1;
        }
        else {
          pixel = 0;
        }
      }
      myfree(subscripts);
      myfree(oyy);
      myfree(oy);
      myfree(ox);

      //rotate and poject point only if it is set (> 0)
      if (pixel < 1) continue; // pixel not set -> resume loops

      //transform 2D graphics to origin before rotation
      sx = ax - swh;
      sy = ay - shh;

      //rotate point
      //z component of 2D source is always 0
      rx = sx * A + sy * B; // + z*C;
      ry = sx * D + sy * E; // + z*F;
      rz = sx * G + sy * H; // + z*I;

      //translate point
      rx = rx + ta;
      ry = ry + tb;
      rz = rz + tc;

      //project point (and move center to center of screen)
      px = ((cx*rz-rx*cz)/(rz-cz)) + twh;
      py = ((cy*rz-ry*cz)/(rz-cz)) + thh;

      //clip point: Write pixel to target array if it's visible
      if ((px >= 0) && (px < tw) && (py >= 0) && (py < th)) {
        if (comp) {
          pixel = pixel + compmatrix[px][py];
        }

        if (py < 9) {
          matrixlist[0].matrix.drawPixel(px, py, pixel);
        }
        else {
          matrixlist[1].matrix.drawPixel(px, (py % 9), pixel);
        }
      }
    }
  }
  
  return nil;

}

/*
  Vector math functions
*/
/*
  (rad-to-deg n)
  Convert radians to degrees.
*/
object *fn_RadToDeg (object *args, object *env) {
  (void) env;

  return makefloat(checkintfloat(first(args))*RAD_TO_DEG);
}

/*
  (deg-to-rad n)
  Convert degree to radians.
*/
object *fn_DegToRad (object *args, object *env) {
  (void) env;

  return makefloat(checkintfloat(first(args))*DEG_TO_RAD);
}

/*
  (vector-sub v1 v2)
  Subtract vector v2 from vector v1 (two lists of number elements).
*/
object *fn_VectorSub (object *args, object *env) {
  (void) env;

  object *v1 = first(args);
  object *v2 = second(args);
  if (!listp(v1) || !listp(v2)) error2("arguments must be two lists of numbers");

  object *retlist = NULL;
  float a, b = 0;

  while ((v1 != NULL) && (v2 != NULL)) {
    a = checkintfloat(car(v1));
    b = checkintfloat(car(v2));
    retlist = cons(makefloat(a-b), retlist);
    v1 = cdr(v1);
    v2 = cdr(v2);
  }

  return fn_reverse(cons(retlist, NULL), NULL);
}

/*
  (vector-add v1 v2)
  Add vector v2 to vector v1 (two lists of number elements).
*/
object *fn_VectorAdd (object *args, object *env) {
  (void) env;

  object *v1 = first(args);
  object *v2 = second(args);
  if (!listp(v1) || !listp(v2)) error2("arguments must be two lists of numbers");

  object *retlist = NULL;
  float a, b = 0;

  while ((v1 != NULL) && (v2 != NULL)) {
    a = checkintfloat(car(v1));
    b = checkintfloat(car(v2));
    retlist = cons(makefloat(a+b), retlist);
    v1 = cdr(v1);
    v2 = cdr(v2);
  }

  return fn_reverse(cons(retlist, NULL), NULL);
}

/*
  (vector-norm v)
  Calculate magnitude/norm of vector v (list of number elements).
*/
object *fn_VectorNorm (object *args, object *env) {
  (void) env;

  object *v1 = first(args);
  if (!listp(v1)) error2("argument must be a list");

  float a, sum = 0;
  while (v1 != NULL) {
    a = checkintfloat(car(v1));
    sum = sum + (a*a);
    v1 = cdr(v1);
  }  

  return makefloat(sqrt(sum));
}

/*
  (scalar-mult v s)
  Multiply vector v (list of number elements) by number s (scalar).
*/
object *fn_ScalarMult (object *args, object *env) {
  (void) env;

  object *v1 = first(args);
  if (!listp(v1)) error2("first argument must be a list");
  float s = checkintfloat(second(args));

  object *retlist = NULL;
  float a;
  while (v1 != NULL) {
    a = checkintfloat(car(v1));
    retlist = cons(makefloat(a*s), retlist);
    v1 = cdr(v1);
  }

  return fn_reverse(cons(retlist, NULL), NULL);
}

/*
  (dot-product v1 v2)
  Calculate dot product of two vectors v1, v2 (lists of number elements).
*/
object *fn_DotProduct (object *args, object *env) {
  (void) env;

  object *v1 = first(args);
  object *v2 = second(args);
  if (!listp(v1) || !listp(v2)) error2("arguments must be two lists of numbers");

  float a, b, sum;

  while ((v1 != NULL) && (v2 != NULL)) {
    a = checkintfloat(car(v1));
    b = checkintfloat(car(v2));
    sum = sum + a*b;
    v1 = cdr(v1);
    v2 = cdr(v2);
  }

  return makefloat(sum);
}

/*
  (cross-product v1 v2)
  Calculate cross product of two three-dimensional vectors v1, v2 (lists with 3 elements).
*/
object *fn_CrossProduct (object *args, object *env) {
  (void) env;

  if (!listp(first(args)) || !listp(second(args))) error2("arguments must be lists of three numbers");

  float a1 = checkintfloat(car(first(args)));
  float a2 = checkintfloat(car(cdr(first(args))));
  float a3 = checkintfloat(car(cddr(first(args))));

  float b1 = checkintfloat(car(second(args)));
  float b2 = checkintfloat(car(cdr(second(args))));
  float b3 = checkintfloat(car(cddr(second(args))));

  float c1 = a2*b3 - a3*b2;
  float c2 = a3*b1 - a1*b3;
  float c3 = a1*b2 - a2*b1;

  return cons(makefloat(c1), cons(makefloat(c2), cons(makefloat(c3), NULL)));
}

/*
  (vector-angle v1 v2)
  Calculate angle (rad) between two three-dimensional vectors v1, v2 (lists with 3 elements).
*/
object *fn_VectorAngle (object *args, object *env) {
  (void) env;

  if (!listp(first(args)) || !listp(second(args))) error2("arguments must be lists of three numbers");

  float a1 = checkintfloat(car(first(args)));
  float a2 = checkintfloat(car(cdr(first(args))));
  float a3 = checkintfloat(car(cddr(first(args))));

  float b1 = checkintfloat(car(second(args)));
  float b2 = checkintfloat(car(cdr(second(args))));
  float b3 = checkintfloat(car(cddr(second(args))));

  //dot product
  double dot = (a1*b1 + a2*b2 + a3*b3);

  //norms
  double na = sqrt(a1*a1 + a2*a2 + a3*a3);
  double nb = sqrt(b1*b1 + b2*b2 + b3*b3);

  float cphi = dot/(na*nb);

  return makefloat(acos(cphi));
}


// Symbol names
const char stringRandomSeed[] PROGMEM = "random-seed";

// ADXL345 sensor
const char stringAccelBegin[] PROGMEM = "accel-begin";
const char stringAccelSetRange[] PROGMEM = "accel-set-range";
const char stringAccelGetEvent[] PROGMEM = "accel-get-event";

// Charlieplexed LED matrix
const char stringLEDMatrixBegin[] PROGMEM = "led-matrix-begin";
const char stringLEDMatrixDrawPixel[] PROGMEM = "led-matrix-draw-pixel";
const char stringLEDMatrixSetFrame[] PROGMEM = "led-matrix-set-frame";
const char stringLEDMatrixDisplayFrame[] PROGMEM = "led-matrix-display-frame";
const char stringLEDMatrixClear[] PROGMEM = "led-matrix-clear";
const char stringLEDMatrixSetRotation[] PROGMEM = "led-matrix-set-rotation";
const char stringLEDMatrixSetTextColor[] PROGMEM = "led-matrix-set-text-color";
const char stringLEDMatrixSetTextSize[] PROGMEM = "led-matrix-set-text-size";
const char stringLEDMatrixSetTextWrap[] PROGMEM = "led-matrix-set-text-wrap";
const char stringLEDMatrixSetCursor[] PROGMEM = "led-matrix-set-cursor";
const char stringLEDMatrixDrawChar[] PROGMEM = "led-matrix-draw-char";
const char stringLEDMatrixPrint[] PROGMEM = "led-matrix-print";
const char stringLEDMatrixDrawLine[] PROGMEM = "led-matrix-draw-line";
const char stringLEDMatrixDrawRect[] PROGMEM = "led-matrix-draw-rect";
const char stringLEDMatrixFillRect[] PROGMEM = "led-matrix-fill-rect";
const char stringLEDMatrixDrawCircle[] PROGMEM = "led-matrix-draw-circle";
const char stringLEDMatrixFillCircle[] PROGMEM = "led-matrix-fill-circle";
const char stringLEDMatrixDrawRoundRect[] PROGMEM = "led-matrix-draw-round-rect";
const char stringLEDMatrixFillRoundRect[] PROGMEM = "led-matrix-fill-round-rect";
const char stringLEDMatrixDrawTriangle[] PROGMEM = "led-matrix-draw-triangle";
const char stringLEDMatrixFillTriangle[] PROGMEM = "led-matrix-fill-triangle";

const char stringLoadMono[] PROGMEM = "load-mono";

//Canvas consisting of two combined LED matrices named 0 and 1
const char stringCanvasBegin[] PROGMEM = "canvas-begin";
const char stringCanvasDrawPixel[] PROGMEM = "canvas-draw-pixel";
const char stringCanvasClear[] PROGMEM = "canvas-clear";
const char stringCanvasClearAll[] PROGMEM = "canvas-clear-all";
const char stringCanvasSetFrame[] PROGMEM = "canvas-set-frame";
const char stringCanvasDisplayFrame[] PROGMEM = "canvas-display-frame";
const char stringCanvasDrawArray[] PROGMEM = "canvas-draw-array";
const char stringCanvasTransform[] PROGMEM = "canvas-transform";
const char stringCanvasTransformDraw[] PROGMEM = "canvas-transform-draw";

// Documentation strings
const char docRandomSeed[] PROGMEM = "(random-seed [seed])\n"
"Initialize pseudo-random algorithm using number seed. If seed is omitted,\n"
"analogRead(27) sets the seed using noise from uninitialized pin.";

// ADXL345 sensor
const char docAccelBegin[] PROGMEM = "(accel-begin)\n"
"Initialize ADXL345 using I2C and standard address (#x53).";
const char docAccelSetRange[] PROGMEM = "(accel-set-range g)\n"
"Set g range for the accelerometer (2, 4, 8 or 16).";
const char docAccelGetEvent[] PROGMEM = "(accel-get-event)\n"
"Return list with most recent sensor event data.";

// Charlieplexed LED matrix
const char docLEDMatrixBegin[] PROGMEM = "(led-matrix-begin [mnum = 0] [adr = #x74])\n"
"Initialize the LED matrix driver. Optionally assign it number mnum, optionally use I2C address adr.";
const char docLEDMatrixDrawPixel[] PROGMEM = "(led-matrix-draw-pixel x y b [mnum = 0])\n"
"Set LED at position x, y to brightness b. Optionally use matrix module number mnum.";
const char docLEDMatrixSetFrame[] PROGMEM = "(led-matrix-set-frame f [mnum = 0])\n"
"Direct all graphics commands to frame buffer f. Optionally use matrix module number mnum.";
const char docLEDMatrixDisplayFrame[] PROGMEM = "(led-matrix-display-frame f [mnum = 0])\n"
"Display frame buffer f. Optionally use matrix module number mnum.";
const char docLEDMatrixClear[] PROGMEM = "(led-matrix-clear [mnum = 0])\n"
"Clear LED matrix. Optionally use matrix module number mnum.";
const char docLEDMatrixSetRotation[] PROGMEM = "(led-matrix-set-rotation rot [mnum = 0])\n"
"Set rotation of matrix.\n"
"0 = no rotation, 1 = 90 degrees, 2 = 180 degrees, 3 = 270 degrees,\n"
"4 = hor. mirrored, 5 = vert. mirrored.\n"
"Optionally use matrix module number mnum.";
const char docLEDMatrixSetTextColor[] PROGMEM = "(led-matrix-set-text-color br [mnum = 0])\n"
"Set brightness br for text (range 0-255). Optionally use matrix module number mnum.";
const char docLEDMatrixSetTextSize[] PROGMEM = "(led-matrix-set-text-size mf [mnum = 0])\n"
"Set text magnification factor. Optionally use matrix module number mnum.";
const char docLEDMatrixSetTextWrap[] PROGMEM = "(led-matrix-set-text-wrap w [mnum = 0])\n"
"Set text wrap false/true. Optionally use matrix module number mnum.";
const char docLEDMatrixSetCursor[] PROGMEM = "(led-matrix-set-cursor x y [mnum = 0])\n"
"Set text cursor to position x y. Optionally use matrix module number mnum.";
const char docLEDMatrixDrawChar[] PROGMEM = "(led-matrix-draw-char x y c fg bg mf [mnum = 0])\n"
"Draw char c to screen at location x y with foreground/background brightness fg/bg and size mf.\n"
"Optionally use matrix module number mnum.";
const char docLEDMatrixPrint[] PROGMEM = "(led-matrix-print str [mnum = 0])\n"
"Print str to screen at current cursor location using text brightness.\n"
"Optionally use matrix module number mnum.";
const char docLEDMatrixDrawLine[] PROGMEM = "(led-matrix-draw-line x0 y0 x1 y1 br [mnum = 0])\n"
"Draw a line between positions x0/y0 and x1/y1 with brightness br (range 0-255).\n"
"Optionally use matrix module number mnum.";
const char docLEDMatrixDrawRect[] PROGMEM = "(led-matrix-draw-rect x y w h br [mnum = 0])\n"
"Draw empty rectangle at x y with width w, height h and brightness br (range 0-255).\n"
"Optionally use matrix module number mnum.";
const char docLEDMatrixFillRect[] PROGMEM = "(led-matrix-fill-rect x y w h br [mnum = 0])\n"
"Draw filled rectangle at x y with width w and height h, height h and brightness br (range 0-255).\n"
"Optionally use matrix module number mnum.";
const char docLEDMatrixDrawCircle[] PROGMEM = "(led-matrix-draw-circle x y r br [mnum = 0])\n"
"Draw empty circle at position x y with radius r and brightness br (range 0-255).\n"
"Optionally use matrix module number mnum.";
const char docLEDMatrixFillCircle[] PROGMEM = "(led-matrix-fill-circle x y r br [mnum = 0])\n"
"Draw filled circle at position x y with radius r and brightness br (range 0-255).\n"
"Optionally use matrix module number mnum.";
const char docLEDMatrixDrawRoundRect[] PROGMEM = "(led-matrix-draw-round-rect x y w h r br [mnum = 0])\n"
"Draw empty rectangle at x y with width w, height h and brightness br (range 0-255).\n"
"Edges are rounded with radius r.\n"
"Optionally use matrix module number mnum.";
const char docLEDMatrixFillRoundRect[] PROGMEM = "(led-matrix-fill-round-rect x y w h r br [mnum = 0])\n"
"Draw filled rectangle at x y with width w, height h and brightness br (range 0-255).\n"
"Edges are rounded with radius r.\n"
"Optionally use matrix module number mnum.";
const char docLEDMatrixDrawTriangle[] PROGMEM = "(led-matrix-draw-triangle x0 y0 x1 y1 x2 y2 br [mnum = 0])\n"
"Draw empty triangle with corners at x0/y0, x1/y1, x2/y2 and brightness br (range 0-255).\n"
"Optionally use matrix module number mnum.";
const char docLEDMatrixFillTriangle[] PROGMEM = "(led-matrix-fill-triangle x0 y0 x1 y1 x2 y2 br [mnum = 0])\n"
"Draw filled triangle with corners at x0/y0, x1/y1, x2/y2 and brightness br (range 0-255).\n"
"Optionally use matrix module number mnum.";

const char docLoadMono[] PROGMEM = "(load-mono fname arr [offx] [offy])\n"
"Open monochrome BMP file fname from SD if it exits and copy it into the two-dimensional uLisp bit array provided.\n"
"Note that this allocates massive amounts of RAM. Use for small bitmaps/icons only.\n"
"When the image is larger than the array, only the upper leftmost area of the bitmap fitting into the array is loaded.\n"
"Providing offx and offy you may move the 'window' of the array to other parts of the bitmap (useful e.g. for tiling).";

//Canvas consisting of two combined LED matrices named 0 and 1
const char docCanvasBegin[] PROGMEM = "(canvas-begin adr0 adr1)\n"
"Initialize two LED matrices at I2C adresses adr0 and adr1 and assign them to matrix numbers 0 and 1.\n"
"Create brightness compensation array for tilted matrices.";
const char docCanvasDrawPixel[] PROGMEM = "(canvas-draw-pixel x y b)\n"
"Draw pixel on two combined LED matrices at position x y with brightness b.";
const char docCanvasClear[] PROGMEM = "(canvas-clear)\n"
"Clear current frame of combined LED matrices.";
const char docCanvasClearAll[] PROGMEM = "(canvas-clear-all)\n"
"Clear all frames of combined LED matrices.";
const char docCanvasSetFrame[] PROGMEM = "(canvas-set-frame f)\n"
"Set current frame of combined LED matrices to f.";
const char docCanvasDisplayFrame[] PROGMEM = "(canvas-display-frame f)\n"
"Display frame f of combined LED matrices.";
const char docCanvasDrawArray[] PROGMEM = "(canvas-draw-array arr [comp = nil] [bright = 0] [offx = 0] [offy = 0])\n"
"Draw uLisp array arr on two combined LED matrices. If comp is true, use brightness compensation.\n"
"Optionally draw array using offset offx and offy. The function automatically distinguishes between\n"
"integer and bit arrays. If a bit array is provided, the pixels are drawn using brightness bright.\n"
"Otherwise the integer value at an array position (plus, optionally, brightness bright) determines the brightness of the pixel.";
const char docCanvasTransform[] PROGMEM = "(canvas-transform source target angles vect cz)\n"
"Rotate 2D image array source around all three axes using list angles,\n"
"translate it using list vect and project it to target area using observer z position cz.\n"
"Angles given in degrees (int), result written into target array (omitting points not set,\n"
"so target array needs to be empty).";
const char docCanvasTransformDraw[] PROGMEM = "(canvas-transform-draw source angles vect cz [comp = nil])\n"
"Rotate 2D image array source around all three axes using list angles,\n"
"translate it using list vect and project it to canvas using observer z position cz.\n"
"Angles given in degrees (int), result directly drawn to canvas.\n"
"Optionally use compensation matrix if comp is t.";

// Symbol lookup table
const tbl_entry_t lookup_table2[] PROGMEM = {
  { stringRandomSeed, fn_RandomSeed, 0201, docRandomSeed },

  { stringAccelBegin, fn_AccelBegin, 0200, docAccelBegin },
  { stringAccelSetRange, fn_AccelSetRange, 0211, docAccelSetRange },
  { stringAccelGetEvent, fn_AccelGetEvent, 0200, docAccelGetEvent },

  { stringLEDMatrixBegin, fn_LEDMatrixBegin, 0202, docLEDMatrixBegin },
  { stringLEDMatrixDrawPixel, fn_LEDMatrixDrawPixel, 0234, docLEDMatrixDrawPixel },
  { stringLEDMatrixSetFrame, fn_LEDMatrixSetFrame, 0212, docLEDMatrixSetFrame },
  { stringLEDMatrixDisplayFrame, fn_LEDMatrixDisplayFrame, 0212, docLEDMatrixDisplayFrame },
  { stringLEDMatrixClear, fn_LEDMatrixClear, 0201, docLEDMatrixClear },
  { stringLEDMatrixSetRotation, fn_LEDMatrixSetRotation, 0212, docLEDMatrixSetRotation },
  { stringLEDMatrixSetTextColor, fn_LEDMatrixSetTextColor, 0212, docLEDMatrixSetTextColor },
  { stringLEDMatrixSetTextSize, fn_LEDMatrixSetTextSize, 0212, docLEDMatrixSetTextSize },
  { stringLEDMatrixSetTextWrap, fn_LEDMatrixSetTextWrap, 0212, docLEDMatrixSetTextWrap },
  { stringLEDMatrixSetCursor, fn_LEDMatrixSetCursor, 0223, docLEDMatrixSetCursor },
  { stringLEDMatrixDrawChar, fn_LEDMatrixDrawChar, 0267, docLEDMatrixDrawChar },
  { stringLEDMatrixPrint, fn_LEDMatrixPrint, 0212, docLEDMatrixPrint },
  { stringLEDMatrixDrawLine, fn_LEDMatrixDrawLine, 0256, docLEDMatrixDrawLine },
  { stringLEDMatrixDrawRect, fn_LEDMatrixDrawRect, 0256, docLEDMatrixDrawRect },
  { stringLEDMatrixFillRect, fn_LEDMatrixFillRect, 0256, docLEDMatrixFillRect },
  { stringLEDMatrixDrawCircle, fn_LEDMatrixDrawCircle, 0245, docLEDMatrixDrawCircle },
  { stringLEDMatrixFillCircle, fn_LEDMatrixFillCircle, 0245, docLEDMatrixFillCircle },
  { stringLEDMatrixDrawRoundRect, fn_LEDMatrixDrawRoundRect, 0267, docLEDMatrixDrawRoundRect },
  { stringLEDMatrixFillRoundRect, fn_LEDMatrixFillRoundRect, 0267, docLEDMatrixFillRoundRect },
  { stringLEDMatrixDrawTriangle, fn_LEDMatrixDrawTriangle, 0277, docLEDMatrixDrawTriangle },
  { stringLEDMatrixFillTriangle, fn_LEDMatrixFillTriangle, 0277, docLEDMatrixFillTriangle },

  { stringLoadMono, fn_LoadMono, 0224, docLoadMono },

  { stringCanvasBegin, fn_CanvasBegin, 0222, docCanvasBegin },
  { stringCanvasDrawPixel, fn_CanvasDrawPixel, 0233, docCanvasDrawPixel },
  { stringCanvasClear, fn_CanvasClear, 0200, docCanvasClear },
  { stringCanvasClearAll, fn_CanvasClearAll, 0200, docCanvasClearAll },
  { stringCanvasSetFrame, fn_CanvasSetFrame, 0211, docCanvasSetFrame },
  { stringCanvasDisplayFrame, fn_CanvasDisplayFrame, 0211, docCanvasDisplayFrame },
  { stringCanvasDrawArray, fn_CanvasDrawArray, 0215, docCanvasDrawArray },
  { stringCanvasTransform, fn_CanvasTransform, 0255, docCanvasTransform },
  { stringCanvasTransformDraw, fn_CanvasTransformDraw, 0245, docCanvasTransformDraw },

};

// Table cross-reference functions - do not edit below this line

tbl_entry_t *tables[] = {lookup_table, lookup_table2};
const unsigned int tablesizes[] = { arraysize(lookup_table), arraysize(lookup_table2) };

const tbl_entry_t *table (int n) {
  return tables[n];
}

unsigned int tablesize (int n) {
  return tablesizes[n];
}
