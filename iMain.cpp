
#include "iGraphics.h"
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <mmsystem.h>

// ================== GAME CONSTANTS ==================
#define N_CLOUDS 4
#define N_PIPES 8
#define N_BEAM_GROUP 5 // FIX: Set to match N_BEAMS
#define SCREEN_WIDTH 1300
#define SCREEN_HEIGHT 704
#define N_COINS 10
#define COIN_WIDTH 30
#define COIN_HEIGHT 30
#define COIN_SPEED 9
#define COIN_FRAMES 6
#define BIRD_WIDTH 50
#define BIRD_HEIGHT 50
#define PIPE_WIDTH 70
#define PIPE_GAP 250
#define GRAVITY 0.5f
#define JUMP_VELOCITY 10.0f
#define PIPE_SPEED 7
#define N_FRAMES 5
#define N_BEAMS 3
#define BEAM_WIDTH 20
#define BEAM_HEIGHT 10
#define BEAM_VELOCITY 12
#define N_GERMS 2
#define GERM_WIDTH 40
#define GERM_HEIGHT 40
#define GERM_SPEED 7
#define GAME_STATE_LEVEL_SELECT 6
#define GERM_FRAMES 3
#ifndef MAX_TIMERS
#define MAX_TIMERS 20

#endif

// Level button positions and sizes
int easyX = 500, easyY = 500;
int mediumX = 700, mediumY = 500;
int hardX = 900, hardY = 500;
int levelBtnW = 150, levelBtnH = 60;

Image germFrames[GERM_FRAMES];
int germFrameIndex = 0;
int germAnimTimer;

bool hoverEasy = false;
bool hoverMedium = false;
bool hoverHard = false;

bool isHardLevel = false;
bool  isGamePaused = false;


Image backBtn, backBtnHover;
int backBtnX = 125, backBtnY = 50;
int backBtnW = 125, backBtnH = 50;
bool hoverBack = false;



float germ_x[N_GERMS];
float germ_y[N_GERMS];
bool germ_active[N_GERMS] = {false};
float ground_x = 0;

float rectAngle1 = 0.0f;
float rectAngle2 = 180.0f; // Opposite side of circle
float rectAngularVelocity = 2.0f; // degrees per frame or timer tick
float circleCenterX = SCREEN_WIDTH / 2;
float circleCenterY = SCREEN_HEIGHT / 2;
float circleRadius = 150;
int rectWidth = 60;
int rectHeight = 30;


int germSpawnTimer;

Image scoreDisplayImage;
float beam_x[N_BEAMS];
float beam_y[N_BEAMS];
bool beam_active[N_BEAMS] = {false};



int beamSpawnTimer;

bool gameOverSoundPlayed = false; // FIX: Prevent sound overlap

// ================== GLOBAL VARIABLES ==================
int gameState = 0;
Image background;
int btnW = 200, btnH = 60;
int playX = 140, playY = 270;
int continueX = 430, continueY = 270;
int helpX = 140, helpY = 145;
int scoreX = 140, scoreY = 25;
int exitX = 430, exitY = 25;
int levelX = 430, levelY = 145;
bool hoverPlay = false;
bool hoverHelp = false;
bool hoverExit = false;
bool hoverContinue = false;
bool hoverScore = false;
bool hoverLevel = false;
Image play, playHover;
Image help, helpHover;
Image quit, quitHover;
Image cont, contHover;
Image scoreBtn, scoreBtnHover;
Image level, levelHover;

Image gameOverImage;
Image groundImage;
Image helpImage;
int helpScrollY = 0;
const int helpContentHeight = 1600;

Image cloudImages[N_CLOUDS];
float cloud_x[N_CLOUDS];
float cloud_y[N_CLOUDS] = {300, 350, 300, 320};
Image lowerPipeImages[N_PIPES];
Image upperPipeImages[N_PIPES];
Image BG;
Image birdFrames[N_FRAMES];
int flyingFrame = 0;
Image coinFrames[N_COINS][COIN_FRAMES];
float coin_x[N_COINS];
float coin_y[N_COINS];
int coinFrameIndex[N_COINS] = {0};
int pipe_x[N_PIPES];
int pipe_gap_y[N_PIPES];
float bird_x = 200;
float bird_y = 400;
float bird_velocity = 0;
int score = 0;
bool gameOver = false;
bool scoreCountedPerPipe[N_PIPES] = {false};
int animTimer, physicsTimer, coinAnimTimer;
int scoreScrollY = 0;
const int scoreContentHeight = 1600;
Image scoreImage;










// FIX: Function prototypes
void updateGame();
void updateClouds();
void updateCoins();
void updateBeams();
void updateGerms();
void updateBirdAnimation();
void updateCoinAnimation();
void updateGermAnimation();
void updateAll();

// ================== GAME FUNCTIONS ==================



typedef struct {
    char name[50];
    int score;
} HighScore;

HighScore highScores[5];

// Variables for name input after game over
bool isEnteringName = false;
char playerName[50] = "";
int nameCharIndex = 0;




