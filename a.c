#include <SDL2/SDL.h>
#include <stdbool.h>
#include <SDL2/SDL_ttf.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define WINDOW_HEIGHT 480
#define WINDOW_WIDTH 480

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Event e;
bool running = true;
SDL_Texture *backgroundTexture = NULL;

void init()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        exit(1);
    }
    window = SDL_CreateWindow("Guessing Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_HEIGHT, WINDOW_WIDTH, SDL_WINDOW_SHOWN);
    if (window == NULL)
    {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        exit(1);
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL)
    {
        printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        exit(1);
    }
}

void loadBackground(const char *imagePath)
{
    SDL_Surface *backgroundSurface = SDL_LoadBMP(imagePath);
    if (!backgroundSurface)
    {
        printf("SDL_LoadBMP Error: %s\n", SDL_GetError());
        return;
    }
    backgroundTexture = SDL_CreateTextureFromSurface(renderer, backgroundSurface);
    SDL_FreeSurface(backgroundSurface);
}

void randomNumber(char *magicNumber, int number_length)
{
    srand((unsigned int)time(NULL));
    for (int i = 0; i < number_length; i++)
    {
        magicNumber[i] = rand() % 10 + '0';
    }
    magicNumber[number_length] = '\0';
}

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

void getUsername(char *username, int maxLen, TTF_Font *font, SDL_Color color)
{
    SDL_StartTextInput();
    strcpy(username, "");

    SDL_Window *inputWindow = SDL_CreateWindow("Enter Username", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 400, 200, SDL_WINDOW_SHOWN);
    SDL_Renderer *inputRenderer = SDL_CreateRenderer(inputWindow, -1, SDL_RENDERER_ACCELERATED);

    bool inputRunning = true;
    while (inputRunning)
    {
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
            {
                inputRunning = false;
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
                    inputRunning = false;
                }
            }
        }

        SDL_SetRenderDrawColor(inputRenderer, 0, 0, 0, 255);
        SDL_RenderClear(inputRenderer);

        char promptText[100];
        snprintf(promptText, sizeof(promptText), "Username: %s", username);

        SDL_Surface *promptSurface = TTF_RenderText_Solid(font, promptText, color);
        SDL_Texture *promptTexture = SDL_CreateTextureFromSurface(inputRenderer, promptSurface);
        SDL_Rect promptRect = {20, 80, promptSurface->w, promptSurface->h};
        SDL_FreeSurface(promptSurface);

        SDL_RenderCopy(inputRenderer, promptTexture, NULL, &promptRect);
        SDL_RenderPresent(inputRenderer);
        SDL_DestroyTexture(promptTexture);

        SDL_Delay(10);
    }

    SDL_DestroyRenderer(inputRenderer);
    SDL_DestroyWindow(inputWindow);
    SDL_StopTextInput();
}

