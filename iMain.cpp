
#include "iGraphics.h"
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <mmsystem.h> 


// ================== GAME CONSTANTS ==================
#define N_CLOUDS 4
#define N_PIPES 8
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
#define BEAM_WIDTH 10
#define BEAM_HEIGHT 60
#define BEAM_VELOCITY 8
#define N_GERMS 4
#define GERM_WIDTH 40
#define GERM_HEIGHT 40
#define GERM_SPEED 6



//germs
float germ_x[N_GERMS];
float germ_y[N_GERMS];
bool germ_active[N_GERMS] = {false};
Image germImage;

int germSpawnTimer;


Image scoreDisplayImage;  
float beam_x[N_BEAMS];
float beam_y[N_BEAMS];
bool beam_active[N_BEAMS] = {false};

int beamSpawnTimer;

// ==================GLOBAL VARIABLES ==================

// Game state
int gameState = 0; // 0 = Home, 1 = Game, 2 = Help, 3 = Continue, 4 = Score, 5 = Level

// Homepage variables
Image background; // Homepage background
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
Image cont, contHover;//
Image scoreBtn, scoreBtnHover;  
Image level, levelHover;

Image gameOverImage;

Image groundImage;
int helpScrollY = 0; 
const int helpContentHeight = 1600;
Image helpImage;

// Game variables
Image cloudImages[N_CLOUDS];
float cloud_x[N_CLOUDS];
float cloud_y[N_CLOUDS] = {300, 350, 300, 320};
Image lowerPipeImages[N_PIPES];
Image upperPipeImages[N_PIPES];
Image BG; // Game background
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
int score = 0; // Game score (int)
bool gameOver = false;
bool scoreCountedPerPipe[N_PIPES] = {false};
int animTimer, physicsTimer, coinAnimTimer;

//score
int scoreScrollY = 0; // Vertical scroll offset for score page
const int scoreContentHeight = 1600;
Image scoreImage;
   

//sound


