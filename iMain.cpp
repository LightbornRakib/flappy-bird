
#include "iGraphics.h"
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <mmsystem.h>

// ================== GAME CONSTANTS ==================
#define N_CLOUDS 4
#define N_PIPES 8
#define N_BEAM_GROUP 3 // FIX: Set to match N_BEAMS
#define SCREEN_WIDTH 1600
#define SCREEN_HEIGHT 768
#define N_COINS 5
#define COIN_WIDTH 30
#define COIN_HEIGHT 30
#define COIN_SPEED 7
#define COIN_FRAMES 6
#define BIRD_WIDTH 50
#define BIRD_HEIGHT 50
#define PIPE_WIDTH 80
#define PIPE_GAP 250
#define GRAVITY 0.5f
#define JUMP_VELOCITY 10.0f
#define PIPE_SPEED 7
#define N_FRAMES 5
#define N_BEAMS 3
#define BEAM_WIDTH 20
#define BEAM_HEIGHT 10
#define BEAM_VELOCITY 8
#define N_GERMS 2
#define GERM_WIDTH 40
#define GERM_HEIGHT 40
#define GERM_SPEED 6
#define GAME_STATE_LEVEL_SELECT 6
#define GERM_FRAMES 3

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

float germ_x[N_GERMS];
float germ_y[N_GERMS];
bool germ_active[N_GERMS] = {false};
float ground_x = 0;

int germSpawnTimer;

Image scoreDisplayImage;
float beam_x[N_BEAMS];
float beam_y[N_BEAMS];
bool beam_active[N_BEAMS] = {false};

float pipeRotationAngle[N_PIPES] = {0};
float pipeRotationSpeed[N_PIPES] = {0};

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
    for (int i = 0; i < N_BEAMS; i++) {
        float candidateY;
        bool overlaps;
        int attempts = 0;

        do {
            overlaps = false;
            candidateY = 100 + rand() % (SCREEN_HEIGHT - 200);

            for (int group = 0; group < N_PIPES / 4; group++) {
                int basePipeIndex = group * 4;
                int gapY = pipe_gap_y[basePipeIndex];
                int gapTop = gapY + PIPE_GAP;

                if (candidateY + BEAM_HEIGHT > gapY && candidateY < gapTop) {
                    overlaps = true;
                    break;
                }
            }
            attempts++;
            if (attempts > 10) break;
        } while (overlaps);

        beam_x[i] = baseX + i * (BEAM_WIDTH + 30);
        beam_y[i] = candidateY;
        beam_active[i] = true;
    }
    // FIX: Corrected loop to use N_BEAMS
    for (int i = N_BEAMS; i < N_BEAMS; i++) {
        beam_active[i] = false;
    }
}

void updateBeams()
{
    for (int i = 0; i < N_BEAMS; i++) {
        if (!beam_active[i]) continue;

        beam_x[i] -= BEAM_VELOCITY;
        if (beam_x[i] + BEAM_WIDTH < 0) {
            beam_active[i] = false;
        }
    }
}

