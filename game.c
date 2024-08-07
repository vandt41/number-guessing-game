#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_NAME_LENGTH 50
#define MAX_GUESSES 100
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define BUTTON_WIDTH 200
#define BUTTON_HEIGHT 50

typedef struct
{
    char name[MAX_NAME_LENGTH];
    double ratio;
    double time_taken;
} Score;

typedef struct
{
    SDL_Rect rect;
    SDL_Color color;
    const char *text;
} Button;

int selectLevel()
{
    int number_length = 0, level = 0;
    printf("Select Level (1, 2, or 3): ");
    while (scanf("%d", &level) != 1 || level <= 0 || level > 3)
    {
        while (getchar() != '\n')
            ; // Clear the input buffer
        printf("Invalid level. Please enter 1, 2, or 3: ");
    }
    switch (level)
    {
    case 1:
        number_length = 4;
        break;
    case 2:
        number_length = 5;
        break;
    case 3:
        number_length = 6;
        break;
    default:
        number_length = 6;
        break;
    }
    return number_length;
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

int checkGuess(const char *magicNumber, int number_length)
{
    char guessed[number_length], remainedNumber[number_length];
    guessed[number_length] = '\0';
    remainedNumber[number_length] = '\0';
    int attempts = 0, correct = 0;
    while (correct != number_length)
    {
        correct = 0;
        for (int i = 0; i < number_length; i++)
        {
            remainedNumber[i] = '-';
        }
        printf("Enter your guess: ");
        scanf("%s", guessed);
        if (strlen(guessed) != number_length)
        {
            printf("Invalid input, please enter a %d-digit number.\n", number_length);
            printf("Attempts %d. \tGuessed Number: %s\n", attempts, remainedNumber);
            attempts++;
            continue;
        }
        attempts++;
        for (int i = 0; i < number_length; i++)
        {
            if (guessed[i] == magicNumber[i])
            {
                remainedNumber[i] = magicNumber[i];
                correct++;
            }
        }
        printf("Attempts %d. \tGuessed Number: %s\n", attempts, remainedNumber);
    }
    printf("Congratulations! You've won in %d attempts.\n", attempts);
    return attempts;
}

double calculateRatio(int attempts, int number_length)
{
    return (double)number_length / attempts;
}

void saveHighScore(const char *name, double ratio, double time_taken)
{
    FILE *file = fopen("highscores.txt", "a");
    if (file != NULL)
    {
        fprintf(file, "%s %.2f %.2f\n", name, ratio, time_taken);
        fclose(file);
    }
    else
    {
        printf("Error: Unable to open high score file.\n");
    }
}

void showTop5Scores()
{
    Score scores[MAX_GUESSES];
    int count = 0;
    FILE *file = fopen("highscores.txt", "r");
    if (file != NULL)
    {
        while (fscanf(file, "%s %lf %lf", scores[count].name, &scores[count].ratio, &scores[count].time_taken) == 3)
        {
            count++;
        }
        fclose(file);
    }
    else
    {
        printf("Error: Unable to open high score file.\n");
        return;
    }

    // Sort scores by ratio in descending order
    for (int i = 0; i < count - 1; i++)
    {
        for (int j = 0; j < count - i - 1; j++)
        {
            if (scores[j].ratio < scores[j + 1].ratio)
            {
                Score temp = scores[j];
                scores[j] = scores[j + 1];
                scores[j + 1] = temp;
            }
        }
    }

    printf("Top 5 High Scores:\n");
    for (int i = 0; i < count && i < 5; i++)
    {
        printf("%d. %s - Ratio: %.2f - Time Taken: %.2f seconds\n", i + 1, scores[i].name, scores[i].ratio, scores[i].time_taken);
    }
}

void renderButton(SDL_Renderer *renderer, TTF_Font *font, Button *button)
{
    SDL_SetRenderDrawColor(renderer, button->color.r, button->color.g, button->color.b, 255);
    SDL_RenderFillRect(renderer, &button->rect);

    SDL_Surface *surface = TTF_RenderText_Solid(font, button->text, (SDL_Color){255, 255, 255});
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

    int text_width = surface->w;
    int text_height = surface->h;
    SDL_Rect textRect = {button->rect.x + (button->rect.w - text_width) / 2, button->rect.y + (button->rect.h - text_height) / 2, text_width, text_height};

    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, NULL, &textRect);
    SDL_DestroyTexture(texture);
}

int main(int argc, char *argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    if (TTF_Init() != 0)
    {
        printf("TTF_Init Error: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Number Guessing Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL)
    {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL)
    {
        SDL_DestroyWindow(window);
        printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    TTF_Font *font = TTF_OpenFont("arial.ttf", 24);
    if (font == NULL)
    {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        printf("TTF_OpenFont Error: %s\n", TTF_GetError());
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    Button startButton = {{(WINDOW_WIDTH - BUTTON_WIDTH) / 2, (WINDOW_HEIGHT - BUTTON_HEIGHT) / 2, BUTTON_WIDTH, BUTTON_HEIGHT}, {0, 0, 255}, "Start Game"};

    SDL_Event e;
    int running = 1;
    int gameStarted = 0;

    while (running)
    {
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
            {
                running = 0;
            }
            else if (e.type == SDL_MOUSEBUTTONDOWN)
            {
                int x, y;
                SDL_GetMouseState(&x, &y);
                if (x >= startButton.rect.x && x <= startButton.rect.x + startButton.rect.w && y >= startButton.rect.y && y <= startButton.rect.y + startButton.rect.h)
                {
                    gameStarted = 1;
                    running = 0;
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        renderButton(renderer, font, &startButton);

        SDL_RenderPresent(renderer);
    }

    if (gameStarted)
    {
        // Game logic here
        char name[MAX_NAME_LENGTH];
        while (1)
        {
            printf("Enter your name: ");
            fgets(name, MAX_NAME_LENGTH, stdin);
            name[strcspn(name, "\n")] = '\0'; // Remove trailing newline

            // Check if the name contains spaces
            if (strchr(name, ' ') != NULL)
            {
                printf("Invalid name. Names should not contain spaces. Please try again.\n");
                fflush(stdin); // Clear the newline left by the previous input
                continue;
            }
            break;
        }

        while (1)
        {
            int number_length = selectLevel();
            char magicNumber[number_length];
            randomNumber(magicNumber, number_length);
            printf("Magic number (for debugging): %s\n", magicNumber); // Remove this in production

            time_t start_time = time(NULL);
            int attempts = checkGuess(magicNumber, number_length);
            time_t end_time = time(NULL);

            double time_taken = difftime(end_time, start_time);
            double ratio = calculateRatio(attempts, number_length);
            saveHighScore(name, ratio, time_taken);

            char play_again;
            printf("Do you want to play again? (y/n): ");
            scanf(" %c", &play_again);
            if (play_again != 'y' && play_again != 'Y')
            {
                break;
            }
            getchar(); // Clear the newline left by the previous input
        }
        showTop5Scores();
    }

    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}
