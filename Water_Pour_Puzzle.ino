#include <Adafruit_Arcada.h>

Adafruit_Arcada arcada;
GFXcanvas16 *canvas;

// ============================================================
// Water Sort Puzzle for Adafruit EdgeBadge
// ============================================================

#define NUM_TUBES       8
#define TUBE_HEIGHT     4
#define NUM_COLORS      6
#define SCRAMBLE_MOVES  200

enum
{
  EMPTY = 0,
  RED,
  BLUE,
  GREEN,
  YELLOW,
  PURPLE,
  CYAN
};

uint8_t tubes[NUM_TUBES][TUBE_HEIGHT];

int cursorTube = 0;
int selectedTube = -1;
int moves = 0;
bool gameOver = false;
uint32_t bounceTimer = 0;
int bounceOffset = 0;
uint8_t last_buttons = 0;

// ============================================================
// Color Helper
// ============================================================

uint16_t getColor(uint8_t color)
{
  switch (color)
  {
    case RED:    return ARCADA_RED;
    case BLUE:   return ARCADA_BLUE;
    case GREEN:  return ARCADA_GREEN;
    case YELLOW: return ARCADA_YELLOW;
    case PURPLE: return ARCADA_MAGENTA;
    case CYAN:   return ARCADA_CYAN;
    default:     return ARCADA_BLACK;
  }
}

// ============================================================
// Tube Helpers
// ============================================================

int findTop(int tube)
{
  for (int i = TUBE_HEIGHT - 1; i >= 0; i--)
  {
    if (tubes[tube][i] != EMPTY)
      return i;
  }

  return -1;
}

int findEmpty(int tube)
{
  for (int i = 0; i < TUBE_HEIGHT; i++)
  {
    if (tubes[tube][i] == EMPTY)
      return i;
  }

  return -1;
}

bool tubeFull(int tube)
{
  return findEmpty(tube) == -1;
}

bool tubeEmpty(int tube)
{
  return findTop(tube) == -1;
}

// ============================================================
// Pour Logic
// ============================================================

bool pour(int src, int dst)
{
  if (src == dst)
    return false;

  int srcTop = findTop(src);

  if (srcTop < 0)
    return false;

  uint8_t color = tubes[src][srcTop];

  int dstTop = findTop(dst);

  if (dstTop >= 0)
  {
    if (tubes[dst][dstTop] != color)
      return false;
  }

  int contiguous = 1;

  for (int i = srcTop - 1; i >= 0; i--)
  {
    if (tubes[src][i] == color)
      contiguous++;
    else
      break;
  }

  int freeSpace = 0;

  for (int i = 0; i < TUBE_HEIGHT; i++)
  {
    if (tubes[dst][i] == EMPTY)
      freeSpace++;
  }

  if (freeSpace == 0)
    return false;

  if (contiguous > freeSpace)
    contiguous = freeSpace;

  for (int i = 0; i < contiguous; i++)
  {
    int s = findTop(src);
    int d = findEmpty(dst);

    tubes[dst][d] = tubes[src][s];
    tubes[src][s] = EMPTY;
  }

  return true;
}

bool scramblePour(int src, int dst)
{
  if (src == dst) return false;

  int srcTop = findTop(src);
  if (srcTop < 0) return false; // Source is empty

  // Check if target is full
  if (tubeFull(dst)) return false;

  // Crucial: Unlike normal gameplay, a reverse scramble pour should 
  // let us pour *different* colors on top of each other to mix them up.
  // We only care that there is physical room in the destination tube.
  
  int s = findTop(src);
  int d = findEmpty(dst);

  tubes[dst][d] = tubes[src][s];
  tubes[src][s] = EMPTY;

  return true;
}

// ============================================================
// Puzzle Generation
// ============================================================

void createSolvedBoard()
{
  memset(tubes, 0, sizeof(tubes));

  for (int color = 0; color < NUM_COLORS; color++)
  {
    for (int level = 0; level < TUBE_HEIGHT; level++)
    {
      tubes[color][level] = color + 1;
    }
  }

  // Tube 6 and 7 remain empty.
}

void scrambleBoard()
{
  for (int i = 0; i < SCRAMBLE_MOVES; i++)
  {
    int src = random(NUM_TUBES);
    int dst = random(NUM_TUBES);

    if (src == dst)
      continue;

    scramblePour(src, dst);
  }
}

void initPuzzle()
{
  createSolvedBoard();

  scrambleBoard();

  moves = 0;
  gameOver = false;
  cursorTube = 0;
  selectedTube = -1;
}

// ============================================================
// Win Detection
// ============================================================

bool tubeSolved(int tube)
{
  int top = findTop(tube);

  if (top == -1)
    return true;

  uint8_t color = tubes[tube][0];

  if (color == EMPTY)
    return false;

  for (int i = 1; i < TUBE_HEIGHT; i++)
  {
    if (tubes[tube][i] != color)
      return false;
  }

  return true;
}

bool checkWin()
{
  for (int i = 0; i < NUM_TUBES; i++)
  {
    if (!tubeSolved(i))
      return false;
  }

  return true;
}

// ============================================================
// Drawing
// ============================================================