void resetGame()
{
    bird_y = 400;
    bird_velocity = 0;
    gameOverSoundPlayed = false; // FIX: Reset sound flag

    for (int i = 0; i < N_PIPES; i++) {
        pipeRotationAngle[i] = 0;
        pipeRotationSpeed[i] = 0;
    }

    int totalGroups = N_PIPES / 4;
    for (int group = 0; group < totalGroups; group++) {
        bool rotateGroup = ((group + 1) % 3 == 0);
        for (int i = 0; i < 4; i++) {
            int idx = group * 4 + i;
            pipeRotationSpeed[idx] = rotateGroup ? 1.0f : 0.0f;
        }
    }

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

void updateGame()
{
    if (gameOver) return;

    bird_velocity -= GRAVITY;
    if (bird_velocity < -2 * JUMP_VELOCITY) bird_velocity = -2 * JUMP_VELOCITY;
    bird_y += bird_velocity;

    for (int i = 0; i < N_PIPES; i++) {
        pipeRotationAngle[i] += pipeRotationSpeed[i];
        if (pipeRotationAngle[i] >= 360.0f) pipeRotationAngle[i] -= 360.0f;
        else if (pipeRotationAngle[i] < 0) pipeRotationAngle[i] += 360.0f;
    }

    if (bird_y < 0) {
        bird_y = 0;
        gameOver = true;
        iPauseTimer(physicsTimer);
        if (!gameOverSoundPlayed) { // FIX: Play sound once
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
            if (!gameOverSoundPlayed) { // FIX: Play sound once
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

                int topPipeHeight = SCREEN_HEIGHT - (pipe_gap_y[idx] + PIPE_GAP); // FIX: Local declaration
                if (topPipeHeight < 0) topPipeHeight = 0;

                iResizeImage(&lowerPipeImages[idx], PIPE_WIDTH, pipe_gap_y[idx]);
                iResizeImage(&upperPipeImages[idx], PIPE_WIDTH, topPipeHeight);

                scoreCountedPerPipe[idx] = false;
            }
        }
    }

    for (int i = 0; i < N_PIPES; i++) {
        // Unchanged pipe collision logic as per user request
        if (bird_x + BIRD_WIDTH > pipe_x[i] && bird_x < pipe_x[i] + PIPE_WIDTH &&
            (bird_y < pipe_gap_y[i] || bird_y + BIRD_HEIGHT > pipe_gap_y[i] + PIPE_GAP)) {
            gameOver = true;
            iPauseTimer(physicsTimer);
            if (!gameOverSoundPlayed) { // FIX: Play sound once
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
       
        for (int i = 0; i < N_PIPES; i++) {
            double lowerCenterX = pipe_x[i] + PIPE_WIDTH / 2.0;
            double lowerCenterY = pipe_gap_y[i] / 2.0;
            double upperPipeY = pipe_gap_y[i] + PIPE_GAP;
            double upperPipeHeight = SCREEN_HEIGHT - upperPipeY;
            double upperCenterX = pipe_x[i] + PIPE_WIDTH / 2.0;
            double upperCenterY = upperPipeY + upperPipeHeight / 2.0;

            iRotate(lowerCenterX, lowerCenterY, pipeRotationAngle[i]);
            iShowLoadedImage((int)(lowerCenterX - PIPE_WIDTH / 2), (int)(lowerCenterY - pipe_gap_y[i] / 2), &lowerPipeImages[i]);
            iRotate(lowerCenterX, lowerCenterY, -pipeRotationAngle[i]);

            iRotate(upperCenterX, upperCenterY, pipeRotationAngle[i]);
            iShowLoadedImage((int)(upperCenterX - PIPE_WIDTH / 2), (int)(upperCenterY - upperPipeHeight / 2), &upperPipeImages[i]);
            iRotate(upperCenterX, upperCenterY, -pipeRotationAngle[i]);
        }


        
         if (!gameOver) {
            iShowLoadedImage((int)bird_x, (int)bird_y, &birdFrames[flyingFrame]);
        }

        for (int i = 0; i < N_GERMS; i++) {
            if (germ_active[i]) {
                iShowLoadedImage((int)germ_x[i], (int)germ_y[i], &germFrames[germFrameIndex]);
            }
        }


         iShowLoadedImage((int)ground_x, 0, &groundImage);
        iShowLoadedImage((int)ground_x + SCREEN_WIDTH, 0, &groundImage);

        iSetColor(0, 0, 0);
        char scoreText[20];
        // FIX: Dynamic score text
        if (isHardLevel)
            sprintf(scoreText, "Score: Hard %d", score);
        else
            sprintf(scoreText, "Score: Medium %d", score);
        iText(10, SCREEN_HEIGHT - 130, scoreText);
        if (gameOver) {
            int imgX = SCREEN_WIDTH / 2 - 500;
            int imgY = SCREEN_HEIGHT / 2 - 150;
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
    }
    else if (gameState == 5) {
        iText(300, 300, "Level Screen. Press 'H' to return.");
    }
}

// ================== EVENT FUNCTIONS ==================

void iMouseMove(int mx, int my)
{
    if (gameState == 0) {
        hoverPlay = (mx >= playX && mx <= playX + btnW && my >= playY && my <= playY + btnH);
        hoverHelp = (mx >= helpX && mx <= helpX + btnW && my >= helpY && my <= helpY + btnH);
        hoverExit = (mx >= exitX && mx <= exitX + btnW && my >= exitY && my <= exitY + btnH);
        hoverContinue = (mx >= continueX && mx <= continueX + btnW && my >= continueY && my <= continueY + btnH);
        hoverScore = (mx >= scoreX && mx <= scoreX + btnW && my >= scoreY && my <= scoreY + btnH);
        hoverLevel = (mx >= levelX && mx <= levelX + btnW && my >= levelY && my <= levelY + btnH);
    }
    else if (gameState == GAME_STATE_LEVEL_SELECT) {
        hoverEasy = (mx >= easyX && mx <= easyX + levelBtnW && my >= easyY && my <= easyY + levelBtnH);
        hoverMedium = (mx >= mediumX && mx <= mediumX + levelBtnW && my >= mediumY && my <= mediumY + levelBtnH);
        hoverHard = (mx >= hardX && mx <= hardX + levelBtnW && my >= hardY && my <= hardY + levelBtnH);
    }
}

void iMouse(int button, int state, int mx, int my)
{
    if (gameState == 0 && button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        if (hoverPlay) {
            gameState = 1;
            resetGame();
        }
        else if (hoverHelp) {
            gameState = 2;
        }
        else if (hoverExit) {
            exit(0);
        }
        else if (hoverContinue) {
            gameState = 1;
            iResumeTimer(physicsTimer);
        }
        else if (hoverScore) {
            gameState = 4;
        }
        else if (hoverLevel) {
            gameState = GAME_STATE_LEVEL_SELECT;
        }
    }
    else if (gameState == GAME_STATE_LEVEL_SELECT && button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        if (hoverEasy) {
            isHardLevel = false; // FIX: Set difficulty
            gameState = 1;
            resetGame();
        }
        else if (hoverMedium) {
            isHardLevel = false;
            gameState = 1;
            resetGame();
        }
        else if (hoverHard) {
            isHardLevel = true;
            gameState = 1;
            resetGame();
        }
    }
}

void iKeyboard(unsigned char key)
{
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

int main(int argc, char *argv[])
{
    srand((unsigned)time(NULL));
    glutInit(&argc, argv);

    iLoadImage(&helpImage, "helpbg.png");
    iResizeImage(&helpImage, 800, 533); // example size for 0.66 scale

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
    iResizeImage(&background, 1056, 704); // example size for 0.66 scale

    iLoadImage(&play, "newgame.png");
    iResizeImage(&play, 60, 60); // example size for 0.30 scale

    iLoadImage(&playHover, "newgamehover.png");
    iResizeImage(&playHover, 60, 60);

    iLoadImage(&help, "help.png");
    iResizeImage(&help, 60, 60);

    iLoadImage(&helpHover, "helphover.png");
    iResizeImage(&helpHover, 60, 60);

    iLoadImage(&quit, "quit.png");
    iResizeImage(&quit, 60, 60);

    iLoadImage(&quitHover, "exithover.png");
    iResizeImage(&quitHover, 60, 60);

    iLoadImage(&cont, "continue.png");
    iResizeImage(&cont, 60, 60);

    iLoadImage(&contHover, "continuehover.png");
    iResizeImage(&contHover, 60, 60);

    iLoadImage(&scoreBtn, "score.png");
    iResizeImage(&scoreBtn, 60, 60);

    iLoadImage(&scoreBtnHover, "scorehover.png");
    iResizeImage(&scoreBtnHover, 60, 60);

    iLoadImage(&level, "level.png");
    iResizeImage(&level, 60, 60);

    iLoadImage(&levelHover, "levelhover.png");
    iResizeImage(&levelHover, 60, 60);

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