void spawnGerms()
{
    for (int i = 0; i < N_GERMS; i++) {
        germ_x[i] = SCREEN_WIDTH + i * 300; // spaced out horizontally
        germ_y[i] = 100 + rand() % (SCREEN_HEIGHT - 200);
        germ_active[i] = true;
    }
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

        // Collision with bird
      if (!gameOver && bird_x + BIRD_WIDTH > germ_x[i] && bird_x < germ_x[i] + GERM_WIDTH &&
            bird_y + BIRD_HEIGHT > germ_y[i] && bird_y < germ_y[i] + GERM_HEIGHT) {
            gameOver = true;
            bird_velocity = 0; // stop bird movement
            iPauseTimer(physicsTimer);
            PlaySound(NULL, 0, 0);
            PlaySound(TEXT("game_over.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
            germ_active[i] = false;
        }
    }
}


void spawnBeams()
{
    float baseY = 100 + rand() % (SCREEN_HEIGHT - 200);
    float baseX = SCREEN_WIDTH;

    for (int i = 0; i < N_BEAMS; i++) {
        beam_x[i] = baseX + i * (BEAM_WIDTH + 30); // horizontal spacing
        beam_y[i] = baseY + i * 70; // vertical spacing
        beam_active[i] = true;
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




// ================== GAME FUNCTIONS ==================

void resetGame()
{
    bird_y = 400;
    bird_velocity = 0;

    int groupSpacing = SCREEN_WIDTH / 2;

    for (int group = 0; group < N_PIPES / 4; group++) {
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

    for (int i = 0; i < N_COINS; i++) {
        coin_x[i] = SCREEN_WIDTH + i * 300;
        coin_y[i] = 100 + rand() % (SCREEN_HEIGHT - 200);
    }
  spawnBeams();
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
        PlaySound(NULL, 0, 0); 
        PlaySound(TEXT("game_over.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
    }
     for (int i = 0; i < N_BEAMS; i++) {
        if (beam_active[i] &&
            bird_x + BIRD_WIDTH > beam_x[i] && bird_x < beam_x[i] + BEAM_WIDTH &&
            bird_y + BIRD_HEIGHT > beam_y[i] && bird_y < beam_y[i] + BEAM_HEIGHT) {
            gameOver = true;
            iPauseTimer(physicsTimer);
            PlaySound(NULL, 0, 0);
            PlaySound(TEXT("game_over.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
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
             PlaySound(NULL, 0, 0); 
            PlaySound(TEXT("game_over.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
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
        // Home screen
        iShowLoadedImage(0, 0, &background);
        iShowLoadedImage(playX, playY, hoverPlay ? &playHover : &play);
        iShowLoadedImage(helpX, helpY, hoverHelp ? &helpHover : &help);
        iShowLoadedImage(exitX, exitY, hoverExit ? &quitHover : &quit);
        iShowLoadedImage(continueX, continueY, hoverContinue ? &contHover : &cont);
        iShowLoadedImage(scoreX, scoreY, hoverScore ? &scoreBtnHover : &scoreBtn);
        iShowLoadedImage(levelX, levelY, hoverLevel ? &levelHover : &level);
    }
    else if (gameState == 1) {

       
        
         for (int i = 0; i < N_BEAMS; i++) {
            if (beam_active[i]) {
                iSetColor(255, 0, 0); // red color
                iFilledRectangle((int)beam_x[i], (int)beam_y[i], BEAM_WIDTH, BEAM_HEIGHT);
            }
        }
        // Game screen (Flappy Bird)
        // iShowLoadedImage(0, 0, &BG);
         iSetColor(135, 206, 235); // RGB for sky blue
        iFilledRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        for (int i = 0; i < N_CLOUDS; i++) {
            iShowLoadedImage((int)cloud_x[i], (int)cloud_y[i], &cloudImages[i]);
        }
        for (int i = 0; i < N_COINS; i++) {
            iShowLoadedImage((int)coin_x[i], (int)coin_y[i], &coinFrames[i][coinFrameIndex[i]]);
        }
       if (!gameOver) {
            iShowLoadedImage((int)bird_x, (int)bird_y, &birdFrames[flyingFrame]);
        }

         for (int i = 0; i < N_GERMS; i++) {
            if (germ_active[i]) {
                iShowLoadedImage((int)germ_x[i], (int)germ_y[i], &germImage);
            }
        }  
        for (int i = 0; i < N_PIPES; i++) {
            iShowLoadedImage(pipe_x[i], 0, &lowerPipeImages[i]);
            iShowLoadedImage(pipe_x[i], pipe_gap_y[i] + PIPE_GAP, &upperPipeImages[i]);
        }


      
           
    


        iSetColor(0,0,0);
        char scoreText[20];
        sprintf(scoreText, "Score:This is medium %d", score);
      iText(10, SCREEN_HEIGHT - 130, scoreText);
        if (gameOver) {
         
            int imgX = SCREEN_WIDTH / 2 - 500; 
            int imgY = SCREEN_HEIGHT / 2 - 150;
            iShowLoadedImage(imgX, imgY, &gameOverImage);

           
        }
          iShowLoadedImage(0, 0, &groundImage);
    }
    else if (gameState == 2) {
        


iShowLoadedImage(0, SCREEN_HEIGHT - helpContentHeight + helpScrollY-200, &helpImage);
       
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
    else if (gameState == 1) {
       
    }
}

void iMouse(int button, int state, int mx, int my)
{
    if (gameState == 0 && button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        if (hoverPlay) {
            gameState = 1;
            resetGame(); // Start new game
        }
        else if (hoverHelp) gameState = 2;
        else if (hoverExit) exit(0);
        else if (hoverContinue) {
            gameState = 1; // Continue game
            iResumeTimer(physicsTimer);
        }
        else if (hoverScore) gameState = 4;
        else if (hoverLevel) gameState = 5;
    }
    else if (gameState == 1) {
        
    }
}

void iKeyboard(unsigned char key)
{
    if (gameState == 0) {
        if (key == 'h' || key == 'b') gameState = 0;
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
        if (key == 'h' || key == 'b') gameState = 0;
    }
}

void iSpecialKeyboard(unsigned char key)
{ if (gameState == 2) {
        if (key == GLUT_KEY_UP) {
            helpScrollY -= 20; // Scroll up
            if (helpScrollY < 0) helpScrollY = 0;
        }
        else if (key == GLUT_KEY_DOWN) {
            helpScrollY += 20;

        }
          if (helpScrollY < 0) helpScrollY = 0;
        if (helpScrollY > helpContentHeight - SCREEN_HEIGHT)
            helpScrollY = helpContentHeight - SCREEN_HEIGHT;
    }
       else if (gameState == 4) { 
        if (key == GLUT_KEY_UP) {
            scoreScrollY -= 20; // Scroll up
            if (scoreScrollY < 0) scoreScrollY = 0;
        }
        else if (key == GLUT_KEY_DOWN) {
            scoreScrollY += 20; // Scroll down
        }
        if (scoreScrollY < 0) scoreScrollY = 0;
        if (scoreScrollY > scoreContentHeight - SCREEN_HEIGHT)
            scoreScrollY = scoreContentHeight - SCREEN_HEIGHT;
    }
   else if (gameState == 1) {
        if (key == GLUT_KEY_END) exit(0);
    }
}

void iMouseDrag(int mx, int my)
{
    if (gameState == 1) {
        
    }
}

void iMouseWheel(int dir, int mx, int my)
{
    if (gameState == 1) {
       
    }
}

// ================== CLEANUP FUNCTION ==================






void iCleanup()
{
    // Homepage resources
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

    // Game resources
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
}

// ================== MAIN FUNCTION ==================

int main(int argc, char *argv[])
{
    srand((unsigned)time(NULL));
    glutInit(&argc, argv);

    // Load homepage 
       iLoadImage(&germImage, "kit (1).png"); // Load germ image
    iResizeImage(&germImage, GERM_WIDTH, GERM_HEIGHT);
       iLoadImage(&helpImage, "helpbg.png"); 
   
        iScaleImage(&helpImage, 0.66);

        
    iLoadImage(&scoreImage, "scorebg.png");
     iScaleImage(&scoreImage, 1.3);

     iLoadImage(&gameOverImage, "gameoverpage.png");
    iScaleImage(&gameOverImage, 1.0);
    iLoadImage(&groundImage, "ground.png"); 

       iLoadImage(&scoreDisplayImage, "scr.png");
    iScaleImage(&scoreDisplayImage, 1.0);

int newWidth = SCREEN_WIDTH;
int newHeight = 100;

iResizeImage(&groundImage, newWidth, newHeight);
  
    iLoadImage(&background, "jjkrealm.png");
    iScaleImage(&background, 0.66);
    iLoadImage(&play, "newgame.png");
    iScaleImage(&play, 0.30);
    iLoadImage(&playHover, "newgamehover.png");
    iScaleImage(&playHover, 0.30);
    iLoadImage(&help, "help.png");
    iScaleImage(&help, 0.30);
    iLoadImage(&helpHover, "helphover.png");
    iScaleImage(&helpHover, 0.30);
    iLoadImage(&quit, "quit.png");
    iScaleImage(&quit, 0.30);
    iLoadImage(&quitHover, "exithover.png");
    iScaleImage(&quitHover, 0.30);
    iLoadImage(&cont, "continue.png");
    iScaleImage(&cont, 0.30);
    iLoadImage(&contHover, "continuehover.png");
    iScaleImage(&contHover, 0.30);
    iLoadImage(&scoreBtn, "score.png");
    iScaleImage(&scoreBtn, 0.30);
    iLoadImage(&scoreBtnHover, "scorehover.png");
    iScaleImage(&scoreBtnHover, 0.30);
    iLoadImage(&level, "level.png");
    iScaleImage(&level, 0.30);
    iLoadImage(&levelHover, "levelhover.png");
    iScaleImage(&levelHover, 0.30);

    // Load game resources
    iLoadImage(&BG, "new2.jpg");
    iScaleImage(&BG, 1.0);
    for (int i = 0; i < N_FRAMES; i++) {
        char filename[50];
        sprintf(filename, "bird (%d).png", i+1);
        iLoadImage(&birdFrames[i], filename);
        iResizeImage(&birdFrames[i], BIRD_WIDTH+10, BIRD_HEIGHT+10);
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
    iSetTimer(8, updateClouds);
    animTimer = iSetTimer(80, updateBirdAnimation);
    physicsTimer = iSetTimer(20, updateGame);
      iPauseTimer(physicsTimer);
    iSetTimer(20, updateCoins);
        iSetTimer(20, updateBeams);
         iSetTimer(20, updateGerms);
    beamSpawnTimer = iSetTimer(1000, spawnBeams);

    coinAnimTimer = iSetTimer(100, updateCoinAnimation);

    iInitialize(SCREEN_WIDTH, SCREEN_HEIGHT, "Flappy Bird");
    atexit(iCleanup);
    return 0;
}
