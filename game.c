// This is the newest state of the game
// type make in terminal to build the game and .\game.exe to play

// Include the game's libraries
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <SDL2/SDL_ttf.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <SDL2/SDL_mixer.h>

// Define constants
#define WINDOW_HEIGHT 480
#define WINDOW_WIDTH 720
#define MAX_SCORES 5
#define MAX_GAME_LEVEL 3
#define DEFAULT_NUM_LENGTH 4

// Structure to store player scores
typedef struct
{
    char username[50];
    int time;
    double successRatio;
} Score;

// Initialize UX/UI components
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Event e;
SDL_Texture *backgroundTexture = NULL;
TTF_Font *font = NULL;
Mix_Music *bgMusic = NULL;
SDL_Color color = {0, 0, 0};

// Declare global variables
bool running = true;
bool gameFinished = true;
bool inputRunning = true;
int gameLevel = 1;
int number_length = DEFAULT_NUM_LENGTH;
time_t startTime;
int attempts = 0, correctGuesses = 0;
int timeTaken;
double successRatio = (double)0;

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
void showHighScores(int *attempts, int *correctGuesses);

// Main function
int main(int argc, char *argv[])
{
    // Initialize SDL
    initSDL();

    // Load resources
    loadResources();

    // Get player's username
    char username[50];
    while (inputRunning)
    {
        getUsername(username, sizeof(username));
    }
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

        // Won't save user's play if not finished all 3 levels
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
        if (gameLevel == MAX_GAME_LEVEL)
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

            // Read existing high scores
            Score scores[MAX_SCORES + 1];
            int scoreCount = 0;
            readHighScores(scores, &scoreCount);

            // Add the new score to the array
            if (scoreCount < MAX_SCORES)
            {
                // There's still room for a new score
                scores[scoreCount].time = timeTaken;
                scores[scoreCount].successRatio = successRatio;
                strcpy(scores[scoreCount].username, username);
                scoreCount++;
            }
            else
            {
                // Replace the lowest score if the new one is better
                int minIndex = 0;
                for (int i = 1; i < scoreCount; i++)
                {
                    if (scores[i].successRatio < scores[minIndex].successRatio)
                    {
                        minIndex = i;
                    }
                }

                if (successRatio > scores[minIndex].successRatio)
                {
                    scores[minIndex].time = timeTaken;
                    scores[minIndex].successRatio = successRatio;
                    strcpy(scores[minIndex].username, username);
                }
            }

            // Sort the scores in descending order
            qsort(scores, scoreCount, sizeof(Score), compareScores);

            // Save the top scores
            saveHighScores(scores, scoreCount > MAX_SCORES ? MAX_SCORES : scoreCount);

            // Show high scores and reset to the first level
            showHighScores(&attempts, &correctGuesses);
            gameLevel = 1;
            number_length = DEFAULT_NUM_LENGTH;
            continue;
        }

        // Render next level prompt
        color = {237, 170, 125};
        SDL_Surface *messageSurface = TTF_RenderText_Solid(font, "Correct guess! Press Enter to move to next level...", color);
        SDL_Texture *messageTexture = SDL_CreateTextureFromSurface(renderer, messageSurface);
        SDL_Rect messageRect = {20, 100, messageSurface->w, messageSurface->h};
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
    font = TTF_OpenFont("VeniteAdoremusStraight-Yzo6v.ttf", 20);
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
            snprintf(promptText, sizeof(promptText), "Enter Username (without spaces): %s", username);
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
void gameLoop(const char *magicNumber, int number_length, const char *username, int *attempts, int *correctGuesses)
{
    // Debugging output to show the magic number (remove in the final version)
    printf("Magic number(for debugging): %s\n", magicNumber);

    // Initialize arrays to store the guessed number, formatted display, and initial display
    char guessed[number_length + 1];
    char formatted[number_length + 1];
    char initialDisplay[number_length + 1];

    // Set terminating character for the guessed and formatted arrays
    guessed[number_length] = '\0';
    formatted[number_length] = '\0';

    // Initialize the initial display with dashes and guessed with spaces
    memset(initialDisplay, '-', number_length);
    memset(guessed, ' ', number_length);
    memset(formatted, '-', number_length);

    // Terminate the initial display string
    initialDisplay[number_length] = '\0';

    // Counter to track the number of digits entered by the player
    int DigitCount = 0;

    // Set the color for the text to be rendered
    SDL_Color color = {255, 153, 51};

    // Initialize variables for SDL textures and rectangles
    SDL_Texture *messageTexture = NULL;
    SDL_Rect messageRect;

    // Create and render the username text
    char usernameText[100];
    snprintf(usernameText, sizeof(usernameText), "Player: %s", username);
    SDL_Surface *usernameSurface = TTF_RenderText_Solid(font, usernameText, color);
    SDL_Texture *usernameTexture = SDL_CreateTextureFromSurface(renderer, usernameSurface);
    SDL_Rect usernameRect = {20, 40, usernameSurface->w, usernameSurface->h};
    // Free the surface after creating the texture
    SDL_FreeSurface(usernameSurface);

    // Create and render the level text
    char levelText[10];
    snprintf(levelText, sizeof(levelText), "Level: %d", gameLevel);
    SDL_Surface *levelSurface = TTF_RenderText_Solid(font, levelText, color);
    SDL_Texture *levelTexture = SDL_CreateTextureFromSurface(renderer, levelSurface);
    SDL_Rect levelRect = {20, 10, levelSurface->w, levelSurface->h};
    // Free the surface after creating the texture
    SDL_FreeSurface(levelSurface);

    // Flag to keep the game loop running
    bool gameRunning = true;

    // Main game loop
    while (gameRunning && running)
    {
        // Poll for events like key presses or quitting the game
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
            {
                // Handle quitting the game
                gameFinished = false;
                gameRunning = false;
                running = false;
                break;
            }
            else if (e.type == SDL_KEYDOWN)
            {
                // Handle digit input if there is still space for more digits
                if (DigitCount < number_length)
                {
                    // Check if a number key was pressed
                    if (e.key.keysym.sym >= SDLK_0 && e.key.keysym.sym <= SDLK_9)
                    {
                        // Convert the key to a character and add it to the guessed number
                        guessed[DigitCount] = e.key.keysym.sym - SDLK_0 + '0';
                        DigitCount++;

                        // If the number is fully guessed, check if it's correct
                        if (DigitCount == number_length)
                        {
                            // Format the guessed number
                            formatGuess(magicNumber, guessed, formatted, number_length);
                            // Increment the attempts counter
                            (*attempts)++;

                            // Check if any digits are correct
                            bool hasCorrectDigit = false;
                            for (int i = 0; i < number_length; i++)
                            {
                                if (guessed[i] == magicNumber[i])
                                {
                                    hasCorrectDigit = true;
                                    break;
                                }
                            }
                            // Increment correct guesses if any digit is correct
                            if (hasCorrectDigit)
                            {
                                (*correctGuesses)++;
                            }

                            const char *resultText;

                            // Check if the guessed number matches the magic number
                            if (strncmp(guessed, magicNumber, number_length) == 0)
                            {
                                // Stop the game loop
                                gameRunning = false;
                            }
                            else
                            {
                                // Reset digit count
                                DigitCount = 0;
                                // Incorrect guess, prompt to try again
                                resultText = "Incorrect guess. Try again!";
                                // Clear guessed number
                                memset(guessed, '\0', number_length);
                            }

                            // Create and render the result message
                            SDL_Surface *messageSurface = TTF_RenderText_Solid(font, resultText, color);
                            messageTexture = SDL_CreateTextureFromSurface(renderer, messageSurface);
                            messageRect.x = 20;
                            messageRect.y = 170;
                            messageRect.w = messageSurface->w;
                            messageRect.h = messageSurface->h;
                            // Free the surface after creating the texture
                            SDL_FreeSurface(messageSurface);
                        }
                    }
                    else if (e.key.keysym.sym == SDLK_BACKSPACE && DigitCount > 0)
                    {
                        // Handle backspace key to remove the last entered digit
                        DigitCount--;
                        guessed[DigitCount] = '\0';
                    }
                }
            }
        }

        // Render everything to the screen
        // Set background color to black
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        // Clear the renderer
        SDL_RenderClear(renderer);

        // Render background texture if it exists
        if (backgroundTexture)
        {
            SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);
        }

        // Render the username text
        SDL_RenderCopy(renderer, usernameTexture, NULL, &usernameRect);

        // Create and render the formatted guessed number
        char displayText[number_length + 20];
        snprintf(displayText, sizeof(displayText), "Guess: %s", formatted);
        SDL_Surface *displaySurface = TTF_RenderText_Solid(font, displayText, color);
        SDL_Texture *displayTexture = SDL_CreateTextureFromSurface(renderer, displaySurface);
        SDL_Rect displayRect = {20, 100, displaySurface->w, displaySurface->h};
        // Free the surface after creating the texture
        SDL_FreeSurface(displaySurface);
        // Render the guessed number
        SDL_RenderCopy(renderer, displayTexture, NULL, &displayRect);
        // Render the level text
        SDL_RenderCopy(renderer, levelTexture, NULL, &levelRect);

        // Create and render the input display text
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
        SDL_Rect inputRect = {20, 140, inputSurface->w, inputSurface->h};
        // Free the surface after creating the texture
        SDL_FreeSurface(inputSurface);
        // Render the input display
        SDL_RenderCopy(renderer, inputTexture, NULL, &inputRect);

        // Calculate and render the elapsed time
        time_t currentTime = time(NULL);
        int elapsedTime = (int)difftime(currentTime, startTime);

        char timeText[50];
        snprintf(timeText, sizeof(timeText), "Time: %d s", elapsedTime);
        SDL_Surface *timeSurface = TTF_RenderText_Solid(font, timeText, color);
        SDL_Texture *timeTexture = SDL_CreateTextureFromSurface(renderer, timeSurface);
        SDL_Rect timeRect = {WINDOW_WIDTH - timeSurface->w - 20, 10, timeSurface->w, timeSurface->h};

        // Free the surface after creating the texture
        SDL_FreeSurface(timeSurface);

        // Render the time
        SDL_RenderCopy(renderer, timeTexture, NULL, &timeRect);

        // Render the result message if it exists
        if (messageTexture)
        {
            SDL_RenderCopy(renderer, messageTexture, NULL, &messageRect);
        }

        // Update the screen with the rendered content
        SDL_RenderPresent(renderer);

        // Clean up textures
        SDL_DestroyTexture(displayTexture);
        SDL_DestroyTexture(inputTexture);
        // Add a small delay to control the loop speed
        SDL_Delay(10);
    }

    // Clean up message and username textures at the end of the loop
    SDL_DestroyTexture(messageTexture);
    SDL_DestroyTexture(usernameTexture);

    // Calculate the time taken for this game session
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
    Score existingScores[MAX_SCORES * 10]; // Allow space for many scores
    int existingCount = 0;

    // Read existing high scores
    readHighScores(existingScores, &existingCount);

    // Add the new scores to the existing scores list
    for (int i = 0; i < count; i++)
    {
        bool updated = false;
        for (int j = 0; j < existingCount; j++)
        {
            if (strcmp(existingScores[j].username, scores[i].username) == 0)
            {
                if (scores[i].successRatio > existingScores[j].successRatio)
                {
                    existingScores[j] = scores[i]; // Replace with the better run
                }
                updated = true;
                break;
            }
        }
        if (!updated && existingCount < MAX_SCORES * 10)
        {
            existingScores[existingCount++] = scores[i]; // Add new entry if not found
        }
    }

    // Sort all scores in descending order
    qsort(existingScores, existingCount, sizeof(Score), compareScores);

    // Save all scores back to the file
    FILE *file = fopen("highscore.txt", "w");
    if (file != NULL)
    {
        for (int i = 0; i < existingCount; i++)
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
void showHighScores(int *attempts, int *correctGuesses)
{
    Score scores[MAX_SCORES * 10];
    int scoreCount = 0;
    readHighScores(scores, &scoreCount);

    // Sort the scores in descending order by success ratio
    qsort(scores, scoreCount, sizeof(Score), compareScores);

    // Show only the top 5 scores
    char highScoreText[] = "Leaderboard";
    color = {237, 170, 125};
    SDL_Surface *highScoreSurface = TTF_RenderText_Solid(font, highScoreText, color);
    SDL_Texture *highScoreTexture = SDL_CreateTextureFromSurface(renderer, highScoreSurface);
    SDL_Rect highScoreRect = {250, 20, highScoreSurface->w, highScoreSurface->h};
    SDL_RenderCopy(renderer, highScoreTexture, NULL, &highScoreRect);

    char divideText[] = "------------------------------------------------------------------------------";
    color = {237, 170, 125};
    highScoreSurface = TTF_RenderText_Solid(font, divideText, color);
    highScoreTexture = SDL_CreateTextureFromSurface(renderer, highScoreSurface);
    highScoreRect = {0, 40, highScoreSurface->w, highScoreSurface->h};
    SDL_FreeSurface(highScoreSurface);
    SDL_RenderCopy(renderer, highScoreTexture, NULL, &highScoreRect);
    SDL_DestroyTexture(highScoreTexture);

    // Start position for the first score line
    int yOffset = 100;
    for (int i = 0; i < scoreCount && i < 5; i++)
    { // Display only the top 5
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
        color = {172, 153, 193};
        SDL_Surface *messageSurface = TTF_RenderText_Solid(font, "Press Enter to play again with the same username", color);
        SDL_Texture *messageTexture = SDL_CreateTextureFromSurface(renderer, messageSurface);
        SDL_Rect messageRect = {20, 300, messageSurface->w, messageSurface->h};
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
                // Reset timer to 0 if user wants to play again
                startTime = time(NULL);
                // Reset ratio if decided to play again with the same username
                *attempts = 0;
                *correctGuesses = 0;
                break;
            }
        }
    }
}
