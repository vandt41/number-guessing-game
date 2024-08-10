// This is the newest state of the game
// type make in terminal to build the game
// This is the newest state of the game
// type make in terminal to build the game
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <SDL2/SDL_ttf.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <SDL2/SDL_mixer.h>

#define WINDOW_HEIGHT 480
#define WINDOW_WIDTH 720
#define MAX_SCORES 5

// Structure to store player scores
typedef struct
{
    char username[50];
    int time;
    double successRatio;
} Score;

// Declare global variables
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Event e;
bool running = true;
bool gameFinished = true;
int gameLevel = 1;
int number_length = 4;
time_t startTime;
int attempts = 0, correctGuesses = 0;
int timeTaken;
double successRatio = (double)0;
SDL_Texture *backgroundTexture = NULL;
TTF_Font *font = NULL;
Mix_Music *bgMusic = NULL;
SDL_Color color = {0, 0, 0};

// Function prototypes
void initSDL();
void closeSDL();
void loadResources();
void closeResources();
void randomNumber(char *magicNumber, int number_length);
void formatGuess(const char *magicNumber, const char *guessed, char *formatted, int number_length);
void getUsername(char *username, int maxLen);
void gameLoop(const char *magicNumber, int number_length, const char *username, int *attempts, int *correctGuesses);
void readHighScores(Score scores[], int *scoreCount);
void saveHighScores(Score scores[], int scoreCount);
int compareScores(const void *a, const void *b);
void showHighScores();

// Main function
// Main function
int main(int argc, char *argv[])
{
    // Initialize SDL
    initSDL();

    // Load resources
    loadResources();

    // Get player's username
    char username[50];
    getUsername(username, sizeof(username));
    startTime = time(NULL);

    // Initialize attempts and correctGuesses
    attempts = 0;
    correctGuesses = 0;

    // Start the game loop
    while (running)
    {
        // Generate a random magic number
        char magicNumber[number_length + 1];
        randomNumber(magicNumber, number_length);
        gameLoop(magicNumber, number_length, username, &attempts, &correctGuesses);

        if (!gameFinished)
        {
            break;
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        if (backgroundTexture)
        {
            SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);
        }

        // Check if player has completed all levels
        if (gameLevel == 3)
        {
            // Calculate ratio and save scores
            if (attempts > 0)
            {
                successRatio = (double)correctGuesses / attempts * 100.0;
            }
            else
            {
                successRatio = 0.0;
            }

            Score scores[MAX_SCORES + 1];
            int scoreCount = 0;
            readHighScores(scores, &scoreCount);

            if (scoreCount < MAX_SCORES)
            {
                scores[scoreCount].time = timeTaken;
                scores[scoreCount].successRatio = successRatio;
                strcpy(scores[scoreCount].username, username);
                scoreCount++;
            }
            else
            {
                scores[MAX_SCORES].time = timeTaken;
                scores[MAX_SCORES].successRatio = successRatio;
                strcpy(scores[MAX_SCORES].username, username);
                scoreCount = MAX_SCORES + 1;
            }

            if (scoreCount > MAX_SCORES)
            {
                saveHighScores(scores, MAX_SCORES);
            }
            else
            {
                saveHighScores(scores, scoreCount);
            }

            showHighScores();
            gameLevel = 1;
            number_length = 4;
            continue;
        }

        // Render next level prompt
        SDL_Surface *messageSurface = TTF_RenderText_Solid(font, "Press Enter to move to next level...", color);
        SDL_Texture *messageTexture = SDL_CreateTextureFromSurface(renderer, messageSurface);
        SDL_Rect messageRect = {20, 400, messageSurface->w, messageSurface->h};
        SDL_FreeSurface(messageSurface);
        SDL_RenderCopy(renderer, messageTexture, NULL, &messageRect);
        SDL_RenderPresent(renderer);
        SDL_DestroyTexture(messageTexture);
        while (SDL_WaitEvent(&e))
        {
            if (e.type == SDL_QUIT)
            {
                gameFinished = false;
                running = false;
                break;
            }
            else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RETURN)
            {
                gameLevel++;
                number_length++;
                randomNumber(magicNumber, number_length);
                break;
            }
        }
    }

    // Close resources and quit SDL
    closeResources();
    closeSDL();
    return 0;
}