int gameLoop(const char *magicNumber, int number_length, const char *username)
{
    char guessed[number_length + 1];
    char formatted[number_length + 1];
    char initialDisplay[number_length + 1];
    guessed[number_length] = '\0';
    formatted[number_length] = '\0';
    memset(initialDisplay, '-', number_length);
    memset(guessed, ' ', number_length);
    memset(formatted, '-', number_length);

    initialDisplay[number_length] = '\0';
    int attempts = 0, correctedGuessed = 0;

    if (TTF_Init() == -1)
    {
        printf("TTF_Init: %s\n", TTF_GetError());
        return -1;
    }

    TTF_Font *font = TTF_OpenFont("Arial.ttf", 24);
    if (!font)
    {
        printf("TTF_OpenFont: %s\n", TTF_GetError());
        TTF_Quit();
        return -1;
    }

    SDL_Color color = {255, 255, 255};
    SDL_Texture *messageTexture = NULL;
    SDL_Rect messageRect;

    char usernameText[100];
    snprintf(usernameText, sizeof(usernameText), "Player: %s", username);
    SDL_Surface *usernameSurface = TTF_RenderText_Solid(font, usernameText, color);
    SDL_Texture *usernameTexture = SDL_CreateTextureFromSurface(renderer, usernameSurface);
    SDL_Rect usernameRect = {20, 10, usernameSurface->w, usernameSurface->h};
    SDL_FreeSurface(usernameSurface);

    bool gameRunning = true;
    time_t startTime = time(NULL);

    while (gameRunning)
    {
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
            {
                gameRunning = false;
            }
            else if (e.type == SDL_KEYDOWN)
            {
                if (correctedGuessed < number_length)
                {
                    switch (e.key.keysym.sym)
                    {
                    case SDLK_0:
                    case SDLK_1:
                    case SDLK_2:
                    case SDLK_3:
                    case SDLK_4:
                    case SDLK_5:
                    case SDLK_6:
                    case SDLK_7:
                    case SDLK_8:
                    case SDLK_9:
                        guessed[correctedGuessed] = e.key.keysym.sym - SDLK_0 + '0';
                        correctedGuessed++;
                        if (correctedGuessed == number_length)
                        {
                            formatGuess(magicNumber, guessed, formatted, number_length);

                            const char *resultText;
                            if (strncmp(guessed, magicNumber, number_length) == 0)
                            {
                                resultText = "Correct guess!";
                                gameRunning = false;
                            }
                            else
                            {
                                resultText = "Incorrect guess. Try again!";
                                correctedGuessed = 0;
                                memset(guessed, '\0', number_length);
                            }

                            SDL_Surface *messageSurface = TTF_RenderText_Solid(font, resultText, color);
                            messageTexture = SDL_CreateTextureFromSurface(renderer, messageSurface);
                            messageRect.x = 10;
                            messageRect.y = 130; // Move prompt down
                            messageRect.w = messageSurface->w;
                            messageRect.h = messageSurface->h;
                            SDL_FreeSurface(messageSurface);
                        }
                        break;
                    case SDLK_BACKSPACE:
                        if (correctedGuessed > 0)
                        {
                            correctedGuessed--;
                            guessed[correctedGuessed] = '\0';
                        }
                        break;
                    default:
                        break;
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
        SDL_Rect displayRect = {20, 50, displaySurface->w, displaySurface->h};
        SDL_FreeSurface(displaySurface);

        SDL_RenderCopy(renderer, displayTexture, NULL, &displayRect);

        // Update the input display based on the number of digits entered
        char inputText[number_length + 20];
        if (correctedGuessed == number_length)
        {
            snprintf(inputText, sizeof(inputText), "Input:   %s", initialDisplay);
        }
        else
        {
            snprintf(inputText, sizeof(inputText), "Input:   %s", guessed);
        }

        SDL_Surface *inputSurface = TTF_RenderText_Solid(font, inputText, color);
        SDL_Texture *inputTexture = SDL_CreateTextureFromSurface(renderer, inputSurface);
        SDL_Rect inputRect = {20, 90, inputSurface->w, inputSurface->h};
        SDL_FreeSurface(inputSurface);

        SDL_RenderCopy(renderer, inputTexture, NULL, &inputRect);

        if (messageTexture)
        {
            SDL_RenderCopy(renderer, messageTexture, NULL, &messageRect);
        }

        SDL_RenderPresent(renderer);
        SDL_DestroyTexture(displayTexture);
        SDL_DestroyTexture(inputTexture);

        SDL_Delay(10);
    }

    time_t endTime = time(NULL);
    int timeTaken = (int)difftime(endTime, startTime);
    printf("Username: %s, Time taken: %d seconds\n", username, timeTaken);

    FILE *file = fopen("highscore.txt", "a");
    if (file)
    {
        fprintf(file, "Username: %s, Time: %d seconds\n", username, timeTaken);
        fclose(file);
    }
    else
    {
        printf("Unable to open highscore.txt for writing.\n");
    }

    SDL_DestroyTexture(messageTexture);
    SDL_DestroyTexture(usernameTexture);
    SDL_DestroyTexture(backgroundTexture);
    TTF_CloseFont(font);
    TTF_Quit();

    return attempts;
}

int main(int argc, char *argv[])
{
    init();

    if (TTF_Init() == -1)
    {
        printf("TTF_Init: %s\n", TTF_GetError());
        exit(1);
    }

    TTF_Font *font = TTF_OpenFont("Arial.ttf", 24);
    if (!font)
    {
        printf("TTF_OpenFont: %s\n", TTF_GetError());
        TTF_Quit();
        exit(1);
    }

    SDL_Color color = {255, 255, 255};
    char username[50];
    getUsername(username, sizeof(username), font, color);

    loadBackground("background.bmp");
    const int number_length = 6;
    char magicNumber[number_length + 1];
    randomNumber(magicNumber, number_length);

    gameLoop(magicNumber, number_length, username);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}