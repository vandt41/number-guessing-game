#include <SDL2/SDL.h>
#include <stdbool.h>
#include <SDL2/SDL_ttf.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

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
    window = SDL_CreateWindow("Guessing Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, SDL_WINDOW_SHOWN);
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

int gameLoop(const char *magicNumber, int number_length, const char *username)
{
    char guessed[number_length + 1];
    char formatted[number_length + 1];
    char initialDisplay[number_length + 1];
    guessed[number_length] = '\0';
    formatted[number_length] = '\0';
    memset(initialDisplay, '-', number_length);
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

    SDL_Color color = {51, 0, 0};
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

        char inputText[number_length + 20];
        snprintf(inputText, sizeof(inputText), "Input: %s", guessed[0] ? guessed : initialDisplay);
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
    char username[50];
    printf("Enter your username: ");
    scanf("%49s", username);

    init();
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