// Function to initialize SDL
void initSDL()
{
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
    {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        exit(1);
    }

    // Initialize Window
    window = SDL_CreateWindow("Number Guessing Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL)
    {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        exit(1);
    }

    // Render Window
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL)
    {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        exit(1);
    }

    // Check if Font is opened
    if (TTF_Init() == -1)
    {
        printf("TTF_Init: %s\n", TTF_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        exit(1);
    }

    // Initialize Music
    if (Mix_Init(MIX_INIT_MP3 | MIX_INIT_OGG) != (MIX_INIT_MP3 | MIX_INIT_OGG))
    {
        printf("Mix_Init: %s\n", Mix_GetError());
        TTF_Quit();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        exit(1);
    }

    // Load music
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        printf("Mix_OpenAudio: %s\n", Mix_GetError());
        Mix_Quit();
        TTF_Quit();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        exit(1);
    }

    // Load background music
    bgMusic = Mix_LoadMUS("background_music.mp3");
    if (bgMusic == NULL)
    {
        printf("Mix_LoadMUS: %s\n", Mix_GetError());
        Mix_CloseAudio();
        Mix_Quit();
        TTF_Quit();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        exit(1);
    }

    // Play background music in an infinite loop
    if (Mix_PlayMusic(bgMusic, -1) == -1)
    {
        printf("Mix_PlayMusic: %s\n", Mix_GetError());
        Mix_FreeMusic(bgMusic);
        Mix_CloseAudio();
        Mix_Quit();
        TTF_Quit();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        exit(1);
    }
}

// Function to close SDL
void closeSDL()
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    TTF_Quit();
    if (bgMusic != NULL)
    {
        Mix_FreeMusic(bgMusic);
    }
    Mix_CloseAudio();
    Mix_Quit();
}

// Function to load resources
void loadResources()
{
    // Load font
    font = TTF_OpenFont("game.otf", 24);
    if (!font)
    {
        printf("TTF_OpenFont: %s\n", TTF_GetError());
        closeSDL();
        exit(1);
    }

    // Load background texture
    SDL_Surface *backgroundSurface = SDL_LoadBMP("background.bmp");
    if (!backgroundSurface)
    {
        printf("SDL_LoadBMP Error: %s\n", SDL_GetError());
        return;
    }
    backgroundTexture = SDL_CreateTextureFromSurface(renderer, backgroundSurface);
    SDL_FreeSurface(backgroundSurface);
}

// Function to close resources
void closeResources()
{
    if (font)
    {
        TTF_CloseFont(font);
    }
    if (backgroundTexture)
    {
        SDL_DestroyTexture(backgroundTexture);
    }
}

// Function to generate a random magic number
void randomNumber(char *magicNumber, int number_length)
{
    srand((unsigned int)time(NULL));
    for (int i = 0; i < number_length; i++)
    {
        magicNumber[i] = rand() % 10 + '0';
    }
    magicNumber[number_length] = '\0';
}

// Function to format the guessed number
void formatGuess(const char *magicNumber, const char *guessed, char *formatted, int number_length)
{
    for (int i = 0; i < number_length; i++)
    {
        if (guessed[i] == magicNumber[i])
        {
            formatted[i] = guessed[i];
        }
        else
        {
            formatted[i] = '-';
        }
    }
    formatted[number_length] = '\0';
}