void loadHighScores() {
    FILE *file = fopen("highscores.txt", "r");
    if (!file) {
        // Initialize empty scores if file doesn't exist
        for (int i = 0; i < 5; i++) {
            strcpy(highScores[i].name, "---");
            highScores[i].score = 0;
        }
        return;
    }
    for (int i = 0; i < 5; i++) {
        if (fscanf(file, "%49s %d", highScores[i].name, &highScores[i].score) != 2) {
            strcpy(highScores[i].name, "---");
            highScores[i].score = 0;
        }
    }
    fclose(file);
}


void updateRotatingRectangles()
{
    rectAngle1 += rectAngularVelocity;
    if (rectAngle1 >= 360) rectAngle1 -= 360;

    rectAngle2 += rectAngularVelocity;
    if (rectAngle2 >= 360) rectAngle2 -= 360;

    // Calculate positions
    float rad_1 = rectAngle1 * 3.14159265f / 180.0f;
    float rad_2 = rectAngle2 * 3.14159265f / 180.0f;

    float rect1X = circleCenterX + circleRadius * cos(rad_1) - rectWidth / 2;
    float rect1Y = circleCenterY + circleRadius * sin(rad_1) - rectHeight / 2;

    float rect2X = circleCenterX + circleRadius * cos(rad_2) - rectWidth / 2;
    float rect2Y = circleCenterY + circleRadius * sin(rad_2) - rectHeight / 2;

    // Collision detection with bird
    bool collision1 = (bird_x < rect1X + rectWidth) && (bird_x + BIRD_WIDTH > rect1X) &&
                      (bird_y < rect1Y + rectHeight) && (bird_y + BIRD_HEIGHT > rect1Y);

    bool collision2 = (bird_x < rect2X + rectWidth) && (bird_x + BIRD_WIDTH > rect2X) &&
                      (bird_y < rect2Y + rectHeight) && (bird_y + BIRD_HEIGHT > rect2Y);

    if ((collision1 || collision2) && !gameOver) {
        gameOver = true;
        iPauseTimer(physicsTimer);
        if (!gameOverSoundPlayed) {
            PlaySound(NULL, 0, 0);
            PlaySound(TEXT("game_over.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
            gameOverSoundPlayed = true;
        }
    }
}



// Function to save high scores to file
void saveHighScores() {
    FILE *file = fopen("highscores.txt", "w");
    if (!file) return;
    for (int i = 0; i < 5; i++) {
        fprintf(file, "%s %d\n", highScores[i].name, highScores[i].score);
    }
    fclose(file);
}

// Insert new score into highScores array if qualifies
void addHighScore(const char *name, int score) {
    // Find position to insert
    int pos = 5;
    for (int i = 0; i < 5; i++) {
        if (score > highScores[i].score) {
            pos = i;
            break;
        }
    }
    if (pos == 5) return; // Not high enough

    // Shift lower scores down
    for (int i = 5 - 1; i > pos; i--) {
        highScores[i] = highScores[i - 1];
    }
    // Insert new score
    strncpy(highScores[pos].name, name, 49);
    highScores[pos].name[49] = '\0';
    highScores[pos].score = score;

    saveHighScores();
}





void safePauseTimer(int timerId) {
    if (timerId >= 0 && timerId < MAX_TIMERS) {
        iPauseTimer(timerId);
    }
}

// Keep only one definition of safeResumeTimer
void safeResumeTimer(int timerId) {
    if (timerId >= 0 && timerId < MAX_TIMERS) {
        iResumeTimer(timerId);
    }
}

// Keep only one definition of iGetDeltaTime
int iGetDeltaTime() {
    static int oldT = -1;
    int newT = glutGet(GLUT_ELAPSED_TIME);
    if (oldT == -1) oldT = newT;
    int delta = newT - oldT;
    oldT = newT;
    return delta;
}

// Fix iPlaySound to avoid Unicode mismatch error:
// Option 1: If your project is Unicode-enabled, keep this:
void iPlaySound(const char *filename, bool loop = false)
{
#ifdef UNICODE
    // Convert const char* to wchar_t* for Unicode builds
    wchar_t wfilename[MAX_PATH];
    MultiByteToWideChar(CP_ACP, 0, filename, -1, wfilename, MAX_PATH);
    PlaySound(NULL, 0, 0); // Stop any currently playing sound
    PlaySound(wfilename, NULL, SND_FILENAME | SND_ASYNC | (loop ? SND_LOOP : 0));
#else
    // Use const char* directly for ANSI builds
    PlaySound(NULL, 0, 0); // Stop any currently playing sound
    PlaySound(filename, NULL, SND_FILENAME | SND_ASYNC | (loop ? SND_LOOP : 0));
#endif
}

void iPauseGame() {
    isGamePaused = true;
    safePauseTimer(animTimer);
    safePauseTimer(coinAnimTimer);
    safePauseTimer(germAnimTimer);
    safePauseTimer(physicsTimer);
    safePauseTimer(beamSpawnTimer);
}


void iResumeGame() {
    isGamePaused = false;
    safeResumeTimer(animTimer);
    safeResumeTimer(coinAnimTimer);
    safeResumeTimer(germAnimTimer);
    safeResumeTimer(physicsTimer);
    safeResumeTimer(beamSpawnTimer);
}



void updateGround()
{
    ground_x -= PIPE_SPEED; // Move ground left at pipe speed
    if (ground_x <= -SCREEN_WIDTH) {
        ground_x = 0; // Reset position to loop
    }
}


void spawnGerms()
{
    for (int i = 0; i < N_GERMS; i++) {
        germ_x[i] = SCREEN_WIDTH + i * 300;
        germ_y[i] = 100 + rand() % (SCREEN_HEIGHT - 200);
        germ_active[i] = true;
    }
}

void updateGermAnimation()
{
    germFrameIndex = (germFrameIndex + 1) % GERM_FRAMES;
}

void updateGerms()
{
    for (int i = 0; i < N_GERMS; i++) {
        if (!germ_active[i]) continue;

        germ_x[i] -= GERM_SPEED;
        if (germ_x[i] + GERM_WIDTH < 0) {
            germ_x[i] = SCREEN_WIDTH;
            germ_y[i] = 100 + rand() % (SCREEN_HEIGHT - 200);
        }

        if (!gameOver && bird_x + BIRD_WIDTH > germ_x[i] && bird_x < germ_x[i] + GERM_WIDTH &&
            bird_y + BIRD_HEIGHT > germ_y[i] && bird_y < germ_y[i] + GERM_HEIGHT) {
            gameOver = true;
            bird_velocity = 0;
            iPauseTimer(physicsTimer);
            if (!gameOverSoundPlayed) { // FIX: Play sound once
                PlaySound(NULL, 0, 0);
                PlaySound(TEXT("game_over.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
                gameOverSoundPlayed = true;
            }
            germ_active[i] = false;
        }
    }
}

void spawnBeams()
{
    float baseX = SCREEN_WIDTH;
    float lineY = rand() % (SCREEN_HEIGHT - 200);// Fixed y position for the line of beams

    for (int i = 0; i < N_BEAMS; i++) {
        beam_x[i] = baseX + i * (BEAM_WIDTH + 50); // space beams evenly with 50 px gap
        beam_y[i] = lineY;
        beam_active[i] = true;
    }
}

void updateBeams()
{
    for (int i = 0; i < N_BEAMS; i++) {
        if (!beam_active[i]) continue;

        beam_x[i] -= BEAM_VELOCITY;
        if (beam_x[i] + BEAM_WIDTH < 0) {
            beam_x[i] = SCREEN_WIDTH + (N_BEAMS - 1) * (BEAM_WIDTH + 50); // reset to right end of line
        }
    }
}

void resetGame()
{
    bird_y = 400;
    bird_velocity = 0;
    gameOverSoundPlayed = false;
      int totalGroups = N_PIPES / 4;

    int groupSpacing = SCREEN_WIDTH / 2;

    for (int group = 0; group < totalGroups; group++) {
        int baseX = SCREEN_WIDTH + group * groupSpacing;
        int baseGapY = 150 + rand() % 200;

        for (int i = 0; i < 4; i++) {
            int idx = group * 4 + i;

            pipe_x[idx] = baseX + i * (PIPE_WIDTH + 10);
            pipe_gap_y[idx] = baseGapY + (i % 2) * 80;

            int topPipeHeight = SCREEN_HEIGHT - (pipe_gap_y[idx] + PIPE_GAP);
            if (topPipeHeight < 0) topPipeHeight = 0;

            iResizeImage(&lowerPipeImages[idx], PIPE_WIDTH, pipe_gap_y[idx]);
            iResizeImage(&upperPipeImages[idx], PIPE_WIDTH, topPipeHeight);

            scoreCountedPerPipe[idx] = false;
        }
    }

    spawnBeams();

    for (int i = 0; i < N_COINS; i++) {
        coin_x[i] = SCREEN_WIDTH + i * 300;
        coin_y[i] = 100 + rand() % (SCREEN_HEIGHT - 200);
    }

    score = 0;
    gameOver = false;
    iResumeTimer(physicsTimer);
}
void updateClouds()
{
    for (int i = 0; i < N_CLOUDS; i++) {
        cloud_x[i] -= 2;
        if (cloud_x[i] + 300 < 0) {
            cloud_x[i] = SCREEN_WIDTH;
        }
    }
}

void updateCoins()
{
    for (int i = 0; i < N_COINS; i++) {
        coin_x[i] -= COIN_SPEED;
        if (coin_x[i] + COIN_WIDTH < 0) {
            coin_x[i] = SCREEN_WIDTH;
            coin_y[i] = 100 + rand() % (SCREEN_HEIGHT - 200);
        }
    }
}

void updateCoinAnimation()
{
    for (int i = 0; i < N_COINS; i++) {
        coinFrameIndex[i] = (coinFrameIndex[i] + 1) % COIN_FRAMES;
    }
}

void updateBirdAnimation()
{
    flyingFrame = (flyingFrame + 1) % N_FRAMES;
}

// FIX: Consolidated update function
void updateAll()
{
    if (gameState != 1) return;
    updateClouds();
    updateCoins();
    updateBeams();
    updateGerms();
    updateGame();
    updateGround(); 
}

// ... existing code ...

void updateGame()
{
    if (gameOver) return;

    bird_velocity -= GRAVITY;
    if (bird_velocity < -2 * JUMP_VELOCITY) bird_velocity = -2 * JUMP_VELOCITY;
    bird_y += bird_velocity;

  
    if (bird_y < 0) {
        bird_y = 0;
        gameOver = true;
        iPauseTimer(physicsTimer);
        if (!gameOverSoundPlayed) {
            PlaySound(NULL, 0, 0);
            PlaySound(TEXT("game_over.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
            gameOverSoundPlayed = true;
        }
    }

    for (int i = 0; i < N_BEAMS; i++) {
        if (beam_active[i] &&
            bird_x + BIRD_WIDTH > beam_x[i] && bird_x < beam_x[i] + BEAM_WIDTH &&
            bird_y + BIRD_HEIGHT > beam_y[i] && bird_y < beam_y[i] + BEAM_HEIGHT) {
            gameOver = true;
            iPauseTimer(physicsTimer);
            if (!gameOverSoundPlayed) {
                PlaySound(NULL, 0, 0);
                PlaySound(TEXT("game_over.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
                gameOverSoundPlayed = true;
            }
        }
    }

    if (bird_y + BIRD_HEIGHT > SCREEN_HEIGHT) {
        bird_y = SCREEN_HEIGHT - BIRD_HEIGHT;
    }

    for (int group = 0; group < N_PIPES / 4; group++) {
        bool groupReset = false;
        for (int i = 0; i < 4; i++) {
            int idx = group * 4 + i;
            pipe_x[idx] -= PIPE_SPEED;
            if (pipe_x[idx] + PIPE_WIDTH < -250)
                groupReset = true;
        }

        if (groupReset) {
            int baseX = SCREEN_WIDTH;
            int baseGapY = 150 + rand() % 200;

            for (int i = 0; i < 4; i++) {
                int idx = group * 4 + i;

                pipe_x[idx] = baseX + i * (PIPE_WIDTH + 10);
                pipe_gap_y[idx] = baseGapY + (i % 2) * 80;

                int topPipeHeight = SCREEN_HEIGHT - (pipe_gap_y[idx] + PIPE_GAP);
                if (topPipeHeight < 0) topPipeHeight = 0;

                iResizeImage(&lowerPipeImages[idx], PIPE_WIDTH, pipe_gap_y[idx]);
                iResizeImage(&upperPipeImages[idx], PIPE_WIDTH, topPipeHeight);

                scoreCountedPerPipe[idx] = false;
            }
        }
    }

    for (int i = 0; i < N_PIPES; i++) {
        if (bird_x + BIRD_WIDTH > pipe_x[i] && bird_x < pipe_x[i] + PIPE_WIDTH &&
            (bird_y < pipe_gap_y[i] || bird_y + BIRD_HEIGHT > pipe_gap_y[i] + PIPE_GAP)) {
            gameOver = true;
            iPauseTimer(physicsTimer);
            if (!gameOverSoundPlayed) {
                PlaySound(NULL, 0, 0);
                PlaySound(TEXT("game_over.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
                gameOverSoundPlayed = true;
            }
        }

        if (!scoreCountedPerPipe[i] && pipe_x[i] + PIPE_WIDTH < bird_x) {
            score++;
            scoreCountedPerPipe[i] = true;
        }
    }

    for (int i = 0; i < N_COINS; i++) {
        if (bird_x + BIRD_WIDTH > coin_x[i] && bird_x < coin_x[i] + COIN_WIDTH &&
            bird_y + BIRD_HEIGHT > coin_y[i] && bird_y < coin_y[i] + COIN_HEIGHT) {
            score += 1;
            coin_x[i] = SCREEN_WIDTH;
            coin_y[i] = 100 + rand() % (SCREEN_HEIGHT - 200);
            PlaySound(TEXT("coin_collect.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
        }
    }
}
// ================== DRAW FUNCTION ==================

void iDraw()
{
  
    iClear();
       if (gameState != 0) {
        iShowLoadedImage(backBtnX, backBtnY, hoverBack ? &backBtnHover : &backBtn);
    }

    if (gameState == 0) {
        iShowLoadedImage(0, 0, &background);
        iShowLoadedImage(playX, playY, hoverPlay ? &playHover : &play);
        iShowLoadedImage(helpX, helpY, hoverHelp ? &helpHover : &help);
        iShowLoadedImage(exitX, exitY, hoverExit ? &quitHover : &quit);
        iShowLoadedImage(continueX, continueY, hoverContinue ? &contHover : &cont);
        iShowLoadedImage(scoreX, scoreY, hoverScore ? &scoreBtnHover : &scoreBtn);
        iShowLoadedImage(levelX, levelY, hoverLevel ? &levelHover : &level);
    }
    else if (gameState == GAME_STATE_LEVEL_SELECT) {
        iSetColor(200, 200, 200);
        iFilledRectangle(easyX, easyY, levelBtnW, levelBtnH);
        iFilledRectangle(mediumX, mediumY, levelBtnW, levelBtnH);
        iFilledRectangle(hardX, hardY, levelBtnW, levelBtnH);

        iSetColor(0, 0, 0);
        iText(easyX + 40, easyY + 20, "Easy");
        iText(mediumX + 30, mediumY + 20, "Medium");
        iText(hardX + 40, hardY + 20, "Hard");

        iText(50, SCREEN_HEIGHT - 50, "Press 'H' to return to Home");
    }
    else if (gameState == 1) {
        iSetColor(135, 206, 235);
        iFilledRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

        for (int i = 0; i < N_CLOUDS; i++) {
            iShowLoadedImage((int)cloud_x[i], (int)cloud_y[i], &cloudImages[i]);
        }

        for (int i = 0; i < N_BEAMS; i++) {
            if (beam_active[i]) {
                int centerX = (int)beam_x[i] + BEAM_WIDTH / 2;
                int centerY = (int)beam_y[i] + BEAM_HEIGHT / 2;
                int radius = BEAM_WIDTH / 2;

                for (int r = radius; r > 0; r--) {
                    int red = 255;
                    int green = (int)(100 * (float)r / radius);
                    int blue = (int)(100 * (float)r / radius);
                    iSetColor(red, green, blue);
                    iFilledCircle(centerX, centerY, r);
                }
            }
        }

        for (int i = 0; i < N_COINS; i++) {
            iShowLoadedImage((int)coin_x[i], (int)coin_y[i], &coinFrames[i][coinFrameIndex[i]]);
        }

        for (int i = 0; i < N_PIPES; i++) {iShowLoadedImage(pipe_x[i], 0, &lowerPipeImages[i]);
        iShowLoadedImage(pipe_x[i], pipe_gap_y[i] + PIPE_GAP, &upperPipeImages[i]);
            
        }

        if (!gameOver) {
            iShowLoadedImage((int)bird_x, (int)bird_y, &birdFrames[flyingFrame]);
        }

        for (int i = 0; i < N_GERMS; i++) {
            if (germ_active[i]) {
                iShowLoadedImage((int)germ_x[i], (int)germ_y[i], &germFrames[germFrameIndex]);
            }
        }
  float rad_1 = rectAngle1 * 3.14159265f / 180.0f;
        float rad_2 = rectAngle2 * 3.14159265f / 180.0f;

        float rect1X = circleCenterX + circleRadius * cos(rad_1) - rectWidth / 2;
        float rect1Y = circleCenterY + circleRadius * sin(rad_1) - rectHeight / 2;

        float rect2X = circleCenterX + circleRadius * cos(rad_2) - rectWidth / 2;
        float rect2Y = circleCenterY + circleRadius * sin(rad_2) - rectHeight / 2;

        iSetColor(255, 0, 0); // red color
        iFilledRectangle((int)rect1X, (int)rect1Y, rectWidth, rectHeight);
        iFilledRectangle((int)rect2X, (int)rect2Y, rectWidth, rectHeight);  


        iShowLoadedImage((int)ground_x, 0, &groundImage);
        iShowLoadedImage((int)ground_x + SCREEN_WIDTH, 0, &groundImage);

        iSetColor(0, 0, 0);
        char scoreText[20];
        if (isHardLevel)
            sprintf(scoreText, "Score: Hard %d", score);
        else
            sprintf(scoreText, "Score: Medium %d", score);
        iText(10, SCREEN_HEIGHT - 130, scoreText);

        if (isEnteringName) {
        // Dim the background with a semi-transparent black overlay
        iSetColor(0, 0, 150); // RGBA, 150 alpha for transparency
        iFilledRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

        // Draw input box background
        int boxWidth = 400;
        int boxHeight = 100;
        int boxX = (SCREEN_WIDTH - boxWidth) / 2;
        int boxY = (SCREEN_HEIGHT - boxHeight) / 2;
        iSetColor(50, 50, 50);
        iFilledRectangle(boxX, boxY, boxWidth, boxHeight);

        // Draw input box border
        iSetColor(255, 255, 255);
        iRectangle(boxX, boxY, boxWidth, boxHeight);

        // Draw prompt text
        iText(boxX + 20, boxY + boxHeight - 30, "Game Over! Enter your name:");

        // Draw player name text
        iText(boxX + 20, boxY + boxHeight / 2 - 10, playerName);

        // Draw blinking cursor
        static int blinkTimer = 0;
        blinkTimer = (blinkTimer + 1) % 60; // roughly 1 second cycle at 60 FPS
        if (blinkTimer < 30) {
            int charWidth = 12;
             int cursorX = boxX + 20 + (int)(strlen(playerName) * charWidth);
            int cursorY = boxY + boxHeight / 2 - 10;
            iLine(cursorX, cursorY, cursorX, cursorY + 20);
        }

        // Draw instructions
        iText(boxX + 20, boxY + 10, "Press Enter to submit, Backspace to delete");
    }


      else  if (gameOver) {
            int imgX = SCREEN_WIDTH / 2 - 400;
            int imgY = SCREEN_HEIGHT / 2 - 250;
            iShowLoadedImage(imgX, imgY, &gameOverImage);
        }
    }
    else if (gameState == 2) {
        iShowLoadedImage(0, SCREEN_HEIGHT - helpContentHeight + helpScrollY - 200, &helpImage);
    }
    else if (gameState == 3) {
        iText(300, 300, "Continue Screen. Press 'H' to return.");
    }
    else if (gameState == 4) {
        if (scoreScrollY < 0) scoreScrollY = 0;
        if (scoreScrollY > scoreContentHeight - SCREEN_HEIGHT)
            scoreScrollY = scoreContentHeight - SCREEN_HEIGHT;
        iShowLoadedImage(0, SCREEN_HEIGHT - scoreContentHeight + scoreScrollY, &scoreImage);
    // Draw high scores
        iSetColor(255, 255, 255);
        int startY = SCREEN_HEIGHT - 400;
        iText(600, startY + 40, "High Scores:");
        for (int i = 0; i < 5; i++) {
            char buf[100];
            sprintf(buf, "%d. %s - %d", i + 1, highScores[i].name, highScores[i].score);
            iText(600, startY - i * 30, buf);
        }
    
    
    }
    else if (gameState == 5) {
        iText(300, 300, "Level Screen. Press 'H' to return.");
    }
}

// ================== EVENT FUNCTIONS ==================

void iMouseMove(int mx, int my)
{
    // In iGraphics.h, iMouseY = iScreenHeight - my, so use mx, my directly as they are already transformed
    // Reset hover states
    hoverPlay = false;
    hoverHelp = false;
    hoverExit = false;
    hoverContinue = false;
    hoverScore = false;
    hoverLevel = false;
    hoverEasy = false;
    hoverMedium = false;
    hoverHard = false;
    hoverBack = false;

    // Check for back button hover in all states except main menu
    if (gameState != 0) {
        hoverBack = (mx >= backBtnX && mx <= backBtnX + backBtnW &&
                     my >= backBtnY && my <= backBtnY + backBtnH);
    }

    // Check for main menu button hovers
    if (gameState == 0) {
        hoverPlay = (mx >= playX && mx <= playX + btnW && my >= playY && my <= playY + btnH);
        hoverHelp = (mx >= helpX && mx <= helpX + btnW && my >= helpY && my <= helpY + btnH);
        hoverExit = (mx >= exitX && mx <= exitX + btnW && my >= exitY && my <= exitY + btnH);
        hoverContinue = (mx >= continueX && mx <= continueX + btnW && my >= continueY && my <= continueY + btnH);
        hoverScore = (mx >= scoreX && mx <= scoreX + btnW && my >= scoreY && my <= scoreY + btnH);
        hoverLevel = (mx >= levelX && mx <= levelX + btnW && my >= levelY && my <= levelY + btnH);
    }
    // Check for level select button hovers
    else if (gameState == GAME_STATE_LEVEL_SELECT) {
        hoverEasy = (mx >= easyX && mx <= easyX + levelBtnW && my >= easyY && my <= easyY + levelBtnH);
        hoverMedium = (mx >= mediumX && mx <= mediumX + levelBtnW && my >= mediumY && my <= mediumY + levelBtnH);
        hoverHard = (mx >= hardX && mx <= hardX + levelBtnW && my >= hardY && my <= hardY + levelBtnH);
    }
}

void iMouse(int button, int state, int mx, int my)
{
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        // Handle back button click in all states except main menu
        if (gameState != 0 && mx >= backBtnX && mx <= backBtnX + backBtnW &&
            my >= backBtnY && my <= backBtnY + backBtnH) {
            gameState = 0;
            if (gameState == 1) {
                iPauseGame(); // Pause game timers when returning to main menu
            }
            return;
        }

        // Handle main menu button clicks
        if (gameState == 0) {
            if (mx >= playX && mx <= playX + btnW && my >= playY && my <= playY + btnH) {
                gameState = 1;
                resetGame();
            }
            else if (mx >= helpX && mx <= helpX + btnW && my >= helpY && my <= helpY + btnH) {
                gameState = 2;
            }
            else if (mx >= exitX && mx <= exitX + btnW && my >= exitY && my <= exitY + btnH) {
                exit(0);
            }
            else if (mx >= continueX && mx <= continueX + btnW && my >= continueY && my <= continueY + btnH) {
                gameState = 1;
                iResumeTimer(physicsTimer);
            }
            else if (mx >= scoreX && mx <= scoreX + btnW && my >= scoreY && my <= scoreY + btnH) {
                gameState = 4;
            }
            else if (mx >= levelX && mx <= levelX + btnW && my >= levelY && my <= levelY + btnH) {
                gameState = GAME_STATE_LEVEL_SELECT;
            }
        }
        // Handle level select button clicks
        else if (gameState == GAME_STATE_LEVEL_SELECT) {
            if (mx >= easyX && mx <= easyX + levelBtnW && my >= easyY && my <= easyY + levelBtnH) {
                isHardLevel = false;
                gameState = 1;
                resetGame();
            }
            else if (mx >= mediumX && mx <= mediumX + levelBtnW && my >= mediumY && my <= mediumY + levelBtnH) {
                isHardLevel = false;
                gameState = 1;
                resetGame();
            }
            else if (mx >= hardX && mx <= hardX + levelBtnW && my >= hardY && my <= hardY + levelBtnH) {
                isHardLevel = true;
                gameState = 1;
                resetGame();
            }
        }
    }
}

void iKeyboard(unsigned char key)
{

if (isEnteringName) {
        if (key == 13) { // Enter key
            isEnteringName = false;
            addHighScore(playerName, score);
            gameState = 4; // Show score screen
            nameCharIndex = 0;
            playerName[0] = '\0';
        }
        else if (key == 8) { // Backspace
            if (nameCharIndex > 0) {
                nameCharIndex--;
                playerName[nameCharIndex] = '\0';
            }
        }
        else if (nameCharIndex < 49 && key >= 32 && key <= 126) { // Printable chars
            playerName[nameCharIndex++] = key;
            playerName[nameCharIndex] = '\0';
        }
        return;
    }











    if (gameState == GAME_STATE_LEVEL_SELECT) {
        if (key == 'h' || key == 'b') {
            gameState = 0;
        }
    }
    else if (gameState == 0) {
        if (key == 'h' || key == 'b') {
            gameState = 0;
        }
    }
    else if (gameState == 1) {
        if (key == ' ') {
            if (!gameOver) bird_velocity = JUMP_VELOCITY;
            PlaySound(NULL, 0, 0);
            PlaySound(TEXT("flappy_jump.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT | SND_NOSTOP);
        }
        else if (key == 'r' || key == 'R') {
            resetGame();
        }
        else if (key == 27) {
            if (gameOver) {
                gameState = 0;
                iPauseTimer(physicsTimer);
            }
        }

        else if (key == 13) { // Enter key after game over
            if (gameOver) {
                isEnteringName = true;
                playerName[0] = '\0';
                nameCharIndex = 0;
            }
        }



    }
    else if (gameState == 2 || gameState == 3 || gameState == 4 || gameState == 5) {
        if (key == 'h' || key == 'b') {
            gameState = 0;
        }
    }
}

void iSpecialKeyboard(unsigned char key)
{
    // FIX: Consolidated scrolling logic
    if (gameState == 2 || gameState == 4) {
        int *scrollY = (gameState == 2) ? &helpScrollY : &scoreScrollY;
        int contentHeight = (gameState == 2) ? helpContentHeight : scoreContentHeight;

        if (key == GLUT_KEY_UP) {
            *scrollY -= 20;
            if (*scrollY < 0) *scrollY = 0;
        }
        else if (key == GLUT_KEY_DOWN) {
            *scrollY += 20;
            if (*scrollY > contentHeight - SCREEN_HEIGHT)
                *scrollY = contentHeight - SCREEN_HEIGHT;
        }
    }
    else if (gameState == 1) {
        if (key == GLUT_KEY_END) exit(0);
    }
}

void iMouseDrag(int mx, int my) {}
void iMouseWheel(int dir, int mx, int my) {}

// ================== CLEANUP FUNCTION ==================

void iCleanup()
{
    iFreeImage(&groundImage);
    iFreeImage(&background);
    iFreeImage(&play);
    iFreeImage(&playHover);
    iFreeImage(&help);
    iFreeImage(&helpHover);
    iFreeImage(&quit);
    iFreeImage(&quitHover);
    iFreeImage(&cont);
    iFreeImage(&contHover);
    iFreeImage(&scoreBtn);
    iFreeImage(&scoreBtnHover);
    iFreeImage(&level);
    iFreeImage(&levelHover);
    iFreeImage(&gameOverImage);
    iFreeImage(&BG);
    for (int i = 0; i < N_FRAMES; i++) {
        iFreeImage(&birdFrames[i]);
    }
    for (int i = 0; i < N_COINS; i++) {
        for (int f = 0; f < COIN_FRAMES; f++) {
            iFreeImage(&coinFrames[i][f]);
        }
    }
    for (int i = 0; i < N_CLOUDS; i++) {
        iFreeImage(&cloudImages[i]);
    }
    for (int i = 0; i < N_PIPES; i++) {
        iFreeImage(&lowerPipeImages[i]);
        iFreeImage(&upperPipeImages[i]);
    }
    for (int i = 0; i < GERM_FRAMES; i++) {
        iFreeImage(&germFrames[i]);
    }
    iFreeImage(&scoreImage);
    iFreeImage(&scoreDisplayImage);
    iFreeImage(&helpImage);
}

// ================== MAIN FUNCTION ==================
int rotatingRectTimer;

int main(int argc, char *argv[])
{
    srand((unsigned)time(NULL));
    glutInit(&argc, argv);

       iLoadImage(&backBtn, "back.png");
    iResizeImage(&backBtn, backBtnW, backBtnH);

    iLoadImage(&backBtnHover, "back_hover.png");
    iResizeImage(&backBtnHover, backBtnW, backBtnH);

    iLoadImage(&helpImage, "helpbg.png");
    iResizeImage(&helpImage, 800, 533); // example size for 0.66 scale

  rotatingRectTimer = iSetTimer(30, updateRotatingRectangles); // update every 30 ms


    iLoadImage(&scoreImage, "scorebg.png");
    iResizeImage(&scoreImage, 1040, 780); // example size for 1.3 scale

    iLoadImage(&gameOverImage, "gameoverpage.png");
    iResizeImage(&gameOverImage, 800, 600); // example size for 1.0 scale

    iLoadImage(&groundImage, "ground.png");
    int newWidth = SCREEN_WIDTH;
    int newHeight = 50; // adjusted from 100 - 50
    iResizeImage(&groundImage, newWidth, newHeight);

    iLoadImage(&scoreDisplayImage, "scr.png");
    iResizeImage(&scoreDisplayImage, 300, 100); // example size

    iLoadImage(&background, "jjkrealm.png");
    iResizeImage(&background, SCREEN_WIDTH, SCREEN_HEIGHT); // example size for 0.66 scale

    iLoadImage(&play, "newgame.png");
    iResizeImage(&play, 200, 100); // example size for 0.30 scale

    iLoadImage(&playHover, "newgamehover.png");
    iResizeImage(&playHover, 200, 100);

    iLoadImage(&help, "help.png");
    iResizeImage(&help,200 , 100);

    iLoadImage(&helpHover, "helphover.png");
    iResizeImage(&helpHover, 200, 100);

    iLoadImage(&quit, "quit.png");
    iResizeImage(&quit, 200, 100);

    iLoadImage(&quitHover, "exithover.png");
    iResizeImage(&quitHover, 200, 100);

    iLoadImage(&cont, "continue.png");
    iResizeImage(&cont, 200, 100);

    iLoadImage(&contHover, "continuehover.png");
    iResizeImage(&contHover, 200, 100);

    iLoadImage(&scoreBtn, "score.png");
    iResizeImage(&scoreBtn, 200, 100);

    iLoadImage(&scoreBtnHover, "scorehover.png");
    iResizeImage(&scoreBtnHover, 200, 100);

    iLoadImage(&level, "level.png");
    iResizeImage(&level, 200, 100);

    iLoadImage(&levelHover, "levelhover.png");
    iResizeImage(&levelHover, 200, 100);

    for (int i = 0; i < GERM_FRAMES; i++) {
        char filename[50];
        sprintf(filename, "kit (%d).png", i + 1);
        iLoadImage(&germFrames[i], filename);
        iResizeImage(&germFrames[i], GERM_WIDTH, GERM_HEIGHT);
    }

    iLoadImage(&BG, "new2.jpg");
    iResizeImage(&BG, SCREEN_WIDTH, SCREEN_HEIGHT);

    for (int i = 0; i < N_FRAMES; i++) {
        char filename[50];
        sprintf(filename, "bird (%d).png", i + 1);
        iLoadImage(&birdFrames[i], filename);
        iResizeImage(&birdFrames[i], BIRD_WIDTH + 10, BIRD_HEIGHT + 10);
    }

    for (int i = 0; i < N_COINS; i++) {
        for (int f = 0; f < COIN_FRAMES; f++) {
            char filename[50];
            sprintf(filename, "coin (%d).png", f + 1);
            iLoadImage(&coinFrames[i][f], filename);
            iResizeImage(&coinFrames[i][f], COIN_WIDTH, COIN_HEIGHT);
        }
    }

    for (int i = 0; i < N_CLOUDS; i++) {
        iLoadImage(&cloudImages[i], "cloud.png");
        iResizeImage(&cloudImages[i], 300, 150);
        cloud_x[i] = SCREEN_WIDTH + i * 400;
    }

    for (int i = 0; i < N_PIPES; i++) {
        iLoadImage(&lowerPipeImages[i], "lowerpipe.png");
        iLoadImage(&upperPipeImages[i], "upperpipe.png");
    }

      loadHighScores();

    resetGame();
    spawnGerms();

    animTimer = iSetTimer(46, updateBirdAnimation);
    coinAnimTimer = iSetTimer(16, updateCoinAnimation);
    germAnimTimer = iSetTimer(16, updateGermAnimation);
    physicsTimer = iSetTimer(16, updateAll);
    beamSpawnTimer = iSetTimer(3000, spawnBeams);
    iPauseTimer(physicsTimer);

    iInitialize(SCREEN_WIDTH, SCREEN_HEIGHT, "Flappy Bird");
    atexit(iCleanup);
    return 0;
}