void drawTube(int tube, int x, int y)
{
const int tubeWidth = 14;
const int layerHeight = 14;


// Bounce selected tube
int drawY = y;

if (tube == selectedTube)
{
  drawY += bounceOffset;
}


// Draw liquid first
for (int level = 0; level < TUBE_HEIGHT; level++)
{
  uint8_t color = tubes[tube][level];

  if (color == EMPTY)
    continue;

  int py = drawY + (3 - level) * layerHeight;

  canvas->fillRect(
    x + 2,
    py + 1,
    tubeWidth - 4,
    layerHeight - 2,
    getColor(color));
}


// Draw tube outline without top
canvas->drawLine(
  x,
  drawY,
  x,
  drawY + 4 * layerHeight,
  ARCADA_WHITE);

canvas->drawLine(
  x + tubeWidth - 1,
  drawY,
  x + tubeWidth - 1,
  drawY + 4 * layerHeight,
  ARCADA_WHITE);

canvas->drawLine(
  x,
  drawY + 4 * layerHeight,
  x + tubeWidth - 1,
  drawY + 4 * layerHeight,
  ARCADA_WHITE);


// Selected tube: 2px white outline
if (tube == selectedTube)
{
  canvas->drawRect(
    x - 2,
    drawY - 2,
    tubeWidth + 4,
    4 * layerHeight + 5,
    ARCADA_WHITE);

  canvas->drawRect(
    x - 3,
    drawY - 3,
    tubeWidth + 6,
    4 * layerHeight + 7,
    ARCADA_WHITE);
}

// Cursor highlight
if (tube == cursorTube)
{
  canvas->drawRect(
    x - 2,
    drawY - 2,
    tubeWidth + 4,
    4 * layerHeight + 5,
    ARCADA_WHITE);

  canvas->drawRect(
    x - 1,
    drawY - 1,
    tubeWidth + 2,
    4 * layerHeight + 3,
    ARCADA_WHITE);
}
}

void drawGame()
{
  canvas->fillScreen(ARCADA_BLACK);

  canvas->setCursor(2, 2);
  canvas->setTextColor(ARCADA_WHITE);
  canvas->setTextSize(1);
  canvas->print("Water Sort");

  canvas->setCursor(95, 2);
  canvas->printf("M:%d", moves);

  // Animate selected tube bounce
  if (millis() - bounceTimer > 350)
  {
    bounceTimer = millis();

    if (selectedTube != -1)
    {
      if (bounceOffset == 0)
        bounceOffset = -2;
      else
        bounceOffset = 0;
    }
    else
    {
      bounceOffset = 0;
    }
  }

  canvas->setCursor(2, 118);

  if (selectedTube == -1)
    canvas->print("A=Select");
  else
    canvas->print("A=Pour");

  int startX = 8;
  int spacing = 5;
  int y = 30;

  for (int tube = 0; tube < NUM_TUBES; tube++)
  {
    int x = startX + tube * (14 + spacing);
    drawTube(tube, x, y);
  }
}

void drawWinScreen()
{
  canvas->fillScreen(ARCADA_GREEN);

  canvas->setTextColor(ARCADA_BLACK);
  canvas->setTextSize(2);

  canvas->setCursor(15, 30);
  canvas->print("SOLVED!");

  canvas->setTextSize(1);

  canvas->setCursor(40, 65);
  canvas->printf("Moves: %d", moves);

  canvas->setCursor(15, 95);
  canvas->print("Press A for new game");

  for (int i = 0; i < arcada.pixels.numPixels(); i++)
  {
    arcada.pixels.setPixelColor(
      i,
      arcada.pixels.Color(0, 255, 0));
  }

  arcada.pixels.show();
}

// ============================================================
// Setup
// ============================================================

void setup()
{
  Serial.begin(115200);

  if (!arcada.arcadaBegin())
  {
    while (1);
  }

  arcada.displayBegin();

  if (!arcada.createFrameBuffer(
        ARCADA_TFT_WIDTH,
        ARCADA_TFT_HEIGHT))
  {
    while (1);
  }

  canvas = arcada.getCanvas();

  arcada.setBacklight(255);

  randomSeed(
    analogRead(A8) +
    arcada.readLightSensor());

  initPuzzle();
}

// ============================================================
// Main Loop
// ============================================================

void loop()
{
  uint8_t buttons = arcada.readButtons();
  uint8_t justPressed = buttons & ~last_buttons;
  last_buttons = buttons;

  if (gameOver)
  {
    drawWinScreen();

    if (justPressed & ARCADA_BUTTONMASK_A)
    {
      for (int i = 0; i < arcada.pixels.numPixels(); i++)
      {
        arcada.pixels.setPixelColor(i, 0);
      }

      arcada.pixels.show();

      initPuzzle();
    }

    arcada.blitFrameBuffer(0, 0, false, true);
    return;
  }

  // Navigation

  if (justPressed & ARCADA_BUTTONMASK_LEFT)
  {
    cursorTube--;

    if (cursorTube < 0)
      cursorTube = NUM_TUBES - 1;
  }

  if (justPressed & ARCADA_BUTTONMASK_RIGHT)
  {
    cursorTube++;

    if (cursorTube >= NUM_TUBES)
      cursorTube = 0;
  }

  // Select / Pour

  if (justPressed & ARCADA_BUTTONMASK_A)
  {
    if (selectedTube == -1)
    {
      if (!tubeEmpty(cursorTube))
      {
        selectedTube = cursorTube;
        bounceOffset = 0;
      }
    }
    else
    {
      if (pour(selectedTube, cursorTube))
      {
        moves++;

        if (checkWin())
        {
          gameOver = true;
        }
      }

      selectedTube = -1;
      
    }
  }

  // Cancel Selection

  if (justPressed & ARCADA_BUTTONMASK_B)
  {
    selectedTube = -1;
    bounceOffset = 0;
  }

  drawGame();

  arcada.blitFrameBuffer(0, 0, false, true);
}