// Function to get the player's username
void getUsername(char *username, int maxLen)
{
    SDL_StartTextInput();
    strcpy(username, "");

    bool inputRunning = true;
    bool validInput = false;
    SDL_Event e;

    while (inputRunning && running)
    {
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
            {
                gameFinished = false;
                inputRunning = false;
                running = false;
                break;
            }
            else if (e.type == SDL_TEXTINPUT)
            {
                if (strlen(username) < maxLen - 1)
                {
                    strcat(username, e.text.text);
                }
            }
            else if (e.type == SDL_KEYDOWN)
            {
                if (e.key.keysym.sym == SDLK_BACKSPACE && strlen(username) > 0)
                {
                    username[strlen(username) - 1] = '\0';
                }
                else if (e.key.keysym.sym == SDLK_RETURN)
                {
                    // Check for spaces in the username
                    if (strchr(username, ' ') != NULL)
                    {
                        // Clear the username
                        strcpy(username, "");
                        // Mark input as invalid
                        validInput = false;
                        // Break out of the loop to re-prompt
                        break;
                    }
                    // Mark input as valid
                    validInput = true;
                    // End the input loop
                    inputRunning = false;
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        if (backgroundTexture)
        {
            SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);
        }

        char promptText[100];
        if (!validInput)
        {
            snprintf(promptText, sizeof(promptText), "Enter Username (no spaces allowed): %s", username);
        }
        else
        {
            snprintf(promptText, sizeof(promptText), "Enter Username: %s", username);
        }

        SDL_Surface *promptSurface = TTF_RenderText_Solid(font, promptText, color);
        SDL_Texture *promptTexture = SDL_CreateTextureFromSurface(renderer, promptSurface);
        SDL_Rect promptRect = {20, 20, promptSurface->w, promptSurface->h};
        SDL_FreeSurface(promptSurface);

        SDL_RenderCopy(renderer, promptTexture, NULL, &promptRect);
        SDL_RenderPresent(renderer);
        SDL_DestroyTexture(promptTexture);

        SDL_Delay(10);
    }
    SDL_StopTextInput();
}

// Function to handle the game loop
// Function to handle the game loop
void gameLoop(const char *magicNumber, int number_length, const char *username, int *attempts, int *correctGuesses)
{
    printf("Magic number(for debugging): %s\n", magicNumber);
    char guessed[number_length + 1];
    char formatted[number_length + 1];
    char initialDisplay[number_length + 1];
    guessed[number_length] = '\0';
    formatted[number_length] = '\0';
    memset(initialDisplay, '-', number_length);
    memset(guessed, ' ', number_length);
    memset(formatted, '-', number_length);

    initialDisplay[number_length] = '\0';
    int DigitCount = 0;
    SDL_Color color = {255, 153, 51};
    SDL_Texture *messageTexture = NULL;
    SDL_Rect messageRect;

    char usernameText[100];
    snprintf(usernameText, sizeof(usernameText), "Player: %s", username);
    SDL_Surface *usernameSurface = TTF_RenderText_Solid(font, usernameText, color);
    SDL_Texture *usernameTexture = SDL_CreateTextureFromSurface(renderer, usernameSurface);
    SDL_Rect usernameRect = {20, 40, usernameSurface->w, usernameSurface->h};
    SDL_FreeSurface(usernameSurface);

    char levelText[10];
    snprintf(levelText, sizeof(levelText), "Level: %d", gameLevel);
    SDL_Surface *levelSurface = TTF_RenderText_Solid(font, levelText, color);
    SDL_Texture *levelTexture = SDL_CreateTextureFromSurface(renderer, levelSurface);
    SDL_Rect levelRect = {20, 10, levelSurface->w, levelSurface->h};
    SDL_FreeSurface(levelSurface);

    bool gameRunning = true;

    while (gameRunning && running)
    {
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
            {
                gameFinished = false;
                gameRunning = false;
                running = false;
                break;
            }
            else if (e.type == SDL_KEYDOWN)
            {
                if (DigitCount < number_length)
                {
                    if (e.key.keysym.sym >= SDLK_0 && e.key.keysym.sym <= SDLK_9)
                    {
                        guessed[DigitCount] = e.key.keysym.sym - SDLK_0 + '0';
                        DigitCount++;
                        if (DigitCount == number_length)
                        {
                            formatGuess(magicNumber, guessed, formatted, number_length);
                            (*attempts)++;

                            bool hasCorrectDigit = false;
                            for (int i = 0; i < number_length; i++)
                            {
                                if (guessed[i] == magicNumber[i])
                                {
                                    hasCorrectDigit = true;
                                    break;
                                }
                            }
                            if (hasCorrectDigit)
                            {
                                (*correctGuesses)++;
                            }

                            const char *resultText;
                            if (strncmp(guessed, magicNumber, number_length) == 0)
                            {
                                resultText = "Correct guess!";
                                gameRunning = false;
                            }
                            else
                            {
                                resultText = "Incorrect guess. Try again!";
                                DigitCount = 0;
                                memset(guessed, '\0', number_length);
                            }

                            SDL_Surface *messageSurface = TTF_RenderText_Solid(font, resultText, color);
                            messageTexture = SDL_CreateTextureFromSurface(renderer, messageSurface);
                            messageRect.x = 20;
                            messageRect.y = 160;
                            messageRect.w = messageSurface->w;
                            messageRect.h = messageSurface->h;
                            SDL_FreeSurface(messageSurface);
                        }
                    }
                    else if (e.key.keysym.sym == SDLK_BACKSPACE && DigitCount > 0)
                    {
                        DigitCount--;
                        guessed[DigitCount] = '\0';
                    }
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        if (backgroundTexture)
        {
            SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);
        }
        SDL_RenderCopy(renderer, usernameTexture, NULL, &usernameRect);

        char displayText[number_length + 20];
        snprintf(displayText, sizeof(displayText), "Guess: %s", formatted);
        SDL_Surface *displaySurface = TTF_RenderText_Solid(font, displayText, color);
        SDL_Texture *displayTexture = SDL_CreateTextureFromSurface(renderer, displaySurface);
        SDL_Rect displayRect = {20, 80, displaySurface->w, displaySurface->h};
        SDL_FreeSurface(displaySurface);

        SDL_RenderCopy(renderer, displayTexture, NULL, &displayRect);
        SDL_RenderCopy(renderer, levelTexture, NULL, &levelRect);

        char inputText[number_length + 20];

        if (DigitCount == number_length)
        {
            snprintf(inputText, sizeof(inputText), "Input: %s", initialDisplay);
        }
        else
        {
            snprintf(inputText, sizeof(inputText), "Input: %s", guessed);
        }

        SDL_Surface *inputSurface = TTF_RenderText_Solid(font, inputText, color);
        SDL_Texture *inputTexture = SDL_CreateTextureFromSurface(renderer, inputSurface);
        SDL_Rect inputRect = {20, 120, inputSurface->w, inputSurface->h};
        SDL_FreeSurface(inputSurface);
        SDL_RenderCopy(renderer, inputTexture, NULL, &inputRect);

        // Calculate elapsed time
        time_t currentTime = time(NULL);
        int elapsedTime = (int)difftime(currentTime, startTime);

        char timeText[50];
        snprintf(timeText, sizeof(timeText), "Time: %d s ", elapsedTime);
        SDL_Surface *timeSurface = TTF_RenderText_Solid(font, timeText, color);
        SDL_Texture *timeTexture = SDL_CreateTextureFromSurface(renderer, timeSurface);
        SDL_Rect timeRect = {WINDOW_WIDTH - timeSurface->w - 20, 10, timeSurface->w, timeSurface->h};
        SDL_FreeSurface(timeSurface);
        SDL_RenderCopy(renderer, timeTexture, NULL, &timeRect);
        SDL_DestroyTexture(timeTexture);

        if (messageTexture)
        {
            SDL_RenderCopy(renderer, messageTexture, NULL, &messageRect);
        }
        SDL_RenderPresent(renderer);
        SDL_DestroyTexture(displayTexture);
        SDL_DestroyTexture(inputTexture);
        SDL_Delay(10);
    }

    SDL_DestroyTexture(messageTexture);
    SDL_DestroyTexture(usernameTexture);

    time_t endTime = time(NULL);
    timeTaken = (int)difftime(endTime, startTime);
}

// Function to read high scores from a file
void readHighScores(Score scores[], int *scoreCount)
{
    FILE *file = fopen("highscore.txt", "r");
    if (file == NULL)
    {
        *scoreCount = 0;
        return;
    }

    *scoreCount = 0;
    while (*scoreCount < MAX_SCORES && fscanf(file, "%s %d %lf", scores[*scoreCount].username, &scores[*scoreCount].time, &scores[*scoreCount].successRatio) == 3)
    {
        (*scoreCount)++;
    }

    fclose(file);
}

// Function to save high scores to a file
void saveHighScores(Score scores[], int count)
{
    Score existingScores[MAX_SCORES];
    int existingCount = 0;

    // Read existing high scores
    readHighScores(existingScores, &existingCount);

    // Add the new scores to the existing scores if they are higher
    for (int i = 0; i < count; i++)
    {
        bool updated = false;
        for (int j = 0; j < existingCount; j++)
        {
            if (strcmp(existingScores[j].username, scores[i].username) == 0)
            {
                if (scores[i].successRatio > existingScores[j].successRatio)
                {
                    existingScores[j] = scores[i];
                }
                updated = true;
                break;
            }
        }
        if (!updated && existingCount < MAX_SCORES)
        {
            existingScores[existingCount++] = scores[i];
        }
    }

    // Sort the updated list in descending order
    qsort(existingScores, existingCount, sizeof(Score), compareScores);

    // Save the top scores
    FILE *file = fopen("highscore.txt", "w");
    if (file != NULL)
    {
        for (int i = 0; i < existingCount && i < MAX_SCORES; i++)
        {
            fprintf(file, "%s\t%d\t%.2f\n", existingScores[i].username, existingScores[i].time, existingScores[i].successRatio);
        }
        fclose(file);
    }
}
// Function to compare scores for sorting
int compareScores(const void *a, const void *b)
{
    const Score *scoreA = (const Score *)a;
    const Score *scoreB = (const Score *)b;

    // Sort in descending order based on successRatio
    if (scoreA->successRatio < scoreB->successRatio)
        return 1;
    else if (scoreA->successRatio > scoreB->successRatio)
        return -1;
    else
        return 0;
}
// Function to show high scores
void showHighScores()
{
    Score scores[MAX_SCORES];
    int scoreCount = 0;
    readHighScores(scores, &scoreCount);
    char highScoreText[] = "High Scores:";
    SDL_Surface *highScoreSurface = TTF_RenderText_Solid(font, highScoreText, color);
    SDL_Texture *highScoreTexture = SDL_CreateTextureFromSurface(renderer, highScoreSurface);
    SDL_Rect highScoreRect = {20, 20, highScoreSurface->w, highScoreSurface->h};
    SDL_FreeSurface(highScoreSurface);
    SDL_RenderCopy(renderer, highScoreTexture, NULL, &highScoreRect);
    SDL_DestroyTexture(highScoreTexture);

    // Start position for the first score line
    int yOffset = 100;
    for (int i = 0; i < scoreCount; i++)
    {
        char scoreLine[100];
        snprintf(scoreLine, sizeof(scoreLine), "%d. %s - Time: %d - Success: %.2f%%",
                 i + 1, scores[i].username, scores[i].time, scores[i].successRatio);

        SDL_Surface *scoreSurface = TTF_RenderText_Solid(font, scoreLine, color);
        SDL_Texture *scoreTexture = SDL_CreateTextureFromSurface(renderer, scoreSurface);
        SDL_Rect scoreRect = {20, yOffset, scoreSurface->w, scoreSurface->h};
        SDL_FreeSurface(scoreSurface);

        SDL_RenderCopy(renderer, scoreTexture, NULL, &scoreRect);
        SDL_DestroyTexture(scoreTexture);

        // Move down for the next score line, with a small gap
        yOffset += scoreRect.h + 10;
    }
    if (gameFinished)
    {
        SDL_Surface *messageSurface = TTF_RenderText_Solid(font, "Press Enter to play again...", color);
        SDL_Texture *messageTexture = SDL_CreateTextureFromSurface(renderer, messageSurface);
        SDL_Rect messageRect = {20, 400, messageSurface->w, messageSurface->h};
        SDL_FreeSurface(messageSurface);

        SDL_RenderCopy(renderer, messageTexture, NULL, &messageRect);
        SDL_RenderPresent(renderer);
        SDL_DestroyTexture(messageTexture);

        while (SDL_WaitEvent(&e))
        {
            if (e.type == SDL_QUIT)
            {
                gameFinished = false;
                running = false;
                break;
            }
            else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RETURN)
            {
                startTime = time(NULL);
                break;
            }
        }
    }
}
