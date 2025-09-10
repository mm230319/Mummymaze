#define _CRT_SECURE_NO_WARNINGS
#define HASH_TABLE_SIZE 10003 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdbool.h>
#include <time.h>
#include <SDL2p/SDL_ttf.h>
#include <SDL2p/SDL_image.h>
#include <SDL2p/SDL_mixer.h>

SDL_Window* window;
SDL_Renderer* renderer;
SDL_Event event;
TTF_Font* font[2];
SDL_Texture* mumijadesna;
SDL_Texture* mumijaleva;
SDL_Texture* Igrac;
Mix_Music* pozadinska;
Mix_Chunk* zvukovi[8];
int Pozadinska;
int Zvukovi;
int Prigusenje = Pozadinska / 3;
int izlazx, izlazy, IzlazX, IzlazY, izgubio = 0;
int TrenutniNivo, UsoNivo;
int ModIgre = 0;
int PrviPut = 1, mapa, Sacuvaj, PrviPut1 = 0;
char minut[10] = "0:00";

Uint32 ProsloVreme1, interval1 = 1000, TrenutnoVreme1, PauzaVreme;
int n, n1, velicina, Tezina, Algoritam, BrZombija, MaxWidth, MaxHeight, Izgledi[3], IgracX, IgracY, trbrojmumija, StranaMumije[2];
typedef struct Maze {
    int matrica;
};
struct Node {
    int data[2];
    struct Node* prev;
};
struct Stack {
    struct Node* LastIn;
};
typedef struct Pozicija {
    int x, y, xm, ym;
}Pozicija;
struct State {
    int x, y;
    struct Pozicija* mumije;
    int br_mumija;
    int parent;
}State;
struct HashEntry {
    struct State state;
    struct HashEntry* next;
}HashEntry;
struct HashTable {
    struct HashEntry** table;
};
typedef struct Nivo {
    int size;
    struct Maze* talon;
    Pozicija igrac;
    Pozicija izlaz;
    int pametnamumija;
    int brojmumija;
    Pozicija* mumijenivo;
    int brojkoraka;
}Nivo;
Pozicija* mumije;
void Push(struct Stack* stack, int x, int y) {
    struct Node* newNode = (struct Node*)malloc(sizeof(struct Node));
    newNode->data[0] = x;
    newNode->data[1] = y;
    newNode->prev = stack->LastIn;
    stack->LastIn = newNode;
}
void Pop(struct Stack* stack) {
    struct Node* temp = stack->LastIn;
    stack->LastIn = stack->LastIn->prev;
    free(temp);
}
void Pokret(SDL_Surface** surface, struct Maze* talon, int x1, int y1, int velicina1, int index, int* intervalx, char* minut, int zombi);
void Igra(SDL_Surface** surface, Uint32 PauzaVreme);
void GlavniMeni(SDL_Surface** surface);
void Opcije(SDL_Surface** surface);
void PrelazNaOpcije(SDL_Surface** surface, int smer, int zvuk);
void PrelazNaGlavniMeni(SDL_Surface** surface);
int Zid(int x, int y, int x1, int y1, struct Maze* talon) {
    if ((x1 - x) == 0) return (talon[2 * x * (2 * n - 1) + 2 * y + 1].matrica == 1) ? 0 : 1;
    else if ((y1 - y) == 0) return (talon[(2 * x + 1) * (2 * n - 1) + 2 * y].matrica == 1) ? 0 : 1;
}
int Validan(int x, int y) {
    return  ((x >= 0) && (y >= 0) && (x < n) && (y < n)) ? 1 : 0;
}
int NemaDalje(int x, int y, struct Maze* maze) {
    int br = 0, komsija[4][2] = { {0,1},{0,-1},{1,0},{-1,0} };
    for (int i = 0; i < 4; i++) {
        if (Validan(x + komsija[i][0], y + komsija[i][1]) && (maze[(x + komsija[i][0]) * n + y + komsija[i][1]].matrica == 0)) {
            br++;
        }
    }
    return (br != 0) ? 1 : 0;
}
void Barijera(int x, int y, struct Maze* maze, struct Maze* talon, struct Stack* stack) {
    int komsija[4][2] = { {0,1},{0,-1},{1,0},{-1,0} };
    int x1 = stack->LastIn->data[0], y1 = stack->LastIn->data[1];
    for (int i = 0; i < 4; i++) {
        int x2 = x + komsija[i][0], y2 = y + komsija[i][1];
        if ((Validan(x + komsija[i][0], y + komsija[i][1])) && (x2 != x1 || y2 != y1) && (maze[(x + komsija[i][0]) * n + y + komsija[i][1]].matrica == 1) && rand() % 3 != 0) {
            if (i < 2) talon[x * (2 * n - 1) * 2 + ((y < y + komsija[i][1]) ? (y * 2 + 1) : (y * 2 - 1))].matrica = 1;
            else talon[((x < x + komsija[i][0]) ? (x * 2 + 1) : (x * 2 - 1)) * (2 * n - 1) + y * 2].matrica = 1;
        }
    }
}
void lavirint(int x, int y, struct Maze* maze, struct Maze* talon) {
    struct Stack stack;
    stack.LastIn = NULL;
    Push(&stack, x, y);
    int index, BrPosecenih = 1, BrCelija = pow(n, 2);
    int komsija[4][2] = { {0,  1},{0,  -1},{1,  0},{-1, 0} };
    while (BrPosecenih < BrCelija) {
        index = rand() % 4;
        int x1 = x + komsija[index][0], y1 = y + komsija[index][1];
        if (NemaDalje(x, y, maze)) {
            if ((Validan(x1, y1)) && (maze[x1 * n + y1].matrica == 0)) {
                x = x1;
                y = y1;
                maze[x * n + y].matrica = 1;
                Barijera(x, y, maze, talon, &stack);
                Push(&stack, x, y);
                BrPosecenih++;
            }
            else continue;
        }
        else {
            while (!NemaDalje(x, y, maze)) {
                x = stack.LastIn->data[0];
                y = stack.LastIn->data[1];
                Pop(&stack);
            }
            Push(&stack, x, y);
        }
    }
}
void lavirint1(int x, int y, struct Maze* maze, struct Maze* talon) {
    struct Stack stack;
    stack.LastIn = NULL;
    Push(&stack, x, y);
    int index, BrPosecenih = 1, BrCelija = pow(n, 2), StartX = x, StartY = y, radi = 1;
    int komsija[4][2] = { {0,  1},{0,  -1},{1,  0},{-1, 0} };
    while (BrPosecenih < BrCelija) {
        index = rand() % 4;
        int x1 = x + komsija[index][0], y1 = y + komsija[index][1];
        if (NemaDalje(x, y, maze)) {
            if ((Validan(x1, y1)) && (maze[x1 * n + y1].matrica == 0)) {
                x = x1;
                y = y1;
                maze[x * n + y].matrica = 1;
                Barijera(x, y, maze, talon, &stack);
                Push(&stack, x, y);
                BrPosecenih++;
            }
            else continue;
        }
        else {
            x = StartX;
            y = StartY;
            for (int i = x; i < n; i++) {
                for (int j = y; j < n; j++)
                    if (NemaDalje(i, j, maze)) {
                        x = i;
                        y = j;
                        radi = 0;
                        break;
                    }
                if (radi == 0)
                    break;
                else {
                    for (int j = y; j >= 0; j--)
                        if (NemaDalje(i, j, maze)) {
                            x = i;
                            y = j;
                            radi = 0;
                            break;
                        }
                }
                if (radi == 0)
                    break;
                else
                    y = 0;
            }
            if (radi == 1 && y == 0) {
                for (int i = x; i >= 0; i--) {
                    for (int j = y; j < n; j++)
                        if (NemaDalje(i, j, maze)) {
                            x = i;
                            y = j;
                            radi = 0;
                            break;
                        }
                    if (radi == 0)
                        break;
                    else
                        y = 0;
                }
            }
            StartX = x;
            StartY = y;
            radi = 1;
            Push(&stack, x, y);
        }
    }
}
void swap(Nivo* a, Nivo* b) {
    Nivo temp = *a;
    *a = *b;
    *b = temp;
}
int partition(Nivo* nivoi, int low, int high) {
    int pivot = nivoi[high].brojkoraka;
    int i = low - 1;
    for (int j = low; j < high; j++) {
        if (nivoi[j].brojkoraka < pivot) {
            i++;
            swap(&nivoi[i], &nivoi[j]);
        }
    }
    swap(&nivoi[i + 1], &nivoi[high]);
    return i + 1;
}
void quickSort(Nivo* nivoi, int low, int high) {
    if (low < high) {
        int pi = partition(nivoi, low, high);
        quickSort(nivoi, low, pi - 1);
        quickSort(nivoi, pi + 1, high);
    }
}
int Velicina() {
    return ((int)sqrt((double)(MaxHeight * MaxWidth) / (double)(2.8 * n * n)));
}
int Velicina1() {
    return ((int)sqrt((double)(MaxHeight * MaxWidth) / (double)(2.8 * 36)));
}
double distanca(int x1, int y1, int x2, int y2) {
    return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}
void PustiZvuk(Mix_Chunk* zvuk) {
    Mix_PlayChannel(-1, zvuk, 0);
}
void NapraviZid(int x, int y, int w, int h, SDL_Texture* texture) {
    SDL_Rect rect = { x,y,w,h };
    SDL_RenderCopy(renderer, texture, NULL, &rect);
}
void DeoTeksture(int x, int y, int w, int h, int x1, int y1, int w1, int h1, SDL_Texture* texture) {
    SDL_Rect rect = { x,y,w,h };
    SDL_Rect rect1 = { x1,y1,w1,h1 };
    SDL_RenderCopy(renderer, texture, &rect1, &rect);
}
void ObojiZid(int x, int y, int w, int h) {
    SDL_Rect rect = { x,y,w,h };
    SDL_RenderFillRect(renderer, &rect);
}
void PisiTekst(TTF_Font* Font, SDL_Color BojaTeksta, SDL_Rect rect, char* str) {
    SDL_Surface* TextSurface = TTF_RenderText_Solid(Font, (const char*)str, BojaTeksta);
    SDL_Texture* TextTexture = SDL_CreateTextureFromSurface(renderer, TextSurface);
    SDL_FreeSurface(TextSurface);
    SDL_QueryTexture(TextTexture, NULL, NULL, &rect.w, &rect.h);
    SDL_RenderCopy(renderer, TextTexture, NULL, &rect);
    SDL_DestroyTexture(TextTexture);
}
void CrtajSve(SDL_Surface** surface, struct Maze* talon, int index, int LastIndex = -1, int* x = new int, int* y = new int) {
    int w[10], h[10], velicina1 = Velicina();
    TTF_SetFontSize(font[0], 0.5 * velicina);
    if (ModIgre != 1) {
        TTF_SizeText(font[0], (const char*)"Nova Igra", &w[0], &h[0]);
        TTF_SizeText(font[0], (const char*)"Sacuvaj Igru", &w[1], &h[1]);
        TTF_SizeText(font[0], (const char*)"Glavni Meni", &w[5], &h[5]);
    }
    else
        TTF_SizeText(font[0], (const char*)"Mapa", &w[5], &h[5]);
    TTF_SizeText(font[0], (const char*)"Pomoc", &w[2], &h[2]);
    TTF_SizeText(font[0], (const char*)"Opcije", &w[3], &h[3]);
    TTF_SizeText(font[0], (const char*)"Skorovi", &w[4], &h[4]);

    *x = 0;
    *y = 0;
    if (index == 0) {
        SDL_Texture* texture1 = NULL;
        if (n == 6)
            texture1 = SDL_CreateTextureFromSurface(renderer, surface[0]);
        else if (n == 7)
            texture1 = SDL_CreateTextureFromSurface(renderer, surface[1]);
        else if (n == 8)
            texture1 = SDL_CreateTextureFromSurface(renderer, surface[2]);
        SDL_Texture* texture2 = SDL_CreateTextureFromSurface(renderer, surface[7]);
        SDL_Texture* texture3 = SDL_CreateTextureFromSurface(renderer, surface[3]);
        SDL_Texture* texture4 = SDL_CreateTextureFromSurface(renderer, surface[8]);
        SDL_Texture* texture5 = SDL_CreateTextureFromSurface(renderer, surface[9]);
        SDL_Texture* texture6 = SDL_CreateTextureFromSurface(renderer, surface[10]);
        DeoTeksture(0, (int)(72.0 / 699.0 * MaxHeight), 12 * MaxWidth / 25 - velicina, MaxHeight - 2 * (int)(72.0 / 699.0 * MaxHeight), 0, 72, 564, 555, texture3);
        //talon
        NapraviZid(12 * MaxWidth / 25, 2 * MaxHeight / 15, velicina * 6, velicina * 6, texture1);
        //levi i desni zidovi
        NapraviZid(12 * MaxWidth / 25 + velicina * 6, 2 * MaxHeight / 15, velicina, velicina * 6, texture4);
        NapraviZid(12 * MaxWidth / 25 - velicina, 2 * MaxHeight / 15, velicina, velicina * 6, texture2);
        //gornji zid
        NapraviZid(12 * MaxWidth / 25 - velicina, 2 * MaxHeight / 15 - velicina - 2, velicina * 8, velicina + 2, texture5);
        //Opcije
        SDL_DestroyTexture(texture1);
        SDL_DestroyTexture(texture2);
        SDL_DestroyTexture(texture3);
        SDL_DestroyTexture(texture4);
        SDL_DestroyTexture(texture5);
        if (ModIgre != 1) {
            PisiTekst(font[0], { 60,42,5,255 }, { (int)(12 * MaxWidth / 25 - velicina - w[0]) / 2, (int)(0.22 * MaxHeight), (int)(13 * MaxWidth / 25 - 0.9 * velicina), (int)(velicina) }, (char*)"Nova igra");
            PisiTekst(font[0], { 60,42,5,255 }, { (int)(12 * MaxWidth / 25 - velicina - w[1]) / 2, (int)(0.32 * MaxHeight), (int)(13 * MaxWidth / 25 - 0.9 * velicina), (int)(velicina) }, (char*)"Sacuvaj igru");
            PisiTekst(font[0], { 60,42,5,255 }, { (int)(12 * MaxWidth / 25 - velicina - w[2]) / 2, (int)(0.42 * MaxHeight), (int)(13 * MaxWidth / 25 - 0.9 * velicina), (int)(velicina) }, (char*)"Pomoc");
            PisiTekst(font[0], { 60,42,5,255 }, { (int)(12 * MaxWidth / 25 - velicina - w[3]) / 2, (int)(0.52 * MaxHeight), (int)(13 * MaxWidth / 25 - 0.9 * velicina), (int)(velicina) }, (char*)"Opcije");
            PisiTekst(font[0], { 60,42,5,255 }, { (int)(12 * MaxWidth / 25 - velicina - w[4]) / 2, (int)(0.62 * MaxHeight), (int)(13 * MaxWidth / 25 - 0.9 * velicina), (int)(velicina) }, (char*)"Skorovi");
            PisiTekst(font[0], { 60,42,5,255 }, { (int)(12 * MaxWidth / 25 - velicina - w[5]) / 2, (int)(0.72 * MaxHeight), (int)(13 * MaxWidth / 25 - 0.9 * velicina), (int)(velicina) }, (char*)"Glavni Meni");
        }
        else {
            PisiTekst(font[0], { 60,42,5,255 }, { (int)(12 * MaxWidth / 25 - velicina - w[2]) / 2, (int)(0.28 * MaxHeight), (int)(13 * MaxWidth / 25 - 0.9 * velicina), (int)(velicina) }, (char*)"Pomoc");
            PisiTekst(font[0], { 60,42,5,255 }, { (int)(12 * MaxWidth / 25 - velicina - w[3]) / 2, (int)(0.4 * MaxHeight), (int)(13 * MaxWidth / 25 - 0.9 * velicina), (int)(velicina) }, (char*)"Opcije");
            PisiTekst(font[0], { 60,42,5,255 }, { (int)(12 * MaxWidth / 25 - velicina - w[4]) / 2, (int)(0.52 * MaxHeight), (int)(13 * MaxWidth / 25 - 0.9 * velicina), (int)(velicina) }, (char*)"Skorovi");
            PisiTekst(font[0], { 60,42,5,255 }, { (int)(12 * MaxWidth / 25 - velicina - w[5]) / 2, (int)(0.64 * MaxHeight), (int)(13 * MaxWidth / 25 - 0.9 * velicina), (int)(velicina) }, (char*)"Mapa");
        }
        texture1 = SDL_CreateTextureFromSurface(renderer, surface[15]);
        texture2 = SDL_CreateTextureFromSurface(renderer, surface[16]);
        for (int i = 0; i < 2 * n - 1; i++) {
            for (int j = 0; j < 2 * n - 1; j++) {
                if (talon[i * (2 * n - 1) + j].matrica == 2) {
                    *x = i;
                    *y = j;
                    IzlazX = i;
                    IzlazY = j;
                }
                else if ((i % 2 == 0) && (j % 2 == 1) && (talon[i * (2 * n - 1) + j].matrica == 1)) NapraviZid(12 * MaxWidth / 25 + 9 * velicina1 / 10 + velicina1 * (j / 2), 2 * MaxHeight / 15 + velicina1 * (i / 2) - 11 * velicina1 / 100, velicina1 / 5, 6 * velicina1 / 5, texture1);
                else if ((i % 2 == 1) && (j % 2 == 0) && (talon[i * (2 * n - 1) + j].matrica == 1)) NapraviZid(12 * MaxWidth / 25 + velicina1 * (j / 2) - 20 * velicina1 / 192, 2 * MaxHeight / 15 + 9 * velicina1 / 10 + velicina1 * (i / 2) - velicina1 * 0.005, 1.201 * velicina1, velicina1 / 5, texture2);
            }
        }
        SDL_DestroyTexture(texture1);
        SDL_DestroyTexture(texture2);
        //donji zid
        NapraviZid(12 * MaxWidth / 25 - velicina, 2 * MaxHeight / 15 + velicina * 6, velicina * 8, 3 * velicina / 5, texture6);
        SDL_DestroyTexture(texture6);
        if (*x == 0 && *y >= 0) {
            texture1 = SDL_CreateTextureFromSurface(renderer, surface[11]);
            NapraviZid(12 * MaxWidth / 25 + velicina1 / 8 + velicina1 * ((*y) / 2), 2 * MaxHeight / 15 + velicina * ((*x) / 2) - 11 * velicina / 10, 3 * velicina1 / 4, 11 * velicina / 10, texture1);
        }
        else if (*x == (2 * n - 2) && *y >= 0) {
            texture1 = SDL_CreateTextureFromSurface(renderer, surface[12]);
            NapraviZid(12 * MaxWidth / 25 + velicina1 / 8 + velicina1 * (*y / 2), 2 * MaxHeight / 15 + velicina * 6, 3 * velicina1 / 4, 3 * velicina / 5, texture1);
        }
        else if (*y == 0 && *x >= 0) {
            texture1 = SDL_CreateTextureFromSurface(renderer, surface[13]);
            NapraviZid(12 * MaxWidth / 25 - velicina, 2 * MaxHeight / 15 + velicina1 * (*x / 2) + velicina1 / 8, velicina, 3 * velicina1 / 4, texture1);
        }
        else if (*y == (2 * n - 2) && *x >= 0) {
            texture1 = SDL_CreateTextureFromSurface(renderer, surface[14]);
            NapraviZid(12 * MaxWidth / 25 + velicina * 6, 2 * MaxHeight / 15 + velicina1 * (*x / 2) + velicina1 / 8, velicina, 3 * velicina1 / 4, texture1);
        }
        SDL_DestroyTexture(texture1);
    }
    else if (index == 1) {
        SDL_Texture* texture1 = NULL;
        if (n == 6)
            texture1 = SDL_CreateTextureFromSurface(renderer, surface[0]);
        else if (n == 7)
            texture1 = SDL_CreateTextureFromSurface(renderer, surface[1]);
        else if (n == 8)
            texture1 = SDL_CreateTextureFromSurface(renderer, surface[2]);
        NapraviZid(12 * MaxWidth / 25, 2 * MaxHeight / 15, velicina * 6, velicina * 6, texture1);
        SDL_DestroyTexture(texture1);
        texture1 = SDL_CreateTextureFromSurface(renderer, surface[15]);
        SDL_Texture* texture2 = SDL_CreateTextureFromSurface(renderer, surface[16]);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        for (int i = 0; i < 2 * n - 1; i++) {
            for (int j = 0; j < 2 * n - 1; j++) {
                if (talon[i * (2 * n - 1) + j].matrica == 2) {
                    *x = i;
                    *y = j;
                }
                else if ((i % 2 == 0) && (j % 2 == 1) && (talon[i * (2 * n - 1) + j].matrica == 1)) NapraviZid(12 * MaxWidth / 25 + 9 * velicina1 / 10 + velicina1 * (j / 2), 2 * MaxHeight / 15 + velicina1 * (i / 2) - 11 * velicina1 / 100, velicina1 / 5, 6 * velicina1 / 5, texture1);
                else if ((i % 2 == 1) && (j % 2 == 0) && (talon[i * (2 * n - 1) + j].matrica == 1)) NapraviZid(12 * MaxWidth / 25 + velicina1 * (j / 2) - 20 * velicina1 / 192, 2 * MaxHeight / 15 + 9 * velicina1 / 10 + velicina1 * (i / 2) - velicina1 * 0.005, 1.201 * velicina1, velicina1 / 5, texture2);
            }
        }
        SDL_Texture* texture3 = SDL_CreateTextureFromSurface(renderer, surface[10]);
        NapraviZid(12 * MaxWidth / 25 - velicina, 2 * MaxHeight / 15 + velicina * 6, velicina * 8, 3 * velicina / 5, texture3);
        SDL_DestroyTexture(texture1);
        SDL_DestroyTexture(texture2);
        SDL_DestroyTexture(texture3);
        if (*x == 0 && *y >= 0) {
            texture1 = SDL_CreateTextureFromSurface(renderer, surface[11]);
            NapraviZid(12 * MaxWidth / 25 + velicina1 / 8 + velicina1 * ((*y) / 2), 2 * MaxHeight / 15 + velicina * ((*x) / 2) - 11 * velicina / 10, 3 * velicina1 / 4, 11 * velicina / 10, texture1);
        }
        else if (*x == (2 * n - 2) && *y >= 0) {
            texture1 = SDL_CreateTextureFromSurface(renderer, surface[12]);
            NapraviZid(12 * MaxWidth / 25 + velicina1 / 8 + velicina1 * (*y / 2), 2 * MaxHeight / 15 + velicina * 6, 3 * velicina1 / 4, 3 * velicina / 5, texture1);
        }
        else if (*y == 0 && *x >= 0) {
            texture1 = SDL_CreateTextureFromSurface(renderer, surface[13]);
            NapraviZid(12 * MaxWidth / 25 - velicina, 2 * MaxHeight / 15 + velicina1 * (*x / 2) + velicina1 / 8, velicina, 3 * velicina1 / 4, texture1);
        }
        else if (*y == (2 * n - 2) && *x >= 0) {
            texture1 = SDL_CreateTextureFromSurface(renderer, surface[14]);
            NapraviZid(12 * MaxWidth / 25 + velicina * 6, 2 * MaxHeight / 15 + velicina1 * (*x / 2) + velicina1 / 8, velicina, 3 * velicina1 / 4, texture1);
        }
        SDL_DestroyTexture(texture1);
    }
    else if (index == 3)
        PisiTekst(font[0], { 188,155,86,255 }, { (int)(12 * MaxWidth / 25 - velicina - w[0]) / 2, (int)(0.22 * MaxHeight), (int)(13 * MaxWidth / 25 - 0.9 * velicina), (int)(velicina) }, (char*)"Nova igra");
    else if (index == 4)
        PisiTekst(font[0], { 188,155,86,255 }, { (int)(12 * MaxWidth / 25 - velicina - w[1]) / 2, (int)(0.32 * MaxHeight), (int)(13 * MaxWidth / 25 - 0.9 * velicina), (int)(velicina) }, (char*)"Sacuvaj igru");
    else if (index == 5) {
        if (ModIgre != 1)
            PisiTekst(font[0], { 188,155,86,255 }, { (int)(12 * MaxWidth / 25 - velicina - w[2]) / 2, (int)(0.42 * MaxHeight), (int)(13 * MaxWidth / 25 - 0.9 * velicina), (int)(velicina) }, (char*)"Pomoc");
        else
            PisiTekst(font[0], { 188,155,86,255 }, { (int)(12 * MaxWidth / 25 - velicina - w[2]) / 2, (int)(0.28 * MaxHeight), (int)(13 * MaxWidth / 25 - 0.9 * velicina), (int)(velicina) }, (char*)"Pomoc");
    }
    else if (index == 6) {
        if (ModIgre != 1)
            PisiTekst(font[0], { 188,155,86,255 }, { (int)(12 * MaxWidth / 25 - velicina - w[3]) / 2, (int)(0.52 * MaxHeight), (int)(13 * MaxWidth / 25 - 0.9 * velicina), (int)(velicina) }, (char*)"Opcije");
        else
            PisiTekst(font[0], { 188,155,86,255 }, { (int)(12 * MaxWidth / 25 - velicina - w[3]) / 2, (int)(0.4 * MaxHeight), (int)(13 * MaxWidth / 25 - 0.9 * velicina), (int)(velicina) }, (char*)"Opcije");
    }
    else if (index == 7) {
        if (ModIgre != 1)
            PisiTekst(font[0], { 188,155,86,255 }, { (int)(12 * MaxWidth / 25 - velicina - w[4]) / 2, (int)(0.62 * MaxHeight), (int)(13 * MaxWidth / 25 - 0.9 * velicina), (int)(velicina) }, (char*)"Skorovi");
        else
            PisiTekst(font[0], { 188,155,86,255 }, { (int)(12 * MaxWidth / 25 - velicina - w[4]) / 2, (int)(0.52 * MaxHeight), (int)(13 * MaxWidth / 25 - 0.9 * velicina), (int)(velicina) }, (char*)"Skorovi");
    }
    else if (index == 8) {
        if (ModIgre != 1)
            PisiTekst(font[0], { 188,155,86,255 }, { (int)(12 * MaxWidth / 25 - velicina - w[5]) / 2, (int)(0.72 * MaxHeight), (int)(13 * MaxWidth / 25 - 0.9 * velicina), (int)(velicina) }, (char*)"Glavni Meni");
        else
            PisiTekst(font[0], { 188,155,86,255 }, { (int)(12 * MaxWidth / 25 - velicina - w[5]) / 2, (int)(0.64 * MaxHeight), (int)(13 * MaxWidth / 25 - 0.9 * velicina), (int)(velicina) }, (char*)"Mapa");
    }
    else if (index == 9) {
        if (ModIgre != 1) {
            PisiTekst(font[0], { 60,42,5,255 }, { (int)(12 * MaxWidth / 25 - velicina - w[0]) / 2, (int)(0.22 * MaxHeight), (int)(13 * MaxWidth / 25 - 0.9 * velicina), (int)(velicina) }, (char*)"Nova igra");
            PisiTekst(font[0], { 60,42,5,255 }, { (int)(12 * MaxWidth / 25 - velicina - w[1]) / 2, (int)(0.32 * MaxHeight), (int)(13 * MaxWidth / 25 - 0.9 * velicina), (int)(velicina) }, (char*)"Sacuvaj igru");
            PisiTekst(font[0], { 60,42,5,255 }, { (int)(12 * MaxWidth / 25 - velicina - w[2]) / 2, (int)(0.42 * MaxHeight), (int)(13 * MaxWidth / 25 - 0.9 * velicina), (int)(velicina) }, (char*)"Pomoc");
            PisiTekst(font[0], { 60,42,5,255 }, { (int)(12 * MaxWidth / 25 - velicina - w[3]) / 2, (int)(0.52 * MaxHeight), (int)(13 * MaxWidth / 25 - 0.9 * velicina), (int)(velicina) }, (char*)"Opcije");
            PisiTekst(font[0], { 60,42,5,255 }, { (int)(12 * MaxWidth / 25 - velicina - w[4]) / 2, (int)(0.62 * MaxHeight), (int)(13 * MaxWidth / 25 - 0.9 * velicina), (int)(velicina) }, (char*)"Skorovi");
            PisiTekst(font[0], { 60,42,5,255 }, { (int)(12 * MaxWidth / 25 - velicina - w[5]) / 2, (int)(0.72 * MaxHeight), (int)(13 * MaxWidth / 25 - 0.9 * velicina), (int)(velicina) }, (char*)"Glavni Meni");
        }
        else {
            PisiTekst(font[0], { 60,42,5,255 }, { (int)(12 * MaxWidth / 25 - velicina - w[2]) / 2, (int)(0.28 * MaxHeight), (int)(13 * MaxWidth / 25 - 0.9 * velicina), (int)(velicina) }, (char*)"Pomoc");
            PisiTekst(font[0], { 60,42,5,255 }, { (int)(12 * MaxWidth / 25 - velicina - w[3]) / 2, (int)(0.4 * MaxHeight), (int)(13 * MaxWidth / 25 - 0.9 * velicina), (int)(velicina) }, (char*)"Opcije");
            PisiTekst(font[0], { 60,42,5,255 }, { (int)(12 * MaxWidth / 25 - velicina - w[4]) / 2, (int)(0.52 * MaxHeight), (int)(13 * MaxWidth / 25 - 0.9 * velicina), (int)(velicina) }, (char*)"Skorovi");
            PisiTekst(font[0], { 60,42,5,255 }, { (int)(12 * MaxWidth / 25 - velicina - w[5]) / 2, (int)(0.64 * MaxHeight), (int)(13 * MaxWidth / 25 - 0.9 * velicina), (int)(velicina) }, (char*)"Mapa");
        }
    }
}
int ProveraPobede(int x, int y, int x1, int y1) {
    if (x == 0 && y >= 0) return (x1 == -1 && y / 2 == y1) ? 1 : 0;
    else if (x == (2 * n - 2) && y >= 0) return (x1 == n && y / 2 == y1) ? 1 : 0;
    else if (y == 0 && x >= 0) return (x1 == x / 2 && y1 == -1) ? 1 : 0;
    else if (y == (2 * n - 2) && x >= 0) return (x1 == x / 2 && y1 == n) ? 1 : 0;
}
void Mapa(SDL_Surface** surface, int index1);
void CrtajPobeda(SDL_Surface** surface, int index, int LastIndex, char* score = new char, int BrTrofeja = 0, int IgracBr = 0, int boja = 0) {
    int x[5], y[5], w, h;
    if (index == 0 && (LastIndex == -1 || LastIndex == -2 || LastIndex == -3)) {
        if (LastIndex == -1 || LastIndex == -2) {
            float uvecanje, intervalx, intervaly;
            Uint32 ProsloVreme = SDL_GetTicks(), interval = 1000 / 60, TrenutnoVreme;
            SDL_Surface* surface1 = IMG_Load("Slike/PobedaPozadina.png");
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface1);
            SDL_FreeSurface(surface1);
            NapraviZid(0, 0, MaxWidth, MaxHeight, texture);
            SDL_DestroyTexture(texture);
            surface1 = IMG_Load("Slike/Pobeda.png");
            texture = SDL_CreateTextureFromSurface(renderer, surface1);
            SDL_FreeSurface(surface1);
            NapraviZid(0.05 * MaxWidth, 0.05 * MaxHeight, 0.9 * MaxWidth, 0.9 * MaxHeight, texture);
            SDL_DestroyTexture(texture);
            TTF_SetFontSize(font[1], 0.5 * velicina);
            TTF_SetFontSize(font[0], 0.46 * velicina);
            TTF_SizeText(font[0], (const char*)"Rezultat", &x[0], &y[0]);
            PisiTekst(font[0], { 100,80,40,255 }, { (int)(MaxWidth - x[0]) / 2, (int)(0.48 * MaxHeight), x[0],y[0] }, (char*)"Rezultat");
            TTF_SizeText(font[1], (const char*)":", &x[1], &y[1]);
            PisiTekst(font[1], { 100,80,40,255 }, { (int)(MaxWidth + x[0]) / 2, (int)(0.48 * MaxHeight - y[1] / 5), x[1],y[1] }, (char*)":");
            TTF_SetFontSize(font[0], 0.9 * velicina);
            TTF_SizeText(font[0], (const char*)score, &x[2], &y[2]);
            PisiTekst(font[0], { 81,67,35,255 }, { (int)(MaxWidth - x[2]) / 2, (int)(0.572 * MaxHeight), x[2],y[2] }, (char*)score);
            TTF_SetFontSize(font[0], 0.6 * velicina);
            if (LastIndex == -1) {
                if (ModIgre != 1) {
                    TTF_SizeText(font[0], (const char*)"Sledeci Nivo", &x[3], &y[3]);
                    PisiTekst(font[0], { 100,80,40,255 }, { (int)(MaxWidth - x[3]) / 2, (int)(0.72 * MaxHeight), x[3],y[3] }, (char*)"Sledeci Nivo");
                }
                else {
                    TTF_SizeText(font[0], (const char*)"Mapa", &x[3], &y[3]);
                    PisiTekst(font[0], { 100,80,40,255 }, { (int)(MaxWidth - x[3]) / 2, (int)(0.72 * MaxHeight), x[3],y[3] }, (char*)"Mapa");
                }
            }
            else {
                TTF_SetFontSize(font[0], 0.55 * velicina);
                TTF_SizeText(font[0], (const char*)"Pokusate  Ponovo", &x[3], &y[3]);
                if (boja == 0)
                    PisiTekst(font[0], { 100,80,40,255 }, { (int)(MaxWidth - x[3]) / 2, (int)(0.72 * MaxHeight), x[3],y[3] }, (char*)"Pokusate  Ponovo");
                else
                    PisiTekst(font[0], { 144,110,36,255 }, { (int)(MaxWidth - x[3]) / 2, (int)(0.72 * MaxHeight), x[3],y[3] }, (char*)"Pokusate  Ponovo");
            }
            if (LastIndex == -1) {
                surface1 = IMG_Load("Slike/TrofejPozadina.png");
                SDL_Texture* TrofejPozadina = SDL_CreateTextureFromSurface(renderer, surface1);
                SDL_FreeSurface(surface1);
                surface1 = IMG_Load("Slike/Trofej.png");
                SDL_Texture* Trofej = SDL_CreateTextureFromSurface(renderer, surface1);
                SDL_FreeSurface(surface1);
                surface1 = IMG_Load("Slike/Pobeda.png");
                texture = SDL_CreateTextureFromSurface(renderer, surface1);
                SDL_FreeSurface(surface1);
                SDL_Texture* texture1 = NULL;
                for (int i = 0; i < BrTrofeja; i++) {
                    intervalx = (i != 2) ? (0.4 * velicina) : (0.575 * velicina);
                    intervaly = (i != 2) ? (0.6 * velicina) : (0.8625 * velicina);
                    uvecanje = 2.0;
                    if (i == 2) {
                        SDL_Surface* surface1 = IMG_Load("Slike/PobedaPozadina.png");
                        texture1 = SDL_CreateTextureFromSurface(renderer, surface1);
                        SDL_FreeSurface(surface1);
                    }
                    while (1) {
                        TrenutnoVreme = SDL_GetTicks();
                        if (TrenutnoVreme - ProsloVreme >= interval) {
                            if (i == 0) {
                                NapraviZid(0.05 * MaxWidth, 0.05 * MaxHeight, 0.9 * MaxWidth, 0.9 * MaxHeight, texture);
                                NapraviZid(0.38 * MaxWidth, 0.22 * MaxHeight + 0.2625 * velicina, 0.8 * velicina, 1.2 * velicina, TrofejPozadina);
                                NapraviZid(0.38 * MaxWidth + 0.08 * MaxWidth, 0.22 * MaxHeight, 1.15 * velicina, 1.725 * velicina, TrofejPozadina);
                                NapraviZid(0.38 * MaxWidth + 0.16 * MaxWidth + 0.35 * velicina, 0.22 * MaxHeight + 0.2625 * velicina, 0.8 * velicina, 1.2 * velicina, TrofejPozadina);
                                NapraviZid(0.38 * MaxWidth - intervalx, 0.22 * MaxHeight + 0.2625 * velicina - intervaly, uvecanje * 0.8 * velicina, uvecanje * 1.2 * velicina, Trofej);
                                TTF_SetFontSize(font[0], 0.46 * velicina);
                                PisiTekst(font[0], { 100,80,40,255 }, { (int)(MaxWidth - x[0]) / 2, (int)(0.48 * MaxHeight), x[0],y[0] }, (char*)"Rezultat");
                            }
                            else if (i == 1) {
                                NapraviZid(0.05 * MaxWidth, 0.05 * MaxHeight, 0.9 * MaxWidth, 0.9 * MaxHeight, texture);
                                NapraviZid(0.38 * MaxWidth + 0.16 * MaxWidth + 0.35 * velicina, 0.22 * MaxHeight + 0.2625 * velicina, 0.8 * velicina, 1.2 * velicina, TrofejPozadina);
                                NapraviZid(0.38 * MaxWidth + 0.08 * MaxWidth, 0.22 * MaxHeight, 1.15 * velicina, 1.725 * velicina, TrofejPozadina);
                                NapraviZid(0.38 * MaxWidth + 0.16 * MaxWidth + 0.35 * velicina - intervalx, 0.22 * MaxHeight + 0.2625 * velicina - intervaly, uvecanje * 0.8 * velicina, uvecanje * 1.2 * velicina, Trofej);
                                TTF_SetFontSize(font[0], 0.46 * velicina);
                                PisiTekst(font[0], { 100,80,40,255 }, { (int)(MaxWidth - x[0]) / 2, (int)(0.48 * MaxHeight), x[0],y[0] }, (char*)"Rezultat");
                            }
                            else if (i == 2) {
                                NapraviZid(0, 0, MaxWidth, MaxHeight, texture1);
                                NapraviZid(0.05 * MaxWidth, 0.05 * MaxHeight, 0.9 * MaxWidth, 0.9 * MaxHeight, texture);
                                NapraviZid(0.38 * MaxWidth + 0.08 * MaxWidth, 0.22 * MaxHeight, 1.15 * velicina, 1.725 * velicina, TrofejPozadina);
                                TTF_SetFontSize(font[0], 0.46 * velicina);
                                PisiTekst(font[0], { 100,80,40,255 }, { (int)(MaxWidth - x[0]) / 2, (int)(0.48 * MaxHeight), x[0],y[0] }, (char*)"Rezultat");
                                PisiTekst(font[1], { 100,80,40,255 }, { (int)(MaxWidth + x[0]) / 2, (int)(0.48 * MaxHeight - y[1] / 5), x[1],y[1] }, (char*)":");
                                TTF_SetFontSize(font[0], 0.9 * velicina);
                                PisiTekst(font[0], { 81,67,35,255 }, { (int)(MaxWidth - x[2]) / 2, (int)(0.572 * MaxHeight), x[2],y[2] }, (char*)score);
                                TTF_SetFontSize(font[0], 0.6 * velicina);
                                if (ModIgre != 1)
                                    PisiTekst(font[0], { 100,80,40,255 }, { (int)(MaxWidth - x[3]) / 2, (int)(0.72 * MaxHeight), x[3],y[3] }, (char*)"Sledeci Nivo");
                                else
                                    PisiTekst(font[0], { 100,80,40,255 }, { (int)(MaxWidth - x[3]) / 2, (int)(0.72 * MaxHeight), x[3],y[3] }, (char*)"Mapa");
                                NapraviZid(0.38 * MaxWidth + 0.08 * MaxWidth - intervalx, 0.22 * MaxHeight - intervaly, uvecanje * 1.15 * velicina, uvecanje * 1.725 * velicina, Trofej);
                            }
                            if (i != 0) {
                                NapraviZid(0.38 * MaxWidth, 0.22 * MaxHeight + 0.2625 * velicina, 0.8 * velicina, 1.2 * velicina, Trofej);
                                if (i == 2)
                                    NapraviZid(0.38 * MaxWidth + 0.16 * MaxWidth + 0.35 * velicina, 0.22 * MaxHeight + 0.2625 * velicina, 0.8 * velicina, 1.2 * velicina, Trofej);
                            }
                            PisiTekst(font[1], { 100,80,40,255 }, { (int)(MaxWidth + x[0]) / 2, (int)(0.48 * MaxHeight - y[1] / 5), x[1],y[1] }, (char*)":");
                            TTF_SetFontSize(font[0], 0.9 * velicina);
                            PisiTekst(font[0], { 81,67,35,255 }, { (int)(MaxWidth - x[2]) / 2, (int)(0.572 * MaxHeight), x[2],y[2] }, (char*)score);
                            TTF_SetFontSize(font[0], 0.6 * velicina);
                            if (ModIgre != 1)
                                PisiTekst(font[0], { 100,80,40,255 }, { (int)(MaxWidth - x[3]) / 2, (int)(0.72 * MaxHeight), x[3],y[3] }, (char*)"Sledeci Nivo");
                            else
                                PisiTekst(font[0], { 100,80,40,255 }, { (int)(MaxWidth - x[3]) / 2, (int)(0.72 * MaxHeight), x[3],y[3] }, (char*)"Mapa");
                            SDL_RenderPresent(renderer);
                            ProsloVreme = TrenutnoVreme;
                            uvecanje -= 1.0 / 60;
                            intervalx -= ((i != 2) ? (0.4 * velicina) : (0.575 * velicina)) / 60;
                            intervaly -= ((i != 2) ? (0.6 * velicina) : (0.8625 * velicina)) / 60;
                            if (intervalx <= 0)
                                break;
                        }
                    }
                    if (i == 2)
                        SDL_DestroyTexture(texture1);
                }
                SDL_DestroyTexture(texture);
                SDL_DestroyTexture(Trofej);
                SDL_DestroyTexture(TrofejPozadina);
            }
            else if (LastIndex == -2) {
                char adresa[40];
                surface1 = IMG_Load("Slike/Grob.png");
                SDL_Texture* Grob = SDL_CreateTextureFromSurface(renderer, surface1);
                SDL_FreeSurface(surface1);
                NapraviZid((MaxWidth - 1.6 * velicina) / 2, 0.22 * MaxHeight, 1.5 * velicina, 1.5 * velicina, Grob);
                SDL_DestroyTexture(Grob);
                sprintf_s(adresa, "Slike/Igrac/Nazad/%d/Died/29.png", Izgledi[1]);
                surface1 = IMG_Load(adresa);
                SDL_Texture* Mrtav = SDL_CreateTextureFromSurface(renderer, surface1);
                SDL_FreeSurface(surface1);
                NapraviZid((MaxWidth - 1.2 * velicina) / 2, 0.24 * MaxHeight, 1.8 * velicina, 1.8 * velicina, Mrtav);
                SDL_DestroyTexture(Mrtav);
            }
        }
    }
    else if (index == 1) {
        TTF_SetFontSize(font[0], 0.6 * velicina);
        if (ModIgre != 1) {
            TTF_SizeText(font[0], (const char*)"Sledeci Nivo", &x[3], &y[3]);
            PisiTekst(font[0], { 144,110,36,255 }, { (int)(MaxWidth - x[3]) / 2, (int)(0.72 * MaxHeight), x[3],y[3] }, (char*)"Sledeci Nivo");
        }
        else {
            TTF_SizeText(font[0], (const char*)"Mapa", &x[3], &y[3]);
            PisiTekst(font[0], { 144,110,36,255 }, { (int)(MaxWidth - x[3]) / 2, (int)(0.72 * MaxHeight), x[3],y[3] }, (char*)"Mapa");
        }
    }
    else  if (index == 2) {
        TTF_SetFontSize(font[0], 0.6 * velicina);
        if (ModIgre != 1) {
            TTF_SizeText(font[0], (const char*)"Sledeci Nivo", &x[3], &y[3]);
            PisiTekst(font[0], { 100,80,40,255 }, { (int)(MaxWidth - x[3]) / 2, (int)(0.72 * MaxHeight), x[3],y[3] }, (char*)"Sledeci Nivo");
        }
        else {
            TTF_SizeText(font[0], (const char*)"Mapa", &x[3], &y[3]);
            PisiTekst(font[0], { 100,80,40,255 }, { (int)(MaxWidth - x[3]) / 2, (int)(0.72 * MaxHeight), x[3],y[3] }, (char*)"Mapa");
        }
    }
    else if (index == 3) {
        TTF_SetFontSize(font[0], 0.55 * velicina);
        TTF_SizeText(font[0], (const char*)"Pokusate  Ponovo", &x[3], &y[3]);
        PisiTekst(font[0], { 144,110,36,255 }, { (int)(MaxWidth - x[3]) / 2, (int)(0.72 * MaxHeight), x[3],y[3] }, (char*)"Pokusate  Ponovo");
    }
    else  if (index == 4) {
        TTF_SetFontSize(font[0], 0.55 * velicina);
        TTF_SizeText(font[0], (const char*)"Pokusate  Ponovo", &x[3], &y[3]);
        PisiTekst(font[0], { 100,80,40,255 }, { (int)(MaxWidth - x[3]) / 2, (int)(0.72 * MaxHeight), x[3],y[3] }, (char*)"Pokusate  Ponovo");
    }
}
void Gubitak(SDL_Surface** surface, int radi, struct Maze* talon) {
    Mix_VolumeMusic(Prigusenje);
    PustiZvuk(zvukovi[4]);
    int i = 0, j, x[2], y[2], index = 0, velicina1 = Velicina(), interval = 1000 / 90, smer = 0;
    float pomerix1 = 0.1 * MaxWidth, pomerix = 0.45 * MaxWidth;
    Uint32 TrenutnoVreme, ProsloVreme = SDL_GetTicks();
    char adresa[50];
    SDL_Surface* pozadina = IMG_Load("Slike/PobedaPozadina.png");
    SDL_Texture* Pozadina = SDL_CreateTextureFromSurface(renderer, pozadina);
    SDL_FreeSurface(pozadina);
    NapraviZid(0, 0, MaxWidth, MaxHeight, Pozadina);
    SDL_Surface* upitnik = IMG_Load("Slike/Pobeda.png");
    SDL_Texture* Upitnik = SDL_CreateTextureFromSurface(renderer, upitnik);
    SDL_FreeSurface(upitnik);
    izgubio = 1;
    TTF_SetFontSize(font[0], 0.55 * velicina);
    TTF_SizeText(font[0], (const char*)"Pokusate  Ponovo", &x[1], &y[1]);
    SDL_RenderPresent(renderer);
    while (1) {
        TrenutnoVreme = SDL_GetTicks();
        if (TrenutnoVreme - ProsloVreme >= interval && smer == 0) {
            if (smer == 0) {
                DeoTeksture(0.1 * MaxWidth, 0.05 * MaxHeight, 0.8 * MaxWidth, 0.9 * MaxHeight, 0.1 * 1920., 0.05 * 1080.0, 0.8 * 1920, 0.9 * 1080, Pozadina);
                NapraviZid(pomerix, 0.05 * MaxHeight, pomerix1, 0.9 * MaxHeight, Upitnik);
                SDL_RenderPresent(renderer);
                pomerix -= 0.4 * MaxWidth / 90;
                pomerix1 += 0.8 * MaxWidth / 90;
                if (pomerix <= 0.05 * MaxWidth) {
                    smer = 1;
                    CrtajPobeda(surface, 0, -2, (char*)"0", 0);
                    SDL_RenderPresent(renderer);
                }
            }
            ProsloVreme = TrenutnoVreme;
        }
        while (SDL_PollEvent(&event) != 0 && smer == 1) {
            SDL_GetMouseState(&x[0], &y[0]);
            if (event.type == SDL_QUIT) {
                SDL_Quit();
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (x[0] >= ((MaxWidth - x[1]) / 2) && x[0] <= ((MaxWidth + x[1]) / 2) && y[0] >= (0.72 * MaxHeight) && y[0] <= (0.72 * MaxHeight + y[1])) {
                    PustiZvuk(zvukovi[1]);
                    while (1) {
                        TrenutnoVreme = SDL_GetTicks();
                        if (TrenutnoVreme - ProsloVreme >= interval && smer == 1) {
                            if (smer == 1) {
                                DeoTeksture(0.1 * MaxWidth, 0.05 * MaxHeight, 0.8 * MaxWidth, 0.9 * MaxHeight, 0.1 * 1920., 0.05 * 1080.0, 0.8 * 1920, 0.9 * 1080, Pozadina);
                                NapraviZid(pomerix, 0.05 * MaxHeight, pomerix1, 0.9 * MaxHeight, Upitnik);
                                SDL_RenderPresent(renderer);
                                pomerix += 0.4 * MaxWidth / 90;
                                pomerix1 -= 0.8 * MaxWidth / 90;
                                if (pomerix >= 0.4 * MaxWidth) {
                                    break;
                                }
                            }
                            ProsloVreme = TrenutnoVreme;
                        }
                    }
                    SDL_DestroyTexture(Upitnik);
                    SDL_DestroyTexture(Pozadina);
                    Mix_VolumeMusic(Pozadinska);
                    Igra(surface, SDL_GetTicks());
                }
            }
            else {
                if (x[0] >= ((MaxWidth - x[1]) / 2) && x[0] <= ((MaxWidth + x[1]) / 2) && y[0] >= (0.72 * MaxHeight) && y[0] <= (0.72 * MaxHeight + y[1])) {
                    CrtajPobeda(surface, 3, 1);
                    SDL_RenderPresent(renderer);
                }
                else {
                    CrtajPobeda(surface, 4, 1);
                    SDL_RenderPresent(renderer);
                }
            }
        }
    }
}
void Pobeda(SDL_Surface** surface, int BrKoraka, Uint32 TrenutnoVreme1, int radi = 1) {

    int x[2], y[2], Rezultat = 10000 - 50 * (TrenutnoVreme1 / 1000) - 100 * BrKoraka, interval = 1000 / 90, smer = 0;
    float pomerix1 = 0.1 * MaxWidth, pomerix = 0.45 * MaxWidth;
    Uint32 TrenutnoVreme, ProsloVreme = SDL_GetTicks();
    char score[10];
    SDL_Surface* pozadina = IMG_Load("Slike/PobedaPozadina.png");
    SDL_Texture* Pozadina = SDL_CreateTextureFromSurface(renderer, pozadina);
    SDL_FreeSurface(pozadina);
    NapraviZid(0, 0, MaxWidth, MaxHeight, Pozadina);
    SDL_Surface* upitnik = IMG_Load("Slike/Pobeda.png");
    SDL_Texture* Upitnik = SDL_CreateTextureFromSurface(renderer, upitnik);
    SDL_FreeSurface(upitnik);
    sprintf_s(score, "%d", Rezultat);
    TTF_SetFontSize(font[0], 0.6 * velicina);
    izgubio = 0;
    if (ModIgre != 1)
        TTF_SizeText(font[0], (const char*)"Sledeci Nivo", &x[1], &y[1]);
    else
        TTF_SizeText(font[0], (const char*)"Mapa", &x[1], &y[1]);
    SDL_RenderPresent(renderer);
    Mix_VolumeMusic(Prigusenje);
    PustiZvuk(zvukovi[3]);
    while (1) {
        TrenutnoVreme = SDL_GetTicks();
        if (TrenutnoVreme - ProsloVreme >= interval && smer == 0) {
            if (smer == 0) {
                DeoTeksture(0.1 * MaxWidth, 0.05 * MaxHeight, 0.8 * MaxWidth, 0.9 * MaxHeight, 0.1 * 1920., 0.05 * 1080.0, 0.8 * 1920, 0.9 * 1080, Pozadina);
                NapraviZid(pomerix, 0.05 * MaxHeight, pomerix1, 0.9 * MaxHeight, Upitnik);
                SDL_RenderPresent(renderer);
                pomerix -= 0.4 * MaxWidth / 90;
                pomerix1 += 0.8 * MaxWidth / 90;
                if (pomerix <= 0.05 * MaxWidth) {
                    smer = 1;
                    CrtajPobeda(surface, 0, -1, (char*)score, (Rezultat >= 6666) ? 3 : ((Rezultat >= 3333) ? 2 : 1));
                    SDL_RenderPresent(renderer);
                }
            }
            ProsloVreme = TrenutnoVreme;
        }
        while (SDL_PollEvent(&event) != 0 && smer == 1) {
            SDL_GetMouseState(&x[0], &y[0]);
            if (event.type == SDL_QUIT) SDL_Quit();
            else if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (x[0] >= ((MaxWidth - x[1]) / 2) && x[0] <= ((MaxWidth + x[1]) / 2) && y[0] >= (0.72 * MaxHeight) && y[0] <= (0.72 * MaxHeight + y[1])) {
                    PustiZvuk(zvukovi[1]);
                    if (ModIgre != 1) {
                        if (ModIgre == 2 && Sacuvaj == 1) {
                            Sacuvaj = 0;
                            n = n1;
                        }
                        while (1) {
                            TrenutnoVreme = SDL_GetTicks();
                            if (TrenutnoVreme - ProsloVreme >= interval && smer == 1) {
                                if (smer == 1) {
                                    DeoTeksture(0.1 * MaxWidth, 0.05 * MaxHeight, 0.8 * MaxWidth, 0.9 * MaxHeight, 0.1 * 1920., 0.05 * 1080.0, 0.8 * 1920, 0.9 * 1080, Pozadina);
                                    NapraviZid(pomerix, 0.05 * MaxHeight, pomerix1, 0.9 * MaxHeight, Upitnik);
                                    SDL_RenderPresent(renderer);
                                    pomerix += 0.4 * MaxWidth / 90;
                                    pomerix1 -= 0.8 * MaxWidth / 90;
                                    if (pomerix >= 0.4 * MaxWidth) {
                                        break;
                                    }
                                }
                                ProsloVreme = TrenutnoVreme;
                            }
                        }
                        SDL_DestroyTexture(Upitnik);
                        SDL_DestroyTexture(Pozadina);
                        Mix_VolumeMusic(Pozadinska);
                        Igra(surface, SDL_GetTicks());
                    }
                    else {
                        if (UsoNivo == TrenutniNivo) {
                            TrenutniNivo++;
                            UsoNivo++;
                        }
                        while (1) {
                            TrenutnoVreme = SDL_GetTicks();
                            if (TrenutnoVreme - ProsloVreme >= interval && smer == 1) {
                                if (smer == 1) {
                                    DeoTeksture(0.1 * MaxWidth, 0.05 * MaxHeight, 0.8 * MaxWidth, 0.9 * MaxHeight, 0.1 * 1920., 0.05 * 1080.0, 0.8 * 1920, 0.9 * 1080, Pozadina);
                                    NapraviZid(pomerix, 0.05 * MaxHeight, pomerix1, 0.9 * MaxHeight, Upitnik);
                                    SDL_RenderPresent(renderer);
                                    pomerix += 0.4 * MaxWidth / 90;
                                    pomerix1 -= 0.8 * MaxWidth / 90;
                                    if (pomerix >= 0.4 * MaxWidth) {
                                        break;
                                    }
                                }
                                ProsloVreme = TrenutnoVreme;
                            }
                        }
                        SDL_DestroyTexture(Upitnik);
                        SDL_DestroyTexture(Pozadina);
                        FILE* file = NULL;
                        fopen_s(&file, (const char*)"Opcije.txt", (const char*)"w");
                        fprintf_s(file, "%d %d\n%d\n%d %d %d\n", MaxWidth, MaxHeight, n, Tezina, Algoritam, BrZombija);
                        fprintf_s(file, "%d %d %d\n", Izgledi[0], Izgledi[1], Izgledi[2]);
                        fprintf_s(file, "%d %d %d\n", TrenutniNivo, mapa, Sacuvaj);
                        fprintf_s(file, "%d %d", Zvukovi, Pozadinska);
                        fclose(file);
                        if (TrenutniNivo > 6 && mapa < 2) {
                            Mix_VolumeMusic(Pozadinska);
                            PustiZvuk(zvukovi[1]);
                            Mapa(surface, 2);
                        }
                        else {
                            Mix_VolumeMusic(Pozadinska);
                            if (UsoNivo == TrenutniNivo) {
                                //PustiZvuk(zvukovi[5]);
                                Mapa(surface, 1);
                            }
                            else {
                                PustiZvuk(zvukovi[5]);
                                Mapa(surface, 0);
                            }
                        }
                    }
                }
            }
            else {
                if (x[0] >= ((MaxWidth - x[1]) / 2) && x[0] <= ((MaxWidth + x[1]) / 2) && y[0] >= (0.72 * MaxHeight) && y[0] <= (0.72 * MaxHeight + y[1])) {
                    CrtajPobeda(surface, 1, 1);
                    SDL_RenderPresent(renderer);
                }
                else {
                    CrtajPobeda(surface, 2, 1);
                    SDL_RenderPresent(renderer);
                }
            }
        }
    }
}
int Mumija(int x, int y, int xm, int ym) {
    return (x == xm && ym == y) ? 0 : 1;
}
int MumijaNaPoziciji(Pozicija* mumije, int index, int x, int y) {
    for (int i = 0; i < trbrojmumija; i++)
        if (i != index && mumije[i].x == x && mumije[i].y == y)
            return i;
    return -1;
}
int PomeriMumiju(SDL_Surface** surface, int x, int y, struct Maze* talon, int mumija, int* intervalx, char* minut) {
    int disx, disy, dirx, diry, index, k, velicina1 = Velicina();
    char adresa[40];
    SDL_Surface* surfaces[32];
    SDL_Texture* textures[32];
    for (int i = 0; i < 2; i++) {
        disx = abs(x - mumije[mumija].x);
        disy = abs(y - mumije[mumija].y);
        if (disx == 0 && disy == 0)
            return 0;
        k = 0;
        dirx = (disx == 0) ? 0 : (x > mumije[mumija].x) ? 1 : -1;
        diry = (disy == 0) ? 0 : (y > mumije[mumija].y) ? 1 : -1;
        if ((disx <= disy && disx != 0 && Validan(mumije[mumija].x + dirx, mumije[mumija].y) && (Zid(mumije[mumija].x + dirx, mumije[mumija].y, mumije[mumija].x, mumije[mumija].y, talon) || Zid(mumije[mumija].x, mumije[mumija].y, mumije[mumija].x + dirx, mumije[mumija].y, talon))) || (disy == 0)) {
            if (Validan(mumije[mumija].x + dirx, mumije[mumija].y)) {
                if (dirx < 0 && Zid(mumije[mumija].x + dirx, mumije[mumija].y, mumije[mumija].x, mumije[mumija].y, talon)) {
                    int channel = Mix_PlayChannel(-1, zvukovi[6], 0);
                    Mix_PlayChannelTimed(channel, zvukovi[6], 1, 800);
                    Pokret(surface, talon, 0.48 * MaxWidth + velicina1 * mumije[mumija].y, 2 * MaxHeight / 15 + velicina1 * mumije[mumija].x, velicina1, 4, intervalx, minut, 1);
                    StranaMumije[mumija] = 0;
                    mumije[mumija].x += dirx;
                    mumije[mumija].ym = 2 * MaxHeight / 15 + mumije[mumija].x * velicina1;
                    k = 1;
                }
                if (dirx > 0 && Zid(mumije[mumija].x, mumije[mumija].y, mumije[mumija].x + dirx, mumije[mumija].y, talon)) {
                    int channel = Mix_PlayChannel(-1, zvukovi[6], 0);
                    Mix_PlayChannelTimed(channel, zvukovi[6], 1, 800);
                    Pokret(surface, talon, 0.48 * MaxWidth + velicina1 * mumije[mumija].y, 2 * MaxHeight / 15 + velicina1 * mumije[mumija].x, velicina1, 1, intervalx, minut, 1);
                    StranaMumije[mumija] = 1;
                    mumije[mumija].x += dirx;
                    mumije[mumija].ym = 2 * MaxHeight / 15 + mumije[mumija].x * velicina1;
                    k = 1;
                }
            }
        }
        if ((disy < disx && disy != 0 && Validan(mumije[mumija].x, mumije[mumija].y + diry) && (Zid(mumije[mumija].x, mumije[mumija].y, mumije[mumija].x, mumije[mumija].y + diry, talon) || Zid(mumije[mumija].x, mumije[mumija].y + diry, mumije[mumija].x, mumije[mumija].y, talon))) || (disx == 0 && k == 0) || (k == 0)) {
            if (Validan(mumije[mumija].x, mumije[mumija].y + diry)) {
                if (diry < 0 && Zid(mumije[mumija].x, mumije[mumija].y + diry, mumije[mumija].x, mumije[mumija].y, talon)) {
                    int channel = Mix_PlayChannel(-1, zvukovi[6], 0);
                    Mix_PlayChannelTimed(channel, zvukovi[6], 1, 800);
                    Pokret(surface, talon, 0.48 * MaxWidth + velicina1 * mumije[mumija].y, 2 * MaxHeight / 15 + velicina1 * mumije[mumija].x, velicina1, 3, intervalx, minut, 1);
                    StranaMumije[mumija] = 0;
                    mumije[mumija].y += diry;
                    mumije[mumija].xm = 0.48 * MaxWidth + velicina1 * mumije[mumija].y;
                }
                if (diry > 0 && Zid(mumije[mumija].x, mumije[mumija].y, mumije[mumija].x, mumije[mumija].y + diry, talon)) {
                    int channel = Mix_PlayChannel(-1, zvukovi[6], 0);
                    Mix_PlayChannelTimed(channel, zvukovi[6], 1, 800);
                    Pokret(surface, talon, 0.48 * MaxWidth + velicina1 * mumije[mumija].y, 2 * MaxHeight / 15 + velicina1 * mumije[mumija].x, velicina1, 2, intervalx, minut, 1);
                    StranaMumije[mumija] = 1;
                    mumije[mumija].y += diry;
                    mumije[mumija].xm = 0.48 * MaxWidth + velicina1 * mumije[mumija].y;
                }
            }
        }
        if (mumije[mumija].x == x && mumije[mumija].y == y)
            Gubitak(surface, 1, talon);
        index = MumijaNaPoziciji(mumije, mumija, mumije[mumija].x, mumije[mumija].y);
        if (index != -1) {
            trbrojmumija--;
        }
    }
    return 1;
}
int PomeriMumijuH(int* xm, int* ym, int x, int y, struct Maze* talon) {
    int disx = abs(x - *xm);
    int disy = abs(y - *ym);
    if (disx == 0 && disy == 0) {
        return 0;
    }
    int k = 0;
    int dirx = (disx == 0) ? 0 : (x > *xm) ? 1 : -1;
    int diry = (disy == 0) ? 0 : (y > *ym) ? 1 : -1;
    if ((disx <= disy && disx != 0 && Validan(*xm + dirx, *ym) && (Zid(*xm + dirx, *ym, *xm, *ym, talon) || Zid(*xm, *ym, *xm + dirx, *ym, talon))) || (disy == 0)) {
        if (Validan(*xm + dirx, *ym)) {
            if (dirx < 0 && Zid(*xm + dirx, *ym, *xm, *ym, talon)) {
                *xm += dirx;
                k = 1;
            }
            if (dirx > 0 && Zid(*xm, *ym, *xm + dirx, *ym, talon)) {
                *xm += dirx;
                k = 1;
            }
        }
    }
    if ((disy < disx && disy != 0 && Validan(*xm, *ym + diry) && (Zid(*xm, *ym, *xm, *ym + diry, talon) || Zid(*xm, *ym + diry, *xm, *ym, talon))) || (disx == 0 && k == 0) || (k == 0)) {
        if (Validan(*xm, *ym + diry)) {
            if (diry < 0 && Zid(*xm, *ym + diry, *xm, *ym, talon)) {
                *ym += diry;
                k = 1;
            }
            if (diry > 0 && Zid(*xm, *ym, *xm, *ym + diry, talon)) {
                *ym += diry;
                k = 1;
            }
        }
    }
    return 1;
}
int MumijaNaPozicijiHint(Pozicija* mumijeh, int index, int x, int y, int trbr) {
    for (int i = 0; i < trbr; i++) {
        if (i != index && mumijeh[i].x == x && mumijeh[i].y == y) {
            return i;
        }
    }
    return -1;
}
int PomeriMumijeHint(struct Pozicija* mumijeh, int x, int y, struct Maze* talon, int* trbr) {
    for (int i = 0; i < *trbr; i++) {
        PomeriMumijuH(&mumijeh[i].x, &mumijeh[i].y, x, y, talon);
        int index = MumijaNaPozicijiHint(mumijeh, i, mumijeh[i].x, mumijeh[i].y, *trbr);
        if (index != -1) {
            for (int j = index; j < *trbr - 1; j++) {
                mumijeh[j] = mumijeh[j + 1];
            }
            (*trbr)--;
        }
        if (mumijeh[i].x == x && mumijeh[i].y == y) {
            return 1;
        }
        PomeriMumijuH(&mumijeh[i].x, &mumijeh[i].y, x, y, talon);
        index = MumijaNaPozicijiHint(mumijeh, i, mumijeh[i].x, mumijeh[i].y, *trbr);
        if (index != -1) {
            for (int j = index; j < *trbr - 1; j++) {
                mumijeh[j] = mumijeh[j + 1];
            }
            (*trbr)--;
        }
        if (mumijeh[i].x == x && mumijeh[i].y == y) {
            return 1;
        }
    }
    for (int i = 0; i < *trbr; i++) {
        if (mumijeh[i].x == x && mumijeh[i].y == y) {
            return 1;
        }
    }
    return *trbr * (-1);
}
int ValidanPokret(int x, int y) {
    return  ((x >= 0) && (y >= 0) && (x < n) && (y < n) && MumijaNaPoziciji(mumije, -1, x, y) == -1) ? 1 : 0;
}
bool ProveriKretanjeIgraca(struct State* stanje, struct Maze* talon) {
    int x = stanje->x, y = stanje->y;
    struct Pozicija* mumije = stanje->mumije;
    int br_mumija = stanje->br_mumija;
    int possible = PomeriMumijeHint(mumije, x, y, talon, &br_mumija);
    if (possible == 1) return false;
    if (x == izlazx && y == izlazy) return true;
    return true;
}
struct HashTable* initHashTable() {
    struct HashTable* hashTable = (struct HashTable*)malloc(sizeof(struct HashTable));
    hashTable->table = (struct HashEntry**)calloc(HASH_TABLE_SIZE, sizeof(struct HashEntry*));
    return hashTable;
}
void freeHashTable(struct HashTable* hashTable) {
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        struct HashEntry* entry = hashTable->table[i];
        while (entry) {
            struct HashEntry* temp = entry;
            entry = entry->next;
            free(temp);
        }
    }
    free(hashTable->table);
    free(hashTable);
}
unsigned int hash_state(struct State* state, int width, int height) {
    unsigned int hash = state->x * width + state->y;
    for (int i = 0; i < state->br_mumija; i++) {
        hash = hash * width * height + state->mumije[i].x * width + state->mumije[i].y;
    }
    return hash % HASH_TABLE_SIZE;
}
bool hashtable_contains(struct HashTable* hashTable, struct State* state, int width, int height) {
    unsigned int hash = hash_state(state, width, height);
    struct HashEntry* entry = hashTable->table[hash];
    while (entry) {
        struct State* existing = &entry->state;
        if (existing->x == state->x && existing->y == state->y && existing->br_mumija == state->br_mumija) {
            bool same = true;
            for (int i = 0; i < state->br_mumija; i++) {
                if (existing->mumije[i].x != state->mumije[i].x || existing->mumije[i].y != state->mumije[i].y) {
                    same = false;
                    break;
                }
            }
            if (same) return true;
        }
        entry = entry->next;
    }
    return false;
}
void hashtable_add(struct HashTable* hashTable, struct State* state, int width, int height) {
    unsigned int hash = hash_state(state, width, height);
    struct HashEntry* entry = (struct HashEntry*)malloc(sizeof(struct HashEntry));
    entry->state = *state;
    entry->next = hashTable->table[hash];
    hashTable->table[hash] = entry;
}
int MogucPotez(int x, int  y, int  dirx, int diry, Maze* maze, Maze* talon) {
    int sledx = x + dirx;
    int sledy = y + diry;
    int dead;
    if (dirx != 0 && Validan(sledx, sledy) && (Zid(sledx, sledy, x, y, talon) || Zid(x, y, sledx, sledy, talon))) {
        if (dirx < 0 && Zid(sledx, sledy, x, y, talon)) {
            return 1;
        }
        if (dirx > 0 && Zid(x, y, sledx, sledy, talon)) {
            return 1;
        }
    }
    if (diry != 0 && Validan(sledx, sledy) && (Zid(sledx, sledy, x, y, talon) || Zid(x, y, sledx, sledy, talon))) {
        if (diry < 0 && Zid(sledx, sledy, x, y, talon)) {
            return 1;
        }
        if (diry > 0 && Zid(x, y, sledx, sledy, talon)) {
            return 1;
        }
    }
    return 0;
}
struct State* Put(int x, int y, struct Maze* maze, struct Maze* talon, int br_mumija, Pozicija* mumijehint, int* brojkoraka) {
    int startx = x;
    int starty = y;
    int directions[4][2] = { {0,1}, {0,-1}, {1,0}, {-1,0} };
    Pozicija* mumijest = (Pozicija*)malloc(sizeof(Pozicija) * br_mumija);
    for (int i = 0; i < br_mumija; i++)
        mumijest[i] = mumijehint[i];
    struct State start_state = { startx, starty, mumijest, br_mumija, -1 };
    struct State* queue = (struct State*)malloc(100000 * sizeof(struct State));
    int front = 0, rear = 0;
    int broj = 0;
    queue[rear++] = start_state;
    struct HashTable* poseceno = initHashTable();
    hashtable_add(poseceno, &start_state, 100000, 10000);
    while (front < rear && broj < 1000) {
        broj++;
        struct State current = queue[front++];
        for (int i = 0; i < 4; i++) {
            int new_x = current.x + directions[i][0];
            int new_y = current.y + directions[i][1];
            if (!MogucPotez(current.x, current.y, directions[i][0], directions[i][1], maze, talon)) continue;
            struct State new_state = { new_x, new_y, (struct Pozicija*)malloc(br_mumija * sizeof(struct Pozicija)), br_mumija, front - 1 };
            for (int j = 0; j < br_mumija; j++)
                new_state.mumije[j] = current.mumije[j];
            if (ProveriKretanjeIgraca(&new_state, talon)) {
                if (new_x == izlazx && new_y == izlazy) {
                    queue[rear++] = new_state;
                    struct State* path = (struct State*)malloc((rear - front + 5) * sizeof(struct State));
                    int path_length = 0;
                    for (int i = rear - 1; i != -1; i = queue[i].parent) {
                        path[path_length++] = queue[i];
                    }
                    for (int i = 0; i < path_length; i++) {
                        printf("Step %d: Player at (%d, %d)\n", path_length - i, path[i].x, path[i].y);
                    }
                    //freeHashTable(poseceno);
                    free(queue);
                    *brojkoraka = path_length;
                    return path;
                }
                if (!hashtable_contains(poseceno, &new_state, n, n)) {
                    queue[rear++] = new_state;
                    hashtable_add(poseceno, &new_state, n, n);
                }
                else
                    free(new_state.mumije);
            }
            else
                free(new_state.mumije);
        }
        free(current.mumije);
    }
    // freeHashTable(poseceno);
    free(queue);
    return NULL;
}
void kreiraj_binarnu_datoteku(int n, struct Maze* talon, int broj_zombija, int x_igrac, int y_igrac, Pozicija* zombi, int izlaz_x, int izlaz_y, int pametnaMumija, int nivo) {
    char ime[20];
    sprintf(ime, "Nivo/nivo%d.bin", nivo);
    FILE* file = fopen(ime, "wb");
    unsigned char prvi_bajt = 0b00000000;
    if (n == 6)
        prvi_bajt |= 0b01000000;
    else if (n == 7)
        prvi_bajt |= 0b10000000;
    else if (n == 8)
        prvi_bajt |= 0b11000000;
    prvi_bajt |= ((x_igrac & 0b111) << 3);
    prvi_bajt |= (y_igrac & 0b111);
    fwrite(&prvi_bajt, sizeof(prvi_bajt), 1, file);
    prvi_bajt = 0b00000000;
    if (pametnaMumija)
        prvi_bajt |= 0b10000000;
    if (broj_zombija == 2)
        prvi_bajt |= 0b01000000;
    prvi_bajt |= ((izlaz_x & 0b111) << 3);
    prvi_bajt |= ((izlaz_y & 0b111));
    fwrite(&prvi_bajt, sizeof(prvi_bajt), 1, file);
    prvi_bajt = 0b00000000;
    for (int i = 0; i < broj_zombija; i++) {
        prvi_bajt |= ((zombi[i].x & 0b111) << 3);
        prvi_bajt |= (zombi[i].y & 0b111);
        fwrite(&prvi_bajt, sizeof(prvi_bajt), 1, file);
        prvi_bajt = 0b00000000;
    }
    unsigned char zidovi = 0b00000000;
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n; j++) {
            if (j == n - 1) {
                if (!Zid(i, j, i + 1, j, talon))
                    zidovi |= 1;
                zidovi <<= 1;
                if (!Zid(j, i, j, i + 1, talon))
                    zidovi |= 1;
                zidovi <<= 2 * (8 - n);
                fwrite(&zidovi, sizeof(zidovi), 1, file);
                zidovi = 0b00000000;
            }
            else {
                if (!Zid(i, j, i, j + 1, talon))
                    zidovi |= 1;
                zidovi <<= 1;
                if (!Zid(i, j, i + 1, j, talon))
                    zidovi |= 1;
                if ((j + 1) != 4)
                    zidovi <<= 1;
                if ((j + 1) % 4 == 0) {
                    fwrite(&zidovi, sizeof(zidovi), 1, file);
                    zidovi = 0b00000000;
                }
            }
        }
    }
    fclose(file);
}
Pozicija* ucitaj_binarnu_datoteku(const char* ime_datoteke, struct Maze** talondat, int* n, int* brojmumije, int* pametnamumija) {
    FILE* file = fopen(ime_datoteke, "rb");
    unsigned char prvi_bajt;
    fread(&prvi_bajt, sizeof(prvi_bajt), 1, file);
    if ((prvi_bajt & 0b11000000) == 0b01000000)
        *n = 6;
    else if ((prvi_bajt & 0b11000000) == 0b10000000)
        *n = 7;
    else if ((prvi_bajt & 0b11000000) == 0b11000000)
        *n = 8;
    int x_igrac = (prvi_bajt >> 3) & 0b111;
    int y_igrac = prvi_bajt & 0b111;
    fread(&prvi_bajt, sizeof(prvi_bajt), 1, file);
    *pametnamumija = (prvi_bajt & 0b10000000) >> 7;
    *brojmumije = ((prvi_bajt & 0b01000000) >> 6) + 1;
    Pozicija* pozicije = (Pozicija*)malloc(sizeof(Pozicija) * (2 + *brojmumije));
    pozicije[0].x = x_igrac;
    pozicije[0].y = y_igrac;
    int izlaz_x = (prvi_bajt >> 3) & 0b111;
    int izlaz_y = prvi_bajt & 0b111;
    pozicije[1].x = izlaz_x;
    pozicije[1].y = izlaz_y;
    for (int i = 0; i < *brojmumije; i++) {
        fread(&prvi_bajt, sizeof(prvi_bajt), 1, file);
        int zombi_x = (prvi_bajt >> 3) & 0b111;
        int zombi_y = prvi_bajt & 0b111;
        pozicije[2 + i].x = zombi_x;
        pozicije[2 + i].y = zombi_y;

    }
    int maze_size = 2 * *n - 1;
    *talondat = (struct Maze*)malloc((2 * *n - 1) * (2 * *n - 1) * sizeof(struct Maze));
    for (int i = 0; i < maze_size * maze_size; i++) {
        (*talondat)[i].matrica = 0;
    }
    unsigned char zidovi;
    for (int i = 0; i < *n - 1; i++) {
        fread(&zidovi, sizeof(zidovi), 1, file);
        for (int j = 0; j < *n; j++) {
            if (j == *n - 1) {
                if ((zidovi & 0b10000000))
                    (*talondat)[(2 * i + 1) * (2 * *n - 1) + 2 * j].matrica = 1;
                if ((zidovi & 0b01000000))
                    (*talondat)[2 * j * (2 * *n - 1) + 2 * (i)+1].matrica = 1;
            }
            else {
                if ((zidovi & 0b10000000))
                    (*talondat)[(2 * i) * (2 * *n - 1) + 2 * j + 1].matrica = 1;
                if ((zidovi & 0b01000000))
                    (*talondat)[(i * 2 + 1) * (2 * *n - 1) + 2 * j].matrica = 1;
                if ((j + 1) % 4 != 0)
                    zidovi <<= 2;
                if ((j + 1) % 4 == 0)
                    fread(&zidovi, sizeof(zidovi), 1, file);
            }
        }
    }
    fclose(file);
    return pozicije;
}
void postaviMumije(struct Maze* maze, int size, Pozicija igracpoz, Pozicija izlaz, Pozicija* mumije, int* brojmumija) {
    double igracDistanca = distanca(igracpoz.x, igracpoz.y, izlaz.x, izlaz.y);
    double targetDistance = igracDistanca * 1.5;
    int maxMumija = *brojmumija;
    int mummyCount = 0;
    double tolerance = 1.0;
    double minMummyDistance = 3.0;
    srand(SDL_GetTicks());
    while (mummyCount < maxMumija) {
        int attempts = 0;
        bool mummyPlaced = false;
        while (attempts < 50 && !mummyPlaced) {
            int i = rand() % size;
            int j = rand() % size;
            Pozicija pos = { i, j };
            if (Validan(i, j) && MumijaNaPoziciji(mumije, mummyCount, i, j) == -1) {
                double distancaM = distanca(i, j, igracpoz.x, igracpoz.y);
                bool tooCloseToOtherMummies = false;
                for (int k = 0; k < mummyCount; k++) {
                    if (distanca(i, j, mumije[k].x, mumije[k].y) < minMummyDistance) {
                        tooCloseToOtherMummies = true;
                        break;
                    }
                }
                if (!tooCloseToOtherMummies && fabs(distancaM - targetDistance) < tolerance) {
                    mumije[mummyCount] = pos;
                    mummyCount++;
                    mummyPlaced = true;
                }
            }
            attempts++;
        }
        if (!mummyPlaced)
            tolerance += 1.0;
    }
}
void postaviIgraca(struct Maze* maze, int size, Pozicija* igracpoz, Pozicija izlaz) {
    double minDistance = 4;
    int maxAttempts = 100;
    for (int attempts = 0; attempts < maxAttempts; attempts++) {
        igracpoz->x = rand() % size;
        igracpoz->y = rand() % size;
        if (!Validan(igracpoz->x, igracpoz->y)) {
            continue;
        }
        double distancaIgraca = distanca(igracpoz->x, igracpoz->y, izlaz.x, izlaz.y);

        if (distancaIgraca >= minDistance && !(igracpoz->x == izlaz.x && igracpoz->y == izlaz.y)) {
            return;
        }
    }
}
void KonstruisiNivoe(int state) {
    int brojnivoa = 0;
    Nivo* nivoi = (Nivo*)malloc(sizeof(Nivo) * 6);
    int vel = (state == 0) ? 6 : ((state == 1) ? 7 : 8);
    int sizes[] = { vel, vel };
    int minBrojkoraka[] = { 8, 12, 16, 9, 12 , 16 };
    int pocetna = (state == 0) ? 1 : ((state == 1) ? 7 : 13);
    int zombieCount[6] = { 1, 1, 1, 2, 2, 2 };
    for (int set = 0; set < 2; set++) {
        while (brojnivoa < 3 + set * 3) {
            int x, y;
            int size = sizes[set];
            int minKoraka = minBrojkoraka[brojnivoa];
            int brzombija = zombieCount[brojnivoa];
            Maze* talon = (Maze*)malloc((2 * size) * (2 * size) * sizeof(Maze));
            Maze* maze = (Maze*)malloc(size * size * sizeof(Maze));
            Pozicija* mumijenivoi = (Pozicija*)calloc(3, sizeof(Pozicija));
            for (int k = 0; k < size; k++)
                for (int j = 0; j < size; j++)
                    maze[k * size + j].matrica = 0;

            for (int k = 0; k < 2 * size - 1; k++)
                for (int j = 0; j < 2 * size - 1; j++)
                    talon[k * (2 * size - 1) + j].matrica = 0;

            switch (rand() % 4) {
            case 0: y = rand() % size; x = 0; break;
            case 1: y = size - 1; x = rand() % size; break;
            case 2: y = rand() % size; x = size - 1; break;
            case 3: y = 0; x = rand() % size; break;
            }
            Pozicija izlaz = { x, y };
            izlazx = izlaz.x;
            izlazy = izlaz.y;
            Pozicija igrac = { 0, 0 };
            maze[x * size + y].matrica = 1;
            talon[2 * x * (2 * size - 1) + y * 2].matrica = 2;
            lavirint(x, y, maze, talon);

            int brojkoraka = 0;
            postaviIgraca(maze, size, &igrac, izlaz);
            postaviMumije(maze, size, igrac, izlaz, mumijenivoi, &brzombija);

            if (Put(igrac.x, igrac.y, maze, talon, brzombija, mumijenivoi, &brojkoraka) == NULL || brojkoraka < minKoraka) {
                continue;
            }
            else {
                nivoi[brojnivoa].brojmumija = brzombija;
                nivoi[brojnivoa].igrac = igrac;
                nivoi[brojnivoa].izlaz = izlaz;
                nivoi[brojnivoa].pametnamumija = 0;
                nivoi[brojnivoa].size = size;
                nivoi[brojnivoa].talon = talon;
                nivoi[brojnivoa].mumijenivo = mumijenivoi;
                nivoi[brojnivoa].brojkoraka = brojkoraka;
                kreiraj_binarnu_datoteku(nivoi[brojnivoa].size, nivoi[brojnivoa].talon, nivoi[brojnivoa].brojmumija, nivoi[brojnivoa].igrac.x, nivoi[brojnivoa].igrac.y, nivoi[brojnivoa].mumijenivo, nivoi[brojnivoa].izlaz.x, nivoi[brojnivoa].izlaz.y, nivoi[brojnivoa].pametnamumija, pocetna + brojnivoa);
                brojnivoa++;
            }
        }
    }

    quickSort(nivoi, 0, brojnivoa);
    for (int i = 0; i < brojnivoa; i++) {
        free(nivoi[i].talon);
        free(nivoi[i].mumijenivo);
    }
    free(nivoi);
}
void SDL_RenderDrawCircle(int x, int y, int radius)
{
    int offsetx, offsety, d;
    int status;

    offsetx = 0;
    offsety = radius;
    d = radius - 1;
    status = 0;

    while (offsety >= offsetx) {
        status += SDL_RenderDrawPoint(renderer, x + offsetx, y + offsety);
        status += SDL_RenderDrawPoint(renderer, x + offsety, y + offsetx);
        status += SDL_RenderDrawPoint(renderer, x - offsetx, y + offsety);
        status += SDL_RenderDrawPoint(renderer, x - offsety, y + offsetx);
        status += SDL_RenderDrawPoint(renderer, x + offsetx, y - offsety);
        status += SDL_RenderDrawPoint(renderer, x + offsety, y - offsetx);
        status += SDL_RenderDrawPoint(renderer, x - offsetx, y - offsety);
        status += SDL_RenderDrawPoint(renderer, x - offsety, y - offsetx);

        if (status < 0) {
            status = -1;
            break;
        }

        if (d >= 2 * offsetx) {
            d -= 2 * offsetx + 1;
            offsetx += 1;
        }
        else if (d < 2 * (radius - offsety)) {
            d += 2 * offsety - 1;
            offsety -= 1;
        }
        else {
            d += 2 * (offsety - offsetx - 1);
            offsety -= 1;
            offsetx += 1;
        }
    }
}
void SDL_RenderFillCircle(int x, int y, int radius)
{
    int offsetx, offsety, d;
    int status;


    offsetx = 0;
    offsety = radius;
    d = radius - 1;
    status = 0;

    while (offsety >= offsetx) {

        status += SDL_RenderDrawLine(renderer, x - offsety, y + offsetx,
            x + offsety, y + offsetx);
        status += SDL_RenderDrawLine(renderer, x - offsetx, y + offsety,
            x + offsetx, y + offsety);
        status += SDL_RenderDrawLine(renderer, x - offsetx, y - offsety,
            x + offsetx, y - offsety);
        status += SDL_RenderDrawLine(renderer, x - offsety, y - offsetx,
            x + offsety, y - offsetx);

        if (status < 0) {
            status = -1;
            break;
        }

        if (d >= 2 * offsetx) {
            d -= 2 * offsetx + 1;
            offsetx += 1;
        }
        else if (d < 2 * (radius - offsety)) {
            d += 2 * offsety - 1;
            offsety -= 1;
        }
        else {
            d += 2 * (offsety - offsetx - 1);
            offsety -= 1;
            offsetx += 1;
        }
    }
}
void Animacije(SDL_Surface** surface, struct Maze* talon, int intervalx, int index, int index1 = 0);
void Pokret(SDL_Surface** surface, struct Maze* talon, int x1, int y1, int velicina1, int index, int* intervalx, char* minut, int zombi = 0) {
    int i = 0;
    float intervalxy = velicina1 / 32;
    Uint32 ProsloVreme = SDL_GetTicks(), interval = 1000 / 60, TrenutnoVreme;
    char adresa[50], sekund[10], minut1[10];
    if (zombi == 0) {
        SDL_Surface* surfaces;
        SDL_Texture* textures = NULL;
        intervalxy = velicina1 / 30;
        if (index == 1) {
            while (1) {
                TrenutnoVreme = SDL_GetTicks();
                if (TrenutnoVreme - ProsloVreme >= interval) {
                    TrenutnoVreme1 = SDL_GetTicks() - PauzaVreme;
                    if (TrenutnoVreme1 - ProsloVreme1 >= interval1) {
                        ProsloVreme1 = TrenutnoVreme1;
                        sprintf_s(minut1, "%d", TrenutnoVreme1 / 60000);
                        sprintf_s(sekund, "%d", TrenutnoVreme1 / 1000 - (TrenutnoVreme1 / 60000) * 60);
                        strcat_s(minut1, ":");
                        if ((TrenutnoVreme1 / 1000 - (TrenutnoVreme1 / 60000) * 60) < 10)strcat_s(minut1, "0");
                        strcat_s(minut1, sekund);
                        strcpy(minut, minut1);
                    }
                    Animacije(surface, talon, *intervalx, 1);
                    *intervalx += 3;
                    if (*intervalx >= (12 * MaxWidth / 25 - velicina))
                        *intervalx = 0;
                    sprintf_s(adresa, "Slike/Igrac/Nazad/%d/Walk/%d.png", Izgledi[1], i);
                    surfaces = (SDL_Surface*)IMG_Load(adresa);
                    textures = SDL_CreateTextureFromSurface(renderer, surfaces);
                    SDL_FreeSurface(surfaces);
                    NapraviZid(x1 + 0.1 * velicina1, y1 + 0.1 * velicina1 + (int)intervalxy, 0.8 * velicina1, 0.8 * velicina1, textures);
                    SDL_DestroyTexture(textures);
                    NapraviZid(mumije[0].xm + 0.1 * velicina1, mumije[0].ym + 0.1 * velicina1, 0.8 * velicina1, 0.8 * velicina1, (StranaMumije[0] == 0) ? mumijaleva : mumijadesna);
                    if (trbrojmumija > 1)
                        NapraviZid(mumije[1].xm + 0.1 * velicina1, mumije[1].ym + 0.1 * velicina1, 0.8 * velicina1, 0.8 * velicina1, (StranaMumije[1] == 0) ? mumijaleva : mumijadesna);
                    SDL_RenderPresent(renderer);
                    ProsloVreme = TrenutnoVreme;
                    i++;
                    intervalxy += (float)velicina1 / 30;
                    if (intervalxy > velicina1)
                        break;
                }
            }
            for (int j = 0; j < 1; j++) {
                if (textures != NULL);
                SDL_DestroyTexture(textures);
            }
        }
        else if (index == 2) {
            while (1) {
                TrenutnoVreme = SDL_GetTicks();
                if (TrenutnoVreme - ProsloVreme >= interval) {
                    TrenutnoVreme1 = SDL_GetTicks() - PauzaVreme;
                    if (TrenutnoVreme1 - ProsloVreme1 >= interval1) {
                        ProsloVreme1 = TrenutnoVreme1;
                        sprintf_s(minut1, "%d", TrenutnoVreme1 / 60000);
                        sprintf_s(sekund, "%d", TrenutnoVreme1 / 1000 - (TrenutnoVreme1 / 60000) * 60);
                        strcat_s(minut1, ":");
                        if ((TrenutnoVreme1 / 1000 - (TrenutnoVreme1 / 60000) * 60) < 10)strcat_s(minut1, "0");
                        strcat_s(minut1, sekund);
                        strcpy(minut, minut1);
                    }
                    Animacije(surface, talon, *intervalx, 1);
                    *intervalx += 3;
                    if (*intervalx >= (12 * MaxWidth / 25 - velicina))
                        *intervalx = 0;
                    sprintf_s(adresa, "Slike/Igrac/Desno/%d/Walk/%d.png", Izgledi[1], i);
                    surfaces = (SDL_Surface*)IMG_Load(adresa);
                    textures = SDL_CreateTextureFromSurface(renderer, surfaces);
                    SDL_FreeSurface(surfaces);
                    NapraviZid(x1 + 0.1 * velicina1 + (int)intervalxy, y1 + 0.1 * velicina1, 0.8 * velicina1, 0.8 * velicina1, textures);
                    SDL_DestroyTexture(textures);
                    NapraviZid(mumije[0].xm + 0.1 * velicina1, mumije[0].ym + 0.1 * velicina1, 0.8 * velicina1, 0.8 * velicina1, (StranaMumije[0] == 0) ? mumijaleva : mumijadesna);
                    if (trbrojmumija > 1)
                        NapraviZid(mumije[1].xm + 0.1 * velicina1, mumije[1].ym + 0.1 * velicina1, 0.8 * velicina1, 0.8 * velicina1, (StranaMumije[1] == 0) ? mumijaleva : mumijadesna);
                    SDL_RenderPresent(renderer);
                    ProsloVreme = TrenutnoVreme;
                    i++;
                    intervalxy += (float)velicina1 / 30;
                    if (intervalxy > velicina1)
                        break;
                }
            }
            for (int j = 0; j < 1; j++) {
                if (textures != NULL);
                SDL_DestroyTexture(textures);
            }
        }
        else if (index == 3) {
            while (1) {
                TrenutnoVreme = SDL_GetTicks();
                if (TrenutnoVreme - ProsloVreme >= interval) {
                    TrenutnoVreme1 = SDL_GetTicks() - PauzaVreme;
                    if (TrenutnoVreme1 - ProsloVreme1 >= interval1) {
                        ProsloVreme1 = TrenutnoVreme1;
                        sprintf_s(minut1, "%d", TrenutnoVreme1 / 60000);
                        sprintf_s(sekund, "%d", TrenutnoVreme1 / 1000 - (TrenutnoVreme1 / 60000) * 60);
                        strcat_s(minut1, ":");
                        if ((TrenutnoVreme1 / 1000 - (TrenutnoVreme1 / 60000) * 60) < 10)strcat_s(minut1, "0");
                        strcat_s(minut1, sekund);
                        strcpy(minut, minut1);
                    }
                    Animacije(surface, talon, *intervalx, 1);
                    *intervalx += 3;
                    if (*intervalx >= (12 * MaxWidth / 25 - velicina))
                        *intervalx = 0;
                    sprintf_s(adresa, "Slike/Igrac/Levo/%d/Walk/%d.png", Izgledi[1], i);
                    surfaces = (SDL_Surface*)IMG_Load(adresa);
                    textures = SDL_CreateTextureFromSurface(renderer, surfaces);
                    SDL_FreeSurface(surfaces);
                    NapraviZid(x1 + 0.1 * velicina1 - (int)intervalxy, y1 + 0.1 * velicina1, 0.8 * velicina1, 0.8 * velicina1, textures);
                    SDL_DestroyTexture(textures);
                    NapraviZid(mumije[0].xm + 0.1 * velicina1, mumije[0].ym + 0.1 * velicina1, 0.8 * velicina1, 0.8 * velicina1, (StranaMumije[0] == 0) ? mumijaleva : mumijadesna);
                    if (trbrojmumija > 1)
                        NapraviZid(mumije[1].xm + 0.1 * velicina1, mumije[1].ym + 0.1 * velicina1, 0.8 * velicina1, 0.8 * velicina1, (StranaMumije[1] == 0) ? mumijaleva : mumijadesna);
                    SDL_RenderPresent(renderer);
                    ProsloVreme = TrenutnoVreme;
                    i++;
                    intervalxy += (float)velicina1 / 30;
                    if (intervalxy > velicina1)
                        break;
                }
            }
            for (int j = 0; j < 1; j++) {
                if (textures != NULL);
                SDL_DestroyTexture(textures);
            }
        }
        else if (index == 4) {
            while (1) {
                TrenutnoVreme = SDL_GetTicks();
                if (TrenutnoVreme - ProsloVreme >= interval) {
                    TrenutnoVreme1 = SDL_GetTicks() - PauzaVreme;
                    if (TrenutnoVreme1 - ProsloVreme1 >= interval1) {
                        ProsloVreme1 = TrenutnoVreme1;
                        sprintf_s(minut1, "%d", TrenutnoVreme1 / 60000);
                        sprintf_s(sekund, "%d", TrenutnoVreme1 / 1000 - (TrenutnoVreme1 / 60000) * 60);
                        strcat_s(minut1, ":");
                        if ((TrenutnoVreme1 / 1000 - (TrenutnoVreme1 / 60000) * 60) < 10)strcat_s(minut1, "0");
                        strcat_s(minut1, sekund);
                        strcpy(minut, minut1);
                    }
                    Animacije(surface, talon, *intervalx, 1, (ModIgre == 0 && PrviPut == 0) ? 1 : 0);
                    if (PrviPut == 0)
                        PrviPut = 1;
                    *intervalx += 3;
                    if (*intervalx >= (12 * MaxWidth / 25 - velicina))
                        *intervalx = 0;
                    sprintf_s(adresa, "Slike/Igrac/Napred/%d/Walk/%d.png", Izgledi[1], i);
                    surfaces = (SDL_Surface*)IMG_Load(adresa);
                    textures = SDL_CreateTextureFromSurface(renderer, surfaces);
                    SDL_FreeSurface(surfaces);
                    NapraviZid(x1 + 0.1 * velicina1, y1 + 0.1 * velicina1 - (int)intervalxy, 0.8 * velicina1, 0.8 * velicina1, textures);
                    SDL_DestroyTexture(textures);
                    NapraviZid(mumije[0].xm + 0.1 * velicina1, mumije[0].ym + 0.1 * velicina1, 0.8 * velicina1, 0.8 * velicina1, (StranaMumije[0] == 0) ? mumijaleva : mumijadesna);
                    if (trbrojmumija > 1)
                        NapraviZid(mumije[1].xm + 0.1 * velicina1, mumije[1].ym + 0.1 * velicina1, 0.8 * velicina1, 0.8 * velicina1, (StranaMumije[1] == 0) ? mumijaleva : mumijadesna);
                    SDL_RenderPresent(renderer);
                    ProsloVreme = TrenutnoVreme;
                    i++;
                    intervalxy += (float)velicina1 / 30;
                    if (intervalxy > velicina1)
                        break;
                }
            }
            for (int j = 0; j < 1; j++) {
                if (textures != NULL);
                SDL_DestroyTexture(textures);
            }
        }
        for (int j = 0; j < 1; j++) {
            if (textures != NULL);
            SDL_DestroyTexture(textures);
        }
    }
    else {
        SDL_Surface* surfaces[32];
        SDL_Texture* textures[32];
        int k = (mumije[0].xm == x1 && mumije[0].ym == y1) ? 1 : 0;
        if (index == 1) {
            for (int j = 0; j < 32; j++) {
                sprintf_s(adresa, "Slike/Mumija/Desno/%d/%d.png", Izgledi[0], j);
                surfaces[j] = (SDL_Surface*)IMG_Load(adresa);
                textures[j] = SDL_CreateTextureFromSurface(renderer, surfaces[j]);
                SDL_FreeSurface(surfaces[j]);
            }
            while (1) {
                TrenutnoVreme = SDL_GetTicks();
                if (TrenutnoVreme - ProsloVreme >= interval) {
                    TrenutnoVreme1 = SDL_GetTicks() - PauzaVreme;
                    if (TrenutnoVreme1 - ProsloVreme1 >= interval1) {
                        ProsloVreme1 = TrenutnoVreme1;
                        sprintf_s(minut1, "%d", TrenutnoVreme1 / 60000);
                        sprintf_s(sekund, "%d", TrenutnoVreme1 / 1000 - (TrenutnoVreme1 / 60000) * 60);
                        strcat_s(minut1, ":");
                        if ((TrenutnoVreme1 / 1000 - (TrenutnoVreme1 / 60000) * 60) < 10)strcat_s(minut1, "0");
                        strcat_s(minut1, sekund);
                        strcpy(minut, minut1);
                    }
                    Animacije(surface, talon, *intervalx, 1, (ModIgre == 0 && PrviPut == 0) ? 1 : 0);
                    if (PrviPut == 0)
                        PrviPut = 1;
                    *intervalx += 3;
                    if (*intervalx >= (12 * MaxWidth / 25 - velicina))
                        *intervalx = 0;
                    NapraviZid(IgracX + 0.1 * velicina1, IgracY + 0.1 * velicina1, 0.8 * velicina1, 0.8 * velicina1, Igrac);
                    if (trbrojmumija > 1)
                        NapraviZid(mumije[k].xm + 0.1 * velicina1, mumije[k].ym + 0.1 * velicina1, 0.8 * velicina1, 0.8 * velicina1, (StranaMumije[k] == 0) ? mumijaleva : mumijadesna);
                    NapraviZid(x1 + 0.1 * velicina1, y1 + 0.1 * velicina1 + (int)intervalxy, 0.8 * velicina1, 0.8 * velicina1, textures[i]);
                    SDL_RenderPresent(renderer);
                    ProsloVreme = TrenutnoVreme;
                    SDL_DestroyTexture(textures[i]);
                    i++;
                    intervalxy += (float)velicina1 / 32;
                    if (intervalxy > velicina1)
                        break;
                }
            }
        }
        else if (index == 2) {
            for (int j = 0; j < 32; j++) {
                sprintf_s(adresa, "Slike/Mumija/Desno/%d/%d.png", Izgledi[0], j);
                surfaces[j] = (SDL_Surface*)IMG_Load(adresa);
                textures[j] = SDL_CreateTextureFromSurface(renderer, surfaces[j]);
                SDL_FreeSurface(surfaces[j]);
            }
            while (1) {
                TrenutnoVreme = SDL_GetTicks();
                if (TrenutnoVreme - ProsloVreme >= interval) {
                    TrenutnoVreme1 = SDL_GetTicks() - PauzaVreme;
                    if (TrenutnoVreme1 - ProsloVreme1 >= interval1) {
                        ProsloVreme1 = TrenutnoVreme1;
                        sprintf_s(minut1, "%d", TrenutnoVreme1 / 60000);
                        sprintf_s(sekund, "%d", TrenutnoVreme1 / 1000 - (TrenutnoVreme1 / 60000) * 60);
                        strcat_s(minut1, ":");
                        if ((TrenutnoVreme1 / 1000 - (TrenutnoVreme1 / 60000) * 60) < 10)strcat_s(minut1, "0");
                        strcat_s(minut1, sekund);
                        strcpy(minut, minut1);
                    }
                    Animacije(surface, talon, *intervalx, 1, (ModIgre == 0 && PrviPut == 0) ? 1 : 0);
                    if (PrviPut == 0)
                        PrviPut = 1;
                    *intervalx += 3;
                    if (*intervalx >= (12 * MaxWidth / 25 - velicina))
                        *intervalx = 0;
                    NapraviZid(IgracX + 0.1 * velicina1, IgracY + 0.1 * velicina1, 0.8 * velicina1, 0.8 * velicina1, Igrac);
                    if (trbrojmumija > 1)
                        NapraviZid(mumije[k].xm + 0.1 * velicina1, mumije[k].ym + 0.1 * velicina1, 0.8 * velicina1, 0.8 * velicina1, (StranaMumije[k] == 0) ? mumijaleva : mumijadesna);
                    NapraviZid(x1 + 0.1 * velicina1 + (int)intervalxy, y1 + 0.1 * velicina1, 0.8 * velicina1, 0.8 * velicina1, textures[i]);
                    SDL_RenderPresent(renderer);
                    ProsloVreme = TrenutnoVreme;
                    SDL_DestroyTexture(textures[i]);
                    i++;
                    intervalxy += (float)velicina1 / 32;
                    if (intervalxy > velicina1)
                        break;
                }
            }
        }
        else if (index == 3) {
            for (int j = 0; j < 32; j++) {
                sprintf_s(adresa, "Slike/Mumija/Levo/%d/%d.png", Izgledi[0], j);
                surfaces[j] = (SDL_Surface*)IMG_Load(adresa);
                textures[j] = SDL_CreateTextureFromSurface(renderer, surfaces[j]);
                SDL_FreeSurface(surfaces[j]);
            }
            while (1) {
                TrenutnoVreme = SDL_GetTicks();
                if (TrenutnoVreme - ProsloVreme >= interval) {
                    TrenutnoVreme1 = SDL_GetTicks() - PauzaVreme;
                    if (TrenutnoVreme1 - ProsloVreme1 >= interval1) {
                        ProsloVreme1 = TrenutnoVreme1;
                        sprintf_s(minut1, "%d", TrenutnoVreme1 / 60000);
                        sprintf_s(sekund, "%d", TrenutnoVreme1 / 1000 - (TrenutnoVreme1 / 60000) * 60);
                        strcat_s(minut1, ":");
                        if ((TrenutnoVreme1 / 1000 - (TrenutnoVreme1 / 60000) * 60) < 10)strcat_s(minut1, "0");
                        strcat_s(minut1, sekund);
                        strcpy(minut, minut1);
                    }
                    Animacije(surface, talon, *intervalx, 1, (ModIgre == 0 && PrviPut == 0) ? 1 : 0);
                    if (PrviPut == 0)
                        PrviPut = 1;
                    *intervalx += 3;
                    if (*intervalx >= (12 * MaxWidth / 25 - velicina))
                        *intervalx = 0;
                    NapraviZid(IgracX + 0.1 * velicina1, IgracY + 0.1 * velicina1, 0.8 * velicina1, 0.8 * velicina1, Igrac);
                    if (trbrojmumija > 1)
                        NapraviZid(mumije[k].xm + 0.1 * velicina1, mumije[k].ym + 0.1 * velicina1, 0.8 * velicina1, 0.8 * velicina1, (StranaMumije[k] == 0) ? mumijaleva : mumijadesna);
                    NapraviZid(x1 + 0.1 * velicina1 - (int)intervalxy, y1 + 0.1 * velicina1, 0.8 * velicina1, 0.8 * velicina1, textures[i]);
                    SDL_RenderPresent(renderer);
                    ProsloVreme = TrenutnoVreme;
                    SDL_DestroyTexture(textures[i]);
                    i++;
                    intervalxy += (float)velicina1 / 32;
                    if (intervalxy > velicina1)
                        break;
                }
            }
        }
        else if (index == 4) {
            for (int j = 0; j < 32; j++) {
                sprintf_s(adresa, "Slike/Mumija/Levo/%d/%d.png", Izgledi[0], j);
                surfaces[j] = (SDL_Surface*)IMG_Load(adresa);
                textures[j] = SDL_CreateTextureFromSurface(renderer, surfaces[j]);
                SDL_FreeSurface(surfaces[j]);
            }
            while (1) {
                TrenutnoVreme = SDL_GetTicks();
                if (TrenutnoVreme - ProsloVreme >= interval) {
                    TrenutnoVreme1 = SDL_GetTicks() - PauzaVreme;
                    if (TrenutnoVreme1 - ProsloVreme1 >= interval1) {
                        ProsloVreme1 = TrenutnoVreme1;
                        sprintf_s(minut1, "%d", TrenutnoVreme1 / 60000);
                        sprintf_s(sekund, "%d", TrenutnoVreme1 / 1000 - (TrenutnoVreme1 / 60000) * 60);
                        strcat_s(minut1, ":");
                        if ((TrenutnoVreme1 / 1000 - (TrenutnoVreme1 / 60000) * 60) < 10)strcat_s(minut1, "0");
                        strcat_s(minut1, sekund);
                        strcpy(minut, minut1);
                    }
                    Animacije(surface, talon, *intervalx, 1, (ModIgre == 0 && PrviPut == 0) ? 1 : 0);
                    if (PrviPut == 0)
                        PrviPut = 1;
                    *intervalx += 3;
                    if (*intervalx >= (12 * MaxWidth / 25 - velicina))
                        *intervalx = 0;
                    NapraviZid(IgracX + 0.1 * velicina1, IgracY + 0.1 * velicina1, 0.8 * velicina1, 0.8 * velicina1, Igrac);
                    if (trbrojmumija > 1)
                        NapraviZid(mumije[k].xm + 0.1 * velicina1, mumije[k].ym + 0.1 * velicina1, 0.8 * velicina1, 0.8 * velicina1, (StranaMumije[k] == 0) ? mumijaleva : mumijadesna);
                    NapraviZid(x1 + 0.1 * velicina1, y1 + 0.1 * velicina1 - (int)intervalxy, 0.8 * velicina1, 0.8 * velicina1, textures[i]);
                    SDL_RenderPresent(renderer);
                    ProsloVreme = TrenutnoVreme;
                    SDL_DestroyTexture(textures[i]);
                    i++;
                    intervalxy += (float)velicina1 / 32;
                    if (intervalxy > velicina1)
                        break;
                }
            }
        }
        for (int j = 0; j < 32; j++) {
            if (textures[j] != NULL)
                SDL_DestroyTexture(textures[j]);
        }
    }
}
void CrtajOpcijeIgra(SDL_Surface** surface, int index, int LastIndex, int Xmis = 0) {
    int x[5], y[5], br, pravougaonik1Y, pravougaonik2Y;
    TTF_SetFontSize(font[0], 0.5 * velicina);
    TTF_SizeText(font[0], (const char*)"Izgled Mumije", &x[0], &y[0]);
    TTF_SizeText(font[0], (const char*)"Jacina zvuka efekata", &x[1], &y[1]);
    TTF_SetFontSize(font[1], 0.5 * velicina);
    TTF_SizeText(font[1], (const char*)"<", &x[4], &y[4]);
    if (index == 0 && LastIndex == -1) {
        float Puno, TrenutnaMuzika, TrenutniZvuk, Xmis1, Xmis2;
        char lokacija[3][80], zvuk[10];
        if (Xmis != -10) {
            TTF_SetFontSize(font[0], 0.5 * velicina);
            TTF_SizeText(font[0], (const char*)"Jacina zvuka efekata", &x[1], &y[1]);
            TTF_SizeText(font[0], (const char*)"Jacina muzike", &x[2], &y[2]);
            PisiTekst(font[0], { 70,60,30,255 }, { (int)(0.1 * MaxWidth), (int)(0.14 * MaxHeight), x[1],y[1] }, (char*)"Jacina zvuka efekata");
            PisiTekst(font[0], { 70,60,30,255 }, { (int)(0.1 * MaxWidth), (int)(0.27 * MaxHeight), x[2],y[2] }, (char*)"Jacina muzike");

            Puno = 0.7 * MaxWidth - x[1];
            TrenutnaMuzika = (int)(Puno - Pozadinska * Puno / 100);
            TrenutniZvuk = (int)(Puno - Zvukovi * Puno / 100);
            Xmis1 = 0.9 * MaxWidth - TrenutniZvuk;
            Xmis2 = 0.9 * MaxWidth - TrenutnaMuzika;
            SDL_SetRenderDrawColor(renderer, 213, 179, 109, 255);
            pravougaonik1Y = (int)(0.14 * MaxHeight + y[1] / 2 - 0.13 * velicina);
            pravougaonik2Y = (int)(0.27 * MaxHeight + y[1] / 2 - 0.13 * velicina);


            ObojiZid(Xmis1, (int)(0.14 * MaxHeight + y[1] / 2 - 0.13 * velicina), TrenutniZvuk, 0.27 * velicina + 1);
            SDL_RenderFillCircle(0.9 * MaxWidth, (int)(0.14 * MaxHeight + y[1] / 2), 0.13 * velicina);
            ObojiZid(Xmis2, (int)(0.27 * MaxHeight + y[1] / 2 - 0.13 * velicina), TrenutnaMuzika, 0.27 * velicina + 1);
            SDL_RenderFillCircle(0.9 * MaxWidth, (int)(0.27 * MaxHeight + y[1] / 2), 0.13 * velicina);
            SDL_SetRenderDrawColor(renderer, 57, 43, 33, 255);
            ObojiZid(0.2 * MaxWidth + x[1], (int)(0.14 * MaxHeight + y[1] / 2 - 0.13 * velicina), Puno - TrenutniZvuk, 0.27 * velicina + 1);
            ObojiZid(0.2 * MaxWidth + x[1], (int)(0.27 * MaxHeight + y[1] / 2 - 0.13 * velicina), Puno - TrenutnaMuzika, 0.27 * velicina + 1);


            SDL_RenderFillCircle(0.2 * MaxWidth + x[1], 0.14 * MaxHeight + y[1] / 2, 0.13 * velicina);
            SDL_RenderFillCircle(0.2 * MaxWidth + x[1], (int)(0.27 * MaxHeight + y[1] / 2), 0.13 * velicina);
            SDL_RenderFillCircle(Xmis1, (int)(0.14 * MaxHeight + y[1] / 2), 0.13 * velicina);
            SDL_RenderFillCircle(Xmis2, (int)(0.27 * MaxHeight + y[1] / 2), 0.13 * velicina);
            SDL_SetRenderDrawColor(renderer, 10, 0, 0, 255);
            SDL_RenderDrawCircle(Xmis1, (int)(0.14 * MaxHeight + y[1] / 2), 0.13 * velicina);
            SDL_RenderDrawCircle(Xmis2, (int)(0.27 * MaxHeight + y[1] / 2), 0.13 * velicina);
            TTF_SetFontSize(font[1], 0.26 * velicina);
            sprintf_s(zvuk, "%d", Zvukovi);
            strcat_s(zvuk, "%");
            TTF_SizeText(font[1], (const char*)zvuk, &x[2], &y[2]);
            PisiTekst(font[1], { 0,0,0,255 }, { (int)(0.9 * MaxWidth - x[2]), (int)(0.14 * MaxHeight + y[1] / 2 - y[2] / 2), x[2],y[2] }, (char*)zvuk);
            sprintf_s(zvuk, "%d", Pozadinska);
            strcat_s(zvuk, "%");
            TTF_SizeText(font[1], (const char*)zvuk, &x[2], &y[2]);
            PisiTekst(font[1], { 0,0,0,255 }, { (int)(0.9 * MaxWidth - x[2]), (int)(0.27 * MaxHeight + y[1] / 2 - y[2] / 2), x[2],y[2] }, (char*)zvuk);


            TTF_SetFontSize(font[1], 0.5 * velicina);
            TTF_SizeText(font[1], (const char*)"<", &x[4], &y[4]);

            TTF_SizeText(font[0], (const char*)"Izgled Mumije", &x[0], &y[0]);
            PisiTekst(font[0], { 70,60,30,255 }, { (int)(0.1 * MaxWidth), (int)(0.4 * MaxHeight), x[0],y[0] }, (char*)"Izgled Mumije");
            TTF_SizeText(font[0], (const char*)"Izgled Igraca", &x[2], &y[2]);
            PisiTekst(font[0], { 70,60,30,255 }, { (int)(0.1 * MaxWidth), (int)(0.53 * MaxHeight), x[1],y[1] }, (char*)"Izgled Igraca");
            TTF_SizeText(font[0], (const char*)"Izgled Zidova", &x[3], &y[3]);
            PisiTekst(font[0], { 70,60,30,255 }, { (int)(0.1 * MaxWidth), (int)(0.66 * MaxHeight), x[2],y[2] }, (char*)"Izgled Zidova");
        }
        sprintf_s(lokacija[0], "Slike/Mumija/Desno/%d/MumijaDesno.png", Izgledi[0]);
        sprintf_s(lokacija[1], "Slike/Igrac/Nazad/%d/walk/0.png", Izgledi[1]);
        sprintf_s(lokacija[2], "Slike/Zid/%d/Zidovi.png", Izgledi[2]);

        SDL_Surface* skinovi[3] = {
            (SDL_Surface*)IMG_Load(lokacija[0]),
            (SDL_Surface*)IMG_Load(lokacija[1]),
            (SDL_Surface*)IMG_Load(lokacija[2])
        };
        SDL_Texture* skin[3];

        TTF_SetFontSize(font[0], 0.5 * velicina);
        TTF_SizeText(font[0], (const char*)"Izgled Mumije", &x[0], &y[0]);
        TTF_SizeText(font[0], (const char*)"Jacina zvuka efekata", &x[1], &y[1]);
        TTF_SetFontSize(font[1], 0.5 * velicina);
        TTF_SizeText(font[1], (const char*)"<", &x[4], &y[4]);

        for (int i = 0; i < 3; i++) {
            skin[i] = SDL_CreateTextureFromSurface(renderer, skinovi[i]);
            SDL_FreeSurface(skinovi[i]);
            NapraviZid((int)(1.1 * MaxWidth - 2 * x[4] + x[1]) / 2, (int)(0.4 * MaxHeight + i * 0.13 * MaxHeight + y[0] / 2 - velicina * 0.4), velicina * 0.8, velicina * 0.8, skin[i]);
            PisiTekst(font[1], { 57,43,33,255 }, { (int)(0.9 * MaxWidth - x[4] / 2), (int)(0.4 * MaxHeight + i * 0.13 * MaxHeight + y[0] / 2 - y[4] / 2), x[4],y[4] }, (char*)">");
            PisiTekst(font[1], { 57,43,33,255 }, { (int)(0.2 * MaxWidth + x[1] - x[4] / 2), (int)(0.4 * MaxHeight + i * 0.13 * MaxHeight + y[0] / 2 - y[4] / 2), x[4],y[4] }, (char*)"<");
            SDL_DestroyTexture(skin[i]);
        }
        TTF_SizeText(font[0], (const char*)"Povratak na igru", &x[1], &y[1]);
        PisiTekst(font[0], { 70,60,30,255 }, { (int)(MaxWidth - x[1]) / 2, (int)(0.79 * MaxHeight), x[1],y[1] }, (char*)"Povratak na igru");
    }
    else if (index == 1) {
        TTF_SizeText(font[0], (const char*)"Povratak na igru", &x[1], &y[1]);
        PisiTekst(font[0], { 255,255,255,255 }, { (int)(MaxWidth - x[1]) / 2, (int)(0.79 * MaxHeight), x[1],y[1] }, (char*)"Povratak na igru");
    }
    else if (index == 2) {
        PisiTekst(font[1], { 255,255,255,255 }, { (int)(0.2 * MaxWidth + x[1] - x[4] / 2), (int)(0.4 * MaxHeight + y[0] / 2 - y[4] / 2), x[4],y[4] }, (char*)"<");
    }
    else if (index == 3) {
        PisiTekst(font[1], { 255,255,255,255 }, { (int)(0.9 * MaxWidth - x[4] / 2), (int)(0.4 * MaxHeight + y[0] / 2 - y[4] / 2), x[4],y[4] }, (char*)">");
    }
    else if (index == 4) {
        PisiTekst(font[1], { 255,255,255,255 }, { (int)(0.2 * MaxWidth + x[1] - x[4] / 2), (int)(0.53 * MaxHeight + y[0] / 2 - y[4] / 2), x[4],y[4] }, (char*)"<");
    }
    else if (index == 5) {
        PisiTekst(font[1], { 255,255,255,255 }, { (int)(0.9 * MaxWidth - x[4] / 2), (int)(0.53 * MaxHeight + y[0] / 2 - y[4] / 2), x[4],y[4] }, (char*)">");
    }
    else if (index == 6) {
        PisiTekst(font[1], { 255,255,255,255 }, { (int)(0.2 * MaxWidth + x[1] - x[4] / 2), (int)(0.66 * MaxHeight + y[0] / 2 - y[4] / 2), x[4],y[4] }, (char*)"<");
    }
    else if (index == 7) {
        PisiTekst(font[1], { 255,255,255,255 }, { (int)(0.9 * MaxWidth - x[4] / 2), (int)(0.66 * MaxHeight + y[0] / 2 - y[4] / 2), x[4],y[4] }, (char*)">");
    }
    else if (index == 8) {
        float Puno, Trenutno;
        char zvuk[10];
        TTF_SetFontSize(font[0], 0.5 * velicina);
        TTF_SizeText(font[0], (const char*)"Jacina zvuka efekata", &x[1], &y[1]);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        if (Xmis <= (int)(0.2 * MaxWidth + x[1]))
            Xmis = (int)(0.2 * MaxWidth + x[1] + 1);
        else if (Xmis > 0.9 * MaxWidth) {
            Xmis = 0.9 * MaxWidth;
        }
        Puno = 0.7 * MaxWidth - x[1];
        Trenutno = 0.9 * MaxWidth - Xmis;
        SDL_SetRenderDrawColor(renderer, 213, 179, 109, 255);
        ObojiZid(Xmis, (int)(0.14 * MaxHeight + y[1] / 2 - 0.13 * velicina), Trenutno, 0.27 * velicina + 1);
        SDL_RenderFillCircle(0.9 * MaxWidth, (int)(0.14 * MaxHeight + y[1] / 2), 0.13 * velicina);
        SDL_SetRenderDrawColor(renderer, 57, 43, 33, 255);
        ObojiZid(0.2 * MaxWidth + x[1], (int)(0.14 * MaxHeight + y[1] / 2 - 0.13 * velicina), Puno - Trenutno, 0.27 * velicina + 1);
        SDL_RenderFillCircle(0.2 * MaxWidth + x[1], (int)(0.14 * MaxHeight + y[1] / 2), 0.13 * velicina);
        SDL_RenderFillCircle(Xmis, (int)(0.14 * MaxHeight + y[1] / 2), 0.13 * velicina);
        SDL_SetRenderDrawColor(renderer, 10, 0, 0, 255);
        SDL_RenderDrawCircle(Xmis, (int)(0.14 * MaxHeight + y[1] / 2), 0.13 * velicina);
        Zvukovi = (int)(((Puno - Trenutno) / Puno) * 100);
        sprintf_s(zvuk, "%d", Zvukovi);
        strcat_s(zvuk, "%");
        TTF_SetFontSize(font[1], 0.26 * velicina);
        TTF_SizeText(font[1], (const char*)zvuk, &x[2], &y[2]);
        PisiTekst(font[1], { 0,0,0,255 }, { (int)(0.9 * MaxWidth - x[2]), (int)(0.14 * MaxHeight + y[1] / 2 - y[2] / 2), x[2],y[2] }, (char*)zvuk);
        for (int i = 0; i < 6; i++) {
            Mix_VolumeChunk(zvukovi[i], Zvukovi);
        }

    }
    else if (index == 9) {
        float Puno, Trenutno;
        char zvuk[10];
        TTF_SetFontSize(font[0], 0.5 * velicina);
        TTF_SizeText(font[0], (const char*)"Jacina zvuka efekata", &x[1], &y[1]);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        if (Xmis <= (int)(0.2 * MaxWidth + x[1]))
            Xmis = (int)(0.2 * MaxWidth + x[1] + 1);
        else if (Xmis > 0.9 * MaxWidth) {
            Xmis = 0.9 * MaxWidth;
        }
        Puno = 0.7 * MaxWidth - x[1];
        Trenutno = 0.9 * MaxWidth - Xmis;
        SDL_SetRenderDrawColor(renderer, 213, 179, 109, 255);
        ObojiZid(Xmis, (int)(0.27 * MaxHeight + y[1] / 2 - 0.13 * velicina), Trenutno, 0.27 * velicina + 1);
        SDL_RenderFillCircle(0.9 * MaxWidth, (int)(0.27 * MaxHeight + y[1] / 2), 0.13 * velicina);
        SDL_SetRenderDrawColor(renderer, 57, 43, 33, 255);
        ObojiZid(0.2 * MaxWidth + x[1], (int)(0.27 * MaxHeight + y[1] / 2 - 0.13 * velicina), Puno - Trenutno, 0.27 * velicina + 1);
        SDL_RenderFillCircle(0.2 * MaxWidth + x[1], (int)(0.27 * MaxHeight + y[1] / 2), 0.13 * velicina);
        SDL_RenderFillCircle(Xmis, (int)(0.27 * MaxHeight + y[1] / 2), 0.13 * velicina);
        SDL_SetRenderDrawColor(renderer, 10, 0, 0, 255);
        SDL_RenderDrawCircle(Xmis, (int)(0.27 * MaxHeight + y[1] / 2), 0.13 * velicina);
        Pozadinska = (int)(((Puno - Trenutno) / Puno) * 100);
        sprintf_s(zvuk, "%d", Pozadinska);
        strcat_s(zvuk, "%");
        TTF_SetFontSize(font[1], 0.26 * velicina);
        TTF_SizeText(font[1], (const char*)zvuk, &x[2], &y[2]);
        PisiTekst(font[1], { 0,0,0,255 }, { (int)(0.9 * MaxWidth - x[2]), (int)(0.27 * MaxHeight + y[1] / 2 - y[2] / 2), x[2], y[2] }, (char*)zvuk);
        Mix_VolumeMusic(Pozadinska);
    }
    else if (index == 10) {
        PisiTekst(font[1], { 57,43,33,255 }, { (int)(0.2 * MaxWidth + x[1] - x[4] / 2), (int)(0.4 * MaxHeight + y[0] / 2 - y[4] / 2), x[4],y[4] }, (char*)"<");
        PisiTekst(font[1], { 57,43,33,255 }, { (int)(0.9 * MaxWidth - x[4] / 2), (int)(0.53 * MaxHeight + y[0] / 2 - y[4] / 2), x[4],y[4] }, (char*)">");
        PisiTekst(font[1], { 57,43,33,255 }, { (int)(0.2 * MaxWidth + x[1] - x[4] / 2), (int)(0.53 * MaxHeight + y[0] / 2 - y[4] / 2), x[4],y[4] }, (char*)"<");
        PisiTekst(font[1], { 57,43,33,255 }, { (int)(0.9 * MaxWidth - x[4] / 2), (int)(0.4 * MaxHeight + y[0] / 2 - y[4] / 2), x[4],y[4] }, (char*)">");
        PisiTekst(font[1], { 57,43,33,255 }, { (int)(0.2 * MaxWidth + x[1] - x[4] / 2), (int)(0.66 * MaxHeight + y[0] / 2 - y[4] / 2), x[4],y[4] }, (char*)"<");
        PisiTekst(font[1], { 57,43,33,255 }, { (int)(0.9 * MaxWidth - x[4] / 2), (int)(0.66 * MaxHeight + y[0] / 2 - y[4] / 2), x[4],y[4] }, (char*)">");
        TTF_SizeText(font[0], (const char*)"Povratak na igru", &x[1], &y[1]);
        PisiTekst(font[0], { 70,60,30,255 }, { (int)(MaxWidth - x[1]) / 2, (int)(0.79 * MaxHeight), x[1],y[1] }, (char*)"Povratak na igru");
    }
}
void OpcijeIgra(SDL_Surface** surface) {
    int x[5], y[5], index = 0, intervalx = 0;
    Uint32 ProsloVreme = SDL_GetTicks(), interval = 1000 / 60, SadVreme, TrenutnoVreme;
    SDL_Surface* surface3 = IMG_Load("Slike/MeniProduzeni1.png");
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface3);
    SDL_FreeSurface(surface3);
    NapraviZid(0, 0, MaxWidth, MaxHeight, texture);
    CrtajOpcijeIgra(surface, index, -1);
    TTF_SetFontSize(font[0], 0.5 * velicina);
    TTF_SizeText(font[0], (const char*)"Povratak na igru", &x[1], &y[1]);
    TTF_SizeText(font[0], (const char*)"Jacina zvuka efekata", &x[2], &y[2]);
    TTF_SetFontSize(font[1], 0.5 * velicina);
    TTF_SizeText(font[1], (const char*)"<", &x[3], &y[3]);
    TTF_SizeText(font[0], (const char*)"Izgled Mumije", &x[4], &y[4]);
    SDL_RenderPresent(renderer);
    while (1) {
        TrenutnoVreme = SDL_GetTicks();
        if (TrenutnoVreme - ProsloVreme >= interval) {
            DeoTeksture(0 + intervalx, 0, MaxWidth, (int)(72.0 / 699.0 * MaxHeight), 0, 0, 1242, 72, texture);
            DeoTeksture(intervalx - MaxWidth, 0, MaxWidth, (int)(72.0 / 699.0 * MaxHeight), 0, 0, 1242, 72, texture);
            DeoTeksture(MaxWidth - intervalx, (int)(627.0 / 699.0 * MaxHeight), MaxWidth, (int)ceil(72.0 / 699.0 * MaxHeight), 0, 0, 1242, 72, texture);
            DeoTeksture(0 - intervalx, (int)(627.0 / 699.0 * MaxHeight), MaxWidth, (int)ceil(72.0 / 699.0 * MaxHeight), 0, 0, 1242, 72, texture);
            SDL_RenderPresent(renderer);
            ProsloVreme = TrenutnoVreme;
            intervalx += 3;
            if (intervalx >= MaxWidth)
                intervalx = 0;
        }
        while (SDL_PollEvent(&event) != 0) {
            SDL_GetMouseState(&x[0], &y[0]);
            if (event.type == SDL_QUIT) {
                SDL_Quit();
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN) {
                SDL_GetMouseState(&x[0], &y[0]);
                if (x[0] >= (int)(0.2 * MaxWidth + x[2] - 0.065 * velicina) && x[0] <= (int)(0.9 * MaxWidth + 0.065 * velicina) && ((y[0] >= (int)(0.14 * MaxHeight + y[2] / 2 - 0.13 * velicina) && y[0] <= (0.14 * MaxHeight + y[2] / 2 + 0.13 * velicina)) || (y[0] >= (int)(0.27 * MaxHeight + y[2] / 2 - 0.13 * velicina) && y[0] <= (0.27 * MaxHeight + y[2] / 2 + 0.13 * velicina)))) {
                    PustiZvuk(zvukovi[1]);
                    int LastIndex = (y[0] >= (int)(0.14 * MaxHeight + y[2] / 2 - 0.13 * velicina) && y[0] <= (0.14 * MaxHeight + y[2] / 2 + 0.13 * velicina)) ? 8 : 9;
                    while (event.type != SDL_MOUSEBUTTONUP) {
                        SDL_PollEvent(&event);
                        SDL_GetMouseState(&x[0], &y[0]);
                        CrtajOpcijeIgra(surface, LastIndex, 0, x[0]);
                        TrenutnoVreme = SDL_GetTicks();
                        if (TrenutnoVreme - ProsloVreme >= interval) {
                            DeoTeksture(0 + intervalx, 0, MaxWidth, (int)(72.0 / 699.0 * MaxHeight), 0, 0, 1242, 72, texture);
                            DeoTeksture(intervalx - MaxWidth, 0, MaxWidth, (int)(72.0 / 699.0 * MaxHeight), 0, 0, 1242, 72, texture);
                            DeoTeksture(MaxWidth - intervalx, (int)(627.0 / 699.0 * MaxHeight), MaxWidth, (int)ceil(72.0 / 699.0 * MaxHeight), 0, 0, 1242, 72, texture);
                            DeoTeksture(0 - intervalx, (int)(627.0 / 699.0 * MaxHeight), MaxWidth, (int)ceil(72.0 / 699.0 * MaxHeight), 0, 0, 1242, 72, texture);
                            SDL_RenderPresent(renderer);
                            ProsloVreme = TrenutnoVreme;
                            intervalx += 3;
                            if (intervalx >= MaxWidth)
                                intervalx = 0;
                        }
                        SDL_RenderPresent(renderer);
                    }
                    FILE* file = NULL;
                    fopen_s(&file, (const char*)"Opcije.txt", (const char*)"w");
                    fprintf_s(file, "%d %d\n%d\n%d %d %d\n", MaxWidth, MaxHeight, n, Tezina, Algoritam, BrZombija);
                    fprintf_s(file, "%d %d %d\n", Izgledi[0], Izgledi[1], Izgledi[2]);
                    fprintf_s(file, "%d %d %d\n", TrenutniNivo, mapa, Sacuvaj);
                    fprintf_s(file, "%d %d", Zvukovi, Pozadinska);
                    fclose(file);
                }
                else if (x[0] >= ((int)(MaxWidth - x[1]) / 2) && x[0] <= ((int)(MaxWidth + x[1]) / 2) && y[0] >= (0.79 * MaxHeight) && y[0] <= (0.79 * MaxHeight + y[1])) {
                    PustiZvuk(zvukovi[1]);
                    return;
                }
                else if ((x[0] >= (int)(0.2 * MaxWidth + x[2] - x[3] / 2) && x[0] <= (int)(0.2 * MaxWidth + x[2] + x[3] / 2) && y[0] >= (0.4 * MaxHeight + y[4] / 2 - y[3] / 2) && y[0] <= (0.4 * MaxHeight + y[4] / 2 + y[3] / 2)) || (x[0] >= (int)(0.9 * MaxWidth - x[3] / 2) && x[0] <= (int)(0.9 * MaxWidth + x[3] / 2) && y[0] >= (0.4 * MaxHeight + y[4] / 2 - y[3] / 2) && y[0] <= (0.4 * MaxHeight + y[4] / 2 + y[3] / 2))) {
                    PustiZvuk(zvukovi[1]);
                    Izgledi[0] = (Izgledi[0] == 0) ? 1 : 0;
                    int LastIndex = (x[0] >= (int)(0.2 * MaxWidth + x[2] - x[3] / 2) && x[0] <= (int)(0.2 * MaxWidth + x[2] + x[3] / 2)) ? (2 * (-1)) : 2;
                    int i = 0, intervalxy = (int)(0.35 * MaxWidth - 0.9 * velicina + 1.5 * x[3] - x[2] / 2) / 32;
                    double x1[2], y1[2], w1[2], h1[2];
                    char adresa[40];
                    SDL_Surface* surfaces[64];
                    SDL_Texture* textures[64];
                    for (int j = 0; j < 32; j++) {
                        TrenutnoVreme = SDL_GetTicks();
                        if (TrenutnoVreme - ProsloVreme >= interval) {
                            DeoTeksture(0 + intervalx, 0, MaxWidth, (int)(72.0 / 699.0 * MaxHeight), 0, 0, 1242, 72, texture);
                            DeoTeksture(intervalx - MaxWidth, 0, MaxWidth, (int)(72.0 / 699.0 * MaxHeight), 0, 0, 1242, 72, texture);
                            DeoTeksture(MaxWidth - intervalx, (int)(627.0 / 699.0 * MaxHeight), MaxWidth, (int)ceil(72.0 / 699.0 * MaxHeight), 0, 0, 1242, 72, texture);
                            DeoTeksture(0 - intervalx, (int)(627.0 / 699.0 * MaxHeight), MaxWidth, (int)ceil(72.0 / 699.0 * MaxHeight), 0, 0, 1242, 72, texture);
                            ProsloVreme = TrenutnoVreme;
                            intervalx += 3;
                            if (intervalx >= MaxWidth)
                                intervalx = 0;
                            SDL_RenderPresent(renderer);
                        }
                        if (LastIndex == -2)
                            sprintf_s(adresa, "Slike/Mumija/Levo/%d/%d.png", (Izgledi[0] == 0) ? 1 : 0, j);
                        else
                            sprintf_s(adresa, "Slike/Mumija/Desno/%d/%d.png", (Izgledi[0] == 0) ? 1 : 0, j);
                        surfaces[j] = (SDL_Surface*)IMG_Load(adresa);
                        textures[j] = SDL_CreateTextureFromSurface(renderer, surfaces[j]);
                        SDL_FreeSurface(surfaces[j]);
                        if (LastIndex == -2)
                            sprintf_s(adresa, "Slike/Mumija/Levo/%d/%d.png", Izgledi[0], j);
                        else
                            sprintf_s(adresa, "Slike/Mumija/Desno/%d/%d.png", Izgledi[0], j);
                        surfaces[j + 32] = (SDL_Surface*)IMG_Load(adresa);
                        textures[j + 32] = SDL_CreateTextureFromSurface(renderer, surfaces[j + 32]);
                        SDL_FreeSurface(surfaces[j + 32]);
                    }
                    x1[0] = (int)(0.2 * MaxWidth + x[2] + 0.4 * velicina);
                    y1[0] = (int)(0.4 * MaxHeight + y[4] / 2 - 0.4 * velicina);
                    w1[0] = (int)(0.9 * MaxWidth - 0.2 * velicina) - x1[0];
                    h1[0] = (int)(0.8 * velicina);
                    x1[1] = (int)((1242.0 / MaxWidth) * x1[0]);
                    y1[1] = (int)((699.0 / MaxHeight) * y1[0]);
                    w1[1] = (int)((1242.0 / MaxWidth) * w1[0]);
                    h1[1] = (int)((699.0 / MaxHeight) * h1[0]);
                    int channel = Mix_PlayChannel(-1, zvukovi[6], 0);
                    Mix_PlayChannelTimed(channel, zvukovi[6], 1, 1200);
                    while (1) {
                        TrenutnoVreme = SDL_GetTicks();
                        if (TrenutnoVreme - ProsloVreme >= interval) {
                            intervalx += 3;
                            DeoTeksture(0 + intervalx, 0, MaxWidth, (int)(72.0 / 699.0 * MaxHeight), 0, 0, 1242, 72, texture);
                            DeoTeksture(intervalx - MaxWidth, 0, MaxWidth, (int)(72.0 / 699.0 * MaxHeight), 0, 0, 1242, 72, texture);
                            DeoTeksture(MaxWidth - intervalx, (int)(627.0 / 699.0 * MaxHeight), MaxWidth, (int)ceil(72.0 / 699.0 * MaxHeight), 0, 0, 1242, 72, texture);
                            DeoTeksture(0 - intervalx, (int)(627.0 / 699.0 * MaxHeight), MaxWidth, (int)ceil(72.0 / 699.0 * MaxHeight), 0, 0, 1242, 72, texture);
                            DeoTeksture(x1[0], y1[0], w1[0], h1[0], x1[1], y1[1], w1[1], h1[1], texture);
                            if (i < 32)
                                NapraviZid((int)((1.1 * MaxWidth - 2 * x[3] + x[2]) / 2 + ((LastIndex == -2) ? (-1 * intervalxy) : (intervalxy))), (int)(0.4 * MaxHeight + y[4] / 2 - velicina * 0.4), velicina * 0.8, velicina * 0.8, textures[i]);
                            else
                                NapraviZid((int)(((LastIndex == -2) ? (0.9 * MaxWidth - 0.75 * velicina - intervalxy) : (0.2 * MaxWidth + x[2] + x[3] + intervalxy))), (int)(0.4 * MaxHeight + y[4] / 2 - velicina * 0.4), velicina * 0.8, velicina * 0.8, textures[i]);
                            SDL_DestroyTexture(textures[i]);
                            SDL_RenderPresent(renderer);
                            ProsloVreme = TrenutnoVreme;
                            i++;
                            intervalxy += (int)(0.35 * MaxWidth - 0.9 * velicina + 1.5 * x[3] - x[2] / 2) / 32;
                            if (intervalx >= MaxWidth)
                                intervalx = 0;
                            if (i == 32) intervalxy = (int)(0.35 * MaxWidth - 0.9 * velicina + 1.5 * x[3] - x[2] / 2) / 32;
                            if (i >= 64) {
                                break;
                            }
                        }
                    }
                    intervalx += 3;
                    DeoTeksture(x1[0], y1[0], w1[0], h1[0], x1[1], y1[1], w1[1], h1[1], texture);
                    for (int j = i; j < 64; j++) {
                        TrenutnoVreme = SDL_GetTicks();
                        if (TrenutnoVreme - ProsloVreme >= interval) {
                            DeoTeksture(0 + intervalx, 0, MaxWidth, (int)(72.0 / 699.0 * MaxHeight), 0, 0, 1242, 72, texture);
                            DeoTeksture(intervalx - MaxWidth, 0, MaxWidth, (int)(72.0 / 699.0 * MaxHeight), 0, 0, 1242, 72, texture);
                            DeoTeksture(MaxWidth - intervalx, (int)(627.0 / 699.0 * MaxHeight), MaxWidth, (int)ceil(72.0 / 699.0 * MaxHeight), 0, 0, 1242, 72, texture);
                            DeoTeksture(0 - intervalx, (int)(627.0 / 699.0 * MaxHeight), MaxWidth, (int)ceil(72.0 / 699.0 * MaxHeight), 0, 0, 1242, 72, texture);
                            ProsloVreme = TrenutnoVreme;
                            intervalx += 3;
                            if (intervalx >= MaxWidth)
                                intervalx = 0;
                            SDL_RenderPresent(renderer);
                        }
                        if (textures[j]) {
                            SDL_DestroyTexture(textures[j]);
                        }
                    }
                    CrtajOpcijeIgra(surface, 0, -1, -1);
                    TrenutnoVreme = SDL_GetTicks();
                    if (TrenutnoVreme - ProsloVreme >= interval) {
                        DeoTeksture(0 + intervalx, 0, MaxWidth, (int)(72.0 / 699.0 * MaxHeight), 0, 0, 1242, 72, texture);
                        DeoTeksture(intervalx - MaxWidth, 0, MaxWidth, (int)(72.0 / 699.0 * MaxHeight), 0, 0, 1242, 72, texture);
                        DeoTeksture(MaxWidth - intervalx, (int)(627.0 / 699.0 * MaxHeight), MaxWidth, (int)ceil(72.0 / 699.0 * MaxHeight), 0, 0, 1242, 72, texture);
                        DeoTeksture(0 - intervalx, (int)(627.0 / 699.0 * MaxHeight), MaxWidth, (int)ceil(72.0 / 699.0 * MaxHeight), 0, 0, 1242, 72, texture);
                        ProsloVreme = TrenutnoVreme;
                        intervalx += 3;
                        if (intervalx >= MaxWidth)
                            intervalx = 0;
                        SDL_RenderPresent(renderer);
                    }
                    else
                        SDL_RenderPresent(renderer);

                    sprintf_s(adresa, "Slike/Mumija/Desno/%d/MumijaDesno.png", Izgledi[0]);
                    SDL_Surface* mumijad = IMG_Load(adresa);
                    sprintf_s(adresa, "Slike/Mumija/Levo/%d/MumijaLevo.png", Izgledi[0]);
                    SDL_Surface* mumijal = IMG_Load(adresa);
                    SDL_DestroyTexture(mumijadesna);
                    SDL_DestroyTexture(mumijaleva);
                    mumijadesna = SDL_CreateTextureFromSurface(renderer, mumijad);
                    mumijaleva = SDL_CreateTextureFromSurface(renderer, mumijal);
                    SDL_FreeSurface(mumijad);
                    SDL_FreeSurface(mumijal);
                    FILE* file = NULL;
                    fopen_s(&file, (const char*)"Opcije.txt", (const char*)"w");
                    fprintf_s(file, "%d %d\n%d\n%d %d %d\n", MaxWidth, MaxHeight, n, Tezina, Algoritam, BrZombija);
                    fprintf_s(file, "%d %d %d\n", Izgledi[0], Izgledi[1], Izgledi[2]);
                    fprintf_s(file, "%d %d %d\n", TrenutniNivo, mapa, Sacuvaj);
                    fprintf_s(file, "%d %d", Zvukovi, Pozadinska);
                    fclose(file);
                }
                else if ((x[0] >= (int)(0.2 * MaxWidth + x[2] - x[3] / 2) && x[0] <= (int)(0.2 * MaxWidth + x[2] + x[3] / 2) && y[0] >= (0.53 * MaxHeight + y[4] / 2 - y[3] / 2) && y[0] <= (0.53 * MaxHeight + y[4] / 2 + y[3] / 2)) || (x[0] >= (int)(0.9 * MaxWidth - x[3] / 2) && x[0] <= (int)(0.9 * MaxWidth + x[3] / 2) && y[0] >= (0.53 * MaxHeight + y[4] / 2 - y[3] / 2) && y[0] <= (0.53 * MaxHeight + y[4] / 2 + y[3] / 2))) {
                    PustiZvuk(zvukovi[1]);
                    Izgledi[1] = (Izgledi[1] == 0) ? 1 : 0;
                    int LastIndex = (x[0] >= (int)(0.2 * MaxWidth + x[2] - x[3] / 2) && x[0] <= (int)(0.2 * MaxWidth + x[2] + x[3] / 2)) ? (2 * (-1)) : 2;
                    int i = 0, intervalxy = (int)(0.35 * MaxWidth - velicina + 1.5 * x[3] - x[2] / 2) / 30;
                    double x1[2], y1[2], w1[2], h1[2];
                    char adresa[40];
                    SDL_Surface* surfaces[60];
                    SDL_Texture* textures[60];
                    for (int j = 0; j < 30; j++) {
                        TrenutnoVreme = SDL_GetTicks();
                        if (TrenutnoVreme - ProsloVreme >= interval) {
                            DeoTeksture(0 + intervalx, 0, MaxWidth, (int)(72.0 / 699.0 * MaxHeight), 0, 0, 1242, 72, texture);
                            DeoTeksture(intervalx - MaxWidth, 0, MaxWidth, (int)(72.0 / 699.0 * MaxHeight), 0, 0, 1242, 72, texture);
                            DeoTeksture(MaxWidth - intervalx, (int)(627.0 / 699.0 * MaxHeight), MaxWidth, (int)ceil(72.0 / 699.0 * MaxHeight), 0, 0, 1242, 72, texture);
                            DeoTeksture(0 - intervalx, (int)(627.0 / 699.0 * MaxHeight), MaxWidth, (int)ceil(72.0 / 699.0 * MaxHeight), 0, 0, 1242, 72, texture);
                            ProsloVreme = TrenutnoVreme;
                            intervalx += 3;
                            if (intervalx >= MaxWidth)
                                intervalx = 0;
                            SDL_RenderPresent(renderer);
                        }
                        if (LastIndex == -2)
                            sprintf_s(adresa, "Slike/Igrac/Levo/%d/walk/%d.png", (Izgledi[1] == 0) ? 1 : 0, j);
                        else
                            sprintf_s(adresa, "Slike/Igrac/Desno/%d/walk/%d.png", (Izgledi[1] == 0) ? 1 : 0, j);
                        surfaces[j] = (SDL_Surface*)IMG_Load(adresa);
                        textures[j] = SDL_CreateTextureFromSurface(renderer, surfaces[j]);
                        SDL_FreeSurface(surfaces[j]);
                        if (LastIndex == -2)
                            sprintf_s(adresa, "Slike/Igrac/Levo/%d/walk/%d.png", Izgledi[1], j);
                        else
                            sprintf_s(adresa, "Slike/Igrac/Desno/%d/walk/%d.png", Izgledi[1], j);
                        surfaces[j + 30] = (SDL_Surface*)IMG_Load(adresa);
                        textures[j + 30] = SDL_CreateTextureFromSurface(renderer, surfaces[j + 30]);
                        SDL_FreeSurface(surfaces[j + 30]);
                    }
                    x1[0] = (int)(0.2 * MaxWidth + x[2] + 0.4 * velicina);
                    y1[0] = (int)(0.53 * MaxHeight + y[4] / 2 - 0.4 * velicina);
                    w1[0] = (int)(0.9 * MaxWidth - 0.2 * velicina) - x1[0];
                    h1[0] = (int)(0.8 * velicina);
                    x1[1] = (int)((1242.0 / MaxWidth) * x1[0]);
                    y1[1] = (int)((699.0 / MaxHeight) * y1[0]);
                    w1[1] = (int)((1242.0 / MaxWidth) * w1[0]);
                    h1[1] = (int)((699.0 / MaxHeight) * h1[0]);
                    int channel = Mix_PlayChannel(-1, zvukovi[6], 0);
                    Mix_PlayChannelTimed(channel, zvukovi[6], 1, 1200);
                    while (1) {
                        TrenutnoVreme = SDL_GetTicks();
                        if (TrenutnoVreme - ProsloVreme >= interval) {
                            intervalx += 3;
                            DeoTeksture(0 + intervalx, 0, MaxWidth, (int)(72.0 / 699.0 * MaxHeight), 0, 0, 1242, 72, texture);
                            DeoTeksture(intervalx - MaxWidth, 0, MaxWidth, (int)(72.0 / 699.0 * MaxHeight), 0, 0, 1242, 72, texture);
                            DeoTeksture(MaxWidth - intervalx, (int)(627.0 / 699.0 * MaxHeight), MaxWidth, (int)ceil(72.0 / 699.0 * MaxHeight), 0, 0, 1242, 72, texture);
                            DeoTeksture(0 - intervalx, (int)(627.0 / 699.0 * MaxHeight), MaxWidth, (int)ceil(72.0 / 699.0 * MaxHeight), 0, 0, 1242, 72, texture);
                            DeoTeksture(x1[0], y1[0], w1[0], h1[0], x1[1], y1[1], w1[1], h1[1], texture);
                            if (i < 30)
                                NapraviZid((int)((1.1 * MaxWidth - 2 * x[3] + x[2]) / 2 + ((LastIndex == -2) ? (-1 * intervalxy) : (intervalxy))), (int)(0.53 * MaxHeight + y[4] / 2 - velicina * 0.4), velicina * 0.8, velicina * 0.8, textures[i]);
                            else
                                NapraviZid((int)(((LastIndex == -2) ? (0.9 * MaxWidth - 0.8 * velicina - intervalxy) : (0.2 * MaxWidth + x[2] + x[3] + intervalxy))), (int)(0.53 * MaxHeight + y[4] / 2 - velicina * 0.4), velicina * 0.8, velicina * 0.8, textures[i]);
                            SDL_DestroyTexture(textures[i]);
                            SDL_RenderPresent(renderer);
                            ProsloVreme = TrenutnoVreme;
                            i++;
                            intervalxy += (int)(0.35 * MaxWidth - velicina + 1.5 * x[3] - x[2] / 2) / 30;
                            if (intervalx >= MaxWidth)
                                intervalx = 0;
                            if (i == 30) intervalxy = (int)(0.35 * MaxWidth - velicina + 1.5 * x[3] - x[2] / 2) / 30;
                            if (i >= 60) {
                                break;
                            }
                        }
                    }
                    intervalx += 3;
                    DeoTeksture(x1[0], y1[0], w1[0], h1[0], x1[1], y1[1], w1[1], h1[1], texture);
                    for (int j = i; j < 60; j++) {
                        TrenutnoVreme = SDL_GetTicks();
                        if (TrenutnoVreme - ProsloVreme >= interval) {
                            DeoTeksture(0 + intervalx, 0, MaxWidth, (int)(72.0 / 699.0 * MaxHeight), 0, 0, 1242, 72, texture);
                            DeoTeksture(intervalx - MaxWidth, 0, MaxWidth, (int)(72.0 / 699.0 * MaxHeight), 0, 0, 1242, 72, texture);
                            DeoTeksture(MaxWidth - intervalx, (int)(627.0 / 699.0 * MaxHeight), MaxWidth, (int)ceil(72.0 / 699.0 * MaxHeight), 0, 0, 1242, 72, texture);
                            DeoTeksture(0 - intervalx, (int)(627.0 / 699.0 * MaxHeight), MaxWidth, (int)ceil(72.0 / 699.0 * MaxHeight), 0, 0, 1242, 72, texture);
                            ProsloVreme = TrenutnoVreme;
                            intervalx += 3;
                            if (intervalx >= MaxWidth)
                                intervalx = 0;
                            SDL_RenderPresent(renderer);
                        }
                        if (textures[j]) {
                            SDL_DestroyTexture(textures[j]);
                        }
                    }
                    CrtajOpcijeIgra(surface, 0, -1, -1);
                    TrenutnoVreme = SDL_GetTicks();
                    if (TrenutnoVreme - ProsloVreme >= interval) {
                        DeoTeksture(0 + intervalx, 0, MaxWidth, (int)(72.0 / 699.0 * MaxHeight), 0, 0, 1242, 72, texture);
                        DeoTeksture(intervalx - MaxWidth, 0, MaxWidth, (int)(72.0 / 699.0 * MaxHeight), 0, 0, 1242, 72, texture);
                        DeoTeksture(MaxWidth - intervalx, (int)(627.0 / 699.0 * MaxHeight), MaxWidth, (int)ceil(72.0 / 699.0 * MaxHeight), 0, 0, 1242, 72, texture);
                        DeoTeksture(0 - intervalx, (int)(627.0 / 699.0 * MaxHeight), MaxWidth, (int)ceil(72.0 / 699.0 * MaxHeight), 0, 0, 1242, 72, texture);
                        ProsloVreme = TrenutnoVreme;
                        intervalx += 3;
                        if (intervalx >= MaxWidth)
                            intervalx = 0;
                        SDL_RenderPresent(renderer);
                    }
                    else
                        SDL_RenderPresent(renderer);
                    sprintf_s(adresa, "Slike/Igrac/Nazad/%d/walk/%d.png", Izgledi[1], 0);
                    SDL_Surface* igrac = IMG_Load(adresa);
                    SDL_DestroyTexture(Igrac);
                    Igrac = SDL_CreateTextureFromSurface(renderer, igrac);
                    SDL_FreeSurface(igrac);
                    FILE* file = NULL;
                    fopen_s(&file, (const char*)"Opcije.txt", (const char*)"w");
                    fprintf_s(file, "%d %d\n%d\n%d %d %d\n", MaxWidth, MaxHeight, n, Tezina, Algoritam, BrZombija);
                    fprintf_s(file, "%d %d %d\n", Izgledi[0], Izgledi[1], Izgledi[2]);
                    fprintf_s(file, "%d %d %d\n", TrenutniNivo, mapa, Sacuvaj);
                    fprintf_s(file, "%d %d", Zvukovi, Pozadinska);
                    fclose(file);
                }
                else if ((x[0] >= (int)(0.2 * MaxWidth + x[2] - x[3] / 2) && x[0] <= (int)(0.2 * MaxWidth + x[2] + x[3] / 2) && y[0] >= (0.66 * MaxHeight + y[4] / 2 - y[3] / 2) && y[0] <= (0.66 * MaxHeight + y[4] / 2 + y[3] / 2)) || (x[0] >= (int)(0.9 * MaxWidth - x[3] / 2) && x[0] <= (int)(0.9 * MaxWidth + x[3] / 2) && y[0] >= (0.66 * MaxHeight + y[4] / 2 - y[3] / 2) && y[0] <= (0.66 * MaxHeight + y[4] / 2 + y[3] / 2))) {
                    PustiZvuk(zvukovi[1]);
                    Izgledi[2] = (Izgledi[2] == 0) ? 1 : 0;
                    int LastIndex = (x[0] >= (int)(0.2 * MaxWidth + x[2] - x[3] / 2) && x[0] <= (int)(0.2 * MaxWidth + x[2] + x[3] / 2)) ? (2 * (-1)) : 2;
                    int i = 0, intervalxy = (int)(0.35 * MaxWidth - 1.1 * velicina + 1.5 * x[3] - x[2] / 2) / 32;
                    Uint32 ProsloVreme1 = SDL_GetTicks(), interval1 = 1000 / 90, TrenutnoVreme1;
                    double x1[2], y1[2], w1[2], h1[2];
                    char adresa[60];
                    SDL_Surface* surfaces[2];
                    SDL_Texture* textures[2];
                    for (int j = 0; j < 2; j++) {
                        TrenutnoVreme = SDL_GetTicks();
                        if (TrenutnoVreme - ProsloVreme >= interval) {
                            DeoTeksture(0 + intervalx, 0, MaxWidth, (int)(72.0 / 699.0 * MaxHeight), 0, 0, 1242, 72, texture);
                            DeoTeksture(intervalx - MaxWidth, 0, MaxWidth, (int)(72.0 / 699.0 * MaxHeight), 0, 0, 1242, 72, texture);
                            DeoTeksture(MaxWidth - intervalx, (int)(627.0 / 699.0 * MaxHeight), MaxWidth, (int)ceil(72.0 / 699.0 * MaxHeight), 0, 0, 1242, 72, texture);
                            DeoTeksture(0 - intervalx, (int)(627.0 / 699.0 * MaxHeight), MaxWidth, (int)ceil(72.0 / 699.0 * MaxHeight), 0, 0, 1242, 72, texture);
                            ProsloVreme = TrenutnoVreme;
                            intervalx += 3;
                            if (intervalx >= MaxWidth)
                                intervalx = 0;
                            SDL_RenderPresent(renderer);
                        }
                        if (j == 0)
                            sprintf_s(adresa, "Slike/Zid/%d/Zidovi.png", (Izgledi[2] == 0) ? 1 : 0);
                        else
                            sprintf_s(adresa, "Slike/Zid/%d/Zidovi.png", Izgledi[2]);
                        surfaces[j] = (SDL_Surface*)IMG_Load(adresa);
                        textures[j] = SDL_CreateTextureFromSurface(renderer, surfaces[j]);
                        SDL_FreeSurface(surfaces[j]);
                    }
                    x1[0] = (int)(0.2 * MaxWidth + x[2] + 0.3 * velicina);
                    y1[0] = (int)(0.66 * MaxHeight + y[4] / 2 - 0.4 * velicina);
                    w1[0] = (int)(0.9 * MaxWidth - 0.2 * velicina) - x1[0];
                    h1[0] = (int)(0.8 * velicina);
                    x1[1] = (int)((1242.0 / MaxWidth) * x1[0]);
                    y1[1] = (int)((699.0 / MaxHeight) * y1[0]);
                    w1[1] = (int)((1242.0 / MaxWidth) * w1[0]);
                    h1[1] = (int)((699.0 / MaxHeight) * h1[0]);
                    while (1) {
                        TrenutnoVreme1 = SDL_GetTicks();
                        TrenutnoVreme = SDL_GetTicks();
                        if (TrenutnoVreme - ProsloVreme >= interval) {
                            intervalx += 3;
                            if (intervalx >= MaxWidth)
                                intervalx = 0;
                            DeoTeksture(0 + intervalx, 0, MaxWidth, (int)(72.0 / 699.0 * MaxHeight), 0, 0, 1242, 72, texture);
                            DeoTeksture(intervalx - MaxWidth, 0, MaxWidth, (int)(72.0 / 699.0 * MaxHeight), 0, 0, 1242, 72, texture);
                            DeoTeksture(MaxWidth - intervalx, (int)(627.0 / 699.0 * MaxHeight), MaxWidth, (int)ceil(72.0 / 699.0 * MaxHeight), 0, 0, 1242, 72, texture);
                            DeoTeksture(0 - intervalx, (int)(627.0 / 699.0 * MaxHeight), MaxWidth, (int)ceil(72.0 / 699.0 * MaxHeight), 0, 0, 1242, 72, texture);
                            ProsloVreme = TrenutnoVreme;
                            SDL_RenderPresent(renderer);
                        }
                        if (TrenutnoVreme1 - ProsloVreme1 >= interval1) {
                            TrenutnoVreme = SDL_GetTicks();
                            if (TrenutnoVreme - ProsloVreme >= interval) {
                                intervalx += 3;
                                if (intervalx >= MaxWidth)
                                    intervalx = 0;
                                DeoTeksture(0 + intervalx, 0, MaxWidth, (int)(72.0 / 699.0 * MaxHeight), 0, 0, 1242, 72, texture);
                                DeoTeksture(intervalx - MaxWidth, 0, MaxWidth, (int)(72.0 / 699.0 * MaxHeight), 0, 0, 1242, 72, texture);
                                DeoTeksture(MaxWidth - intervalx, (int)(627.0 / 699.0 * MaxHeight), MaxWidth, (int)ceil(72.0 / 699.0 * MaxHeight), 0, 0, 1242, 72, texture);
                                DeoTeksture(0 - intervalx, (int)(627.0 / 699.0 * MaxHeight), MaxWidth, (int)ceil(72.0 / 699.0 * MaxHeight), 0, 0, 1242, 72, texture);
                            }
                            DeoTeksture(x1[0], y1[0], w1[0], h1[0], x1[1], y1[1], w1[1], h1[1], texture);
                            if (i <= 31)
                                NapraviZid((int)((1.1 * MaxWidth - 2 * x[3] + x[2]) / 2 + ((LastIndex == -2) ? (-1 * intervalxy) : (intervalxy))), (int)(0.66 * MaxHeight + y[4] / 2 - velicina * 0.4), velicina * 0.8, velicina * 0.8, textures[0]);
                            else
                                NapraviZid((int)(((LastIndex == -2) ? (0.9 * MaxWidth - velicina - intervalxy) : (0.2 * MaxWidth + x[2] + x[3] + intervalxy))), (int)(0.66 * MaxHeight + y[4] / 2 - velicina * 0.4), velicina * 0.8, velicina * 0.8, textures[1]);
                            SDL_RenderPresent(renderer);
                            ProsloVreme1 = TrenutnoVreme1;
                            i++;
                            intervalxy += (int)(0.35 * MaxWidth - 1.1 * velicina + 1.5 * x[3] - x[2] / 2) / 32;
                            if (i == 32) {
                                SDL_DestroyTexture(textures[0]);
                                intervalxy = (int)(0.35 * MaxWidth - 1.1 * velicina + 1.5 * x[3] - x[2] / 2) / 32;
                            }
                            if (i >= 66) {
                                SDL_DestroyTexture(textures[1]);
                                break;
                            }
                        }
                    }
                    intervalx += 3;
                    DeoTeksture(x1[0], y1[0], w1[0], h1[0], x1[1], y1[1], w1[1], h1[1], texture);
                    CrtajOpcijeIgra(surface, 0, -1, -1);
                    TrenutnoVreme = SDL_GetTicks();
                    if (TrenutnoVreme - ProsloVreme >= interval) {
                        DeoTeksture(0 + intervalx, 0, MaxWidth, (int)(72.0 / 699.0 * MaxHeight), 0, 0, 1242, 72, texture);
                        DeoTeksture(intervalx - MaxWidth, 0, MaxWidth, (int)(72.0 / 699.0 * MaxHeight), 0, 0, 1242, 72, texture);
                        DeoTeksture(MaxWidth - intervalx, (int)(627.0 / 699.0 * MaxHeight), MaxWidth, (int)ceil(72.0 / 699.0 * MaxHeight), 0, 0, 1242, 72, texture);
                        DeoTeksture(0 - intervalx, (int)(627.0 / 699.0 * MaxHeight), MaxWidth, (int)ceil(72.0 / 699.0 * MaxHeight), 0, 0, 1242, 72, texture);
                        ProsloVreme = TrenutnoVreme;
                        intervalx += 3;
                        if (intervalx >= MaxWidth)
                            intervalx = 0;
                        SDL_RenderPresent(renderer);
                    }
                    else
                        SDL_RenderPresent(renderer);
                    SDL_FreeSurface(surface[7]);
                    SDL_FreeSurface(surface[8]);
                    SDL_FreeSurface(surface[9]);
                    SDL_FreeSurface(surface[10]);
                    SDL_FreeSurface(surface[15]);
                    SDL_FreeSurface(surface[16]);
                    sprintf_s(adresa, "Slike/Zid/%d/LeviTalon.png", Izgledi[2]);
                    surface[7] = (SDL_Surface*)IMG_Load(adresa);
                    sprintf_s(adresa, "Slike/Zid/%d/DesniTalon.png", Izgledi[2]);
                    surface[8] = (SDL_Surface*)IMG_Load(adresa);
                    sprintf_s(adresa, "Slike/Zid/%d/GoreTalon.png", Izgledi[2]);
                    surface[9] = (SDL_Surface*)IMG_Load(adresa);
                    sprintf_s(adresa, "Slike/Zid/%d/DoleTalon.png", Izgledi[2]);
                    surface[10] = (SDL_Surface*)IMG_Load(adresa);
                    sprintf_s(adresa, "Slike/Zid/%d/VertikalniZid.png", Izgledi[2]);
                    surface[15] = (SDL_Surface*)IMG_Load(adresa);
                    sprintf_s(adresa, "Slike/Zid/%d/HorizontalniZid.png", Izgledi[2]);
                    surface[16] = (SDL_Surface*)IMG_Load(adresa);
                    FILE* file = NULL;
                    fopen_s(&file, (const char*)"Opcije.txt", (const char*)"w");
                    fprintf_s(file, "%d %d\n%d\n%d %d %d\n", MaxWidth, MaxHeight, n, Tezina, Algoritam, BrZombija);
                    fprintf_s(file, "%d %d %d\n", Izgledi[0], Izgledi[1], Izgledi[2]);
                    fprintf_s(file, "%d %d %d\n", TrenutniNivo, mapa, Sacuvaj);
                    fprintf_s(file, "%d %d", Zvukovi, Pozadinska);
                    fclose(file);
                }
            }
            else {
                if (x[0] >= ((int)(MaxWidth - x[1]) / 2) && x[0] <= ((int)(MaxWidth + x[1]) / 2) && y[0] >= (0.79 * MaxHeight) && y[0] <= (0.79 * MaxHeight + y[1])) {
                    index = 1;
                    CrtajOpcijeIgra(surface, index, 0);
                    SDL_RenderPresent(renderer);
                }
                else if (x[0] >= (int)(0.2 * MaxWidth + x[2] - x[3] / 2) && x[0] <= (int)(0.2 * MaxWidth + x[2] + x[3] / 2) && y[0] >= (0.4 * MaxHeight + y[4] / 2 - y[3] / 2) && y[0] <= (0.4 * MaxHeight + y[4] / 2 + y[3] / 2)) {
                    index = 2;
                    CrtajOpcijeIgra(surface, index, 0);
                    SDL_RenderPresent(renderer);
                }
                else if (x[0] >= (int)(0.9 * MaxWidth - x[3] / 2) && x[0] <= (int)(0.9 * MaxWidth + x[3] / 2) && y[0] >= (0.4 * MaxHeight + y[4] / 2 - y[3] / 2) && y[0] <= (0.4 * MaxHeight + y[4] / 2 + y[3] / 2)) {
                    index = 3;
                    CrtajOpcijeIgra(surface, index, 0);
                    SDL_RenderPresent(renderer);
                }
                else if (x[0] >= (int)(0.2 * MaxWidth + x[2] - x[3] / 2) && x[0] <= (int)(0.2 * MaxWidth + x[2] + x[3] / 2) && y[0] >= (0.53 * MaxHeight + y[4] / 2 - y[3] / 2) && y[0] <= (0.53 * MaxHeight + y[4] / 2 + y[3] / 2)) {
                    index = 4;
                    CrtajOpcijeIgra(surface, index, 0);
                    SDL_RenderPresent(renderer);
                }
                else if (x[0] >= (int)(0.9 * MaxWidth - x[3] / 2) && x[0] <= (int)(0.9 * MaxWidth + x[3] / 2) && y[0] >= (0.53 * MaxHeight + y[4] / 2 - y[3] / 2) && y[0] <= (0.53 * MaxHeight + y[4] / 2 + y[3] / 2)) {
                    index = 5;
                    CrtajOpcijeIgra(surface, index, 0);
                    SDL_RenderPresent(renderer);
                }
                else if (x[0] >= (int)(0.2 * MaxWidth + x[2] - x[3] / 2) && x[0] <= (int)(0.2 * MaxWidth + x[2] + x[3] / 2) && y[0] >= (0.66 * MaxHeight + y[4] / 2 - y[3] / 2) && y[0] <= (0.66 * MaxHeight + y[4] / 2 + y[3] / 2)) {
                    index = 6;
                    CrtajOpcijeIgra(surface, index, 0);
                    SDL_RenderPresent(renderer);
                }
                else if (x[0] >= (int)(0.9 * MaxWidth - x[3] / 2) && x[0] <= (int)(0.9 * MaxWidth + x[3] / 2) && y[0] >= (0.66 * MaxHeight + y[4] / 2 - y[3] / 2) && y[0] <= (0.66 * MaxHeight + y[4] / 2 + y[3] / 2)) {
                    index = 7;
                    CrtajOpcijeIgra(surface, index, 0);
                    SDL_RenderPresent(renderer);
                }
                else if (index < 10 && index > 0) {
                    index = 10;
                    CrtajOpcijeIgra(surface, index, 0);
                    SDL_RenderPresent(renderer);
                }
            }
        }
    }
    SDL_DestroyTexture(texture);

}
void Animacije(SDL_Surface** surface, struct Maze* talon, int intervalx, int index, int index1) {
    int velicina = Velicina1(), velicina1 = Velicina(), x, y;
    int Pocetni;
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface[3]);
    if (index1 != 0) {
        DeoTeksture(0, (int)(72.0 / 699.0 * MaxHeight), 12 * MaxWidth / 25 - velicina, MaxHeight - 2 * (int)(72.0 / 699.0 * MaxHeight), 0, 72, 564, 555, texture);
        int w[10], h[10];
        TTF_SetFontSize(font[0], 0.5 * velicina);
        if (PrviPut == 0) {
            TTF_SizeText(font[0], (const char*)"Nova Igra", &w[0], &h[0]);
            TTF_SizeText(font[0], (const char*)"Sacuvaj Igru", &w[1], &h[1]);
            TTF_SizeText(font[0], (const char*)"Pomoc", &w[2], &h[2]);
        }
        TTF_SizeText(font[0], (const char*)"Opcije", &w[3], &h[3]);
        TTF_SizeText(font[0], (const char*)"Skorovi", &w[4], &h[4]);
        TTF_SizeText(font[0], (const char*)"Glavni Meni", &w[5], &h[5]);
        if (PrviPut == 0) {
            PisiTekst(font[0], { 60,42,5,255 }, { (int)(12 * MaxWidth / 25 - velicina - w[0]) / 2, (int)(0.22 * MaxHeight), (int)(13 * MaxWidth / 25 - 0.9 * velicina), (int)(velicina) }, (char*)"Nova igra");
            PisiTekst(font[0], { 60,42,5,255 }, { (int)(12 * MaxWidth / 25 - velicina - w[1]) / 2, (int)(0.32 * MaxHeight), (int)(13 * MaxWidth / 25 - 0.9 * velicina), (int)(velicina) }, (char*)"Sacuvaj igru");
            PisiTekst(font[0], { 60,42,5,255 }, { (int)(12 * MaxWidth / 25 - velicina - w[2]) / 2, (int)(0.42 * MaxHeight), (int)(13 * MaxWidth / 25 - 0.9 * velicina), (int)(velicina) }, (char*)"Pomoc");
        }
        PisiTekst(font[0], { 60,42,5,255 }, { (int)(12 * MaxWidth / 25 - velicina - w[3]) / 2, (int)(0.52 * MaxHeight), (int)(13 * MaxWidth / 25 - 0.9 * velicina), (int)(velicina) }, (char*)"Opcije");
        PisiTekst(font[0], { 60,42,5,255 }, { (int)(12 * MaxWidth / 25 - velicina - w[4]) / 2, (int)(0.62 * MaxHeight), (int)(13 * MaxWidth / 25 - 0.9 * velicina), (int)(velicina) }, (char*)"Skorovi");
        PisiTekst(font[0], { 60,42,5,255 }, { (int)(12 * MaxWidth / 25 - velicina - w[5]) / 2, (int)(0.72 * MaxHeight), (int)(13 * MaxWidth / 25 - 0.9 * velicina), (int)(velicina) }, (char*)"Glavni Meni");
    }
    DeoTeksture(0 + intervalx, 0, 0.48 * MaxWidth - velicina, (int)(72.0 / 699.0 * MaxHeight), 0, 0, 564, 72, texture);
    DeoTeksture(0 - 0.48 * MaxWidth + velicina + intervalx, 0, 0.48 * MaxWidth - velicina, (int)(72.0 / 699.0 * MaxHeight), 0, 0, 564, 72, texture);
    DeoTeksture(0.48 * MaxWidth - velicina - intervalx, (int)(627.0 / 699.0 * MaxHeight), 0.48 * MaxWidth - velicina, (int)ceil(72.0 / 699.0 * MaxHeight), 0, 627, 564, 72, texture);
    DeoTeksture(0 - intervalx, (int)(627.0 / 699.0 * MaxHeight), 0.48 * MaxWidth - velicina, (int)ceil(72.0 / 699.0 * MaxHeight), 0, 627, 564, 72, texture);
    SDL_Texture* texture1 = NULL;
    if (n == 6)
        texture1 = SDL_CreateTextureFromSurface(renderer, surface[0]);
    else if (n == 7)
        texture1 = SDL_CreateTextureFromSurface(renderer, surface[1]);
    else if (n == 8)
        texture1 = SDL_CreateTextureFromSurface(renderer, surface[2]);
    SDL_Texture* texture2 = SDL_CreateTextureFromSurface(renderer, surface[7]);
    SDL_Texture* texture4 = SDL_CreateTextureFromSurface(renderer, surface[9]);
    SDL_Texture* texture5 = SDL_CreateTextureFromSurface(renderer, surface[10]);
    if (index == 0 && index1 == 1)
        DeoTeksture(12 * MaxWidth / 25, (int)(627.0 / 699.0 * MaxHeight), velicina * 6, (int)ceil(72.0 / 699.0 * MaxHeight) - 0.55 * velicina + ((MaxWidth != 1366) ? 1 : 3), 0, 1155, 1200, 45, texture1);
    else
        NapraviZid(12 * MaxWidth / 25, 2 * MaxHeight / 15, velicina * 6, velicina * 6, texture1);
    if (index == 0 && index1 == 1) {
        DeoTeksture(12 * MaxWidth / 25 - velicina, 2 * MaxHeight / 15 - velicina - 2, velicina, velicina, 0, 0, 183, 1.0 / 6 * 1219, texture2);
        DeoTeksture(12 * MaxWidth / 25 - velicina, (int)(627.0 / 699.0 * MaxHeight), velicina, (MaxWidth != 1366) ? ((int)ceil(72.0 / 699.0 * MaxHeight) - 0.55 * velicina + 1) : ((int)ceil(72.0 / 699.0 * MaxHeight) - 0.5 * velicina + 1), 0, 1219 - ((int)ceil(72.0 / 699.0 * MaxHeight) - 0.55 * velicina) * 1219.0 / MaxHeight, 183, ((int)ceil(72.0 / 699.0 * MaxHeight) - 0.55 * velicina) * 1219.0 / MaxHeight, texture2);
    }
    else {
        SDL_Texture* texture4 = SDL_CreateTextureFromSurface(renderer, surface[8]);
        NapraviZid(12 * MaxWidth / 25 - velicina, 2 * MaxHeight / 15, velicina, velicina * 6, texture2);
        NapraviZid(12 * MaxWidth / 25 + velicina * 6, 2 * MaxHeight / 15, velicina, 6 * velicina, texture4);
        SDL_DestroyTexture(texture4);
    }
    NapraviZid(12 * MaxWidth / 25 - velicina, 2 * MaxHeight / 15 - velicina - 2, velicina * 8, velicina + 2, texture4);
    SDL_DestroyTexture(texture1);
    SDL_DestroyTexture(texture2);
    SDL_DestroyTexture(texture4);
    texture1 = SDL_CreateTextureFromSurface(renderer, surface[15]);
    texture2 = SDL_CreateTextureFromSurface(renderer, surface[16]);
    Pocetni = (index == 0 && index1 == 1) ? (2 * n - 2) : 0;
    for (int i = Pocetni; i < 2 * n - 1; i++) {
        for (int j = 0; j < 2 * n - 1; j++) {
            if ((i % 2 == 0) && (j % 2 == 1) && (talon[i * (2 * n - 1) + j].matrica == 1)) NapraviZid(12 * MaxWidth / 25 + 9 * velicina1 / 10 + velicina1 * (j / 2), 2 * MaxHeight / 15 + velicina1 * (i / 2) - 11 * velicina1 / 100, velicina1 / 5, 6 * velicina1 / 5, texture1);
            else if ((i % 2 == 1) && (j % 2 == 0) && (talon[i * (2 * n - 1) + j].matrica == 1)) NapraviZid(12 * MaxWidth / 25 + velicina1 * (j / 2) - 20 * velicina1 / 192, 2 * MaxHeight / 15 + 9 * velicina1 / 10 + velicina1 * (i / 2) - velicina1 * 0.005, 1.201 * velicina1, velicina1 / 5, texture2);
        }
    }
    SDL_DestroyTexture(texture1);
    SDL_DestroyTexture(texture2);
    NapraviZid(12 * MaxWidth / 25 - velicina, 2 * MaxHeight / 15 + velicina * 6, velicina * 8, 3 * velicina / 5, texture5);
    if (index == 0 && index1 == 1) {
        if (IzlazX == 0 && IzlazY >= 0) {
            texture1 = SDL_CreateTextureFromSurface(renderer, surface[11]);
            NapraviZid(12 * MaxWidth / 25 + velicina1 / 8 + velicina1 * ((IzlazY) / 2), 2 * MaxHeight / 15 + velicina * ((IzlazX) / 2) - 11 * velicina / 10, 3 * velicina1 / 4, 11 * velicina / 10, texture1);
        }
        else if (IzlazX == (2 * n - 2) && IzlazY >= 0) {
            texture1 = SDL_CreateTextureFromSurface(renderer, surface[12]);
            NapraviZid(12 * MaxWidth / 25 + velicina1 / 8 + velicina1 * (IzlazY / 2), 2 * MaxHeight / 15 + velicina * 6, 3 * velicina1 / 4, 3 * velicina / 5, texture1);
        }
    }
    else {
        if (IzlazX == 0 && IzlazY >= 0) {
            texture1 = SDL_CreateTextureFromSurface(renderer, surface[11]);
            NapraviZid(12 * MaxWidth / 25 + velicina1 / 8 + velicina1 * ((IzlazY) / 2), 2 * MaxHeight / 15 + velicina * ((IzlazX) / 2) - 11 * velicina / 10, 3 * velicina1 / 4, 11 * velicina / 10, texture1);
        }
        else if (IzlazX == (2 * n - 2) && IzlazY >= 0) {
            texture1 = SDL_CreateTextureFromSurface(renderer, surface[12]);
            NapraviZid(12 * MaxWidth / 25 + velicina1 / 8 + velicina1 * (IzlazY / 2), 2 * MaxHeight / 15 + velicina * 6, 3 * velicina1 / 4, 3 * velicina / 5, texture1);
        }
        else if (IzlazY == 0 && IzlazX >= 0) {
            texture1 = SDL_CreateTextureFromSurface(renderer, surface[13]);
            NapraviZid(12 * MaxWidth / 25 - velicina, 2 * MaxHeight / 15 + velicina1 * (IzlazX / 2) + velicina1 / 8, velicina, 3 * velicina1 / 4, texture1);
        }
        else if (IzlazY == (2 * n - 2) && IzlazX >= 0) {
            texture1 = SDL_CreateTextureFromSurface(renderer, surface[14]);
            NapraviZid(12 * MaxWidth / 25 + velicina * 6, 2 * MaxHeight / 15 + velicina1 * (IzlazX / 2) + velicina1 / 8, velicina, 3 * velicina1 / 4, texture1);
        }
    }
    SDL_DestroyTexture(texture1);
    SDL_DestroyTexture(texture5);
    SDL_DestroyTexture(texture);
    TTF_SetFontSize(font[1], 6 * velicina / 10);
    PisiTekst(font[1], { 0,0,0, 255 }, { (int)(MaxWidth - 1.25 * velicina), 0,(int)(velicina * 1.5),(int)(velicina * 0.5) }, (char*)minut);
    if (index == 0 && ModIgre != 0) {
        NapraviZid(IgracX + 0.1 * velicina1, IgracY + 0.1 * velicina1, 0.8 * velicina1, 0.8 * velicina1, Igrac);
        for (int i = 0; i < trbrojmumija; i++)
            NapraviZid(mumije[i].xm + velicina1 * 0.1, mumije[i].ym + velicina1 * 0.1, 0.8 * velicina1, 0.8 * velicina1, (StranaMumije[i] == 0) ? mumijaleva : mumijadesna);
    }
}
void NovaIgra(SDL_Surface** surface) {
    int w[5], h[5], x, y, index = 0, interval = 1000 / 90, smer = 0;
    float pomerix1 = 0.1 * MaxWidth, pomerix = 0.45 * MaxWidth;
    Uint32 TrenutnoVreme, ProsloVreme = SDL_GetTicks();
    SDL_Surface* pozadina = IMG_Load("Slike/PobedaPozadina.png");
    SDL_Texture* Pozadina = SDL_CreateTextureFromSurface(renderer, pozadina);
    SDL_FreeSurface(pozadina);
    NapraviZid(0, 0, MaxWidth, MaxHeight, Pozadina);
    SDL_Surface* upitnik = IMG_Load("Slike/Pobeda.png");
    SDL_Texture* Upitnik = SDL_CreateTextureFromSurface(renderer, upitnik);
    SDL_FreeSurface(upitnik);
    SDL_Surface* trepce[2] = {
        (SDL_Surface*)IMG_Load("Slike/Igrac/Nazad/1/Prica/0.png"),
        (SDL_Surface*)IMG_Load("Slike/Igrac/Nazad/1/Prica/29.png")
    };
    SDL_Texture* Trepce[2] = {
        SDL_CreateTextureFromSurface(renderer,trepce[0]),
        SDL_CreateTextureFromSurface(renderer,trepce[1])
    };
    SDL_FreeSurface(trepce[0]);
    SDL_FreeSurface(trepce[1]);
    TTF_SetFontSize(font[0], 0.38 * velicina);
    TTF_SizeText(font[0], (const char*)"Ovaj nivo ce se trajno izgubiti.", &w[1], &h[1]);
    TTF_SizeText(font[0], (const char*)"Da li zelite da nastavite?", &w[2], &h[2]);
    TTF_SetFontSize(font[0], 0.6 * velicina);
    TTF_SizeText(font[0], (const char*)"Da", &w[3], &h[3]);
    TTF_SizeText(font[0], (const char*)"Ne", &w[4], &h[4]);
    TTF_SetFontSize(font[0], 0.38 * velicina);
    PisiTekst(font[0], { 100,80,40,255 }, { (int)(MaxWidth - w[1]) / 2, (int)(0.22 * MaxHeight), w[1],w[1] }, (char*)"Ovaj nivo ce se trajno izgubiti.");
    PisiTekst(font[0], { 100,80,40,255 }, { (int)(MaxWidth - w[2]) / 2, (int)(0.28 * MaxHeight), w[2],w[2] }, (char*)"Da li zelite da nastavite?");
    TTF_SetFontSize(font[0], 0.6 * velicina);
    PisiTekst(font[0], { 100,80,40,255 }, { (int)(0.85 * MaxWidth - w[3]) / 2, (int)(0.44 * MaxHeight), w[3],w[3] }, (char*)"Da");
    PisiTekst(font[0], { 100,80,40,255 }, { (int)(1.15 * MaxWidth - w[4]) / 2, (int)(0.44 * MaxHeight), w[4],w[4] }, (char*)"Ne");
    NapraviZid((MaxWidth - 2 * Velicina1()) / 2, 0.56 * MaxHeight, 2 * Velicina1(), 2 * Velicina1(), Trepce[0]);
    SDL_RenderPresent(renderer);
    while (1) {
        TrenutnoVreme = SDL_GetTicks();
        if (TrenutnoVreme - ProsloVreme >= interval && smer == 0) {
            if (smer == 0) {
                DeoTeksture(0.1 * MaxWidth, 0.05 * MaxHeight, 0.8 * MaxWidth, 0.9 * MaxHeight, 0.1 * 1920., 0.05 * 1080.0, 0.8 * 1920, 0.9 * 1080, Pozadina);
                NapraviZid(pomerix, 0.05 * MaxHeight, pomerix1, 0.9 * MaxHeight, Upitnik);
                SDL_RenderPresent(renderer);
                pomerix -= 0.4 * MaxWidth / 90;
                pomerix1 += 0.8 * MaxWidth / 90;
                if (pomerix <= 0.05 * MaxWidth) {
                    smer = 1;
                    NapraviZid(0, 0, MaxWidth, MaxHeight, Pozadina);
                    NapraviZid(0.05 * MaxWidth, 0.05 * MaxHeight, 0.9 * MaxWidth, 0.9 * MaxHeight, Upitnik);
                    TTF_SetFontSize(font[0], 0.38 * velicina);
                    PisiTekst(font[0], { 100,80,40,255 }, { (int)(MaxWidth - w[1]) / 2, (int)(0.22 * MaxHeight), w[1],w[1] }, (char*)"Ovaj nivo ce se trajno izgubiti.");
                    PisiTekst(font[0], { 100,80,40,255 }, { (int)(MaxWidth - w[2]) / 2, (int)(0.28 * MaxHeight), w[2],w[2] }, (char*)"Da li zelite da nastavite?");
                    TTF_SetFontSize(font[0], 0.6 * velicina);
                    PisiTekst(font[0], { 100,80,40,255 }, { (int)(0.85 * MaxWidth - w[3]) / 2, (int)(0.44 * MaxHeight), w[3],w[3] }, (char*)"Da");
                    PisiTekst(font[0], { 100,80,40,255 }, { (int)(1.15 * MaxWidth - w[4]) / 2, (int)(0.44 * MaxHeight), w[4],w[4] }, (char*)"Ne");
                    NapraviZid((MaxWidth - 2 * Velicina1()) / 2, 0.56 * MaxHeight, 2 * Velicina1(), 2 * Velicina1(), Trepce[0]);
                    SDL_RenderPresent(renderer);
                }
            }
            ProsloVreme = TrenutnoVreme;
        }
        while (SDL_PollEvent(&event) != 0) {
            SDL_GetMouseState(&x, &y);
            if (event.type == SDL_QUIT) {
                SDL_Quit();
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN && smer == 1) {
                if (x >= ((0.85 * MaxWidth - w[3]) / 2) && x <= ((0.85 * MaxWidth + w[3]) / 2) && y >= ((int)(0.44 * MaxHeight)) && y <= ((int)(0.44 * MaxHeight) + w[3])) {
                    PustiZvuk(zvukovi[1]);
                    while (1) {
                        TrenutnoVreme = SDL_GetTicks();
                        if (TrenutnoVreme - ProsloVreme >= interval && smer == 1) {
                            if (smer == 1) {
                                DeoTeksture(0.1 * MaxWidth, 0.05 * MaxHeight, 0.8 * MaxWidth, 0.9 * MaxHeight, 0.1 * 1920., 0.05 * 1080.0, 0.8 * 1920, 0.9 * 1080, Pozadina);
                                NapraviZid(pomerix, 0.05 * MaxHeight, pomerix1, 0.9 * MaxHeight, Upitnik);
                                SDL_RenderPresent(renderer);
                                pomerix += 0.4 * MaxWidth / 90;
                                pomerix1 -= 0.8 * MaxWidth / 90;
                                if (pomerix >= 0.4 * MaxWidth) {
                                    break;
                                }
                            }
                            ProsloVreme = TrenutnoVreme;
                        }
                    }
                    SDL_DestroyTexture(Trepce[0]);
                    SDL_DestroyTexture(Trepce[1]);
                    if (ModIgre == 2 && Sacuvaj == 1) {
                        Sacuvaj = 0;
                        n = n1;
                    }
                    izgubio = 0;
                    FILE* file = NULL;
                    fopen_s(&file, (const char*)"Opcije.txt", (const char*)"w");
                    fprintf_s(file, "%d %d\n%d\n%d %d %d\n", MaxWidth, MaxHeight, n, Tezina, Algoritam, BrZombija);
                    fprintf_s(file, "%d %d %d\n", Izgledi[0], Izgledi[1], Izgledi[2]);
                    fprintf_s(file, "%d %d %d\n", TrenutniNivo, mapa, Sacuvaj);
                    fprintf_s(file, "%d %d", Zvukovi, Pozadinska);
                    Mix_VolumeMusic(Pozadinska);
                    for (int i = 0; i < 6; i++) {
                        Mix_VolumeChunk(zvukovi[i], Zvukovi);
                    }
                    fclose(file);
                    SDL_DestroyTexture(Upitnik);
                    SDL_DestroyTexture(Pozadina);
                    Igra(surface, SDL_GetTicks());
                }
                else if (x >= ((1.15 * MaxWidth - w[4]) / 2) && x <= ((1.15 * MaxWidth + w[4]) / 2) && y >= ((int)(0.44 * MaxHeight)) && y <= ((int)(0.44 * MaxHeight) + w[4])) {
                    PustiZvuk(zvukovi[1]);
                    while (1) {
                        TrenutnoVreme = SDL_GetTicks();
                        if (TrenutnoVreme - ProsloVreme >= interval && smer == 1) {
                            if (smer == 1) {
                                DeoTeksture(0.1 * MaxWidth, 0.05 * MaxHeight, 0.8 * MaxWidth, 0.9 * MaxHeight, 0.1 * 1920., 0.05 * 1080.0, 0.8 * 1920, 0.9 * 1080, Pozadina);
                                NapraviZid(pomerix, 0.05 * MaxHeight, pomerix1, 0.9 * MaxHeight, Upitnik);
                                SDL_RenderPresent(renderer);
                                pomerix += 0.4 * MaxWidth / 90;
                                pomerix1 -= 0.8 * MaxWidth / 90;
                                if (pomerix >= 0.4 * MaxWidth) {
                                    break;
                                }
                            }
                            ProsloVreme = TrenutnoVreme;
                        }
                    }
                    SDL_DestroyTexture(Trepce[0]);
                    SDL_DestroyTexture(Trepce[1]);
                    SDL_DestroyTexture(Upitnik);
                    SDL_DestroyTexture(Pozadina);
                    return;
                }
            }
            else if (smer == 1) {
                if (x >= ((0.85 * MaxWidth - w[3]) / 2) && x <= ((0.85 * MaxWidth + w[3]) / 2) && y >= ((int)(0.44 * MaxHeight)) && y <= ((int)(0.44 * MaxHeight) + w[3])) {
                    index = 1;
                    PisiTekst(font[0], { 144, 110, 36, 255 }, { (int)(0.85 * MaxWidth - w[3]) / 2, (int)(0.44 * MaxHeight), w[3],w[3] }, (char*)"Da");
                    NapraviZid((MaxWidth - 2 * Velicina1()) / 2, 0.56 * MaxHeight, 2 * Velicina1(), 2 * Velicina1(), Trepce[1]);
                    SDL_RenderPresent(renderer);
                }
                else if (x >= ((1.15 * MaxWidth - w[4]) / 2) && x <= ((1.15 * MaxWidth + w[4]) / 2) && y >= ((int)(0.44 * MaxHeight)) && y <= ((int)(0.44 * MaxHeight) + w[4])) {
                    index = 2;
                    PisiTekst(font[0], { 144, 110, 36, 255 }, { (int)(1.15 * MaxWidth - w[4]) / 2, (int)(0.44 * MaxHeight), w[4],w[4] }, (char*)"Ne");
                    NapraviZid((MaxWidth - 2 * Velicina1()) / 2, 0.56 * MaxHeight, 2 * Velicina1(), 2 * Velicina1(), Trepce[0]);
                    SDL_RenderPresent(renderer);
                }
                else if (index > 0) {
                    index = 0;
                    PisiTekst(font[0], { 100,80,40,255 }, { (int)(0.85 * MaxWidth - w[3]) / 2, (int)(0.44 * MaxHeight), w[3],w[3] }, (char*)"Da");
                    PisiTekst(font[0], { 100,80,40,255 }, { (int)(1.15 * MaxWidth - w[4]) / 2, (int)(0.44 * MaxHeight), w[4],w[4] }, (char*)"Ne");
                    NapraviZid((MaxWidth - 2 * Velicina1()) / 2, 0.56 * MaxHeight, 2 * Velicina1(), 2 * Velicina1(), Trepce[0]);
                    SDL_RenderPresent(renderer);
                }
            }
        }
    }
}
void SacuvajIgru(SDL_Surface** surface) {
    float pomerix1 = 0.1 * MaxWidth, pomerix = 0.45 * MaxWidth;
    Uint32 TrenutnoVreme, ProsloVreme = SDL_GetTicks();
    int w[5], h[5], x, y, index = 0, interval = 1000 / 90, smer = 0;
    SDL_Surface* pozadina = IMG_Load("Slike/PobedaPozadina.png");
    SDL_Texture* Pozadina = SDL_CreateTextureFromSurface(renderer, pozadina);
    SDL_FreeSurface(pozadina);
    NapraviZid(0, 0, MaxWidth, MaxHeight, Pozadina);
    SDL_Surface* upitnik = IMG_Load("Slike/Pobeda.png");
    SDL_Texture* Upitnik = SDL_CreateTextureFromSurface(renderer, upitnik);
    SDL_FreeSurface(upitnik);
    SDL_Surface* trepce[2] = {
        (SDL_Surface*)IMG_Load("Slike/Igrac/Nazad/1/Prica/0.png"),
        (SDL_Surface*)IMG_Load("Slike/Igrac/Nazad/1/Prica/29.png")
    };
    SDL_Texture* Trepce[2] = {
        SDL_CreateTextureFromSurface(renderer,trepce[0]),
        SDL_CreateTextureFromSurface(renderer,trepce[1])
    };
    SDL_FreeSurface(trepce[0]);
    SDL_FreeSurface(trepce[1]);
    TTF_SetFontSize(font[0], 0.38 * velicina);
    TTF_SizeText(font[0], (const char*)"Igra je uspesno sacuvana.", &w[1], &h[1]);
    TTF_SizeText(font[0], (const char*)"Da li zelite da izadjete?", &w[2], &h[2]);
    TTF_SetFontSize(font[0], 0.6 * velicina);
    TTF_SizeText(font[0], (const char*)"Da", &w[3], &h[3]);
    TTF_SizeText(font[0], (const char*)"Ne", &w[4], &h[4]);
    TTF_SetFontSize(font[0], 0.38 * velicina);
    PisiTekst(font[0], { 100,80,40,255 }, { (int)(MaxWidth - w[1]) / 2, (int)(0.22 * MaxHeight), w[1],w[1] }, (char*)"Igra je uspesno sacuvana.");
    PisiTekst(font[0], { 100,80,40,255 }, { (int)(MaxWidth - w[2]) / 2, (int)(0.28 * MaxHeight), w[2],w[2] }, (char*)"Da li zelite da izadjete?");
    TTF_SetFontSize(font[0], 0.6 * velicina);
    PisiTekst(font[0], { 100,80,40,255 }, { (int)(0.85 * MaxWidth - w[3]) / 2, (int)(0.44 * MaxHeight), w[3],w[3] }, (char*)"Da");
    PisiTekst(font[0], { 100,80,40,255 }, { (int)(1.15 * MaxWidth - w[4]) / 2, (int)(0.44 * MaxHeight), w[4],w[4] }, (char*)"Ne");
    NapraviZid((MaxWidth - 2 * Velicina1()) / 2, 0.56 * MaxHeight, 2 * Velicina1(), 2 * Velicina1(), Trepce[0]);
    SDL_RenderPresent(renderer);
    while (1) {
        TrenutnoVreme = SDL_GetTicks();
        if (TrenutnoVreme - ProsloVreme >= interval && smer == 0) {
            if (smer == 0) {
                DeoTeksture(0.1 * MaxWidth, 0.05 * MaxHeight, 0.8 * MaxWidth, 0.9 * MaxHeight, 0.1 * 1920., 0.05 * 1080.0, 0.8 * 1920, 0.9 * 1080, Pozadina);
                NapraviZid(pomerix, 0.05 * MaxHeight, pomerix1, 0.9 * MaxHeight, Upitnik);
                SDL_RenderPresent(renderer);
                pomerix -= 0.4 * MaxWidth / 90;
                pomerix1 += 0.8 * MaxWidth / 90;
                if (pomerix <= 0.05 * MaxWidth) {
                    smer = 1;
                    NapraviZid(0, 0, MaxWidth, MaxHeight, Pozadina);
                    NapraviZid(0.05 * MaxWidth, 0.05 * MaxHeight, 0.9 * MaxWidth, 0.9 * MaxHeight, Upitnik);
                    TTF_SetFontSize(font[0], 0.38 * velicina);
                    PisiTekst(font[0], { 100,80,40,255 }, { (int)(MaxWidth - w[1]) / 2, (int)(0.22 * MaxHeight), w[1],w[1] }, (char*)"Igra je uspesno sacuvana.");
                    PisiTekst(font[0], { 100,80,40,255 }, { (int)(MaxWidth - w[2]) / 2, (int)(0.28 * MaxHeight), w[2],w[2] }, (char*)"Da li zelite da nastavite?");
                    TTF_SetFontSize(font[0], 0.6 * velicina);
                    PisiTekst(font[0], { 100,80,40,255 }, { (int)(0.85 * MaxWidth - w[3]) / 2, (int)(0.44 * MaxHeight), w[3],w[3] }, (char*)"Da");
                    PisiTekst(font[0], { 100,80,40,255 }, { (int)(1.15 * MaxWidth - w[4]) / 2, (int)(0.44 * MaxHeight), w[4],w[4] }, (char*)"Ne");
                    NapraviZid((MaxWidth - 2 * Velicina1()) / 2, 0.56 * MaxHeight, 2 * Velicina1(), 2 * Velicina1(), Trepce[0]);
                    SDL_RenderPresent(renderer);
                }
            }
            ProsloVreme = TrenutnoVreme;
        }
        while (SDL_PollEvent(&event) != 0) {
            SDL_GetMouseState(&x, &y);
            if (event.type == SDL_QUIT) {
                SDL_Quit();
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN && smer == 1) {
                if (x >= ((0.85 * MaxWidth - w[3]) / 2) && x <= ((0.85 * MaxWidth + w[3]) / 2) && y >= ((int)(0.44 * MaxHeight)) && y <= ((int)(0.44 * MaxHeight) + w[3])) {
                    PustiZvuk(zvukovi[1]);
                    while (1) {
                        TrenutnoVreme = SDL_GetTicks();
                        if (TrenutnoVreme - ProsloVreme >= interval && smer == 1) {
                            if (smer == 1) {
                                DeoTeksture(0.1 * MaxWidth, 0.05 * MaxHeight, 0.8 * MaxWidth, 0.9 * MaxHeight, 0.1 * 1920., 0.05 * 1080.0, 0.8 * 1920, 0.9 * 1080, Pozadina);
                                NapraviZid(pomerix, 0.05 * MaxHeight, pomerix1, 0.9 * MaxHeight, Upitnik);
                                SDL_RenderPresent(renderer);
                                pomerix += 0.4 * MaxWidth / 90;
                                pomerix1 -= 0.8 * MaxWidth / 90;
                                if (pomerix >= 0.4 * MaxWidth) {
                                    break;
                                }
                            }
                            ProsloVreme = TrenutnoVreme;
                        }
                    }
                    SDL_DestroyTexture(Trepce[0]);
                    SDL_DestroyTexture(Trepce[1]);
                    SDL_DestroyTexture(Upitnik);
                    SDL_DestroyTexture(Pozadina);
                    SDL_DestroyTexture(mumijadesna);
                    SDL_DestroyTexture(mumijaleva);
                    SDL_DestroyTexture(Igrac);
                    FILE* file = NULL;
                    fopen_s(&file, (const char*)"Opcije.txt", (const char*)"w");
                    fprintf_s(file, "%d %d\n%d\n%d %d %d\n", MaxWidth, MaxHeight, n, Tezina, Algoritam, BrZombija);
                    fprintf_s(file, "%d %d %d\n", Izgledi[0], Izgledi[1], Izgledi[2]);
                    fprintf_s(file, "%d %d %d\n", TrenutniNivo, mapa, Sacuvaj);
                    fprintf_s(file, "%d %d", Zvukovi, Pozadinska);
                    Mix_VolumeMusic(Pozadinska);
                    for (int i = 0; i < 7; i++) {
                        Mix_VolumeChunk(zvukovi[i], Zvukovi);
                    }
                    fclose(file);
                    PrelazNaGlavniMeni(surface);
                }
                else if (x >= ((1.15 * MaxWidth - w[4]) / 2) && x <= ((1.15 * MaxWidth + w[4]) / 2) && y >= ((int)(0.44 * MaxHeight)) && y <= ((int)(0.44 * MaxHeight) + w[4])) {
                    PustiZvuk(zvukovi[1]);
                    while (1) {
                        TrenutnoVreme = SDL_GetTicks();
                        if (TrenutnoVreme - ProsloVreme >= interval && smer == 1) {
                            if (smer == 1) {
                                DeoTeksture(0.1 * MaxWidth, 0.05 * MaxHeight, 0.8 * MaxWidth, 0.9 * MaxHeight, 0.1 * 1920., 0.05 * 1080.0, 0.8 * 1920, 0.9 * 1080, Pozadina);
                                NapraviZid(pomerix, 0.05 * MaxHeight, pomerix1, 0.9 * MaxHeight, Upitnik);
                                SDL_RenderPresent(renderer);
                                pomerix += 0.4 * MaxWidth / 90;
                                pomerix1 -= 0.8 * MaxWidth / 90;
                                if (pomerix >= 0.4 * MaxWidth) {
                                    break;
                                }
                            }
                            ProsloVreme = TrenutnoVreme;
                        }
                    }
                    SDL_DestroyTexture(Upitnik);
                    SDL_DestroyTexture(Pozadina);
                    SDL_DestroyTexture(Trepce[0]);
                    SDL_DestroyTexture(Trepce[1]);
                    n1 = n;
                    return;
                }
            }
            else if (smer == 1) {
                if (x >= ((0.85 * MaxWidth - w[3]) / 2) && x <= ((0.85 * MaxWidth + w[3]) / 2) && y >= ((int)(0.44 * MaxHeight)) && y <= ((int)(0.44 * MaxHeight) + w[3])) {
                    index = 1;
                    PisiTekst(font[0], { 144, 110, 36, 255 }, { (int)(0.85 * MaxWidth - w[3]) / 2, (int)(0.44 * MaxHeight), w[3],w[3] }, (char*)"Da");
                    NapraviZid((MaxWidth - 2 * Velicina1()) / 2, 0.56 * MaxHeight, 2 * Velicina1(), 2 * Velicina1(), Trepce[1]);
                    SDL_RenderPresent(renderer);
                }
                else if (x >= ((1.15 * MaxWidth - w[4]) / 2) && x <= ((1.15 * MaxWidth + w[4]) / 2) && y >= ((int)(0.44 * MaxHeight)) && y <= ((int)(0.44 * MaxHeight) + w[4])) {
                    index = 2;
                    PisiTekst(font[0], { 144, 110, 36, 255 }, { (int)(1.15 * MaxWidth - w[4]) / 2, (int)(0.44 * MaxHeight), w[4],w[4] }, (char*)"Ne");
                    NapraviZid((MaxWidth - 2 * Velicina1()) / 2, 0.56 * MaxHeight, 2 * Velicina1(), 2 * Velicina1(), Trepce[0]);
                    SDL_RenderPresent(renderer);
                }
                else if (index > 0) {
                    index = 0;
                    PisiTekst(font[0], { 100,80,40,255 }, { (int)(0.85 * MaxWidth - w[3]) / 2, (int)(0.44 * MaxHeight), w[3],w[3] }, (char*)"Da");
                    PisiTekst(font[0], { 100,80,40,255 }, { (int)(1.15 * MaxWidth - w[4]) / 2, (int)(0.44 * MaxHeight), w[4],w[4] }, (char*)"Ne");
                    NapraviZid((MaxWidth - 2 * Velicina1()) / 2, 0.56 * MaxHeight, 2 * Velicina1(), 2 * Velicina1(), Trepce[0]);
                    SDL_RenderPresent(renderer);
                }
            }
        }
    }
}
void Hint(SDL_Surface** surface, struct Maze* talon, Uint32 PauzaVreme, int index, int* intervalx, int* BrKoraka, int* x, int* y, int* x3, int* y3, int x1 = 0, int y1 = 0, int x2 = 0, int y2 = 0) {
    int velicina = Velicina1(), velicina1 = Velicina(), radi = 1, w[2], h[2];
    char sekund[5], adresa[50];
    ProsloVreme1 = SDL_GetTicks() - PauzaVreme;
    Uint32 ProsloVreme = SDL_GetTicks(), interval = 1000 / 60, TrenutnoVreme;
    if (index == 0) {
        SDL_Surface* prica = IMG_Load("Slike/Igrac/chat/2.png");
        SDL_Texture* Prica = SDL_CreateTextureFromSurface(renderer, prica);
        SDL_FreeSurface(prica);
        TTF_SetFontSize(font[1], 0.25 * velicina1);
        TTF_SizeText(font[1], (const char*)"Nema ti spasa", &w[0], &h[0]);
        while (radi) {
            TrenutnoVreme = SDL_GetTicks();
            if (TrenutnoVreme - ProsloVreme >= interval) {
                TrenutnoVreme1 = SDL_GetTicks() - PauzaVreme;
                if (TrenutnoVreme1 - ProsloVreme1 >= interval1) {
                    ProsloVreme1 = TrenutnoVreme1;
                    sprintf_s(minut, "%d", TrenutnoVreme1 / 60000);
                    sprintf_s(sekund, "%d", TrenutnoVreme1 / 1000 - (TrenutnoVreme1 / 60000) * 60);
                    strcat_s(minut, ":");
                    if ((TrenutnoVreme1 / 1000 - (TrenutnoVreme1 / 60000) * 60) < 10)strcat_s(minut, "0");
                    strcat_s(minut, sekund);
                }
                Animacije(surface, talon, *intervalx, 0);
                NapraviZid(mumije[0].xm - 0.4 * velicina1, mumije[0].ym - velicina1, 1.5 * velicina1, 1.5 * velicina1, Prica);
                TTF_SetFontSize(font[1], 0.25 * velicina1);
                PisiTekst(font[1], { 0,0,0,255 }, { (int)(2 * mumije[0].xm + 0.7 * velicina1 - w[0]) / 2, (int)(mumije[0].ym - 0.6 * velicina1), w[0], h[0] }, (char*)"Nema ti spasa");
                *intervalx += 3;
                if (*intervalx >= (12 * MaxWidth / 25 - velicina))
                    *intervalx = 0;
                ProsloVreme = TrenutnoVreme;
                SDL_RenderPresent(renderer);
            }
            while (SDL_PollEvent(&event) != 0) {
                if (event.type == SDL_QUIT) SDL_Quit();
                if (event.type == SDL_KEYDOWN) {
                    if (event.key.keysym.sym == SDLK_w || event.key.keysym.sym == SDLK_UP) {
                        if (ProveraPobede(*x3, *y3, *x - 1, *y)) {
                            int channel = Mix_PlayChannel(-1, zvukovi[6], 0);
                            Mix_PlayChannelTimed(channel, zvukovi[6], 1, 800);
                            Pokret(surface, talon, IgracX, IgracY, velicina1, 4, intervalx, minut);
                            Pobeda(surface, *BrKoraka + 1, TrenutnoVreme1);
                        }
                        if (ValidanPokret(*x - 1, *y) && Zid(*x - 1, *y, *x, *y, talon)) {
                            int channel = Mix_PlayChannel(-1, zvukovi[6], 0);
                            Mix_PlayChannelTimed(channel, zvukovi[6], 1, 800);
                            Pokret(surface, talon, IgracX, IgracY, velicina1, 4, intervalx, minut);
                            *BrKoraka++;
                            *x -= 1;
                            IgracY -= velicina1;
                            CrtajSve(surface, talon, 1);
                            PisiTekst(font[1], { 0,0,0, 255 }, { (int)(MaxWidth - velicina * 1.25), 0,(int)(velicina * 1.5),(int)(velicina * 0.5) }, (char*)minut);
                            for (int i = 0; i < trbrojmumija; i++)
                                PomeriMumiju(surface, *x, *y, talon, i, intervalx, minut);
                            CrtajSve(surface, talon, 1);
                            NapraviZid(IgracX + 0.1 * velicina1, IgracY + 0.1 * velicina1, 0.8 * velicina1, 0.8 * velicina1, Igrac);
                            for (int i = 0; i < trbrojmumija; i++)
                                NapraviZid(mumije[i].xm + velicina1 * 0.1, mumije[i].ym + velicina1 * 0.1, 0.8 * velicina1, 0.8 * velicina1, (StranaMumije[i] == 0) ? mumijaleva : mumijadesna);
                            SDL_RenderPresent(renderer);
                            radi = 0;
                        }
                    }
                    else if (event.key.keysym.sym == SDLK_s || event.key.keysym.sym == SDLK_DOWN) {
                        if (ProveraPobede(*x3, *y3, *x + 1, *y)) {
                            int channel = Mix_PlayChannel(-1, zvukovi[6], 0);
                            Mix_PlayChannelTimed(channel, zvukovi[6], 1, 800);
                            Pokret(surface, talon, IgracX, IgracY, velicina1, 1, intervalx, minut);
                            Pobeda(surface, *BrKoraka + 1, TrenutnoVreme1);
                        }
                        if (ValidanPokret(*x + 1, *y) && Zid(*x, *y, *x + 1, *y, talon)) {
                            int channel = Mix_PlayChannel(-1, zvukovi[6], 0);
                            Mix_PlayChannelTimed(channel, zvukovi[6], 1, 800);
                            Pokret(surface, talon, IgracX, IgracY, velicina1, 1, intervalx, minut);
                            *BrKoraka++;
                            *x += 1;
                            IgracY += velicina1;
                            CrtajSve(surface, talon, 1);
                            PisiTekst(font[1], { 0,0,0, 255 }, { (int)(MaxWidth - velicina * 1.25), 0,(int)(velicina * 1.5),(int)(velicina * 0.5) }, (char*)minut);
                            for (int i = 0; i < trbrojmumija; i++)
                                PomeriMumiju(surface, *x, *y, talon, i, intervalx, minut);
                            CrtajSve(surface, talon, 1);
                            NapraviZid(IgracX + 0.1 * velicina1, IgracY + 0.1 * velicina1, 0.8 * velicina1, 0.8 * velicina1, Igrac);
                            for (int i = 0; i < trbrojmumija; i++)
                                NapraviZid(mumije[i].xm + velicina1 * 0.1, mumije[i].ym + velicina1 * 0.1, 0.8 * velicina1, 0.8 * velicina1, (StranaMumije[i] == 0) ? mumijaleva : mumijadesna);
                            SDL_RenderPresent(renderer);
                            radi = 0;
                        }
                    }
                    else if (event.key.keysym.sym == SDLK_a || event.key.keysym.sym == SDLK_LEFT) {
                        if (ProveraPobede(*x3, *y3, *x, *y - 1)) {
                            int channel = Mix_PlayChannel(-1, zvukovi[6], 0);
                            Mix_PlayChannelTimed(channel, zvukovi[6], 1, 800);
                            Pokret(surface, talon, IgracX, IgracY, velicina1, 3, intervalx, minut);
                            Pobeda(surface, *BrKoraka + 1, TrenutnoVreme1);
                        }
                        if (ValidanPokret(*x, *y - 1) && Zid(*x, *y - 1, *x, *y, talon)) {
                            int channel = Mix_PlayChannel(-1, zvukovi[6], 0);
                            Mix_PlayChannelTimed(channel, zvukovi[6], 1, 1200);
                            Pokret(surface, talon, IgracX, IgracY, velicina1, 3, intervalx, minut);
                            *BrKoraka++;
                            *y -= 1;
                            IgracX -= velicina1;
                            CrtajSve(surface, talon, 1);
                            PisiTekst(font[1], { 0,0,0, 255 }, { (int)(MaxWidth - velicina * 1.25), 0, (int)(velicina * 1.5),(int)(velicina / 2) }, (char*)minut);
                            for (int i = 0; i < trbrojmumija; i++)
                                PomeriMumiju(surface, *x, *y, talon, i, intervalx, minut);
                            CrtajSve(surface, talon, 1);
                            NapraviZid(IgracX + 0.1 * velicina1, IgracY + 0.1 * velicina1, 0.8 * velicina1, 0.8 * velicina1, Igrac);
                            for (int i = 0; i < trbrojmumija; i++)
                                NapraviZid(mumije[i].xm + velicina1 * 0.1, mumije[i].ym + velicina1 * 0.1, 0.8 * velicina1, 0.8 * velicina1, (StranaMumije[i] == 0) ? mumijaleva : mumijadesna);
                            SDL_RenderPresent(renderer);
                            radi = 0;
                        }
                    }
                    else if (event.key.keysym.sym == SDLK_d || event.key.keysym.sym == SDLK_RIGHT) {
                        if (ProveraPobede(*x3, *y3, *x, *y + 1)) {
                            int channel = Mix_PlayChannel(-1, zvukovi[6], 0);
                            Mix_PlayChannelTimed(channel, zvukovi[6], 1, 800);
                            Pokret(surface, talon, IgracX, IgracY, velicina1, 2, intervalx, minut);
                            Pobeda(surface, *BrKoraka + 1, TrenutnoVreme1);
                        }
                        if (ValidanPokret(*x, *y + 1) && Zid(*x, *y, *x, *y + 1, talon)) {
                            int channel = Mix_PlayChannel(-1, zvukovi[6], 0);
                            Mix_PlayChannelTimed(channel, zvukovi[6], 1, 800);
                            Pokret(surface, talon, IgracX, IgracY, velicina1, 2, intervalx, minut);
                            *BrKoraka++;
                            *y += 1;
                            IgracX += velicina1;
                            CrtajSve(surface, talon, 1);
                            PisiTekst(font[1], { 0,0,0, 255 }, { (int)(MaxWidth - velicina * 1.25), 0,(int)(velicina * 1.5),(int)(velicina / 2) }, (char*)minut);
                            for (int i = 0; i < trbrojmumija; i++)
                                PomeriMumiju(surface, *x, *y, talon, i, intervalx, minut);
                            CrtajSve(surface, talon, 1);
                            NapraviZid(IgracX + 0.1 * velicina1, IgracY + 0.1 * velicina1, 0.8 * velicina1, 0.8 * velicina1, Igrac);
                            for (int i = 0; i < trbrojmumija; i++)
                                NapraviZid(mumije[i].xm + velicina1 * 0.1, mumije[i].ym + velicina1 * 0.1, 0.8 * velicina1, 0.8 * velicina1, (StranaMumije[i] == 0) ? mumijaleva : mumijadesna);
                            SDL_RenderPresent(renderer);
                            radi = 0;
                        }
                    }
                }
            }
        }
        SDL_DestroyTexture(Prica);
    }
    else if (index == 1 || index == 2) {
        int smer1 = 0, smer[2], PomerajX, PomerajY, KordinataX, KordinataY;
        float Pomeraj = 0;
        SDL_Surface* hint[2];
        SDL_Texture* Hint[2];
        if (x1 < *x) {
            hint[0] = IMG_Load("Slike/Hint/1.png");
            Hint[0] = SDL_CreateTextureFromSurface(renderer, hint[0]);
            SDL_FreeSurface(hint[0]);
            smer[0] = 1;
        }
        else if (x1 > *x) {
            hint[0] = IMG_Load("Slike/Hint/2.png");
            Hint[0] = SDL_CreateTextureFromSurface(renderer, hint[0]);
            SDL_FreeSurface(hint[0]);
            smer[0] = 2;
        }
        else if (y1 < *y) {
            hint[0] = IMG_Load("Slike/Hint/3.png");
            Hint[0] = SDL_CreateTextureFromSurface(renderer, hint[0]);
            SDL_FreeSurface(hint[0]);
            smer[0] = 3;
        }
        else if (y1 > *y) {
            hint[0] = IMG_Load("Slike/Hint/4.png");
            Hint[0] = SDL_CreateTextureFromSurface(renderer, hint[0]);
            SDL_FreeSurface(hint[0]);
            smer[0] = 4;
        }
        if (index == 2) {
            if (x2 < x1) {
                hint[1] = IMG_Load("Slike/Hint/1.png");
                Hint[1] = SDL_CreateTextureFromSurface(renderer, hint[1]);
                SDL_FreeSurface(hint[1]);
                smer[1] = 1;
            }
            else if (x2 > x1) {
                hint[1] = IMG_Load("Slike/Hint/2.png");
                Hint[1] = SDL_CreateTextureFromSurface(renderer, hint[1]);
                SDL_FreeSurface(hint[1]);
                smer[1] = 2;
            }
            else if (y2 < y1) {
                hint[1] = IMG_Load("Slike/Hint/3.png");
                Hint[1] = SDL_CreateTextureFromSurface(renderer, hint[1]);
                SDL_FreeSurface(hint[1]);
                smer[1] = 3;
            }
            else if (y2 > y1) {
                hint[1] = IMG_Load("Slike/Hint/4.png");
                Hint[1] = SDL_CreateTextureFromSurface(renderer, hint[1]);
                SDL_FreeSurface(hint[1]);
                smer[1] = 4;
            }
        }
        while (radi) {
            TrenutnoVreme = SDL_GetTicks();
            if (TrenutnoVreme - ProsloVreme >= interval) {
                TrenutnoVreme1 = SDL_GetTicks() - PauzaVreme;
                if (TrenutnoVreme1 - ProsloVreme1 >= interval1) {
                    ProsloVreme1 = TrenutnoVreme1;
                    sprintf_s(minut, "%d", TrenutnoVreme1 / 60000);
                    sprintf_s(sekund, "%d", TrenutnoVreme1 / 1000 - (TrenutnoVreme1 / 60000) * 60);
                    strcat_s(minut, ":");
                    if ((TrenutnoVreme1 / 1000 - (TrenutnoVreme1 / 60000) * 60) < 10)strcat_s(minut, "0");
                    strcat_s(minut, sekund);
                }
                Animacije(surface, talon, *intervalx, 0);
                PomerajX = ((smer[0] == 3) ? (-1 * velicina1) : ((smer[0] == 4) ? (velicina1) : 0));
                PomerajY = ((smer[0] == 1) ? (-1 * velicina1) : ((smer[0] == 2) ? (velicina1) : 0));
                KordinataX = IgracX + 0.1 * velicina1 + PomerajX;
                KordinataY = IgracY + 0.1 * velicina1 + PomerajY;
                if (Hint[0] == NULL) {
                    printf("Jebem ti mater\n");
                }
                NapraviZid(KordinataX + Pomeraj / 2, KordinataY + Pomeraj / 2, 0.8 * velicina1 - Pomeraj, 0.8 * velicina1 - Pomeraj, Hint[0]);
                if (index == 2) {
                    PomerajX = ((smer[1] == 3) ? (-1 * velicina1) : ((smer[1] == 4) ? (velicina1) : 0));
                    PomerajY = ((smer[1] == 1) ? (-1 * velicina1) : ((smer[1] == 2) ? (velicina1) : 0));
                    if (Hint[1] == NULL) {
                        printf("Jebem ti mater 2\n");
                    }
                    NapraviZid(KordinataX + PomerajX + Pomeraj / 2, KordinataY + PomerajY + Pomeraj / 2, 0.8 * velicina1 - Pomeraj, 0.8 * velicina1 - Pomeraj, Hint[1]);
                }
                if (smer1 == 0) {
                    Pomeraj += 0.35 * velicina1 / 60;
                }
                else {
                    Pomeraj -= 0.35 * velicina1 / 60;
                }
                if (Pomeraj >= 0.35 * velicina1) {
                    smer1 = 1;
                }
                else if (Pomeraj <= 0) {
                    smer1 = 0;
                }
                *intervalx += 3;
                if (*intervalx >= (12 * MaxWidth / 25 - velicina))
                    *intervalx = 0;
                ProsloVreme = TrenutnoVreme;
                SDL_RenderPresent(renderer);
            }
            while (SDL_PollEvent(&event) != 0) {
                if (event.type == SDL_QUIT) SDL_Quit();
                if (event.type == SDL_KEYDOWN) {
                    if (event.key.keysym.sym == SDLK_w || event.key.keysym.sym == SDLK_UP) {
                        if (ProveraPobede(*x3, *y3, *x - 1, *y)) {
                            Pokret(surface, talon, IgracX, IgracY, velicina1, 4, intervalx, minut);
                            Pobeda(surface, *BrKoraka + 1, TrenutnoVreme1);
                        }
                        if (ValidanPokret(*x - 1, *y) && Zid(*x - 1, *y, *x, *y, talon)) {
                            Pokret(surface, talon, IgracX, IgracY, velicina1, 4, intervalx, minut);
                            *BrKoraka++;
                            *x -= 1;
                            IgracY -= velicina1;
                            CrtajSve(surface, talon, 1);
                            PisiTekst(font[1], { 0,0,0, 255 }, { (int)(MaxWidth - velicina * 1.25), 0,(int)(velicina * 1.5),(int)(velicina * 0.5) }, (char*)minut);
                            for (int i = 0; i < trbrojmumija; i++)
                                PomeriMumiju(surface, *x, *y, talon, i, intervalx, minut);
                            CrtajSve(surface, talon, 1);
                            NapraviZid(IgracX + 0.1 * velicina1, IgracY + 0.1 * velicina1, 0.8 * velicina1, 0.8 * velicina1, Igrac);
                            for (int i = 0; i < trbrojmumija; i++)
                                NapraviZid(mumije[i].xm + velicina1 * 0.1, mumije[i].ym + velicina1 * 0.1, 0.8 * velicina1, 0.8 * velicina1, (StranaMumije[i] == 0) ? mumijaleva : mumijadesna);
                            SDL_RenderPresent(renderer);
                            radi = 0;
                        }
                    }
                    else if (event.key.keysym.sym == SDLK_s || event.key.keysym.sym == SDLK_DOWN) {
                        if (ProveraPobede(*x3, *y3, *x + 1, *y)) {
                            Pokret(surface, talon, IgracX, IgracY, velicina1, 1, intervalx, minut);
                            Pobeda(surface, *BrKoraka + 1, TrenutnoVreme1);
                        }
                        if (ValidanPokret(*x + 1, *y) && Zid(*x, *y, *x + 1, *y, talon)) {
                            Pokret(surface, talon, IgracX, IgracY, velicina1, 1, intervalx, minut);
                            *BrKoraka++;
                            *x += 1;
                            IgracY += velicina1;
                            CrtajSve(surface, talon, 1);
                            PisiTekst(font[1], { 0,0,0, 255 }, { (int)(MaxWidth - velicina * 1.25), 0,(int)(velicina * 1.5),(int)(velicina * 0.5) }, (char*)minut);
                            for (int i = 0; i < trbrojmumija; i++)
                                PomeriMumiju(surface, *x, *y, talon, i, intervalx, minut);
                            CrtajSve(surface, talon, 1);
                            NapraviZid(IgracX + 0.1 * velicina1, IgracY + 0.1 * velicina1, 0.8 * velicina1, 0.8 * velicina1, Igrac);
                            for (int i = 0; i < trbrojmumija; i++)
                                NapraviZid(mumije[i].xm + velicina1 * 0.1, mumije[i].ym + velicina1 * 0.1, 0.8 * velicina1, 0.8 * velicina1, (StranaMumije[i] == 0) ? mumijaleva : mumijadesna);
                            SDL_RenderPresent(renderer);
                            radi = 0;
                        }
                    }
                    else if (event.key.keysym.sym == SDLK_a || event.key.keysym.sym == SDLK_LEFT) {
                        if (ProveraPobede(*x3, *y3, *x, *y - 1)) {
                            Pokret(surface, talon, IgracX, IgracY, velicina1, 3, intervalx, minut);
                            Pobeda(surface, *BrKoraka + 1, TrenutnoVreme1);
                        }
                        if (ValidanPokret(*x, *y - 1) && Zid(*x, *y - 1, *x, *y, talon)) {
                            Pokret(surface, talon, IgracX, IgracY, velicina1, 3, intervalx, minut);
                            *BrKoraka++;
                            *y -= 1;
                            IgracX -= velicina1;
                            CrtajSve(surface, talon, 1);
                            PisiTekst(font[1], { 0,0,0, 255 }, { (int)(MaxWidth - velicina * 1.25), 0, (int)(velicina * 1.5),(int)(velicina / 2) }, (char*)minut);
                            for (int i = 0; i < trbrojmumija; i++)
                                PomeriMumiju(surface, *x, *y, talon, i, intervalx, minut);
                            CrtajSve(surface, talon, 1);
                            NapraviZid(IgracX + 0.1 * velicina1, IgracY + 0.1 * velicina1, 0.8 * velicina1, 0.8 * velicina1, Igrac);
                            for (int i = 0; i < trbrojmumija; i++)
                                NapraviZid(mumije[i].xm + velicina1 * 0.1, mumije[i].ym + velicina1 * 0.1, 0.8 * velicina1, 0.8 * velicina1, (StranaMumije[i] == 0) ? mumijaleva : mumijadesna);
                            SDL_RenderPresent(renderer);
                            radi = 0;
                        }
                    }
                    else if (event.key.keysym.sym == SDLK_d || event.key.keysym.sym == SDLK_RIGHT) {
                        if (ProveraPobede(*x3, *y3, *x, *y + 1)) {
                            Pokret(surface, talon, IgracX, IgracY, velicina1, 2, intervalx, minut);
                            Pobeda(surface, *BrKoraka + 1, TrenutnoVreme1);
                        }
                        if (ValidanPokret(*x, *y + 1) && Zid(*x, *y, *x, *y + 1, talon)) {
                            Pokret(surface, talon, IgracX, IgracY, velicina1, 2, intervalx, minut);
                            *BrKoraka++;
                            *y += 1;
                            IgracX += velicina1;
                            CrtajSve(surface, talon, 1);
                            PisiTekst(font[1], { 0,0,0, 255 }, { (int)(MaxWidth - velicina * 1.25), 0,(int)(velicina * 1.5),(int)(velicina / 2) }, (char*)minut);
                            for (int i = 0; i < trbrojmumija; i++)
                                PomeriMumiju(surface, *x, *y, talon, i, intervalx, minut);
                            CrtajSve(surface, talon, 1);
                            NapraviZid(IgracX + 0.1 * velicina1, IgracY + 0.1 * velicina1, 0.8 * velicina1, 0.8 * velicina1, Igrac);
                            for (int i = 0; i < trbrojmumija; i++)
                                NapraviZid(mumije[i].xm + velicina1 * 0.1, mumije[i].ym + velicina1 * 0.1, 0.8 * velicina1, 0.8 * velicina1, (StranaMumije[i] == 0) ? mumijaleva : mumijadesna);
                            SDL_RenderPresent(renderer);
                            radi = 0;
                        }
                    }
                }
            }
        }
        SDL_DestroyTexture(Hint[0]);
        if (index == 2) {
            SDL_DestroyTexture(Hint[1]);
        }
    }
}
void Igra(SDL_Surface** surface, Uint32 Gubitak) {
    PauzaVreme = Gubitak;
    ProsloVreme1 = SDL_GetTicks() - PauzaVreme;
    Uint32 ProsloVreme = SDL_GetTicks(), interval = 1000 / 60, TrenutnoVreme;
    int x, y, x2 = 0, y2 = 0, radi = 1, BrKoraka = 0, Xmis, Ymis, w[7], h[7], LastIndex = -1, velicina1 = Velicina(), index = 9, intervalx = 0;
    char sekund[5], adresa[50];
    strcpy(minut, "0:00");
    TTF_SetFontSize(font[0], 0.5 * velicina);
    if (ModIgre != 1) {
        TTF_SizeText(font[0], (const char*)"Nova Igra", &w[0], &h[0]);
        TTF_SizeText(font[0], (const char*)"Sacuvaj Igru", &w[1], &h[1]);
        TTF_SizeText(font[0], (const char*)"Glavni Meni", &w[5], &h[5]);
    }
    else
        TTF_SizeText(font[0], (const char*)"Mapa", &w[5], &h[5]);
    TTF_SizeText(font[0], (const char*)"Pomoc", &w[2], &h[2]);
    TTF_SizeText(font[0], (const char*)"Opcije", &w[3], &h[3]);
    TTF_SizeText(font[0], (const char*)"Skorovi", &w[4], &h[4]);
    TTF_SetFontSize(font[1], 6 * velicina / 10);
    struct Maze* talon = NULL;
    struct Maze* maze = NULL;
    int tesko;
    switch (n) {
    case 6:
        tesko = (Tezina == 0) ? 7 : (Tezina == 1) ? 12 : 16;
        break;
    case 7:
        tesko = (Tezina == 0) ? 10 : (Tezina == 1) ? 15 : 19;
        break;
    case 8:
        tesko = (Tezina == 0) ? 12 : (Tezina == 1) ? 16 : 21;
        break;
    }
    Pozicija izlaz;
    if (izgubio == 0 && ModIgre == 2 && Sacuvaj == 0) {
        talon = (struct Maze*)malloc((2 * n - 1) * (2 * n - 1) * sizeof(struct Maze));
        maze = (struct Maze*)malloc(n * n * sizeof(struct Maze));
        trbrojmumija = BrZombija;
        mumije = (Pozicija*)calloc(BrZombija, sizeof(Pozicija));
        Pozicija igracpoz;
        int broj = 0;
        do {
            for (int i = 0; i < n; i++)
                for (int j = 0; j < n; j++)
                    maze[i * n + j].matrica = 0;
            for (int i = 0; i < 2 * n - 1; i++)
                for (int j = 0; j < 2 * n - 1; j++)
                    talon[i * (2 * n - 1) + j].matrica = 0;
            switch (rand() % 4) {
            case 0: y = rand() % n; x = 0; break;
            case 1: y = n - 1; x = rand() % n; break;
            case 2: y = rand() % n; x = n - 1; break;
            case 3: y = 0; x = rand() % n; break;
            }
            maze[x * n + y].matrica = 1;
            talon[2 * x * (2 * n - 1) + y * 2].matrica = 2;
            izlazx = x;
            izlazy = y;
            izlaz = { izlazx, izlazy };
            if (Algoritam == 1)
                lavirint(izlazx, izlazy, maze, talon);
            else
                lavirint1(izlazx, izlazy, maze, talon);
            postaviIgraca(maze, n, &igracpoz, izlaz);
            x = igracpoz.x;
            y = igracpoz.y;
            postaviMumije(maze, n, igracpoz, izlaz, mumije, &trbrojmumija);
            for (int i = 0; i < trbrojmumija; i++) {
                mumije[i].xm = 12 * MaxWidth / 25 + velicina1 * mumije[i].y;
                mumije[i].ym = 2 * MaxHeight / 15 + mumije[i].x * velicina1;
            }
            broj++;
            struct State* put = (struct State*)calloc(1, sizeof(State));
            put = Put(x, y, maze, talon, BrZombija, mumije, &BrKoraka);
            if (put != NULL && BrKoraka > tesko) {
                break;
            }
        } while (1);
        kreiraj_binarnu_datoteku(n, talon, trbrojmumija, igracpoz.x, igracpoz.y, mumije, izlaz.x, izlaz.y, 0, 0);
    }
    else if ((izgubio != 0 && ModIgre == 2) || (ModIgre == 2 && Sacuvaj == 1)) {
        int pametna = 0;
        if (ModIgre == 2 && Sacuvaj == 1 && izgubio == 0) {
            n1 = n;
        }
        Pozicija* pozicijeucitane = ucitaj_binarnu_datoteku("Nivo/nivo0.bin", &talon, &n, &BrZombija, &pametna);
        velicina1 = Velicina();
        velicina = Velicina1();
        maze = (struct Maze*)malloc(n * n * sizeof(struct Maze));
        Pozicija igracpoz = pozicijeucitane[0];
        izlaz = pozicijeucitane[1];
        izlazx = izlaz.x;
        izlazy = izlaz.y;
        mumije = (Pozicija*)calloc(BrZombija, sizeof(Pozicija));
        trbrojmumija = BrZombija;
        for (int i = 0; i < trbrojmumija; i++) {
            mumije[i] = pozicijeucitane[2 + i];
            mumije[i].xm = 12 * MaxWidth / 25 + velicina1 * mumije[i].y;
            mumije[i].ym = 2 * MaxHeight / 15 + mumije[i].x * velicina1;
        }
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                maze[i * n + j].matrica = 0;
        maze[izlaz.x * n + izlaz.y].matrica = 1;
        talon[2 * izlaz.x * (2 * n - 1) + izlaz.y * 2].matrica = 2;
        x = igracpoz.x;
        y = igracpoz.y;
    }
    else if (ModIgre == 1) {
        int pametna = 0;
        int ime1 = UsoNivo + mapa * 6;
        char ime[20];
        sprintf(ime, "Nivo/nivo%d.bin", ime1);
        if (PrviPut1 == 0) {
            n1 = n;
            PrviPut1 = 1;
        }
        Pozicija* pozicijeucitane = ucitaj_binarnu_datoteku(ime, &talon, &n, &BrZombija, &pametna);
        velicina = Velicina1();
        velicina1 = Velicina();
        maze = (struct Maze*)malloc(n * n * sizeof(struct Maze));
        Pozicija igracpoz = pozicijeucitane[0];
        izlaz = pozicijeucitane[1];
        izlazx = izlaz.x;
        izlazy = izlaz.y;
        mumije = (Pozicija*)calloc(BrZombija, sizeof(Pozicija));
        trbrojmumija = BrZombija;
        for (int i = 0; i < trbrojmumija; i++) {
            mumije[i] = pozicijeucitane[2 + i];
            mumije[i].xm = 12 * MaxWidth / 25 + velicina1 * mumije[i].y;
            mumije[i].ym = 2 * MaxHeight / 15 + mumije[i].x * velicina1;
        }
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                maze[i * n + j].matrica = 0;
        maze[izlaz.x * n + izlaz.y].matrica = 1;
        talon[2 * izlaz.x * (2 * n - 1) + izlaz.y * 2].matrica = 2;
        x = igracpoz.x;
        y = igracpoz.y;
    }
    CrtajSve(surface, talon, 0, -1, &x2, &y2);
    IgracX = 0.48 * MaxWidth + velicina1 * y;
    IgracY = 2 * MaxHeight / 15 + x * velicina1;
    sprintf_s(adresa, "Slike/Mumija/Desno/%d/MumijaDesno.png", Izgledi[0]);
    SDL_Surface* mumijad = IMG_Load(adresa);
    sprintf_s(adresa, "Slike/Mumija/Levo/%d/MumijaLevo.png", Izgledi[0]);
    SDL_Surface* mumijal = IMG_Load(adresa);
    sprintf_s(adresa, "Slike/Igrac/Nazad/%d/walk/%d.png", Izgledi[1], 0);
    SDL_Surface* igrac = IMG_Load(adresa);
    mumijadesna = SDL_CreateTextureFromSurface(renderer, mumijad);
    mumijaleva = SDL_CreateTextureFromSurface(renderer, mumijal);
    Igrac = SDL_CreateTextureFromSurface(renderer, igrac);
    SDL_FreeSurface(mumijad);
    SDL_FreeSurface(mumijal);
    SDL_FreeSurface(igrac);
    for (int i = 0; i < trbrojmumija; i++)
        NapraviZid(mumije[i].xm + velicina1 * 0.1, mumije[i].ym + velicina1 * 0.1, 0.8 * velicina1, 0.8 * velicina1, mumijaleva);
    NapraviZid(IgracX + 0.1 * velicina1, IgracY + 0.1 * velicina1, 0.8 * velicina1, 0.8 * velicina1, Igrac);
    PisiTekst(font[1], { 0,0,0, 255 }, { (int)(MaxWidth - 1.25 * velicina), (int)(0),(int)(velicina * 1.5),(int)(velicina * 0.5) }, (char*)minut);
    SDL_RenderPresent(renderer);
    while (radi) {
        TrenutnoVreme = SDL_GetTicks();
        if (TrenutnoVreme - ProsloVreme >= interval) {
            TrenutnoVreme1 = SDL_GetTicks() - PauzaVreme;
            if (TrenutnoVreme1 - ProsloVreme1 >= interval1) {
                ProsloVreme1 = TrenutnoVreme1;
                sprintf_s(minut, "%d", TrenutnoVreme1 / 60000);
                sprintf_s(sekund, "%d", TrenutnoVreme1 / 1000 - (TrenutnoVreme1 / 60000) * 60);
                strcat_s(minut, ":");
                if ((TrenutnoVreme1 / 1000 - (TrenutnoVreme1 / 60000) * 60) < 10)strcat_s(minut, "0");
                strcat_s(minut, sekund);
            }
            Animacije(surface, talon, intervalx, 0);
            intervalx += 3;
            if (intervalx >= (12 * MaxWidth / 25 - velicina))
                intervalx = 0;
            ProsloVreme = TrenutnoVreme;
            SDL_RenderPresent(renderer);
        }
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) radi = 0;
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_w || event.key.keysym.sym == SDLK_UP) {
                    if (ProveraPobede(x2, y2, x - 1, y)) {
                        int channel = Mix_PlayChannel(-1, zvukovi[6], 0);
                        Mix_PlayChannelTimed(channel, zvukovi[6], 1, 1200);
                        Pokret(surface, talon, IgracX, IgracY, velicina1, 4, &intervalx, minut);
                        Pobeda(surface, BrKoraka + 1, TrenutnoVreme1);
                    }
                    if (ValidanPokret(x - 1, y) && Zid(x - 1, y, x, y, talon)) {
                        int channel = Mix_PlayChannel(-1, zvukovi[6], 0);
                        Mix_PlayChannelTimed(channel, zvukovi[6], 1, 1200);
                        Pokret(surface, talon, IgracX, IgracY, velicina1, 4, &intervalx, minut);
                        BrKoraka++;
                        x -= 1;
                        IgracY -= velicina1;
                        CrtajSve(surface, talon, 1);
                        PisiTekst(font[1], { 0,0,0, 255 }, { (int)(MaxWidth - velicina * 1.25), 0,(int)(velicina * 1.5),(int)(velicina * 0.5) }, (char*)minut);
                        for (int i = 0; i < trbrojmumija; i++)
                            PomeriMumiju(surface, x, y, talon, i, &intervalx, minut);
                        CrtajSve(surface, talon, 1);
                        NapraviZid(IgracX + 0.1 * velicina1, IgracY + 0.1 * velicina1, 0.8 * velicina1, 0.8 * velicina1, Igrac);
                        for (int i = 0; i < trbrojmumija; i++)
                            NapraviZid(mumije[i].xm + velicina1 * 0.1, mumije[i].ym + velicina1 * 0.1, 0.8 * velicina1, 0.8 * velicina1, (StranaMumije[i] == 0) ? mumijaleva : mumijadesna);
                        SDL_RenderPresent(renderer);
                    }
                }
                else if (event.key.keysym.sym == SDLK_s || event.key.keysym.sym == SDLK_DOWN) {
                    if (ProveraPobede(x2, y2, x + 1, y)) {
                        int channel = Mix_PlayChannel(-1, zvukovi[6], 0);
                        Mix_PlayChannelTimed(channel, zvukovi[6], 1, 1200);
                        Pokret(surface, talon, IgracX, IgracY, velicina1, 1, &intervalx, minut);
                        Pobeda(surface, BrKoraka + 1, TrenutnoVreme1);
                    }
                    if (ValidanPokret(x + 1, y) && Zid(x, y, x + 1, y, talon)) {
                        int channel = Mix_PlayChannel(-1, zvukovi[6], 0);
                        Mix_PlayChannelTimed(channel, zvukovi[6], 1, 1200);
                        Pokret(surface, talon, IgracX, IgracY, velicina1, 1, &intervalx, minut);
                        BrKoraka++;
                        x += 1;
                        IgracY += velicina1;
                        CrtajSve(surface, talon, 1);
                        PisiTekst(font[1], { 0,0,0, 255 }, { (int)(MaxWidth - velicina * 1.25), 0,(int)(velicina * 1.5),(int)(velicina * 0.5) }, (char*)minut);
                        for (int i = 0; i < trbrojmumija; i++)
                            PomeriMumiju(surface, x, y, talon, i, &intervalx, minut);
                        CrtajSve(surface, talon, 1);
                        NapraviZid(IgracX + 0.1 * velicina1, IgracY + 0.1 * velicina1, 0.8 * velicina1, 0.8 * velicina1, Igrac);
                        for (int i = 0; i < trbrojmumija; i++)
                            NapraviZid(mumije[i].xm + velicina1 * 0.1, mumije[i].ym + velicina1 * 0.1, 0.8 * velicina1, 0.8 * velicina1, (StranaMumije[i] == 0) ? mumijaleva : mumijadesna);
                        SDL_RenderPresent(renderer);
                    }
                }
                else if (event.key.keysym.sym == SDLK_a || event.key.keysym.sym == SDLK_LEFT) {
                    if (ProveraPobede(x2, y2, x, y - 1)) {
                        int channel = Mix_PlayChannel(-1, zvukovi[6], 0);
                        Mix_PlayChannelTimed(channel, zvukovi[6], 1, 1200);
                        Pokret(surface, talon, IgracX, IgracY, velicina1, 3, &intervalx, minut);
                        Pobeda(surface, BrKoraka + 1, TrenutnoVreme1);
                    }
                    if (ValidanPokret(x, y - 1) && Zid(x, y - 1, x, y, talon)) {
                        int channel = Mix_PlayChannel(-1, zvukovi[6], 0);
                        Mix_PlayChannelTimed(channel, zvukovi[6], 1, 1200);
                        Pokret(surface, talon, IgracX, IgracY, velicina1, 3, &intervalx, minut);
                        BrKoraka++;
                        y -= 1;
                        IgracX -= velicina1;
                        CrtajSve(surface, talon, 1);
                        PisiTekst(font[1], { 0,0,0, 255 }, { (int)(MaxWidth - velicina * 1.25), 0, (int)(velicina * 1.5),(int)(velicina / 2) }, (char*)minut);
                        for (int i = 0; i < trbrojmumija; i++)
                            PomeriMumiju(surface, x, y, talon, i, &intervalx, minut);
                        CrtajSve(surface, talon, 1);
                        NapraviZid(IgracX + 0.1 * velicina1, IgracY + 0.1 * velicina1, 0.8 * velicina1, 0.8 * velicina1, Igrac);
                        for (int i = 0; i < trbrojmumija; i++)
                            NapraviZid(mumije[i].xm + velicina1 * 0.1, mumije[i].ym + velicina1 * 0.1, 0.8 * velicina1, 0.8 * velicina1, (StranaMumije[i] == 0) ? mumijaleva : mumijadesna);
                        SDL_RenderPresent(renderer);
                    }
                }
                else if (event.key.keysym.sym == SDLK_d || event.key.keysym.sym == SDLK_RIGHT) {
                    if (ProveraPobede(x2, y2, x, y + 1)) {
                        int channel = Mix_PlayChannel(-1, zvukovi[6], 0);
                        Mix_PlayChannelTimed(channel, zvukovi[6], 1, 1200);
                        Pokret(surface, talon, IgracX, IgracY, velicina1, 2, &intervalx, minut);
                        Pobeda(surface, BrKoraka + 1, TrenutnoVreme1);
                    }
                    if (ValidanPokret(x, y + 1) && Zid(x, y, x, y + 1, talon)) {
                        int channel = Mix_PlayChannel(-1, zvukovi[6], 0);
                        Mix_PlayChannelTimed(channel, zvukovi[6], 1, 1200);
                        Pokret(surface, talon, IgracX, IgracY, velicina1, 2, &intervalx, minut);
                        BrKoraka++;
                        y += 1;
                        IgracX += velicina1;
                        CrtajSve(surface, talon, 1);
                        PisiTekst(font[1], { 0,0,0, 255 }, { (int)(MaxWidth - velicina * 1.25), 0,(int)(velicina * 1.5),(int)(velicina / 2) }, (char*)minut);
                        for (int i = 0; i < trbrojmumija; i++)
                            PomeriMumiju(surface, x, y, talon, i, &intervalx, minut);
                        CrtajSve(surface, talon, 1);
                        NapraviZid(IgracX + 0.1 * velicina1, IgracY + 0.1 * velicina1, 0.8 * velicina1, 0.8 * velicina1, Igrac);
                        for (int i = 0; i < trbrojmumija; i++)
                            NapraviZid(mumije[i].xm + velicina1 * 0.1, mumije[i].ym + velicina1 * 0.1, 0.8 * velicina1, 0.8 * velicina1, (StranaMumije[i] == 0) ? mumijaleva : mumijadesna);
                        SDL_RenderPresent(renderer);
                    }
                }
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN) {
                SDL_GetMouseState(&Xmis, &Ymis);
                if (ModIgre != 1) {
                    if (Xmis >= (0.48 * MaxWidth - velicina - w[0]) / 2 && Xmis <= (0.48 * MaxWidth - velicina + w[0]) / 2 && Ymis >= 0.22 * MaxHeight && Ymis <= (0.22 * MaxHeight + h[0])) {
                        PustiZvuk(zvukovi[1]);
                        Uint32 SadVreme = TrenutnoVreme1;
                        NovaIgra(surface);
                        PauzaVreme = SDL_GetTicks() - SadVreme;
                        CrtajSve(surface, talon, 0, -1);
                        NapraviZid(IgracX + 0.1 * velicina1, IgracY + 0.1 * velicina1, 0.8 * velicina1, 0.8 * velicina1, Igrac);
                        for (int i = 0; i < trbrojmumija; i++)
                            NapraviZid(mumije[i].xm + velicina1 * 0.1, mumije[i].ym + velicina1 * 0.1, 0.8 * velicina1, 0.8 * velicina1, (StranaMumije[i] == 0) ? mumijaleva : mumijadesna);
                        PisiTekst(font[1], { 0,0,0, 255 }, { (int)(MaxWidth - 1.25 * velicina), (int)(0),(int)(velicina * 1.5),(int)(velicina * 0.5) }, (char*)minut);
                        SDL_RenderPresent(renderer);
                    }
                    else if (Xmis >= (12 * MaxWidth / 25 - velicina - w[1]) / 2 && Xmis <= (0.48 * MaxWidth - velicina + w[1]) / 2 && Ymis >= 0.32 * MaxHeight && Ymis <= (0.32 * MaxHeight + h[1])) {
                        PustiZvuk(zvukovi[1]);
                        Uint32 SadVreme = TrenutnoVreme1;
                        kreiraj_binarnu_datoteku(n, talon, BrZombija, x, y, mumije, izlazx, izlazy, 0, 0);
                        Sacuvaj = 1;
                        FILE* file = NULL;
                        fopen_s(&file, (const char*)"Opcije.txt", (const char*)"w");
                        fprintf_s(file, "%d %d\n%d\n%d %d %d\n", MaxWidth, MaxHeight, n, Tezina, Algoritam, BrZombija);
                        fprintf_s(file, "%d %d %d\n", Izgledi[0], Izgledi[1], Izgledi[2]);
                        fprintf_s(file, "%d %d %d\n", TrenutniNivo, mapa, Sacuvaj);
                        fprintf_s(file, "%d %d", Zvukovi, Pozadinska);
                        fclose(file);
                        SacuvajIgru(surface);
                        PauzaVreme = SDL_GetTicks() - SadVreme;
                        CrtajSve(surface, talon, 0, -1);
                        NapraviZid(IgracX + 0.1 * velicina1, IgracY + 0.1 * velicina1, 0.8 * velicina1, 0.8 * velicina1, Igrac);
                        for (int i = 0; i < trbrojmumija; i++)
                            NapraviZid(mumije[i].xm + velicina1 * 0.1, mumije[i].ym + velicina1 * 0.1, 0.8 * velicina1, 0.8 * velicina1, (StranaMumije[i] == 0) ? mumijaleva : mumijadesna);
                        PisiTekst(font[1], { 0,0,0, 255 }, { (int)(MaxWidth - 1.25 * velicina), (int)(0),(int)(velicina * 1.5),(int)(velicina * 0.5) }, (char*)minut);
                        SDL_RenderPresent(renderer);
                    }
                    else if ((Xmis >= (12 * MaxWidth / 25 - velicina - w[2]) / 2 && Xmis <= (12 * MaxWidth / 25 - velicina + w[2]) / 2 && Ymis >= 0.42 * MaxHeight && Ymis <= (0.42 * MaxHeight + h[2]))) {
                        PustiZvuk(zvukovi[1]);
                        index = 9;
                        CrtajSve(surface, talon, index);
                        SDL_RenderPresent(renderer);
                        int BrKoraka1 = 0, Moze = 0;
                        if (ProveraPobede(x2, y2, x - 1, y) || ProveraPobede(x2, y2, x + 1, y) || ProveraPobede(x2, y2, x, y - 1) || ProveraPobede(x2, y2, x - 1, y + 1)) {
                            Moze = 1;
                        }
                        if (Moze == 0) {
                            struct State* put = Put(x, y, maze, talon, trbrojmumija, mumije, &BrKoraka1);
                            if (BrKoraka1 == 2) {
                                struct State next;
                                next = put[BrKoraka1 - 2];
                                Hint(surface, talon, PauzaVreme, 1, &intervalx, &BrKoraka, &x, &y, &x2, &y2, next.x, next.y);
                            }
                            if (BrKoraka1 > 2) {
                                struct State next1, next2;
                                next1 = put[BrKoraka1 - 2];
                                next2 = put[BrKoraka1 - 3];
                                Hint(surface, talon, PauzaVreme, 2, &intervalx, &BrKoraka, &x, &y, &x2, &y2, next1.x, next1.y, next2.x, next2.y);
                            }
                            else {
                                Hint(surface, talon, PauzaVreme, 0, &intervalx, &BrKoraka, &x, &y, &x2, &y2);
                            }
                        }
                    }
                    else if (Xmis >= (12 * MaxWidth / 25 - velicina - w[5]) / 2 && Xmis <= (0.48 * MaxWidth - velicina + w[5]) / 2 && Ymis >= 0.72 * MaxHeight && Ymis <= (0.72 * MaxHeight + h[5])) {
                        PustiZvuk(zvukovi[1]);
                        PustiZvuk(zvukovi[0]);
                        SDL_DestroyTexture(mumijadesna);
                        SDL_DestroyTexture(mumijaleva);
                        SDL_DestroyTexture(Igrac);
                        if (ModIgre == 2 && Sacuvaj == 1) {
                            n = n1;
                        }
                        izgubio = 0;
                        PrelazNaGlavniMeni(surface);
                    }
                    else if (Xmis >= (0.48 * MaxWidth - velicina - w[3]) / 2 && Xmis <= (0.48 * MaxWidth - velicina + w[3]) / 2 && Ymis >= 0.52 * MaxHeight && Ymis <= (0.52 * MaxHeight + h[3])) {
                        PustiZvuk(zvukovi[1]);
                        Uint32 SadVreme = TrenutnoVreme1;
                        OpcijeIgra(surface);
                        PauzaVreme = SDL_GetTicks() - SadVreme;
                        CrtajSve(surface, talon, 0, -1);
                        NapraviZid(IgracX + 0.1 * velicina1, IgracY + 0.1 * velicina1, 0.8 * velicina1, 0.8 * velicina1, Igrac);
                        for (int i = 0; i < trbrojmumija; i++)
                            NapraviZid(mumije[i].xm + velicina1 * 0.1, mumije[i].ym + velicina1 * 0.1, 0.8 * velicina1, 0.8 * velicina1, (StranaMumije[i] == 0) ? mumijaleva : mumijadesna);
                        PisiTekst(font[1], { 0,0,0, 255 }, { (int)(MaxWidth - 1.25 * velicina), (int)(0),(int)(velicina * 1.5),(int)(velicina * 0.5) }, (char*)minut);
                        SDL_RenderPresent(renderer);
                    }
                }
                else {
                    if (Xmis >= (0.48 * MaxWidth - velicina - w[5]) / 2 && Xmis <= (0.48 * MaxWidth - velicina + w[5]) / 2 && Ymis >= 0.64 * MaxHeight && Ymis <= (0.64 * MaxHeight + h[5])) {
                        PustiZvuk(zvukovi[1]);
                        SDL_DestroyTexture(mumijadesna);
                        SDL_DestroyTexture(mumijaleva);
                        SDL_DestroyTexture(Igrac);
                        Mapa(surface, 0);
                    }
                    else if (Xmis >= (0.48 * MaxWidth - velicina - w[3]) / 2 && Xmis <= (0.48 * MaxWidth - velicina + w[3]) / 2 && Ymis >= 0.4 * MaxHeight && Ymis <= (0.4 * MaxHeight + h[3])) {
                        PustiZvuk(zvukovi[1]);
                        Uint32 SadVreme = TrenutnoVreme1;
                        OpcijeIgra(surface);
                        PauzaVreme = SDL_GetTicks() - SadVreme;
                        CrtajSve(surface, talon, 0, -1);
                        NapraviZid(IgracX + 0.1 * velicina1, IgracY + 0.1 * velicina1, 0.8 * velicina1, 0.8 * velicina1, Igrac);
                        for (int i = 0; i < trbrojmumija; i++)
                            NapraviZid(mumije[i].xm + velicina1 * 0.1, mumije[i].ym + velicina1 * 0.1, 0.8 * velicina1, 0.8 * velicina1, (StranaMumije[i] == 0) ? mumijaleva : mumijadesna);
                        PisiTekst(font[1], { 0,0,0, 255 }, { (int)(MaxWidth - 1.25 * velicina), (int)(0),(int)(velicina * 1.5),(int)(velicina * 0.5) }, (char*)minut);
                        SDL_RenderPresent(renderer);
                    }
                    else if (Xmis >= (0.48 * MaxWidth - velicina - w[2]) / 2 && Xmis <= (0.48 * MaxWidth - velicina + w[2]) / 2 && Ymis >= 0.28 * MaxHeight && Ymis <= (0.28 * MaxHeight + h[2])) {
                        PustiZvuk(zvukovi[1]);
                        index = 9;
                        CrtajSve(surface, talon, index);
                        SDL_RenderPresent(renderer);
                        int BrKoraka1 = 0, Moze = 0;
                        if (ProveraPobede(x2, y2, x - 1, y) || ProveraPobede(x2, y2, x + 1, y) || ProveraPobede(x2, y2, x, y - 1) || ProveraPobede(x2, y2, x - 1, y + 1)) {
                            Moze = 1;
                        }
                        if (Moze == 0) {
                            struct State* put = Put(x, y, maze, talon, trbrojmumija, mumije, &BrKoraka1);
                            if (BrKoraka1 == 2) {
                                struct State next;
                                next = put[BrKoraka1 - 2];
                                Hint(surface, talon, PauzaVreme, 1, &intervalx, &BrKoraka, &x, &y, &x2, &y2, next.x, next.y);
                            }
                            if (BrKoraka1 > 2) {
                                struct State next1, next2;
                                next1 = put[BrKoraka1 - 2];
                                next2 = put[BrKoraka1 - 3];
                                Hint(surface, talon, PauzaVreme, 2, &intervalx, &BrKoraka, &x, &y, &x2, &y2, next1.x, next1.y, next2.x, next2.y);
                            }
                            else {
                                Hint(surface, talon, PauzaVreme, 0, &intervalx, &BrKoraka, &x, &y, &x2, &y2);
                            }
                        }
                    }
                }
            }
            else {
                SDL_GetMouseState(&Xmis, &Ymis);
                if (ModIgre != 1) {
                    if (Xmis >= (12 * MaxWidth / 25 - velicina - w[0]) / 2 && Xmis <= (12 * MaxWidth / 25 - velicina + w[0]) / 2 && Ymis >= 0.22 * MaxHeight && Ymis <= (0.22 * MaxHeight + h[0])) {
                        index = 3;
                        CrtajSve(surface, talon, index);
                        SDL_RenderPresent(renderer);
                    }
                    else if (Xmis >= (12 * MaxWidth / 25 - velicina - w[1]) / 2 && Xmis <= (12 * MaxWidth / 25 - velicina + w[1]) / 2 && Ymis >= 0.32 * MaxHeight && Ymis <= (0.32 * MaxHeight + h[1])) {
                        index = 4;
                        CrtajSve(surface, talon, index);
                        SDL_RenderPresent(renderer);
                    }
                    else if (Xmis >= (12 * MaxWidth / 25 - velicina - w[2]) / 2 && Xmis <= (12 * MaxWidth / 25 - velicina + w[2]) / 2 && Ymis >= 0.42 * MaxHeight && Ymis <= (0.42 * MaxHeight + h[2])) {
                        index = 5;
                        CrtajSve(surface, talon, index);
                        SDL_RenderPresent(renderer);
                    }
                    else if (Xmis >= (12 * MaxWidth / 25 - velicina - w[3]) / 2 && Xmis <= (12 * MaxWidth / 25 - velicina + w[3]) / 2 && Ymis >= 0.52 * MaxHeight && Ymis <= (0.52 * MaxHeight + h[3])) {
                        index = 6;
                        CrtajSve(surface, talon, index);
                        SDL_RenderPresent(renderer);
                    }
                    else if (Xmis >= (12 * MaxWidth / 25 - velicina - w[4]) / 2 && Xmis <= (12 * MaxWidth / 25 - velicina + w[4]) / 2 && Ymis >= 0.62 * MaxHeight && Ymis <= (0.62 * MaxHeight + h[4])) {
                        index = 7;
                        CrtajSve(surface, talon, index);
                        SDL_RenderPresent(renderer);
                    }
                    else if (Xmis >= (12 * MaxWidth / 25 - velicina - w[5]) / 2 && Xmis <= (12 * MaxWidth / 25 - velicina + w[5]) / 2 && Ymis >= 0.72 * MaxHeight && Ymis <= (0.72 * MaxHeight + h[5])) {
                        index = 8;
                        CrtajSve(surface, talon, index);
                        SDL_RenderPresent(renderer);
                    }
                    else if (index != 9) {
                        index = 9;
                        CrtajSve(surface, talon, index);
                        SDL_RenderPresent(renderer);
                    }
                }
                else {
                    if (Xmis >= (12 * MaxWidth / 25 - velicina - w[2]) / 2 && Xmis <= (12 * MaxWidth / 25 - velicina + w[2]) / 2 && Ymis >= 0.28 * MaxHeight && Ymis <= (0.28 * MaxHeight + h[2])) {
                        index = 5;
                        CrtajSve(surface, talon, index);
                        SDL_RenderPresent(renderer);
                    }
                    else if (Xmis >= (12 * MaxWidth / 25 - velicina - w[3]) / 2 && Xmis <= (12 * MaxWidth / 25 - velicina + w[3]) / 2 && Ymis >= 0.4 * MaxHeight && Ymis <= (0.4 * MaxHeight + h[3])) {
                        index = 6;
                        CrtajSve(surface, talon, index);
                        SDL_RenderPresent(renderer);
                    }
                    else if (Xmis >= (12 * MaxWidth / 25 - velicina - w[4]) / 2 && Xmis <= (12 * MaxWidth / 25 - velicina + w[4]) / 2 && Ymis >= 0.52 * MaxHeight && Ymis <= (0.52 * MaxHeight + h[4])) {
                        index = 7;
                        CrtajSve(surface, talon, index);
                        SDL_RenderPresent(renderer);
                    }
                    else if (Xmis >= (12 * MaxWidth / 25 - velicina - w[5]) / 2 && Xmis <= (12 * MaxWidth / 25 - velicina + w[5]) / 2 && Ymis >= 0.64 * MaxHeight && Ymis <= (0.64 * MaxHeight + h[5])) {
                        index = 8;
                        CrtajSve(surface, talon, index);
                        SDL_RenderPresent(renderer);
                    }
                    else if (index != 9) {
                        index = 9;
                        CrtajSve(surface, talon, index);
                        SDL_RenderPresent(renderer);
                    }

                }
            }
        }
    }
    SDL_Quit();
}
void CrtajZvukIzgled(SDL_Surface** surface, int index, int LastIndex, int Xmis = 0) {
    int x[5], y[5], br, pravougaonik1Y, pravougaonik2Y;
    TTF_SetFontSize(font[0], 0.5 * velicina);
    TTF_SizeText(font[0], (const char*)"Izgled Mumije", &x[0], &y[0]);
    TTF_SizeText(font[0], (const char*)"Jacina zvuka efekata", &x[1], &y[1]);
    TTF_SetFontSize(font[1], 0.5 * velicina);
    TTF_SizeText(font[1], (const char*)"<", &x[4], &y[4]);
    if (index == 0 && LastIndex != 1) {
        char lokacija[3][80];
        TTF_SetFontSize(font[0], 0.8 * velicina);
        TTF_SizeText(font[0], (const char*)"Izgled i Zvuk", &x[0], &y[0]);
        TTF_SetFontSize(font[0], 0.5 * velicina);
        TTF_SizeText(font[0], (const char*)"Jacina muzike", &x[2], &y[2]);
        if (LastIndex == -1) {
            char zvuk[10];
            float Puno, TrenutnaMuzika, TrenutniZvuk, Xmis1, Xmis2;
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface[4]);
            SDL_Texture* nazad = SDL_CreateTextureFromSurface(renderer, surface[5]);
            NapraviZid(0, 0, MaxWidth, MaxHeight, texture);
            NapraviZid(0.03 * MaxWidth, 0.03 * MaxHeight, velicina, velicina, nazad);
            SDL_DestroyTexture(texture);
            SDL_DestroyTexture(nazad);
            TTF_SetFontSize(font[0], 0.8 * velicina);
            PisiTekst(font[0], { 0,0,0,255 }, { (int)(MaxWidth - x[0]) / 2, (int)(0.05 * MaxHeight), x[0],y[0] }, (char*)"Izgled i Zvuk");
            TTF_SetFontSize(font[0], 0.5 * velicina);
            PisiTekst(font[0], { 0,0,0,255 }, { (int)(0.1 * MaxWidth), (int)(0.2 * MaxHeight), x[1],y[1] }, (char*)"Jacina zvuka efekata");
            PisiTekst(font[0], { 0,0,0,255 }, { (int)(0.1 * MaxWidth), (int)(0.35 * MaxHeight), x[2],y[2] }, (char*)"Jacina muzike");
            Puno = 0.7 * MaxWidth - x[1];
            TrenutnaMuzika = (int)(Puno - Pozadinska * Puno / 100);
            TrenutniZvuk = (int)(Puno - Zvukovi * Puno / 100);
            Xmis1 = 0.9 * MaxWidth - TrenutnaMuzika;
            Xmis2 = 0.9 * MaxWidth - TrenutniZvuk;
            SDL_SetRenderDrawColor(renderer, 142, 126, 103, 255);
            pravougaonik1Y = (int)(0.2 * MaxHeight + y[1] / 2 - 0.13 * velicina);
            pravougaonik2Y = (int)(0.35 * MaxHeight + y[1] / 2 - 0.13 * velicina);
            ObojiZid(Xmis1, (int)(0.2 * MaxHeight + y[1] / 2 - 0.13 * velicina), TrenutnaMuzika, 0.27 * velicina + 1);
            SDL_RenderFillCircle(0.9 * MaxWidth, (int)(0.2 * MaxHeight + y[1] / 2), 0.13 * velicina);
            ObojiZid(Xmis2, (int)(0.35 * MaxHeight + y[1] / 2 - 0.13 * velicina), TrenutniZvuk, 0.27 * velicina + 1);
            SDL_RenderFillCircle(0.9 * MaxWidth, (int)(0.35 * MaxHeight + y[1] / 2), 0.13 * velicina);
            SDL_SetRenderDrawColor(renderer, 99, 78, 61, 255);
            ObojiZid(0.2 * MaxWidth + x[1], (int)(0.2 * MaxHeight + y[1] / 2 - 0.13 * velicina), Puno - TrenutnaMuzika, 0.27 * velicina + 1);
            ObojiZid(0.2 * MaxWidth + x[1], (int)(0.35 * MaxHeight + y[1] / 2 - 0.13 * velicina), Puno - TrenutniZvuk, 0.27 * velicina + 1);
            SDL_RenderFillCircle(0.2 * MaxWidth + x[1], 0.2 * MaxHeight + y[1] / 2, 0.13 * velicina);
            SDL_RenderFillCircle(0.2 * MaxWidth + x[1], (int)(0.35 * MaxHeight + y[1] / 2), 0.13 * velicina);
            SDL_RenderFillCircle(Xmis1, (int)(0.2 * MaxHeight + y[1] / 2), 0.13 * velicina);
            SDL_RenderFillCircle(Xmis2, (int)(0.35 * MaxHeight + y[1] / 2), 0.13 * velicina);
            SDL_SetRenderDrawColor(renderer, 57, 43, 33, 255);
            SDL_RenderDrawCircle(Xmis1, (int)(0.2 * MaxHeight + y[1] / 2), 0.13 * velicina);
            SDL_RenderDrawCircle(Xmis2, (int)(0.35 * MaxHeight + y[1] / 2), 0.13 * velicina);
            TTF_SetFontSize(font[1], 0.26 * velicina);
            sprintf_s(zvuk, "%d", Pozadinska);
            strcat_s(zvuk, "%");
            TTF_SizeText(font[1], (const char*)zvuk, &x[2], &y[2]);
            PisiTekst(font[1], { 0,0,0,255 }, { (int)(0.9 * MaxWidth - x[2]), (int)(0.2 * MaxHeight + y[1] / 2 - y[2] / 2), x[2],y[2] }, (char*)zvuk);
            sprintf_s(zvuk, "%d", Zvukovi);
            strcat_s(zvuk, "%");
            TTF_SizeText(font[1], (const char*)zvuk, &x[2], &y[2]);
            PisiTekst(font[1], { 0,0,0,255 }, { (int)(0.9 * MaxWidth - x[2]), (int)(0.35 * MaxHeight + y[1] / 2 - y[2] / 2), x[2],y[2] }, (char*)zvuk);
            TTF_SetFontSize(font[1], 0.5 * velicina);
            TTF_SizeText(font[1], (const char*)"<", &x[4], &y[4]);
            TTF_SizeText(font[0], (const char*)"Izgled Mumije", &x[0], &y[0]);
            PisiTekst(font[0], { 0,0,0,255 }, { (int)(0.1 * MaxWidth), (int)(0.5 * MaxHeight), x[0],y[0] }, (char*)"Izgled Mumije");
            TTF_SizeText(font[0], (const char*)"Izgled Igraca", &x[2], &y[2]);
            PisiTekst(font[0], { 0,0,0,255 }, { (int)(0.1 * MaxWidth), (int)(0.65 * MaxHeight), x[1],y[1] }, (char*)"Izgled Igraca");
            TTF_SizeText(font[0], (const char*)"Izgled Zidova", &x[3], &y[3]);
            PisiTekst(font[0], { 0,0,0,255 }, { (int)(0.1 * MaxWidth), (int)(0.8 * MaxHeight), x[2],y[2] }, (char*)"Izgled Zidova");
        }
        if (LastIndex == -2 || LastIndex == 2) {
            TTF_SetFontSize(font[1], 0.5 * velicina);
            TTF_SizeText(font[1], (const char*)"<", &x[4], &y[4]);
            TTF_SizeText(font[0], (const char*)"Izgled Mumije", &x[0], &y[0]);
            int i = 0, intervalxy = (int)(0.35 * MaxWidth - 0.9 * velicina + 1.5 * x[4] - x[1] / 2) / 32;
            Uint32 ProsloVreme = SDL_GetTicks(), interval = 1000 / 60, TrenutnoVreme;
            double x1[2], y1[2], w1[2], h1[2];
            char adresa[40];
            SDL_Surface* surfaces[64];
            SDL_Texture* textures[65];
            for (int j = 0; j < 32; j++) {
                if (LastIndex == -2)
                    sprintf_s(adresa, "Slike/Mumija/Levo/%d/%d.png", (Izgledi[0] == 0) ? 1 : 0, j);
                else
                    sprintf_s(adresa, "Slike/Mumija/Desno/%d/%d.png", (Izgledi[0] == 0) ? 1 : 0, j);
                surfaces[j] = (SDL_Surface*)IMG_Load(adresa);
                textures[j] = SDL_CreateTextureFromSurface(renderer, surfaces[j]);
                SDL_FreeSurface(surfaces[j]);
                if (LastIndex == -2)
                    sprintf_s(adresa, "Slike/Mumija/Levo/%d/%d.png", Izgledi[0], j);
                else
                    sprintf_s(adresa, "Slike/Mumija/Desno/%d/%d.png", Izgledi[0], j);
                surfaces[j + 32] = (SDL_Surface*)IMG_Load(adresa);
                textures[j + 32] = SDL_CreateTextureFromSurface(renderer, surfaces[j + 32]);
                SDL_FreeSurface(surfaces[j + 32]);
            }
            textures[64] = SDL_CreateTextureFromSurface(renderer, surface[4]);
            x1[0] = (int)(0.2 * MaxWidth + x[1] + 0.4 * velicina);
            y1[0] = (int)(0.5 * MaxHeight + y[0] / 2 - 0.4 * velicina);
            w1[0] = (int)(0.9 * MaxWidth - 0.2 * velicina) - x1[0];
            h1[0] = (int)(0.8 * velicina);
            x1[1] = (int)((1600.0 / MaxWidth) * x1[0]);
            y1[1] = (int)((914.0 / MaxHeight) * y1[0]);
            w1[1] = (int)((1600.0 / MaxWidth) * w1[0]);
            h1[1] = (int)((914.0 / MaxHeight) * h1[0]);
            while (1) {
                TrenutnoVreme = SDL_GetTicks();
                if (TrenutnoVreme - ProsloVreme >= interval) {
                    DeoTeksture(x1[0], y1[0], w1[0], h1[0], x1[1], y1[1], w1[1], h1[1], textures[64]);
                    if (i <= 31)
                        NapraviZid((int)((1.1 * MaxWidth - 2 * x[4] + x[1]) / 2 + ((LastIndex == -2) ? (-1 * intervalxy) : (intervalxy))), (int)(0.5 * MaxHeight + y[0] / 2 - velicina * 0.4), velicina * 0.8, velicina * 0.8, textures[i]);
                    else
                        NapraviZid((int)(((LastIndex == -2) ? (0.9 * MaxWidth - 0.75 * velicina - intervalxy) : (0.2 * MaxWidth + x[1] + x[4] + intervalxy))), (int)(0.5 * MaxHeight + y[0] / 2 - velicina * 0.4), velicina * 0.8, velicina * 0.8, textures[i]);
                    SDL_DestroyTexture(textures[i]);
                    SDL_RenderPresent(renderer);
                    ProsloVreme = TrenutnoVreme;
                    i++;
                    intervalxy += (int)(0.35 * MaxWidth - 0.9 * velicina + 1.5 * x[4] - x[1] / 2) / 32;
                    if (i == 32) intervalxy = (int)(0.35 * MaxWidth - 0.9 * velicina + 1.5 * x[4] - x[1] / 2) / 32;
                    if (i >= 64) {
                        DeoTeksture(x1[0], y1[0], w1[0], h1[0], x1[1], y1[1], w1[1], h1[1], textures[64]);
                        break;
                    }
                }
            }
            for (int j = i; j < 64; j++) {
                if (textures[j]) {
                    SDL_DestroyTexture(textures[j]);
                }
            }
            SDL_DestroyTexture(textures[64]);
        }
        else if (LastIndex == -3 || LastIndex == 3) {
            TTF_SetFontSize(font[1], 0.5 * velicina);
            TTF_SizeText(font[1], (const char*)"<", &x[4], &y[4]);
            TTF_SizeText(font[0], (const char*)"Izgled Mumije", &x[0], &y[0]);
            int i = 0, intervalxy = (int)(0.35 * MaxWidth - velicina + 1.5 * x[4] - x[1] / 2) / 30;
            Uint32 ProsloVreme = SDL_GetTicks(), interval = 1000 / 60, TrenutnoVreme;
            double x1[2], y1[2], w1[2], h1[2];
            char adresa[60];
            SDL_Surface* surfaces[60];
            SDL_Texture* textures[61];
            for (int j = 0; j < 30; j++) {
                if (LastIndex == -3)
                    sprintf_s(adresa, "Slike/Igrac/Levo/%d/walk/%d.png", (Izgledi[1] == 0) ? 1 : 0, j);
                else
                    sprintf_s(adresa, "Slike/Igrac/Desno/%d/walk/%d.png", (Izgledi[1] == 0) ? 1 : 0, j);
                surfaces[j] = (SDL_Surface*)IMG_Load(adresa);
                textures[j] = SDL_CreateTextureFromSurface(renderer, surfaces[j]);
                SDL_FreeSurface(surfaces[j]);
                if (LastIndex == -3)
                    sprintf_s(adresa, "Slike/Igrac/Levo/%d/walk/%d.png", Izgledi[1], j);
                else
                    sprintf_s(adresa, "Slike/Igrac/Desno/%d/walk/%d.png", Izgledi[1], j);
                surfaces[j + 30] = (SDL_Surface*)IMG_Load(adresa);
                textures[j + 30] = SDL_CreateTextureFromSurface(renderer, surfaces[j + 30]);
                SDL_FreeSurface(surfaces[j + 30]);
            }
            textures[60] = SDL_CreateTextureFromSurface(renderer, surface[4]);
            x1[0] = (int)(0.2 * MaxWidth + x[1] + 0.4 * velicina);
            y1[0] = (int)(0.65 * MaxHeight + y[0] / 2 - 0.4 * velicina);
            w1[0] = (int)(0.9 * MaxWidth - 0.2 * velicina) - x1[0];
            h1[0] = (int)(0.8 * velicina);
            x1[1] = (int)((1600.0 / MaxWidth) * x1[0]);
            y1[1] = (int)((914.0 / MaxHeight) * y1[0]);
            w1[1] = (int)((1600.0 / MaxWidth) * w1[0]);
            h1[1] = (int)((914.0 / MaxHeight) * h1[0]);
            int channel = Mix_PlayChannel(-1, zvukovi[6], 0);
            Mix_PlayChannelTimed(channel, zvukovi[6], 1, 1200);
            while (1) {
                TrenutnoVreme = SDL_GetTicks();
                if (TrenutnoVreme - ProsloVreme >= interval) {
                    DeoTeksture(x1[0], y1[0], w1[0], h1[0], x1[1], y1[1], w1[1], h1[1], textures[60]);
                    if (i < 30)
                        NapraviZid((int)((1.1 * MaxWidth - 2 * x[4] + x[1]) / 2 + ((LastIndex == -3) ? (-1 * intervalxy) : (intervalxy))), (int)(0.65 * MaxHeight + y[0] / 2 - velicina * 0.4), velicina * 0.8, velicina * 0.8, textures[i]);
                    else
                        NapraviZid((int)(((LastIndex == -3) ? (0.9 * MaxWidth - 0.8 * velicina - intervalxy) : (0.2 * MaxWidth + x[1] + x[4] + intervalxy))), (int)(0.65 * MaxHeight + y[0] / 2 - velicina * 0.4), velicina * 0.8, velicina * 0.8, textures[i]);
                    SDL_DestroyTexture(textures[i]);
                    SDL_RenderPresent(renderer);
                    ProsloVreme = TrenutnoVreme;
                    i++;
                    intervalxy += (int)(0.35 * MaxWidth - velicina + 1.5 * x[4] - x[1] / 2) / 30;
                    if (i == 30) intervalxy = (int)(0.35 * MaxWidth - velicina + 1.5 * x[4] - x[1] / 2) / 30;
                    if (i >= 60) {
                        DeoTeksture(x1[0], y1[0], w1[0], h1[0], x1[1], y1[1], w1[1], h1[1], textures[60]);
                        break;
                    }
                }
            }
            for (int j = i; j < 60; j++) {
                if (textures[j]) {
                    SDL_DestroyTexture(textures[j]);
                }
            }
            SDL_DestroyTexture(textures[60]);

        }
        else if (LastIndex == -4 || LastIndex == 4) {
            TTF_SetFontSize(font[1], 0.5 * velicina);
            TTF_SizeText(font[1], (const char*)"<", &x[4], &y[4]);
            TTF_SizeText(font[0], (const char*)"Izgled Mumije", &x[0], &y[0]);
            int i = 0, intervalxy = (int)(0.35 * MaxWidth - 1.1 * velicina + 1.5 * x[4] - x[1] / 2) / 32;
            Uint32 ProsloVreme = SDL_GetTicks(), interval = 1000 / 90, TrenutnoVreme;
            double x1[2], y1[2], w1[2], h1[2];
            char adresa[40];
            SDL_Surface* surfaces[2];
            SDL_Texture* textures[3];
            for (int j = 0; j < 2; j++) {
                if (j == 0)
                    sprintf_s(adresa, "Slike/Zid/%d/Zidovi.png", (Izgledi[2] == 0) ? 1 : 0);
                else
                    sprintf_s(adresa, "Slike/Zid/%d/Zidovi.png", Izgledi[2]);
                surfaces[j] = (SDL_Surface*)IMG_Load(adresa);
                textures[j] = SDL_CreateTextureFromSurface(renderer, surfaces[j]);
                SDL_FreeSurface(surfaces[j]);
            }
            textures[2] = SDL_CreateTextureFromSurface(renderer, surface[4]);
            x1[0] = (int)(0.2 * MaxWidth + x[1] + 0.3 * velicina);
            y1[0] = (int)(0.8 * MaxHeight + y[0] / 2 - 0.4 * velicina);
            w1[0] = (int)(0.9 * MaxWidth - 0.2 * velicina) - x1[0];
            h1[0] = (int)(0.8 * velicina);
            x1[1] = (int)((1600.0 / MaxWidth) * x1[0]);
            y1[1] = (int)((914.0 / MaxHeight) * y1[0]);
            w1[1] = (int)((1600.0 / MaxWidth) * w1[0]);
            h1[1] = (int)((914.0 / MaxHeight) * h1[0]);
            int channel = Mix_PlayChannel(-1, zvukovi[6], 0);
            Mix_PlayChannelTimed(channel, zvukovi[6], 1, 1200);
            while (1) {
                TrenutnoVreme = SDL_GetTicks();
                if (TrenutnoVreme - ProsloVreme >= interval) {
                    DeoTeksture(x1[0], y1[0], w1[0], h1[0], x1[1], y1[1], w1[1], h1[1], textures[2]);
                    if (i <= 31)
                        NapraviZid((int)((1.1 * MaxWidth - 2 * x[4] + x[1]) / 2 + ((LastIndex == -4) ? (-1 * intervalxy) : (intervalxy))), (int)(0.8 * MaxHeight + y[0] / 2 - velicina * 0.4), velicina * 0.8, velicina * 0.8, textures[0]);
                    else
                        NapraviZid((int)(((LastIndex == -4) ? (0.9 * MaxWidth - velicina - intervalxy) : (0.2 * MaxWidth + x[1] + x[4] + intervalxy))), (int)(0.8 * MaxHeight + y[0] / 2 - velicina * 0.4), velicina * 0.8, velicina * 0.8, textures[1]);
                    SDL_RenderPresent(renderer);
                    ProsloVreme = TrenutnoVreme;
                    i++;
                    intervalxy += (int)(0.35 * MaxWidth - 1.1 * velicina + 1.5 * x[4] - x[1] / 2) / 32;
                    if (i == 32) {
                        SDL_DestroyTexture(textures[0]);
                        intervalxy = (int)(0.35 * MaxWidth - 1.1 * velicina + 1.5 * x[4] - x[1] / 2) / 32;
                    }
                    if (i >= 64) {
                        SDL_DestroyTexture(textures[1]);
                        DeoTeksture(x1[0], y1[0], w1[0], h1[0], x1[1], y1[1], w1[1], h1[1], textures[2]);
                        break;
                    }
                }
            }
            SDL_DestroyTexture(textures[2]);
        }
        sprintf_s(lokacija[0], "Slike/Mumija/Desno/%d/MumijaDesno.png", Izgledi[0]);
        sprintf_s(lokacija[1], "Slike/Igrac/Nazad/%d/walk/0.png", Izgledi[1]);
        sprintf_s(lokacija[2], "Slike/Zid/%d/Zidovi.png", Izgledi[2]);
        SDL_Surface* skinovi[3] = {
            (SDL_Surface*)IMG_Load(lokacija[0]),
            (SDL_Surface*)IMG_Load(lokacija[1]),
            (SDL_Surface*)IMG_Load(lokacija[2])
        };
        SDL_Texture* skin[3];







        TTF_SetFontSize(font[0], 0.5 * velicina);
        TTF_SizeText(font[0], (const char*)"Izgled Mumije", &x[0], &y[0]);
        TTF_SizeText(font[0], (const char*)"Jacina zvuka efekata", &x[1], &y[1]);
        TTF_SetFontSize(font[1], 0.5 * velicina);
        TTF_SizeText(font[1], (const char*)"<", &x[4], &y[4]);





        for (int i = 0; i < 3; i++) {
            skin[i] = SDL_CreateTextureFromSurface(renderer, skinovi[i]);
            SDL_FreeSurface(skinovi[i]);
            NapraviZid((int)(1.1 * MaxWidth - 2 * x[4] + x[1]) / 2, (int)(0.5 * MaxHeight + i * 0.15 * MaxHeight + y[0] / 2 - velicina * 0.4), velicina * 0.8, velicina * 0.8, skin[i]);
            PisiTekst(font[1], { 57,43,33,255 }, { (int)(0.9 * MaxWidth - x[4] / 2), (int)(0.5 * MaxHeight + i * 0.15 * MaxHeight + y[0] / 2 - y[4] / 2), x[4],y[4] }, (char*)">");
            PisiTekst(font[1], { 57,43,33,255 }, { (int)(0.2 * MaxWidth + x[1] - x[4] / 2), (int)(0.5 * MaxHeight + i * 0.15 * MaxHeight + y[0] / 2 - y[4] / 2), x[4],y[4] }, (char*)"<");
            SDL_DestroyTexture(skin[i]);
        }
    }
    else if (index == 1 && LastIndex == 0) {
        SDL_Texture* nazad = SDL_CreateTextureFromSurface(renderer, surface[6]);
        NapraviZid(0.03 * MaxWidth, 0.03 * MaxHeight, velicina, velicina, nazad);
        SDL_DestroyTexture(nazad);
    }
    else if (index == 0 && LastIndex == 1) {
        SDL_Texture* nazad = SDL_CreateTextureFromSurface(renderer, surface[5]);
        NapraviZid(0.03 * MaxWidth, 0.03 * MaxHeight, velicina, velicina, nazad);
        SDL_DestroyTexture(nazad);
    }
    else if (index == 2) {
        float Puno, Trenutno;
        char zvuk[10];
        TTF_SetFontSize(font[0], 0.5 * velicina);
        TTF_SizeText(font[0], (const char*)"Jacina zvuka efekata", &x[1], &y[1]);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        if (Xmis <= (int)(0.2 * MaxWidth + x[1]))
            Xmis = (int)(0.2 * MaxWidth + x[1] + 1);
        else if (Xmis > 0.9 * MaxWidth) {
            Xmis = 0.9 * MaxWidth;
        }
        Puno = 0.7 * MaxWidth - x[1];
        Trenutno = 0.9 * MaxWidth - Xmis;
        SDL_SetRenderDrawColor(renderer, 142, 126, 103, 255);
        ObojiZid(Xmis, (int)(0.2 * MaxHeight + y[1] / 2 - 0.13 * velicina), Trenutno, 0.27 * velicina + 1);
        SDL_RenderFillCircle(0.9 * MaxWidth, (int)(0.2 * MaxHeight + y[1] / 2), 0.13 * velicina);
        SDL_SetRenderDrawColor(renderer, 99, 78, 61, 255);
        ObojiZid(0.2 * MaxWidth + x[1], (int)(0.2 * MaxHeight + y[1] / 2 - 0.13 * velicina), Puno - Trenutno, 0.27 * velicina + 1);
        SDL_RenderFillCircle(0.2 * MaxWidth + x[1], (int)(0.2 * MaxHeight + y[1] / 2), 0.13 * velicina);
        SDL_RenderFillCircle(Xmis, (int)(0.2 * MaxHeight + y[1] / 2), 0.13 * velicina);
        SDL_SetRenderDrawColor(renderer, 57, 43, 33, 255);
        SDL_RenderDrawCircle(Xmis, (int)(0.2 * MaxHeight + y[1] / 2), 0.13 * velicina);
        Zvukovi = (int)(((Puno - Trenutno) / Puno) * 100);
        sprintf_s(zvuk, "%d", Zvukovi);
        strcat_s(zvuk, "%");
        TTF_SetFontSize(font[1], 0.26 * velicina);
        TTF_SizeText(font[1], (const char*)zvuk, &x[2], &y[2]);
        PisiTekst(font[1], { 0,0,0,255 }, { (int)(0.9 * MaxWidth - x[2]), (int)(0.2 * MaxHeight + y[1] / 2 - y[2] / 2), x[2],y[2] }, (char*)zvuk);
        for (int i = 0; i < 6; i++) {
            Mix_VolumeChunk(zvukovi[i], Zvukovi);
        }
    }
    else if (index == 3) {
        float Puno, Trenutno;
        char zvuk[10];
        TTF_SetFontSize(font[0], 0.5 * velicina);
        TTF_SizeText(font[0], (const char*)"Jacina zvuka efekata", &x[1], &y[1]);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        if (Xmis <= (int)(0.2 * MaxWidth + x[1]))
            Xmis = (int)(0.2 * MaxWidth + x[1] + 1);
        else if (Xmis > 0.9 * MaxWidth) {
            Xmis = 0.9 * MaxWidth;
        }
        Puno = 0.7 * MaxWidth - x[1];
        Trenutno = 0.9 * MaxWidth - Xmis;
        SDL_SetRenderDrawColor(renderer, 142, 126, 103, 255);
        ObojiZid(Xmis, (int)(0.35 * MaxHeight + y[1] / 2 - 0.13 * velicina), Trenutno, 0.27 * velicina + 1);
        SDL_RenderFillCircle(0.9 * MaxWidth, (int)(0.35 * MaxHeight + y[1] / 2), 0.13 * velicina);
        SDL_SetRenderDrawColor(renderer, 99, 78, 61, 255);
        ObojiZid(0.2 * MaxWidth + x[1], (int)(0.35 * MaxHeight + y[1] / 2 - 0.13 * velicina), Puno - Trenutno, 0.27 * velicina + 1);
        SDL_RenderFillCircle(0.2 * MaxWidth + x[1], (int)(0.35 * MaxHeight + y[1] / 2), 0.13 * velicina);
        SDL_RenderFillCircle(Xmis, (int)(0.35 * MaxHeight + y[1] / 2), 0.13 * velicina);
        SDL_SetRenderDrawColor(renderer, 57, 43, 33, 255);
        SDL_RenderDrawCircle(Xmis, (int)(0.35 * MaxHeight + y[1] / 2), 0.13 * velicina);
        Pozadinska = (int)(((Puno - Trenutno) / Puno) * 100);
        sprintf_s(zvuk, "%d", Pozadinska);
        strcat_s(zvuk, "%");
        TTF_SetFontSize(font[1], 0.26 * velicina);
        TTF_SizeText(font[1], (const char*)zvuk, &x[2], &y[2]);
        PisiTekst(font[1], { 0,0,0,255 }, { (int)(0.9 * MaxWidth - x[2]), (int)(0.35 * MaxHeight + y[1] / 2 - y[2] / 2), x[2], y[2] }, (char*)zvuk);
        Mix_VolumeMusic(Pozadinska);
    }
    else if (index == 5) {
        PisiTekst(font[1], { 255,255,255,255 }, { (int)(0.2 * MaxWidth + x[1] - x[4] / 2), (int)(0.5 * MaxHeight + y[0] / 2 - y[4] / 2), x[4],y[4] }, (char*)"<");
    }
    else if (index == 6) {
        PisiTekst(font[1], { 255,255,255,255 }, { (int)(0.9 * MaxWidth - x[4] / 2), (int)(0.5 * MaxHeight + y[0] / 2 - y[4] / 2), x[4],y[4] }, (char*)">");
    }
    else if (index == 7) {
        PisiTekst(font[1], { 255,255,255,255 }, { (int)(0.2 * MaxWidth + x[1] - x[4] / 2), (int)(0.65 * MaxHeight + y[0] / 2 - y[4] / 2), x[4],y[4] }, (char*)"<");
    }
    else if (index == 8) {
        PisiTekst(font[1], { 255,255,255,255 }, { (int)(0.9 * MaxWidth - x[4] / 2), (int)(0.65 * MaxHeight + y[0] / 2 - y[4] / 2), x[4],y[4] }, (char*)">");
    }
    else if (index == 9) {
        PisiTekst(font[1], { 255,255,255,255 }, { (int)(0.2 * MaxWidth + x[1] - x[4] / 2), (int)(0.8 * MaxHeight + y[0] / 2 - y[4] / 2), x[4],y[4] }, (char*)"<");
    }
    else if (index == 10) {
        PisiTekst(font[1], { 255,255,255,255 }, { (int)(0.9 * MaxWidth - x[4] / 2), (int)(0.8 * MaxHeight + y[0] / 2 - y[4] / 2), x[4],y[4] }, (char*)">");
    }
    else if (index == 11) {
        for (int i = 0; i < 3; i++) {
            PisiTekst(font[1], { 0,0,0,255 }, { (int)(0.9 * MaxWidth - x[4] / 2), (int)(0.5 * MaxHeight + i * 0.15 * MaxHeight + y[0] / 2 - y[4] / 2), x[4],y[4] }, (char*)">");
            PisiTekst(font[1], { 0,0,0,255 }, { (int)(0.2 * MaxWidth + x[1] - x[4] / 2), (int)(0.5 * MaxHeight + i * 0.15 * MaxHeight + y[0] / 2 - y[4] / 2), x[4],y[4] }, (char*)"<");
        }
    }
}
void ZvukIzgled(SDL_Surface** surface) {
    int radi = 1, x[4], y[4], index = 0;
    char adresa[50];
    CrtajZvukIzgled(surface, 0, -1);
    SDL_RenderPresent(renderer);
    TTF_SetFontSize(font[0], 0.5 * velicina);
    TTF_SizeText(font[0], (const char*)"Jacina zvuka efekata", &x[1], &y[1]);
    TTF_SetFontSize(font[1], 0.5 * velicina);
    TTF_SizeText(font[0], (const char*)"Izgled Mumije", &x[2], &y[2]);
    TTF_SizeText(font[1], (const char*)">", &x[3], &y[3]);
    while (radi) {
        while (SDL_PollEvent(&event) != 0) {
            SDL_GetMouseState(&x[0], &y[0]);
            if (event.type == SDL_QUIT) {
                SDL_Quit();
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN) {
                SDL_GetMouseState(&x[0], &y[0]);
                if (x[0] >= (int)(0.2 * MaxWidth + x[1] - 0.065 * velicina) && x[0] <= (int)(0.9 * MaxWidth + 0.065 * velicina) && ((y[0] >= (int)(0.2 * MaxHeight + y[1] / 2 - 0.13 * velicina) && y[0] <= (0.2 * MaxHeight + y[1] / 2 + 0.13 * velicina)) || (y[0] >= (int)(0.35 * MaxHeight + y[1] / 2 - 0.13 * velicina) && y[0] <= (0.35 * MaxHeight + y[1] / 2 + 0.13 * velicina)))) {
                    int LastIndex = (y[0] >= (int)(0.2 * MaxHeight + y[1] / 2 - 0.13 * velicina) && y[0] <= (0.2 * MaxHeight + y[1] / 2 + 0.13 * velicina)) ? 2 : 3;
                    PustiZvuk(zvukovi[1]);
                    while (event.type != SDL_MOUSEBUTTONUP) {
                        SDL_PollEvent(&event);
                        SDL_GetMouseState(&x[0], &y[0]);
                        CrtajZvukIzgled(surface, LastIndex, 5, x[0]);
                        SDL_RenderPresent(renderer);
                    }
                }
                else if ((x[0] >= (0.2 * MaxWidth + x[1] - x[3] / 2) && x[0] <= (0.2 * MaxWidth + x[1] + x[3] / 2) && y[0] >= (0.5 * MaxHeight + y[2] / 2 - y[3] / 2) && y[0] <= (0.5 * MaxHeight + y[2] / 2 + y[3] / 2)) || (x[0] >= (0.9 * MaxWidth - x[3] / 2) && x[0] <= (0.9 * MaxWidth + x[3] / 2) && y[0] >= (0.5 * MaxHeight + y[2] / 2 - y[3] / 2) && y[0] <= (0.5 * MaxHeight + y[2] / 2 + y[3] / 2))) {
                    PustiZvuk(zvukovi[1]);
                    Izgledi[0] = (Izgledi[0] == 0) ? 1 : 0;
                    int LastIndex = (x[0] >= (int)(0.2 * MaxWidth + x[1] - x[3] / 2) && x[0] <= (int)(0.2 * MaxWidth + x[1] + x[3] / 2)) ? (2 * (-1)) : 2;
                    int channel = Mix_PlayChannel(-1, zvukovi[6], 0);
                    Mix_PlayChannelTimed(channel, zvukovi[6], 1, 1200);
                    CrtajZvukIzgled(surface, 0, LastIndex);
                    SDL_RenderPresent(renderer);
                    FILE* file = NULL;
                    fopen_s(&file, (const char*)"Opcije.txt", (const char*)"w");
                    fprintf_s(file, "%d %d\n%d\n%d %d %d\n", MaxWidth, MaxHeight, n, Tezina, Algoritam, BrZombija);
                    fprintf_s(file, "%d %d %d\n", Izgledi[0], Izgledi[1], Izgledi[2]);
                    fprintf_s(file, "%d %d %d\n%d %d", TrenutniNivo, mapa, Sacuvaj, Zvukovi, Pozadinska);
                    Mix_VolumeMusic(Pozadinska);
                    for (int i = 0; i < 7; i++) {
                        Mix_VolumeChunk(zvukovi[i], Zvukovi);
                    }
                    fclose(file);
                }
                else if ((x[0] >= (0.2 * MaxWidth + x[1] - x[3] / 2) && x[0] <= (0.2 * MaxWidth + x[1] + x[3] / 2) && y[0] >= (0.65 * MaxHeight + y[2] / 2 - y[3] / 2) && y[0] <= (0.65 * MaxHeight + y[2] / 2 + y[3] / 2)) || (x[0] >= (0.9 * MaxWidth - x[3] / 2) && x[0] <= (0.9 * MaxWidth + x[3] / 2) && y[0] >= (0.65 * MaxHeight + y[2] / 2 - y[3] / 2) && y[0] <= (0.65 * MaxHeight + y[2] / 2 + y[3] / 2))) {
                    PustiZvuk(zvukovi[1]);
                    Izgledi[1] = (Izgledi[1] == 0) ? 1 : 0;
                    int LastIndex = (x[0] >= (int)(0.2 * MaxWidth + x[1] - x[3] / 2) && x[0] <= (int)(0.2 * MaxWidth + x[1] + x[3] / 2)) ? (3 * (-1)) : 3;
                    int channel = Mix_PlayChannel(-1, zvukovi[6], 0);
                    Mix_PlayChannelTimed(channel, zvukovi[6], 1, 1200);
                    CrtajZvukIzgled(surface, 0, LastIndex);
                    SDL_RenderPresent(renderer);
                    FILE* file = NULL;
                    fopen_s(&file, (const char*)"Opcije.txt", (const char*)"w");
                    fprintf_s(file, "%d %d\n%d\n%d %d %d\n", MaxWidth, MaxHeight, n, Tezina, Algoritam, BrZombija);
                    fprintf_s(file, "%d %d %d\n", Izgledi[0], Izgledi[1], Izgledi[2]);
                    fprintf_s(file, "%d %d %d\n%d %d", TrenutniNivo, mapa, Sacuvaj, Zvukovi, Pozadinska);
                    Mix_VolumeMusic(Pozadinska);
                    for (int i = 0; i < 7; i++) {
                        Mix_VolumeChunk(zvukovi[i], Zvukovi);
                    }
                    fclose(file);
                }
                else if ((x[0] >= (0.2 * MaxWidth + x[1] - x[3] / 2) && x[0] <= (0.2 * MaxWidth + x[1] + x[3] / 2) && y[0] >= (0.8 * MaxHeight + y[2] / 2 - y[3] / 2) && y[0] <= (0.8 * MaxHeight + y[2] / 2 + y[3] / 2)) || (x[0] >= (0.9 * MaxWidth - x[3] / 2) && x[0] <= (0.9 * MaxWidth + x[3] / 2) && y[0] >= (0.8 * MaxHeight + y[2] / 2 - y[3] / 2) && y[0] <= (0.8 * MaxHeight + y[2] / 2 + y[3] / 2))) {
                    PustiZvuk(zvukovi[1]);
                    Izgledi[2] = (Izgledi[2] == 0) ? 1 : 0;
                    int LastIndex = (x[0] >= (int)(0.2 * MaxWidth + x[1] - x[3] / 2) && x[0] <= (int)(0.2 * MaxWidth + x[1] + x[3] / 2)) ? (4 * (-1)) : 4;
                    CrtajZvukIzgled(surface, 0, LastIndex);
                    SDL_RenderPresent(renderer);
                    FILE* file = NULL;
                    fopen_s(&file, (const char*)"Opcije.txt", (const char*)"w");
                    fprintf_s(file, "%d %d\n%d\n%d %d %d\n", MaxWidth, MaxHeight, n, Tezina, Algoritam, BrZombija);
                    fprintf_s(file, "%d %d %d\n", Izgledi[0], Izgledi[1], Izgledi[2]);
                    fprintf_s(file, "%d %d %d\n%d %d", TrenutniNivo, mapa, Sacuvaj, Zvukovi, Pozadinska);
                    Mix_VolumeMusic(Pozadinska);
                    for (int i = 0; i < 7; i++) {
                        Mix_VolumeChunk(zvukovi[i], Zvukovi);
                    }
                    fclose(file);
                    SDL_FreeSurface(surface[7]);
                    SDL_FreeSurface(surface[8]);
                    SDL_FreeSurface(surface[9]);
                    SDL_FreeSurface(surface[10]);
                    SDL_FreeSurface(surface[15]);
                    SDL_FreeSurface(surface[16]);
                    sprintf_s(adresa, "Slike/Zid/%d/LeviTalon.png", Izgledi[2]);
                    surface[7] = (SDL_Surface*)IMG_Load(adresa);
                    sprintf_s(adresa, "Slike/Zid/%d/DesniTalon.png", Izgledi[2]);
                    surface[8] = (SDL_Surface*)IMG_Load(adresa);
                    sprintf_s(adresa, "Slike/Zid/%d/GoreTalon.png", Izgledi[2]);
                    surface[9] = (SDL_Surface*)IMG_Load(adresa);
                    sprintf_s(adresa, "Slike/Zid/%d/DoleTalon.png", Izgledi[2]);
                    surface[10] = (SDL_Surface*)IMG_Load(adresa);
                    sprintf_s(adresa, "Slike/Zid/%d/VertikalniZid.png", Izgledi[2]);
                    surface[15] = (SDL_Surface*)IMG_Load(adresa);
                    sprintf_s(adresa, "Slike/Zid/%d/HorizontalniZid.png", Izgledi[2]);
                    surface[16] = (SDL_Surface*)IMG_Load(adresa);
                }
                else if (x[0] >= 0.03 * MaxWidth && x[0] <= (0.03 * MaxWidth + velicina) && y[0] >= 0.03 * MaxHeight && y[0] <= (0.03 * MaxHeight + velicina)) {
                    PustiZvuk(zvukovi[1]);
                    PustiZvuk(zvukovi[0]);
                    PrelazNaOpcije(surface, 0, 0);
                }
            }
            else {
                if (x[0] >= 0.03 * MaxWidth && x[0] <= (0.03 * MaxWidth + velicina) && y[0] >= 0.03 * MaxHeight && y[0] <= (0.03 * MaxHeight + velicina)) {
                    CrtajZvukIzgled(surface, 1, index);
                    index = 1;
                    SDL_RenderPresent(renderer);
                }
                else if (x[0] >= (0.2 * MaxWidth + x[1] - x[3] / 2) && x[0] <= (0.2 * MaxWidth + x[1] + x[3] / 2) && y[0] >= (0.5 * MaxHeight + y[2] / 2 - y[3] / 2) && y[0] <= (0.5 * MaxHeight + y[2] / 2 + y[3] / 2)) {
                    index = 5;
                    CrtajZvukIzgled(surface, index, 0);
                    SDL_RenderPresent(renderer);
                }
                else if (x[0] >= (0.9 * MaxWidth - x[3] / 2) && x[0] <= (0.9 * MaxWidth + x[3] / 2) && y[0] >= (0.5 * MaxHeight + y[2] / 2 - y[3] / 2) && y[0] <= (0.5 * MaxHeight + y[2] / 2 + y[3] / 2)) {
                    index = 6;
                    CrtajZvukIzgled(surface, index, 0);
                    SDL_RenderPresent(renderer);
                }
                else if (x[0] >= (0.2 * MaxWidth + x[1] - x[3] / 2) && x[0] <= (0.2 * MaxWidth + x[1] + x[3] / 2) && y[0] >= (0.65 * MaxHeight + y[2] / 2 - y[3] / 2) && y[0] <= (0.65 * MaxHeight + y[2] / 2 + y[3] / 2)) {
                    index = 7;
                    CrtajZvukIzgled(surface, index, 0);
                    SDL_RenderPresent(renderer);
                }
                else if (x[0] >= (0.9 * MaxWidth - x[3] / 2) && x[0] <= (0.9 * MaxWidth + x[3] / 2) && y[0] >= (0.65 * MaxHeight + y[2] / 2 - y[3] / 2) && y[0] <= (0.65 * MaxHeight + y[2] / 2 + y[3] / 2)) {
                    index = 8;
                    CrtajZvukIzgled(surface, index, 0);
                    SDL_RenderPresent(renderer);
                }
                else if (x[0] >= (0.2 * MaxWidth + x[1] - x[3] / 2) && x[0] <= (0.2 * MaxWidth + x[1] + x[3] / 2) && y[0] >= (0.8 * MaxHeight + y[2] / 2 - y[3] / 2) && y[0] <= (0.8 * MaxHeight + y[2] / 2 + y[3] / 2)) {
                    index = 9;
                    CrtajZvukIzgled(surface, index, 0);
                    SDL_RenderPresent(renderer);
                }
                else if (x[0] >= (0.9 * MaxWidth - x[3] / 2) && x[0] <= (0.9 * MaxWidth + x[3] / 2) && y[0] >= (0.8 * MaxHeight + y[2] / 2 - y[3] / 2) && y[0] <= (0.8 * MaxHeight + y[2] / 2 + y[3] / 2)) {
                    index = 10;
                    CrtajZvukIzgled(surface, index, 0);
                    SDL_RenderPresent(renderer);
                }
                else if (index > 4 && index != 11) {
                    index = 11;
                    CrtajZvukIzgled(surface, index, 0);
                    SDL_RenderPresent(renderer);
                }
                else if (index != 0) {
                    CrtajZvukIzgled(surface, 0, index);
                    index = 0;
                    SDL_RenderPresent(renderer);
                }
            }
        }
    }
}
void CrtajOpcije(SDL_Surface** surface, int index, int LastIndex) {
    int x[13], y[3];
    char width[10], height[5], n1[3], algoritam[3], brzombija[3], tezina[3][10] = { "lako","normalno", "tesko" };
    sprintf_s(width, "%dx%d", MaxWidth, MaxHeight);
    sprintf_s(n1, "%d", n);
    sprintf_s(algoritam, "%d", Algoritam);
    sprintf_s(brzombija, "%d", BrZombija);
    TTF_SetFontSize(font[0], velicina / 2);
    TTF_SetFontSize(font[1], velicina / 2);
    TTF_SizeText(font[1], (const char*)"<", &x[11], &y[2]);
    TTF_SizeText(font[0], (const char*)"Rezolucija", &x[1], &y[1]);
    TTF_SizeText(font[0], (const char*)width, &x[2], &y[1]);
    TTF_SizeText(font[0], (const char*)"Velicina talona", &x[3], &y[1]);
    TTF_SizeText(font[0], (const char*)n1, &x[4], &y[1]);
    TTF_SizeText(font[0], (const char*)"Tezina nivoa", &x[5], &y[1]);
    TTF_SizeText(font[0], (const char*)tezina[Tezina], &x[6], &y[1]);
    TTF_SizeText(font[0], (const char*)"Algoritam lavirinta", &x[7], &y[1]);
    TTF_SizeText(font[0], (const char*)algoritam, &x[8], &y[1]);
    TTF_SizeText(font[0], (const char*)"Broj zombija", &x[9], &y[1]);
    TTF_SizeText(font[0], (const char*)brzombija, &x[10], &y[1]);
    TTF_SizeText(font[0], (const char*)"Podesavanje izgleda i zvuka", &x[12], &y[1]);
    if (index == 0 && LastIndex == -1) {
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface[4]);
        SDL_Texture* nazad = SDL_CreateTextureFromSurface(renderer, surface[5]);
        NapraviZid(0, 0, MaxWidth, MaxHeight, texture);
        NapraviZid(0.03 * MaxWidth, 0.03 * MaxHeight, velicina, velicina, nazad);
        SDL_DestroyTexture(texture);
        SDL_DestroyTexture(nazad);
        TTF_SetFontSize(font[0], velicina);
        TTF_SizeText(font[0], (const char*)"Opcije", &x[0], &y[0]);
        PisiTekst(font[0], { 0,0,0,255 }, { (MaxWidth - x[0]) / 2, (int)(0.03 * MaxHeight), (int)(0.5 * MaxWidth), (int)(0.2 * MaxHeight) }, (char*)"Opcije");
        TTF_SetFontSize(font[0], velicina / 2);
        PisiTekst(font[0], { 0,0,0,255 }, { (int)(0.15 * MaxWidth), (int)(0.25 * MaxHeight), (int)(0.5 * MaxWidth), (int)(0.2 * MaxHeight) }, (char*)"Rezolucija");
        PisiTekst(font[0], { 0,0,0,255 }, { (int)(1.05 * MaxWidth + x[1] - x[2]) / 2, (int)(0.25 * MaxHeight), (int)(0.5 * MaxWidth), (int)(0.2 * MaxHeight) }, (char*)width);
        PisiTekst(font[1], { 0,0,0,255 }, { (int)(0.675 * MaxWidth + 3 * x[1] / 2 - x[2] / 2 - x[11]) / 2, (int)(0.25 * MaxHeight), (int)(0.5 * MaxWidth), (int)(0.2 * MaxHeight) }, (char*)"<");
        PisiTekst(font[1], { 0,0,0,255 }, { (int)(1.425 * MaxWidth + x[1] / 2 + x[2] / 2 - x[11]) / 2, (int)(0.25 * MaxHeight), (int)(0.5 * MaxWidth), (int)(0.2 * MaxHeight) }, (char*)">");
        PisiTekst(font[0], { 0,0,0,255 }, { (int)(0.15 * MaxWidth), (int)(0.36 * MaxHeight), (int)(0.5 * MaxWidth), (int)(0.2 * MaxHeight) }, (char*)"Velicina talona");
        PisiTekst(font[0], { 0,0,0,255 }, { (int)(1.05 * MaxWidth + x[3] - x[4]) / 2, (int)(0.36 * MaxHeight), (int)(0.5 * MaxWidth), (int)(0.2 * MaxHeight) }, (char*)n1);
        PisiTekst(font[1], { 0,0,0,255 }, { (int)(0.675 * MaxWidth + 3 * x[3] / 2 - x[4] / 2 - x[11]) / 2, (int)(0.36 * MaxHeight), (int)(0.5 * MaxWidth), (int)(0.2 * MaxHeight) }, (char*)"<");
        PisiTekst(font[1], { 0,0,0,255 }, { (int)(1.425 * MaxWidth + x[3] / 2 + x[4] / 2 - x[11]) / 2, (int)(0.36 * MaxHeight), (int)(0.5 * MaxWidth), (int)(0.2 * MaxHeight) }, (char*)">");
        PisiTekst(font[0], { 0,0,0,255 }, { (int)(0.15 * MaxWidth), (int)(0.47 * MaxHeight), (int)(0.5 * MaxWidth), (int)(0.2 * MaxHeight) }, (char*)"Tezina nivoa");
        PisiTekst(font[0], { 0,0,0,255 }, { (int)(1.05 * MaxWidth + x[5] - x[6]) / 2, (int)(0.47 * MaxHeight), (int)(0.5 * MaxWidth), (int)(0.2 * MaxHeight) }, (char*)Tezina[tezina]);
        PisiTekst(font[1], { 0,0,0,255 }, { (int)(0.675 * MaxWidth + 3 * x[5] / 2 - x[6] / 2 - x[11]) / 2, (int)(0.47 * MaxHeight), (int)(0.5 * MaxWidth), (int)(0.2 * MaxHeight) }, (char*)"<");
        PisiTekst(font[1], { 0,0,0,255 }, { (int)(1.425 * MaxWidth + x[5] / 2 + x[6] / 2 - x[11]) / 2, (int)(0.47 * MaxHeight), (int)(0.5 * MaxWidth), (int)(0.2 * MaxHeight) }, (char*)">");
        PisiTekst(font[0], { 0,0,0,255 }, { (int)(0.15 * MaxWidth), (int)(0.58 * MaxHeight), (int)(0.5 * MaxWidth), (int)(0.2 * MaxHeight) }, (char*)"Algoritam lavirinta");
        PisiTekst(font[0], { 0,0,0,255 }, { (int)(1.05 * MaxWidth + x[7] - x[8]) / 2, (int)(0.58 * MaxHeight), (int)(0.5 * MaxWidth), (int)(0.2 * MaxHeight) }, (char*)algoritam);
        PisiTekst(font[1], { 0,0,0,255 }, { (int)(0.675 * MaxWidth + 3 * x[7] / 2 - x[8] / 2 - x[11]) / 2, (int)(0.58 * MaxHeight), (int)(0.5 * MaxWidth), (int)(0.2 * MaxHeight) }, (char*)"<");
        PisiTekst(font[1], { 0,0,0,255 }, { (int)(1.425 * MaxWidth + x[7] / 2 + x[8] / 2 - x[11]) / 2, (int)(0.58 * MaxHeight), (int)(0.5 * MaxWidth), (int)(0.2 * MaxHeight) }, (char*)">");
        PisiTekst(font[0], { 0,0,0,255 }, { (int)(0.15 * MaxWidth), (int)(0.69 * MaxHeight), (int)(0.5 * MaxWidth), (int)(0.2 * MaxHeight) }, (char*)"Broj zombija");
        PisiTekst(font[0], { 0,0,0,255 }, { (int)(1.05 * MaxWidth + x[9] - x[10]) / 2, (int)(0.69 * MaxHeight), (int)(0.5 * MaxWidth), (int)(0.2 * MaxHeight) }, (char*)brzombija);
        PisiTekst(font[1], { 0,0,0,255 }, { (int)(0.675 * MaxWidth + 3 * x[9] / 2 - x[10] / 2 - x[11]) / 2, (int)(0.69 * MaxHeight), (int)(0.5 * MaxWidth), (int)(0.2 * MaxHeight) }, (char*)"<");
        PisiTekst(font[1], { 0,0,0,255 }, { (int)(1.425 * MaxWidth + x[9] / 2 + x[10] / 2 - x[11]) / 2, (int)(0.69 * MaxHeight), (int)(0.5 * MaxWidth), (int)(0.2 * MaxHeight) }, (char*)">");
        PisiTekst(font[0], { 0,0,0,255 }, { (int)(MaxWidth - x[12]) / 2, (int)(0.8 * MaxHeight), (int)(0.5 * MaxWidth), (int)(0.2 * MaxHeight) }, (char*)"Podesavanje izgleda i zvuka");
    }
    else if (LastIndex == 0 && index == 1) PisiTekst(font[1], { 255,255,255,255 }, { (int)(0.675 * MaxWidth + 3 * x[1] / 2 - x[2] / 2 - x[11]) / 2, (int)(0.25 * MaxHeight), (int)(0.5 * MaxWidth), (int)(0.2 * MaxHeight) }, (char*)"<");
    else if (LastIndex == 0 && index == 2) PisiTekst(font[1], { 255,255,255,255 }, { (int)(1.425 * MaxWidth + x[1] / 2 + x[2] / 2 - x[11]) / 2, (int)(0.25 * MaxHeight), (int)(0.5 * MaxWidth), (int)(0.2 * MaxHeight) }, (char*)">");
    else if (LastIndex == 0 && index == 3) PisiTekst(font[1], { 255,255,255,255 }, { (int)(0.675 * MaxWidth + 3 * x[3] / 2 - x[4] / 2 - x[11]) / 2, (int)(0.36 * MaxHeight), (int)(0.5 * MaxWidth), (int)(0.2 * MaxHeight) }, (char*)"<");
    else if (LastIndex == 0 && index == 4) PisiTekst(font[1], { 255,255,255,255 }, { (int)(1.425 * MaxWidth + x[3] / 2 + x[4] / 2 - x[11]) / 2, (int)(0.36 * MaxHeight), (int)(0.5 * MaxWidth), (int)(0.2 * MaxHeight) }, (char*)">");
    else if (LastIndex == 0 && index == 5) PisiTekst(font[1], { 255,255,255,255 }, { (int)(0.675 * MaxWidth + 3 * x[5] / 2 - x[6] / 2 - x[11]) / 2, (int)(0.47 * MaxHeight), (int)(0.5 * MaxWidth), (int)(0.2 * MaxHeight) }, (char*)"<");
    else if (LastIndex == 0 && index == 6) PisiTekst(font[1], { 255,255,255,255 }, { (int)(1.425 * MaxWidth + x[5] / 2 + x[6] / 2 - x[11]) / 2, (int)(0.47 * MaxHeight), (int)(0.5 * MaxWidth), (int)(0.2 * MaxHeight) }, (char*)">");
    else if (LastIndex == 0 && index == 7) PisiTekst(font[1], { 255,255,255,255 }, { (int)(0.675 * MaxWidth + 3 * x[7] / 2 - x[8] / 2 - x[11]) / 2, (int)(0.58 * MaxHeight), (int)(0.5 * MaxWidth), (int)(0.2 * MaxHeight) }, (char*)"<");
    else if (LastIndex == 0 && index == 8) PisiTekst(font[1], { 255,255,255,255 }, { (int)(1.425 * MaxWidth + x[7] / 2 + x[8] / 2 - x[11]) / 2, (int)(0.58 * MaxHeight), (int)(0.5 * MaxWidth), (int)(0.2 * MaxHeight) }, (char*)">");
    else if (LastIndex == 0 && index == 9) PisiTekst(font[1], { 255,255,255,255 }, { (int)(0.675 * MaxWidth + 3 * x[9] / 2 - x[10] / 2 - x[11]) / 2, (int)(0.69 * MaxHeight), (int)(0.5 * MaxWidth), (int)(0.2 * MaxHeight) }, (char*)"<");
    else if (LastIndex == 0 && index == 10) PisiTekst(font[1], { 255,255,255,255 }, { (int)(1.425 * MaxWidth + x[9] / 2 + x[10] / 2 - x[11]) / 2, (int)(0.69 * MaxHeight), (int)(0.5 * MaxWidth), (int)(0.2 * MaxHeight) }, (char*)">");
    else if (LastIndex == 0 && index == 11) PisiTekst(font[0], { 255,255,255,255 }, { (int)(MaxWidth - x[12]) / 2, (int)(0.8 * MaxHeight), (int)(0.5 * MaxWidth), (int)(0.2 * MaxHeight) }, (char*)"Podesavanje izgleda i zvuka");
    else if (LastIndex == 0 && index == 12) {
        SDL_Texture* nazad = SDL_CreateTextureFromSurface(renderer, surface[6]);
        NapraviZid(0.03 * MaxWidth, 0.03 * MaxHeight, velicina, velicina, nazad);
        SDL_DestroyTexture(nazad);
    }
    else if (LastIndex == 1 && index == 0) PisiTekst(font[1], { 0,0,0,255 }, { (int)(0.675 * MaxWidth + 3 * x[1] / 2 - x[2] / 2 - x[11]) / 2, (int)(0.25 * MaxHeight), (int)(0.5 * MaxWidth), (int)(0.2 * MaxHeight) }, (char*)"<");
    else if (LastIndex == 2 && index == 0) PisiTekst(font[1], { 0,0,0,255 }, { (int)(1.425 * MaxWidth + x[1] / 2 + x[2] / 2 - x[11]) / 2, (int)(0.25 * MaxHeight), (int)(0.5 * MaxWidth), (int)(0.2 * MaxHeight) }, (char*)">");
    else if (LastIndex == 3 && index == 0) PisiTekst(font[1], { 0,0,0,255 }, { (int)(0.675 * MaxWidth + 3 * x[3] / 2 - x[4] / 2 - x[11]) / 2, (int)(0.36 * MaxHeight), (int)(0.5 * MaxWidth), (int)(0.2 * MaxHeight) }, (char*)"<");
    else if (LastIndex == 4 && index == 0) PisiTekst(font[1], { 0,0,0,255 }, { (int)(1.425 * MaxWidth + x[3] / 2 + x[4] / 2 - x[11]) / 2, (int)(0.36 * MaxHeight), (int)(0.5 * MaxWidth), (int)(0.2 * MaxHeight) }, (char*)">");
    else if (LastIndex == 5 && index == 0) PisiTekst(font[1], { 0,0,0,255 }, { (int)(0.675 * MaxWidth + 3 * x[5] / 2 - x[6] / 2 - x[11]) / 2, (int)(0.47 * MaxHeight), (int)(0.5 * MaxWidth), (int)(0.2 * MaxHeight) }, (char*)"<");
    else if (LastIndex == 6 && index == 0) PisiTekst(font[1], { 0,0,0,255 }, { (int)(1.425 * MaxWidth + x[5] / 2 + x[6] / 2 - x[11]) / 2, (int)(0.47 * MaxHeight), (int)(0.5 * MaxWidth), (int)(0.2 * MaxHeight) }, (char*)">");
    else if (LastIndex == 7 && index == 0) PisiTekst(font[1], { 0,0,0,255 }, { (int)(0.675 * MaxWidth + 3 * x[7] / 2 - x[8] / 2 - x[11]) / 2, (int)(0.58 * MaxHeight), (int)(0.5 * MaxWidth), (int)(0.2 * MaxHeight) }, (char*)"<");
    else if (LastIndex == 8 && index == 0) PisiTekst(font[1], { 0,0,0,255 }, { (int)(1.425 * MaxWidth + x[7] / 2 + x[8] / 2 - x[11]) / 2, (int)(0.58 * MaxHeight), (int)(0.5 * MaxWidth), (int)(0.2 * MaxHeight) }, (char*)">");
    else if (LastIndex == 9 && index == 0) PisiTekst(font[1], { 0,0,0,255 }, { (int)(0.675 * MaxWidth + 3 * x[9] / 2 - x[10] / 2 - x[11]) / 2, (int)(0.69 * MaxHeight), (int)(0.5 * MaxWidth), (int)(0.2 * MaxHeight) }, (char*)"<");
    else if (LastIndex == 10 && index == 0) PisiTekst(font[1], { 0,0,0,255 }, { (int)(1.425 * MaxWidth + x[9] / 2 + x[10] / 2 - x[11]) / 2, (int)(0.69 * MaxHeight), (int)(0.5 * MaxWidth), (int)(0.2 * MaxHeight) }, (char*)">");
    else if (LastIndex == 11 && index == 0) PisiTekst(font[0], { 0,0,0,255 }, { (int)(MaxWidth - x[12]) / 2, (int)(0.8 * MaxHeight), (int)(0.5 * MaxWidth), (int)(0.2 * MaxHeight) }, (char*)"Podesavanje izgleda i zvuka");
    else if (LastIndex == 12 && index == 0) {
        SDL_Texture* nazad = SDL_CreateTextureFromSurface(renderer, surface[5]);
        NapraviZid(0.03 * MaxWidth, 0.03 * MaxHeight, velicina, velicina, nazad);
        SDL_DestroyTexture(nazad);
    }
}
void Opcije(SDL_Surface** surface) {
    int x[14], y[3], prosli = 0, radi = 1, Width[4] = { 1280,1366,1600,1920 }, Height[4] = { 720,768,900,1080 }, N[3] = { 6 , 7, 8 }, index[2];
    char width[10], height[5], n1[3], algoritam[3], brzombija[3], tezina[3][10] = { "lako","normalno", "tesko" };
    for (int i = 0; i < 4; i++) {
        if (Width[i] == MaxWidth && Height[i] == MaxHeight)
            index[0] = i;
        if (i < 3)
            if (N[i] == n)
                index[1] = i;
    }
Pocetak:
    sprintf_s(width, "%dx%d", MaxWidth, MaxHeight);
    sprintf_s(n1, "%d", n);
    sprintf_s(algoritam, "%d", Algoritam);
    sprintf_s(brzombija, "%d", BrZombija);
    TTF_SetFontSize(font[0], velicina / 2);
    TTF_SetFontSize(font[1], velicina / 2);
    TTF_SizeText(font[1], (const char*)"<", &x[11], &y[2]);
    TTF_SizeText(font[0], (const char*)"Rezolucija", &x[1], &y[1]);
    TTF_SizeText(font[0], (const char*)width, &x[2], &y[1]);
    TTF_SizeText(font[0], (const char*)"Velicina talona", &x[3], &y[1]);
    TTF_SizeText(font[0], (const char*)n1, &x[4], &y[1]);
    TTF_SizeText(font[0], (const char*)"Tezina nivoa", &x[5], &y[1]);
    TTF_SizeText(font[0], (const char*)tezina[Tezina], &x[6], &y[1]);
    TTF_SizeText(font[0], (const char*)"Algoritam lavirinta", &x[7], &y[1]);
    TTF_SizeText(font[0], (const char*)algoritam, &x[8], &y[1]);
    TTF_SizeText(font[0], (const char*)"Broj zombija", &x[9], &y[1]);
    TTF_SizeText(font[0], (const char*)brzombija, &x[10], &y[1]);
    TTF_SizeText(font[0], (const char*)"Podesavanje izgleda i zvuka", &x[12], &y[1]);
    CrtajOpcije(surface, 0, -1);
    while (radi) {
        SDL_GetMouseState(&x[0], &y[0]);
        if (x[0] >= ((0.675 * MaxWidth + 3 * x[1] / 2 - x[2] / 2 - x[11]) / 2) && x[0] <= ((0.675 * MaxWidth + 3 * x[1] / 2 - x[2] / 2 + x[11]) / 2) && y[0] >= 0.25 * MaxHeight && y[0] <= (0.25 * MaxHeight + y[2]) && index[0] > 0) {
            CrtajOpcije(surface, 1, prosli);
            prosli = 1;
        }
        else if (x[0] >= ((1.425 * MaxWidth + x[1] / 2 + x[2] / 2 - x[11]) / 2) && x[0] <= ((1.425 * MaxWidth + x[1] / 2 + x[2] / 2 + x[11]) / 2) && y[0] >= 0.25 * MaxHeight && y[0] <= (0.25 * MaxHeight + y[2]) && index[0] < 3) {
            CrtajOpcije(surface, 2, prosli);
            prosli = 2;
        }
        else if (x[0] >= ((0.675 * MaxWidth + 3 * x[3] / 2 - x[4] / 2 - x[11]) / 2) && x[0] <= ((0.675 * MaxWidth + 3 * x[3] / 2 - x[4] / 2 + x[11]) / 2) && y[0] >= 0.36 * MaxHeight && y[0] <= (0.36 * MaxHeight + y[2]) && index[1] > 0) {
            CrtajOpcije(surface, 3, prosli);
            prosli = 3;
        }
        else if (x[0] >= ((1.425 * MaxWidth + x[3] / 2 + x[4] / 2 - x[11]) / 2) && x[0] <= ((1.425 * MaxWidth + x[3] / 2 + x[4] / 2 + x[11]) / 2) && y[0] >= 0.36 * MaxHeight && y[0] <= (0.36 * MaxHeight + y[2]) && index[1] < 2) {
            CrtajOpcije(surface, 4, prosli);
            prosli = 4;
        }
        else if (x[0] >= ((0.675 * MaxWidth + 3 * x[5] / 2 - x[6] / 2 - x[11]) / 2) && x[0] <= ((0.675 * MaxWidth + 3 * x[5] / 2 - x[6] / 2 + x[11]) / 2) && y[0] >= 0.47 * MaxHeight && y[0] <= (0.47 * MaxHeight + y[2]) && Tezina > 0) {
            CrtajOpcije(surface, 5, prosli);
            prosli = 5;
        }
        else if (x[0] >= ((1.425 * MaxWidth + x[5] / 2 + x[6] / 2 - x[11]) / 2) && x[0] <= ((1.425 * MaxWidth + x[5] / 2 + x[6] / 2 + x[11]) / 2) && y[0] >= 0.47 * MaxHeight && y[0] <= (0.47 * MaxHeight + y[2]) && Tezina < 2) {
            CrtajOpcije(surface, 6, prosli);
            prosli = 6;
        }
        else if (x[0] >= ((0.675 * MaxWidth + 3 * x[7] / 2 - x[8] / 2 - x[11]) / 2) && x[0] <= ((0.675 * MaxWidth + 3 * x[7] / 2 - x[8] / 2 + x[11]) / 2) && y[0] >= 0.58 * MaxHeight && y[0] <= (0.58 * MaxHeight + y[2]) && Algoritam == 2) {
            CrtajOpcije(surface, 7, prosli);
            prosli = 7;
        }
        else if (x[0] >= ((1.425 * MaxWidth + x[7] / 2 + x[8] / 2 - x[11]) / 2) && x[0] <= ((1.425 * MaxWidth + x[7] / 2 + x[8] / 2 + x[11]) / 2) && y[0] >= 0.58 * MaxHeight && y[0] <= (0.58 * MaxHeight + y[2]) && Algoritam == 1) {
            CrtajOpcije(surface, 8, prosli);
            prosli = 8;
        }
        else if (x[0] >= ((0.675 * MaxWidth + 3 * x[9] / 2 - x[10] / 2 - x[11]) / 2) && x[0] <= ((0.675 * MaxWidth + 3 * x[9] / 2 - x[10] / 2 + x[11]) / 2) && y[0] >= 0.69 * MaxHeight && y[0] <= (0.69 * MaxHeight + y[2]) && BrZombija > 1) {
            CrtajOpcije(surface, 9, prosli);
            prosli = 9;
        }
        else if (x[0] >= ((1.425 * MaxWidth + x[9] / 2 + x[10] / 2 - x[11]) / 2) && x[0] <= ((1.425 * MaxWidth + x[9] / 2 + x[10] / 2 + x[11]) / 2) && y[0] >= 0.69 * MaxHeight && y[0] <= (0.69 * MaxHeight + y[2]) && BrZombija < 2) {
            CrtajOpcije(surface, 10, prosli);
            prosli = 10;
        }
        else if (x[0] >= ((int)(MaxWidth - x[12]) / 2) && x[0] <= ((int)(MaxWidth - x[12]) / 2 + x[12]) && y[0] >= 0.8 * MaxHeight && y[0] <= (0.8 * MaxHeight + y[1])) {
            CrtajOpcije(surface, 11, prosli);
            prosli = 11;
        }
        else if (x[0] >= 0.03 * MaxWidth && x[0] <= (0.03 * MaxWidth + velicina) && y[0] >= 0.03 * MaxHeight && y[0] <= (0.03 * MaxHeight + velicina)) {
            CrtajOpcije(surface, 12, prosli);
            prosli = 12;
        }
        else {
            CrtajOpcije(surface, 0, prosli);
            prosli = 0;
        }
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                SDL_Quit();
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN) {
                SDL_GetMouseState(&x[0], &y[0]);
                if (x[0] >= ((0.675 * MaxWidth + 3 * x[1] / 2 - x[2] / 2 - x[11]) / 2) && x[0] <= ((0.675 * MaxWidth + 3 * x[1] / 2 - x[2] / 2 + x[11]) / 2) && y[0] >= 0.25 * MaxHeight && y[0] <= (0.25 * MaxHeight + y[2])) {
                    if ((index[0] - 1) >= 0) {
                        PustiZvuk(zvukovi[1]);
                        MaxWidth = Width[--index[0]];
                        MaxHeight = Height[index[0]];
                        velicina = Velicina1();
                        SDL_SetWindowSize(window, MaxWidth, MaxHeight);
                        FILE* file = NULL;
                        fopen_s(&file, (const char*)"Opcije.txt", (const char*)"w");
                        fprintf_s(file, "%d %d\n%d\n%d %d %d\n", MaxWidth, MaxHeight, n, Tezina, Algoritam, BrZombija);
                        fprintf_s(file, "%d %d %d\n", Izgledi[0], Izgledi[1], Izgledi[2]);
                        fprintf_s(file, "%d %d %d\n", TrenutniNivo, mapa, Sacuvaj);
                        fprintf_s(file, "%d %d", Zvukovi, Pozadinska);
                        Mix_VolumeMusic(Pozadinska);
                        for (int i = 0; i < 7; i++) {
                            Mix_VolumeChunk(zvukovi[i], Zvukovi);
                        }
                        fclose(file);
                        goto Pocetak;
                    }
                }
                else if (x[0] >= ((1.425 * MaxWidth + x[1] / 2 + x[2] / 2 - x[11]) / 2) && x[0] <= ((1.425 * MaxWidth + x[1] / 2 + x[2] / 2 + x[11]) / 2) && y[0] >= 0.25 * MaxHeight && y[0] <= (0.25 * MaxHeight + y[2])) {
                    if ((index[0] + 1) <= 3) {
                        PustiZvuk(zvukovi[1]);
                        MaxWidth = Width[++index[0]];
                        MaxHeight = Height[index[0]];
                        velicina = Velicina1();
                        SDL_SetWindowSize(window, MaxWidth, MaxHeight);
                        FILE* file = NULL;
                        fopen_s(&file, (const char*)"Opcije.txt", (const char*)"w");
                        fprintf_s(file, "%d %d\n%d\n%d %d %d\n", MaxWidth, MaxHeight, n, Tezina, Algoritam, BrZombija);
                        fprintf_s(file, "%d %d %d\n", Izgledi[0], Izgledi[1], Izgledi[2]);
                        fprintf_s(file, "%d %d %d\n", TrenutniNivo, mapa, Sacuvaj);
                        fprintf_s(file, "%d %d", Zvukovi, Pozadinska);
                        Mix_VolumeMusic(Pozadinska);
                        for (int i = 0; i < 7; i++) {
                            Mix_VolumeChunk(zvukovi[i], Zvukovi);
                        }
                        fclose(file);
                        goto Pocetak;
                    }
                }
                else if (x[0] >= ((0.675 * MaxWidth + 3 * x[3] / 2 - x[4] / 2 - x[11]) / 2) && x[0] <= ((0.675 * MaxWidth + 3 * x[3] / 2 - x[4] / 2 + x[11]) / 2) && y[0] >= 0.36 * MaxHeight && y[0] <= (0.36 * MaxHeight + y[2])) {
                    if ((index[1] - 1) >= 0) {
                        PustiZvuk(zvukovi[1]);
                        n = N[--index[1]];
                        FILE* file = NULL;
                        fopen_s(&file, (const char*)"Opcije.txt", (const char*)"w");
                        fprintf_s(file, "%d %d\n%d\n%d %d %d\n", MaxWidth, MaxHeight, n, Tezina, Algoritam, BrZombija);
                        fprintf_s(file, "%d %d %d\n", Izgledi[0], Izgledi[1], Izgledi[2]);
                        fprintf_s(file, "%d %d %d\n", TrenutniNivo, mapa, Sacuvaj);
                        fprintf_s(file, "%d %d", Zvukovi, Pozadinska);
                        Mix_VolumeMusic(Pozadinska);
                        for (int i = 0; i < 7; i++) {
                            Mix_VolumeChunk(zvukovi[i], Zvukovi);
                        }
                        fclose(file);
                        goto Pocetak;
                    }
                }
                else if (x[0] >= ((1.425 * MaxWidth + x[3] / 2 + x[4] / 2 - x[11]) / 2) && x[0] <= ((1.425 * MaxWidth + x[3] / 2 + x[4] / 2 + x[11]) / 2) && y[0] >= 0.36 * MaxHeight && y[0] <= (0.36 * MaxHeight + y[2])) {
                    if ((index[1] + 1) <= 2) {
                        PustiZvuk(zvukovi[1]);
                        n = N[++index[1]];
                        FILE* file = NULL;
                        fopen_s(&file, (const char*)"Opcije.txt", (const char*)"w");
                        fprintf_s(file, "%d %d\n%d\n%d %d %d\n", MaxWidth, MaxHeight, n, Tezina, Algoritam, BrZombija);
                        fprintf_s(file, "%d %d %d\n", Izgledi[0], Izgledi[1], Izgledi[2]);
                        fprintf_s(file, "%d %d %d\n", TrenutniNivo, mapa, Sacuvaj);
                        fprintf_s(file, "%d %d", Zvukovi, Pozadinska);
                        Mix_VolumeMusic(Pozadinska);
                        for (int i = 0; i < 7; i++) {
                            Mix_VolumeChunk(zvukovi[i], Zvukovi);
                        }
                        fclose(file);
                        goto Pocetak;
                    }
                }
                else if (x[0] >= ((0.675 * MaxWidth + 3 * x[5] / 2 - x[6] / 2 - x[11]) / 2) && x[0] <= ((0.675 * MaxWidth + 3 * x[5] / 2 - x[6] / 2 + x[11]) / 2) && y[0] >= 0.47 * MaxHeight && y[0] <= (0.47 * MaxHeight + y[2])) {
                    if ((Tezina - 1) >= 0) {
                        PustiZvuk(zvukovi[1]);
                        Tezina--;
                        FILE* file = NULL;
                        fopen_s(&file, (const char*)"Opcije.txt", (const char*)"w");
                        fprintf_s(file, "%d %d\n%d\n%d %d %d\n", MaxWidth, MaxHeight, n, Tezina, Algoritam, BrZombija);
                        fprintf_s(file, "%d %d %d\n", Izgledi[0], Izgledi[1], Izgledi[2]);
                        fprintf_s(file, "%d %d %d\n", TrenutniNivo, mapa, Sacuvaj);
                        fprintf_s(file, "%d %d", Zvukovi, Pozadinska);
                        Mix_VolumeMusic(Pozadinska);
                        for (int i = 0; i < 7; i++) {
                            Mix_VolumeChunk(zvukovi[i], Zvukovi);
                        }
                        fclose(file);
                        goto Pocetak;
                    }
                }
                else if (x[0] >= ((1.425 * MaxWidth + x[5] / 2 + x[6] / 2 - x[11]) / 2) && x[0] <= ((1.425 * MaxWidth + x[5] / 2 + x[6] / 2 + x[11]) / 2) && y[0] >= 0.47 * MaxHeight && y[0] <= (0.47 * MaxHeight + y[2])) {
                    if ((Tezina + 1) <= 2) {
                        PustiZvuk(zvukovi[1]);
                        Tezina++;
                        FILE* file = NULL;
                        fopen_s(&file, (const char*)"Opcije.txt", (const char*)"w");
                        fprintf_s(file, "%d %d\n%d\n%d %d %d\n", MaxWidth, MaxHeight, n, Tezina, Algoritam, BrZombija);
                        fprintf_s(file, "%d %d %d\n", Izgledi[0], Izgledi[1], Izgledi[2]);
                        fprintf_s(file, "%d %d %d\n", TrenutniNivo, mapa, Sacuvaj);
                        fprintf_s(file, "%d %d", Zvukovi, Pozadinska);
                        Mix_VolumeMusic(Pozadinska);
                        for (int i = 0; i < 7; i++) {
                            Mix_VolumeChunk(zvukovi[i], Zvukovi);
                        }
                        fclose(file);
                        goto Pocetak;
                    }
                }
                else if (x[0] >= ((0.675 * MaxWidth + 3 * x[7] / 2 - x[8] / 2 - x[11]) / 2) && x[0] <= ((0.675 * MaxWidth + 3 * x[7] / 2 - x[8] / 2 + x[11]) / 2) && y[0] >= 0.58 * MaxHeight && y[0] <= (0.58 * MaxHeight + y[2])) {
                    if (Algoritam == 2) {
                        PustiZvuk(zvukovi[1]);
                        Algoritam--;
                        FILE* file = NULL;
                        fopen_s(&file, (const char*)"Opcije.txt", (const char*)"w");
                        fprintf_s(file, "%d %d\n%d\n%d %d %d\n", MaxWidth, MaxHeight, n, Tezina, Algoritam, BrZombija);
                        fprintf_s(file, "%d %d %d\n", Izgledi[0], Izgledi[1], Izgledi[2]);
                        fprintf_s(file, "%d %d %d\n", TrenutniNivo, mapa, Sacuvaj);
                        fprintf_s(file, "%d %d", Zvukovi, Pozadinska);
                        Mix_VolumeMusic(Pozadinska);
                        for (int i = 0; i < 7; i++) {
                            Mix_VolumeChunk(zvukovi[i], Zvukovi);
                        }
                        fclose(file);
                        goto Pocetak;
                    }
                }
                else if (x[0] >= ((1.425 * MaxWidth + x[7] / 2 + x[8] / 2 - x[11]) / 2) && x[0] <= ((1.425 * MaxWidth + x[7] / 2 + x[8] / 2 + x[11]) / 2) && y[0] >= 0.58 * MaxHeight && y[0] <= (0.58 * MaxHeight + y[2])) {
                    if (Algoritam == 1) {
                        PustiZvuk(zvukovi[1]);
                        Algoritam++;
                        FILE* file = NULL;
                        fopen_s(&file, (const char*)"Opcije.txt", (const char*)"w");
                        fprintf_s(file, "%d %d\n%d\n%d %d %d\n", MaxWidth, MaxHeight, n, Tezina, Algoritam, BrZombija);
                        fprintf_s(file, "%d %d %d\n", Izgledi[0], Izgledi[1], Izgledi[2]);
                        fprintf_s(file, "%d %d %d\n", TrenutniNivo, mapa, Sacuvaj);
                        fprintf_s(file, "%d %d", Zvukovi, Pozadinska);
                        Mix_VolumeMusic(Pozadinska);
                        for (int i = 0; i < 7; i++) {
                            Mix_VolumeChunk(zvukovi[i], Zvukovi);
                        }
                        fclose(file);
                        goto Pocetak;
                    }
                }
                else if (x[0] >= ((0.675 * MaxWidth + 3 * x[9] / 2 - x[10] / 2 - x[11]) / 2) && x[0] <= ((0.675 * MaxWidth + 3 * x[9] / 2 - x[10] / 2 + x[11]) / 2) && y[0] >= 0.69 * MaxHeight && y[0] <= (0.69 * MaxHeight + y[2])) {
                    if ((BrZombija - 1) >= 1) {
                        PustiZvuk(zvukovi[1]);
                        BrZombija--;
                        FILE* file = NULL;
                        fopen_s(&file, (const char*)"Opcije.txt", (const char*)"w");
                        fprintf_s(file, "%d %d\n%d\n%d %d %d\n", MaxWidth, MaxHeight, n, Tezina, Algoritam, BrZombija);
                        fprintf_s(file, "%d %d %d\n", Izgledi[0], Izgledi[1], Izgledi[2]);
                        fprintf_s(file, "%d %d %d\n", TrenutniNivo, mapa, Sacuvaj);
                        fprintf_s(file, "%d %d", Zvukovi, Pozadinska);
                        Mix_VolumeMusic(Pozadinska);
                        for (int i = 0; i < 7; i++) {
                            Mix_VolumeChunk(zvukovi[i], Zvukovi);
                        }
                        fclose(file);
                        goto Pocetak;
                    }
                }
                else if (x[0] >= ((1.425 * MaxWidth + x[9] / 2 + x[10] / 2 - x[11]) / 2) && x[0] <= ((1.425 * MaxWidth + x[9] / 2 + x[10] / 2 + x[11]) / 2) && y[0] >= 0.69 * MaxHeight && y[0] <= (0.69 * MaxHeight + y[2])) {
                    if ((BrZombija + 1) <= 2) {
                        PustiZvuk(zvukovi[1]);
                        BrZombija++;
                        FILE* file = NULL;
                        fopen_s(&file, (const char*)"Opcije.txt", (const char*)"w");
                        fprintf_s(file, "%d %d\n%d\n%d %d %d\n", MaxWidth, MaxHeight, n, Tezina, Algoritam, BrZombija);
                        fprintf_s(file, "%d %d %d\n", Izgledi[0], Izgledi[1], Izgledi[2]);
                        fprintf_s(file, "%d %d %d\n", TrenutniNivo, mapa, Sacuvaj);
                        fprintf_s(file, "%d %d", Zvukovi, Pozadinska);
                        Mix_VolumeMusic(Pozadinska);
                        for (int i = 0; i < 7; i++) {
                            Mix_VolumeChunk(zvukovi[i], Zvukovi);
                        }
                        fclose(file);
                        goto Pocetak;
                    }
                }
                else if (x[0] >= ((int)(MaxWidth - x[12]) / 2) && x[0] <= ((int)(MaxWidth - x[12]) / 2 + x[12]) && y[0] >= 0.8 * MaxHeight && y[0] <= (0.8 * MaxHeight + y[1])) {
                    PustiZvuk(zvukovi[1]);
                    PustiZvuk(zvukovi[0]);
                    PrelazNaOpcije(surface, 1, 1);
                }
                else if (x[0] >= 0.03 * MaxWidth && x[0] <= (0.03 * MaxWidth + velicina) && y[0] >= 0.03 * MaxHeight && y[0] <= (0.03 * MaxHeight + velicina)) {
                    PustiZvuk(zvukovi[1]);
                    PustiZvuk(zvukovi[0]);
                    PrelazNaGlavniMeni(surface);
                }

            }
        }
        SDL_RenderPresent(renderer);
    }
}
void CrtajInfo(SDL_Surface** surface, int index, int LastIndex) {
    int x[5], y[3];
    if (index == 0 && LastIndex == -1) {
        char linija[300];
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface[17]);
        SDL_Texture* nazad = SDL_CreateTextureFromSurface(renderer, surface[5]);
        NapraviZid(0, 0, MaxWidth, MaxHeight, texture);
        NapraviZid(0.03 * MaxWidth, 0.03 * MaxHeight, velicina, velicina, nazad);
        SDL_DestroyTexture(texture);
        SDL_DestroyTexture(nazad);
        TTF_SetFontSize(font[0], velicina);
        TTF_SizeText(font[0], (const char*)"Info", &x[0], &y[0]);
        PisiTekst(font[0], { 0,0,0,255 }, { (MaxWidth - x[0]) / 2, (int)(0.04 * MaxHeight), x[0], y[0] }, (char*)"Info");
        TTF_SetFontSize(font[1], velicina * 0.3);
        FILE* file = NULL;
        x[1] = 0;
        fopen_s(&file, (const char*)"Info.txt", (const char*)"r");
        for (int i = 0; i < 15; i++) {
            fgets(linija, 300, file);
            linija[strlen(linija) - 1] = '\0';
            TTF_SetFontSize(font[1], velicina * 0.3);
            TTF_SizeText(font[1], (const char*)linija, &x[0], &y[0]);
            if (i < 9)
                PisiTekst(font[1], { 0,0,0,255 }, { (MaxWidth - x[0]) / 2, (int)(0.25 * MaxHeight + i * 0.05 * MaxHeight), x[0], y[0] }, (char*)linija);
            else {
                PisiTekst(font[1], { 0,0,0,255 }, { (int)((i % 3 + 1) * 0.33 * MaxWidth - x[0] + ((i % 3) * 0.33 * MaxWidth)) / 2, (int)(0.25 * MaxHeight + x[1] + ((i >= 9 && i <= 11) ? 11 : 12) * 0.05 * MaxHeight), x[0], y[0] }, (char*)linija);
            }
        }
        for (int i = 0; i < 3; i++) {
            fgets(linija, 300, file);
            linija[strlen(linija) - 1] = '\0';
            TTF_SetFontSize(font[1], velicina * 0.3);
            TTF_SizeText(font[1], (const char*)linija, &x[0], &y[0]);
            PisiTekst(font[1], { 0,0,0,255 }, { (int)((i % 3 + 1) * 0.33 * MaxWidth - x[0] + ((i % 3) * 0.33 * MaxWidth)) / 2, (int)(0.3 * MaxHeight + x[1] + ((i >= 9 && i <= 11) ? 11 : 12) * 0.05 * MaxHeight), x[0], y[0] }, (char*)linija);
        }
        fclose(file);
    }
    else if (index == 1 && LastIndex == 0) {
        SDL_Texture* nazad = SDL_CreateTextureFromSurface(renderer, surface[6]);
        NapraviZid(0.03 * MaxWidth, 0.03 * MaxHeight, velicina, velicina, nazad);
        SDL_DestroyTexture(nazad);
    }
    else if (index == 0 && LastIndex == 1) {
        SDL_Texture* nazad = SDL_CreateTextureFromSurface(renderer, surface[5]);
        NapraviZid(0.03 * MaxWidth, 0.03 * MaxHeight, velicina, velicina, nazad);
        SDL_DestroyTexture(nazad);
    }
}
void Info(SDL_Surface** surface) {
    int x[5], y[3], radi = 1, prosli = 0;
    CrtajInfo(surface, 0, -1);
    while (radi) {
        while (SDL_PollEvent(&event) != 0) {
            SDL_GetMouseState(&x[0], &y[0]);
            if (event.type == SDL_QUIT) {
                SDL_Quit();
            }
            else if (x[0] >= 0.03 * MaxWidth && x[0] <= (0.03 * MaxWidth + velicina) && y[0] >= 0.03 * MaxHeight && y[0] <= (0.03 * MaxHeight + velicina)) {
                CrtajInfo(surface, 1, prosli);
                prosli = 1;
                SDL_RenderPresent(renderer);
            }
            else {
                CrtajInfo(surface, 0, prosli);
                prosli = 0;
                SDL_RenderPresent(renderer);
            }
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (x[0] >= 0.03 * MaxWidth && x[0] <= (0.03 * MaxWidth + velicina) && y[0] >= 0.03 * MaxHeight && y[0] <= (0.03 * MaxHeight + velicina)) {
                    PustiZvuk(zvukovi[1]);
                    PustiZvuk(zvukovi[0]);
                    PrelazNaGlavniMeni(surface);
                }
            }
        }
    }
}
void PrelazNaOpcije(SDL_Surface** surface, int smer, int zvuk) {
    int interval = 10, Width, intervalw = MaxWidth / 25;
    Uint32 ProsloVreme = SDL_GetTicks(), TrenutnoVreme;
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface[4]);
    if (smer == 1) {
        Width = MaxWidth;
        while (1) {
            TrenutnoVreme = SDL_GetTicks();
            if (TrenutnoVreme - ProsloVreme >= interval) {
                NapraviZid(Width, 0, MaxWidth, MaxHeight, texture);
                ProsloVreme = TrenutnoVreme;
                Width -= intervalw;
                SDL_RenderPresent(renderer);
                if (Width <= 0) {
                    break;
                }
            }
        }
    }
    else {
        Width = -MaxWidth;
        while (1) {
            TrenutnoVreme = SDL_GetTicks();
            if (TrenutnoVreme - ProsloVreme >= interval) {
                NapraviZid(Width, 0, MaxWidth, MaxHeight, texture);
                ProsloVreme = TrenutnoVreme;
                Width += intervalw;
                SDL_RenderPresent(renderer);
                if (Width >= 0) {
                    break;
                }
            }
        }
    }
    SDL_DestroyTexture(texture);
    if (zvuk == 0)
        Opcije(surface);
    else
        ZvukIzgled(surface);
}
void PrelazNaGlavniMeni(SDL_Surface** surface) {
    int interval = 10, Width = -MaxWidth, intervalw = MaxWidth / 25;
    Uint32 ProsloVreme = SDL_GetTicks(), TrenutnoVreme;
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface[19]);
    while (1) {
        TrenutnoVreme = SDL_GetTicks();
        if (TrenutnoVreme - ProsloVreme >= interval) {
            NapraviZid(Width, 0, MaxWidth, MaxHeight, texture);
            ProsloVreme = TrenutnoVreme;
            Width += intervalw;
            SDL_RenderPresent(renderer);
            if (Width >= 0) {
                break;
            }
        }
    }
    SDL_DestroyTexture(texture);
    GlavniMeni(surface);
}
void PrelazNaInfo(SDL_Surface** surface) {
    int interval = 10, Width = MaxWidth, intervalw = MaxWidth / 25;
    Uint32 ProsloVreme = SDL_GetTicks(), TrenutnoVreme;
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface[17]);
    while (1) {
        TrenutnoVreme = SDL_GetTicks();
        if (TrenutnoVreme - ProsloVreme >= interval) {
            NapraviZid(Width, 0, MaxWidth, MaxHeight, texture);
            ProsloVreme = TrenutnoVreme;
            Width -= intervalw;
            SDL_RenderPresent(renderer);
            if (Width <= 0) {
                break;
            }
        }
    }
    SDL_DestroyTexture(texture);
    Info(surface);
}
void CrtajMapa(SDL_Surface** surface, int index) {
    int x[2], y[2], velicina = Velicina1();;
    TTF_SetFontSize(font[0], 0.45 * velicina);
    TTF_SizeText(font[0], (const char*)"1", &x[1], &y[1]);
    if (index == 0) {
        SDL_Surface* pozadina = IMG_Load("Slike/Mapa.png");
        SDL_Texture* Pozadina = SDL_CreateTextureFromSurface(renderer, pozadina);
        SDL_Texture* nazad = SDL_CreateTextureFromSurface(renderer, surface[5]);
        NapraviZid(0, 0, MaxWidth, MaxHeight, Pozadina);
        NapraviZid(0.03 * MaxWidth, 0.03 * MaxHeight, velicina, velicina, nazad);
        SDL_DestroyTexture(nazad);
        SDL_FreeSurface(pozadina);
        SDL_DestroyTexture(Pozadina);
        if (TrenutniNivo == 1)
            PisiTekst(font[0], { 200, 150, 5, 255 }, { (int)(0.03 * MaxWidth), (int)(0.62 * MaxHeight), x[1], y[1] }, (char*)"1");
        else
            PisiTekst(font[0], { 106,78,23,255 }, { (int)(0.03 * MaxWidth), (int)(0.62 * MaxHeight), x[1], y[1] }, (char*)"1");
        if (TrenutniNivo == 2)
            PisiTekst(font[0], { 200, 150, 5, 255 }, { (int)(0.3 * MaxWidth), (int)(0.52 * MaxHeight), x[1], y[1] }, (char*)"2");
        else
            PisiTekst(font[0], { 106,78,23,255 }, { (int)(0.3 * MaxWidth), (int)(0.52 * MaxHeight), x[1], y[1] }, (char*)"2");
        if (TrenutniNivo == 3)
            PisiTekst(font[0], { 200, 150, 5, 255 }, { (int)(0.425 * MaxWidth), (int)(0.22 * MaxHeight), x[1], y[1] }, (char*)"3");
        else
            PisiTekst(font[0], { 106,78,23,255 }, { (int)(0.425 * MaxWidth), (int)(0.22 * MaxHeight), x[1], y[1] }, (char*)"3");
        if (TrenutniNivo == 4)
            PisiTekst(font[0], { 200, 150, 5, 255 }, { (int)(0.535 * MaxWidth), (int)(0.52 * MaxHeight), x[1], y[1] }, (char*)"4");
        else
            PisiTekst(font[0], { 106,78,23,255 }, { (int)(0.535 * MaxWidth), (int)(0.52 * MaxHeight), x[1], y[1] }, (char*)"4");
        if (TrenutniNivo == 5)
            PisiTekst(font[0], { 200, 150, 5, 255 }, { (int)(0.72 * MaxWidth), (int)(0.785 * MaxHeight), x[1], y[1] }, (char*)"5");
        else
            PisiTekst(font[0], { 106,78,23,255 }, { (int)(0.72 * MaxWidth), (int)(0.785 * MaxHeight), x[1], y[1] }, (char*)"5");
        if (TrenutniNivo == 6)
            PisiTekst(font[0], { 200, 150, 5, 255 }, { (int)(0.885 * MaxWidth), (int)(0.505 * MaxHeight), x[1], y[1] }, (char*)"6");
        else
            PisiTekst(font[0], { 106,78,23,255 }, { (int)(0.885 * MaxWidth), (int)(0.505 * MaxHeight), x[1], y[1] }, (char*)"6");
        TTF_SetFontSize(font[0], 0.5 * velicina);
        char BrojMape[10];
        sprintf(BrojMape, "Mapa: %d", mapa + 1);
        TTF_SizeText(font[0], (const char*)BrojMape, &x[1], &y[1]);
        PisiTekst(font[0], { 68,29,12,255 }, { (int)(0.765 * MaxWidth), (int)(0.01 * MaxHeight), x[1], y[1] }, (char*)BrojMape);
        SDL_Surface* zatvorenkovceg = IMG_Load("Slike/Kovceg/0.png");
        SDL_Texture* ZatvorenKovceg = SDL_CreateTextureFromSurface(renderer, zatvorenkovceg);
        SDL_Surface* poluotvorenenkovceg = IMG_Load("Slike/Kovceg/1.png");
        SDL_Texture* PoluOtvorenenKovceg = SDL_CreateTextureFromSurface(renderer, poluotvorenenkovceg);
        SDL_Surface* otvorenenkovceg = IMG_Load("Slike/Kovceg/5.png");
        SDL_Texture* OtvorenenKovceg = SDL_CreateTextureFromSurface(renderer, otvorenenkovceg);
        NapraviZid((int)(0.025 * MaxWidth), (int)(0.44 * MaxHeight), velicina, velicina, (TrenutniNivo > 1) ? OtvorenenKovceg : ((TrenutniNivo == 1) ? PoluOtvorenenKovceg : ZatvorenKovceg));
        NapraviZid((int)(0.255 * MaxWidth), (int)(0.35 * MaxHeight), velicina, velicina, (TrenutniNivo > 2) ? OtvorenenKovceg : ((TrenutniNivo == 2) ? PoluOtvorenenKovceg : ZatvorenKovceg));
        NapraviZid((int)(0.41 * MaxWidth), (int)(0.03 * MaxHeight), velicina, velicina, (TrenutniNivo > 3) ? OtvorenenKovceg : ((TrenutniNivo == 3) ? PoluOtvorenenKovceg : ZatvorenKovceg));
        NapraviZid((int)(0.59 * MaxWidth), (int)(0.46 * MaxHeight), velicina, velicina, (TrenutniNivo > 4) ? OtvorenenKovceg : ((TrenutniNivo == 4) ? PoluOtvorenenKovceg : ZatvorenKovceg));
        NapraviZid((int)(0.7 * MaxWidth), (int)(0.6 * MaxHeight), velicina, velicina, (TrenutniNivo > 5) ? OtvorenenKovceg : ((TrenutniNivo == 5) ? PoluOtvorenenKovceg : ZatvorenKovceg));
        NapraviZid((int)(0.862 * MaxWidth), (int)(0.318 * MaxHeight), velicina, velicina, (TrenutniNivo > 6) ? OtvorenenKovceg : ((TrenutniNivo == 6) ? PoluOtvorenenKovceg : ZatvorenKovceg));
        SDL_FreeSurface(poluotvorenenkovceg);
        SDL_DestroyTexture(PoluOtvorenenKovceg);
        SDL_FreeSurface(otvorenenkovceg);
        SDL_DestroyTexture(OtvorenenKovceg);
        SDL_FreeSurface(zatvorenkovceg);
        SDL_DestroyTexture(ZatvorenKovceg);
    }
    else if (index == 1) {
        PisiTekst(font[0], { 164,126,53,255 }, { (int)(0.03 * MaxWidth), (int)(0.62 * MaxHeight), x[1], y[1] }, (char*)"1");
    }
    else if (index == 2) {
        PisiTekst(font[0], { 164,126,53,255 }, { (int)(0.3 * MaxWidth), (int)(0.52 * MaxHeight), x[1], y[1] }, (char*)"2");
    }
    else if (index == 3) {
        PisiTekst(font[0], { 164,126,53,255 }, { (int)(0.425 * MaxWidth), (int)(0.22 * MaxHeight), x[1], y[1] }, (char*)"3");
    }
    else if (index == 4) {
        PisiTekst(font[0], { 164,126,53,255 }, { (int)(0.535 * MaxWidth), (int)(0.52 * MaxHeight), x[1], y[1] }, (char*)"4");
    }
    else if (index == 5) {
        PisiTekst(font[0], { 164,126,53,255 }, { (int)(0.72 * MaxWidth), (int)(0.785 * MaxHeight), x[1], y[1] }, (char*)"5");
    }
    else if (index == 6) {
        PisiTekst(font[0], { 164,126,53,255 }, { (int)(0.885 * MaxWidth), (int)(0.505 * MaxHeight), x[1], y[1] }, (char*)"6");
    }
    else if (index == 7) {
        if (TrenutniNivo == 1)
            PisiTekst(font[0], { 200, 150, 5, 255 }, { (int)(0.03 * MaxWidth), (int)(0.62 * MaxHeight), x[1], y[1] }, (char*)"1");
        else
            PisiTekst(font[0], { 106,78,23,255 }, { (int)(0.03 * MaxWidth), (int)(0.62 * MaxHeight), x[1], y[1] }, (char*)"1");
        if (TrenutniNivo == 2)
            PisiTekst(font[0], { 200, 150, 5, 255 }, { (int)(0.3 * MaxWidth), (int)(0.52 * MaxHeight), x[1], y[1] }, (char*)"2");
        else
            PisiTekst(font[0], { 106,78,23,255 }, { (int)(0.3 * MaxWidth), (int)(0.52 * MaxHeight), x[1], y[1] }, (char*)"2");
        if (TrenutniNivo == 3)
            PisiTekst(font[0], { 200, 150, 5, 255 }, { (int)(0.425 * MaxWidth), (int)(0.22 * MaxHeight), x[1], y[1] }, (char*)"3");
        else
            PisiTekst(font[0], { 106,78,23,255 }, { (int)(0.425 * MaxWidth), (int)(0.22 * MaxHeight), x[1], y[1] }, (char*)"3");
        if (TrenutniNivo == 4)
            PisiTekst(font[0], { 200, 150, 5, 255 }, { (int)(0.535 * MaxWidth), (int)(0.52 * MaxHeight), x[1], y[1] }, (char*)"4");
        else
            PisiTekst(font[0], { 106,78,23,255 }, { (int)(0.535 * MaxWidth), (int)(0.52 * MaxHeight), x[1], y[1] }, (char*)"4");
        if (TrenutniNivo == 5)
            PisiTekst(font[0], { 200, 150, 5, 255 }, { (int)(0.72 * MaxWidth), (int)(0.785 * MaxHeight), x[1], y[1] }, (char*)"5");
        else
            PisiTekst(font[0], { 106,78,23,255 }, { (int)(0.72 * MaxWidth), (int)(0.785 * MaxHeight), x[1], y[1] }, (char*)"5");
        if (TrenutniNivo == 6)
            PisiTekst(font[0], { 200, 150, 5, 255 }, { (int)(0.885 * MaxWidth), (int)(0.505 * MaxHeight), x[1], y[1] }, (char*)"6");
        else
            PisiTekst(font[0], { 106,78,23,255 }, { (int)(0.885 * MaxWidth), (int)(0.505 * MaxHeight), x[1], y[1] }, (char*)"6");
    }
    else if (index == 8) {
        Uint32 ProsloVreme = SDL_GetTicks(), TrenutnoVreme;
        int interval = 1000 / 9, velicina = Velicina1(), i = 0;
        char adresa[25];
        SDL_Surface* surfaces[6];
        SDL_Texture* textures[6];
        SDL_Surface* pozadina = IMG_Load("Slike/Mapa.png");
        SDL_Texture* Pozadina = SDL_CreateTextureFromSurface(renderer, pozadina);
        SDL_FreeSurface(pozadina);
        for (i = 0; i < 6; i++) {
            sprintf_s(adresa, "Slike/Kovceg/%d.png", i);
            surfaces[i] = IMG_Load(adresa);
            textures[i] = SDL_CreateTextureFromSurface(renderer, surfaces[i]);
            SDL_FreeSurface(surfaces[i]);
        }
        SDL_Texture* nazad = SDL_CreateTextureFromSurface(renderer, surface[5]);
        i = 0;
        while (1) {
            TrenutnoVreme = SDL_GetTicks();
            if (TrenutnoVreme - ProsloVreme >= interval) {
                NapraviZid(0, 0, MaxWidth, MaxHeight, Pozadina);
                NapraviZid(0.03 * MaxWidth, 0.03 * MaxHeight, velicina, velicina, nazad);
                if (TrenutniNivo - 1 == 1)
                    NapraviZid((int)(0.025 * MaxWidth), (int)(0.44 * MaxHeight), velicina, velicina, textures[i]);
                if (TrenutniNivo - 1 == 2)
                    NapraviZid((int)(0.255 * MaxWidth), (int)(0.35 * MaxHeight), velicina, velicina, textures[i]);
                if (TrenutniNivo - 1 == 3)
                    NapraviZid((int)(0.41 * MaxWidth), (int)(0.03 * MaxHeight), velicina, velicina, textures[i]);
                if (TrenutniNivo - 1 == 4)
                    NapraviZid((int)(0.59 * MaxWidth), (int)(0.46 * MaxHeight), velicina, velicina, textures[i]);
                if (TrenutniNivo - 1 == 5)
                    NapraviZid((int)(0.7 * MaxWidth), (int)(0.6 * MaxHeight), velicina, velicina, textures[i]);
                if (TrenutniNivo - 1 == 6)
                    NapraviZid((int)(0.862 * MaxWidth), (int)(0.318 * MaxHeight), velicina, velicina, textures[i]);
                if (TrenutniNivo - 1 != 1)
                    NapraviZid((int)(0.025 * MaxWidth), (int)(0.44 * MaxHeight), velicina, velicina, (TrenutniNivo > 1) ? textures[5] : ((TrenutniNivo == 1) ? textures[1] : textures[0]));
                if (TrenutniNivo - 1 != 2)
                    NapraviZid((int)(0.255 * MaxWidth), (int)(0.35 * MaxHeight), velicina, velicina, (TrenutniNivo > 2) ? textures[5] : ((TrenutniNivo == 2) ? textures[1] : textures[0]));
                if (TrenutniNivo - 1 != 3)
                    NapraviZid((int)(0.41 * MaxWidth), (int)(0.03 * MaxHeight), velicina, velicina, (TrenutniNivo > 3) ? textures[5] : ((TrenutniNivo == 3) ? textures[1] : textures[0]));
                if (TrenutniNivo - 1 != 4)
                    NapraviZid((int)(0.59 * MaxWidth), (int)(0.46 * MaxHeight), velicina, velicina, (TrenutniNivo > 4) ? textures[5] : ((TrenutniNivo == 4) ? textures[1] : textures[0]));
                if (TrenutniNivo - 1 != 5)
                    NapraviZid((int)(0.7 * MaxWidth), (int)(0.6 * MaxHeight), velicina, velicina, (TrenutniNivo > 5) ? textures[5] : ((TrenutniNivo == 5) ? textures[1] : textures[0]));
                if (TrenutniNivo - 1 != 6)
                    NapraviZid((int)(0.862 * MaxWidth), (int)(0.318 * MaxHeight), velicina, velicina, (TrenutniNivo > 6) ? textures[5] : ((TrenutniNivo == 6) ? textures[1] : textures[0]));
                if (TrenutniNivo == 1)
                    PisiTekst(font[0], { 200, 150, 5, 255 }, { (int)(0.03 * MaxWidth), (int)(0.62 * MaxHeight), x[1], y[1] }, (char*)"1");
                else
                    PisiTekst(font[0], { 106,78,23,255 }, { (int)(0.03 * MaxWidth), (int)(0.62 * MaxHeight), x[1], y[1] }, (char*)"1");
                if (TrenutniNivo == 2)
                    PisiTekst(font[0], { 200, 150, 5, 255 }, { (int)(0.3 * MaxWidth), (int)(0.52 * MaxHeight), x[1], y[1] }, (char*)"2");
                else
                    PisiTekst(font[0], { 106,78,23,255 }, { (int)(0.3 * MaxWidth), (int)(0.52 * MaxHeight), x[1], y[1] }, (char*)"2");
                if (TrenutniNivo == 3)
                    PisiTekst(font[0], { 200, 150, 5, 255 }, { (int)(0.425 * MaxWidth), (int)(0.22 * MaxHeight), x[1], y[1] }, (char*)"3");
                else
                    PisiTekst(font[0], { 106,78,23,255 }, { (int)(0.425 * MaxWidth), (int)(0.22 * MaxHeight), x[1], y[1] }, (char*)"3");
                if (TrenutniNivo == 4)
                    PisiTekst(font[0], { 200, 150, 5, 255 }, { (int)(0.535 * MaxWidth), (int)(0.52 * MaxHeight), x[1], y[1] }, (char*)"4");
                else
                    PisiTekst(font[0], { 106,78,23,255 }, { (int)(0.535 * MaxWidth), (int)(0.52 * MaxHeight), x[1], y[1] }, (char*)"4");
                if (TrenutniNivo == 5)
                    PisiTekst(font[0], { 200, 150, 5, 255 }, { (int)(0.72 * MaxWidth), (int)(0.785 * MaxHeight), x[1], y[1] }, (char*)"5");
                else
                    PisiTekst(font[0], { 106,78,23,255 }, { (int)(0.72 * MaxWidth), (int)(0.785 * MaxHeight), x[1], y[1] }, (char*)"5");
                if (TrenutniNivo == 6)
                    PisiTekst(font[0], { 200, 150, 5, 255 }, { (int)(0.885 * MaxWidth), (int)(0.505 * MaxHeight), x[1], y[1] }, (char*)"6");
                else
                    PisiTekst(font[0], { 106,78,23,255 }, { (int)(0.885 * MaxWidth), (int)(0.505 * MaxHeight), x[1], y[1] }, (char*)"6");
                TTF_SetFontSize(font[0], 0.5 * velicina);
                char BrojMape[10];
                sprintf(BrojMape, "Mapa: %d", mapa + 1);
                TTF_SizeText(font[0], (const char*)BrojMape, &x[1], &y[1]);
                PisiTekst(font[0], { 68,29,12,255 }, { (int)(0.765 * MaxWidth), (int)(0.01 * MaxHeight), x[1], y[1] }, (char*)BrojMape);
                SDL_RenderPresent(renderer);
                i++;
                if (i == 6)
                    break;
                ProsloVreme = TrenutnoVreme;
            }
        }
        for (int i = 0; i < 6; i++) {
            SDL_DestroyTexture(textures[i]);
        }
        SDL_DestroyTexture(nazad);
        SDL_DestroyTexture(Pozadina);
    }
    else if (index == 9) {
        SDL_Texture* nazad = SDL_CreateTextureFromSurface(renderer, surface[6]);
        NapraviZid(0.03 * MaxWidth, 0.03 * MaxHeight, velicina, velicina, nazad);
        SDL_DestroyTexture(nazad);
    }
    else if (index == 10) {
        SDL_Texture* nazad = SDL_CreateTextureFromSurface(renderer, surface[5]);
        NapraviZid(0.03 * MaxWidth, 0.03 * MaxHeight, velicina, velicina, nazad);
        SDL_DestroyTexture(nazad);
    }
    else if (index == 11) {
        Uint32 ProsloVreme = SDL_GetTicks(), TrenutnoVreme;
        int interval = 1000 / 9, velicina = Velicina1(), i = 0;
        char adresa[25];
        SDL_Surface* surfaces[6];
        SDL_Texture* textures[6];
        SDL_Surface* pozadina = IMG_Load("Slike/Mapa.png");
        SDL_Texture* Pozadina = SDL_CreateTextureFromSurface(renderer, pozadina);
        SDL_FreeSurface(pozadina);
        for (i = 0; i < 6; i++) {
            sprintf_s(adresa, "Slike/Kovceg/%d.png", i);
            surfaces[i] = IMG_Load(adresa);
            textures[i] = SDL_CreateTextureFromSurface(renderer, surfaces[i]);
            SDL_FreeSurface(surfaces[i]);
        }
        SDL_Texture* nazad = SDL_CreateTextureFromSurface(renderer, surface[5]);
        i = 0;
        while (1) {
            TrenutnoVreme = SDL_GetTicks();
            if (TrenutnoVreme - ProsloVreme >= interval) {
                NapraviZid(0, 0, MaxWidth, MaxHeight, Pozadina);
                NapraviZid(0.03 * MaxWidth, 0.03 * MaxHeight, velicina, velicina, nazad);
                if (TrenutniNivo - 1 == 1)
                    NapraviZid((int)(0.025 * MaxWidth), (int)(0.44 * MaxHeight), velicina, velicina, textures[i]);
                if (TrenutniNivo - 1 == 2)
                    NapraviZid((int)(0.255 * MaxWidth), (int)(0.35 * MaxHeight), velicina, velicina, textures[i]);
                if (TrenutniNivo - 1 == 3)
                    NapraviZid((int)(0.41 * MaxWidth), (int)(0.03 * MaxHeight), velicina, velicina, textures[i]);
                if (TrenutniNivo - 1 == 4)
                    NapraviZid((int)(0.59 * MaxWidth), (int)(0.46 * MaxHeight), velicina, velicina, textures[i]);
                if (TrenutniNivo - 1 == 5)
                    NapraviZid((int)(0.7 * MaxWidth), (int)(0.6 * MaxHeight), velicina, velicina, textures[i]);
                if (TrenutniNivo - 1 == 6)
                    NapraviZid((int)(0.862 * MaxWidth), (int)(0.318 * MaxHeight), velicina, velicina, textures[i]);
                if (TrenutniNivo - 1 != 1)
                    NapraviZid((int)(0.025 * MaxWidth), (int)(0.44 * MaxHeight), velicina, velicina, (TrenutniNivo > 1) ? textures[5] : ((TrenutniNivo == 1) ? textures[1] : textures[0]));
                if (TrenutniNivo - 1 != 2)
                    NapraviZid((int)(0.255 * MaxWidth), (int)(0.35 * MaxHeight), velicina, velicina, (TrenutniNivo > 2) ? textures[5] : ((TrenutniNivo == 2) ? textures[1] : textures[0]));
                if (TrenutniNivo - 1 != 3)
                    NapraviZid((int)(0.41 * MaxWidth), (int)(0.03 * MaxHeight), velicina, velicina, (TrenutniNivo > 3) ? textures[5] : ((TrenutniNivo == 3) ? textures[1] : textures[0]));
                if (TrenutniNivo - 1 != 4)
                    NapraviZid((int)(0.59 * MaxWidth), (int)(0.46 * MaxHeight), velicina, velicina, (TrenutniNivo > 4) ? textures[5] : ((TrenutniNivo == 4) ? textures[1] : textures[0]));
                if (TrenutniNivo - 1 != 5)
                    NapraviZid((int)(0.7 * MaxWidth), (int)(0.6 * MaxHeight), velicina, velicina, (TrenutniNivo > 5) ? textures[5] : ((TrenutniNivo == 5) ? textures[1] : textures[0]));
                if (TrenutniNivo - 1 != 6)
                    NapraviZid((int)(0.862 * MaxWidth), (int)(0.318 * MaxHeight), velicina, velicina, (TrenutniNivo > 6) ? textures[5] : ((TrenutniNivo == 6) ? textures[1] : textures[0]));
                TTF_SetFontSize(font[0], 0.45 * velicina);
                TTF_SizeText(font[0], (const char*)"1", &x[1], &y[1]);
                if (TrenutniNivo == 1)
                    PisiTekst(font[0], { 200, 150, 5, 255 }, { (int)(0.03 * MaxWidth), (int)(0.62 * MaxHeight), x[1], y[1] }, (char*)"1");
                else
                    PisiTekst(font[0], { 106,78,23,255 }, { (int)(0.03 * MaxWidth), (int)(0.62 * MaxHeight), x[1], y[1] }, (char*)"1");
                if (TrenutniNivo == 2)
                    PisiTekst(font[0], { 200, 150, 5, 255 }, { (int)(0.3 * MaxWidth), (int)(0.52 * MaxHeight), x[1], y[1] }, (char*)"2");
                else
                    PisiTekst(font[0], { 106,78,23,255 }, { (int)(0.3 * MaxWidth), (int)(0.52 * MaxHeight), x[1], y[1] }, (char*)"2");
                if (TrenutniNivo == 3)
                    PisiTekst(font[0], { 200, 150, 5, 255 }, { (int)(0.425 * MaxWidth), (int)(0.22 * MaxHeight), x[1], y[1] }, (char*)"3");
                else
                    PisiTekst(font[0], { 106,78,23,255 }, { (int)(0.425 * MaxWidth), (int)(0.22 * MaxHeight), x[1], y[1] }, (char*)"3");
                if (TrenutniNivo == 4)
                    PisiTekst(font[0], { 200, 150, 5, 255 }, { (int)(0.535 * MaxWidth), (int)(0.52 * MaxHeight), x[1], y[1] }, (char*)"4");
                else
                    PisiTekst(font[0], { 106,78,23,255 }, { (int)(0.535 * MaxWidth), (int)(0.52 * MaxHeight), x[1], y[1] }, (char*)"4");
                if (TrenutniNivo == 5)
                    PisiTekst(font[0], { 200, 150, 5, 255 }, { (int)(0.72 * MaxWidth), (int)(0.785 * MaxHeight), x[1], y[1] }, (char*)"5");
                else
                    PisiTekst(font[0], { 106,78,23,255 }, { (int)(0.72 * MaxWidth), (int)(0.785 * MaxHeight), x[1], y[1] }, (char*)"5");
                if (TrenutniNivo == 6)
                    PisiTekst(font[0], { 200, 150, 5, 255 }, { (int)(0.885 * MaxWidth), (int)(0.505 * MaxHeight), x[1], y[1] }, (char*)"6");
                else
                    PisiTekst(font[0], { 106,78,23,255 }, { (int)(0.885 * MaxWidth), (int)(0.505 * MaxHeight), x[1], y[1] }, (char*)"6");
                TTF_SetFontSize(font[0], 0.55 * velicina);
                TTF_SizeText(font[0], (const char*)"Sledeca Mapa", &x[1], &y[1]);
                PisiTekst(font[0], { 106,78,23,255 }, { (int)(MaxWidth - x[1]) / 2, (int)(0.82 * MaxHeight), x[1], y[1] }, (char*)"Sledeca Mapa");
                TTF_SetFontSize(font[0], 0.5 * velicina);
                char BrojMape[10];
                sprintf(BrojMape, "Mapa: %d", mapa + 1);
                TTF_SizeText(font[0], (const char*)BrojMape, &x[1], &y[1]);
                PisiTekst(font[0], { 68,29,12,255 }, { (int)(0.765 * MaxWidth), (int)(0.01 * MaxHeight), x[1], y[1] }, (char*)BrojMape);
                SDL_RenderPresent(renderer);
                i++;
                if (i == 6)
                    break;
                ProsloVreme = TrenutnoVreme;
            }
        }
        for (int i = 0; i < 6; i++) {
            SDL_DestroyTexture(textures[i]);
        }
        SDL_DestroyTexture(nazad);
        SDL_DestroyTexture(Pozadina);

    }
    else if (index == 12) {
        TTF_SetFontSize(font[0], 0.55 * velicina);
        TTF_SizeText(font[0], (const char*)"Sledeca Mapa", &x[1], &y[1]);
        PisiTekst(font[0], { 164,126,53,255 }, { (int)(MaxWidth - x[1]) / 2, (int)(0.82 * MaxHeight), x[1], y[1] }, (char*)"Sledeca Mapa");
    }
    else if (index == 13) {
        TTF_SetFontSize(font[0], 0.45 * velicina);
        if (TrenutniNivo == 1)
            PisiTekst(font[0], { 200, 150, 5, 255 }, { (int)(0.03 * MaxWidth), (int)(0.62 * MaxHeight), x[1], y[1] }, (char*)"1");
        else
            PisiTekst(font[0], { 106,78,23,255 }, { (int)(0.03 * MaxWidth), (int)(0.62 * MaxHeight), x[1], y[1] }, (char*)"1");
        if (TrenutniNivo == 2)
            PisiTekst(font[0], { 200, 150, 5, 255 }, { (int)(0.3 * MaxWidth), (int)(0.52 * MaxHeight), x[1], y[1] }, (char*)"2");
        else
            PisiTekst(font[0], { 106,78,23,255 }, { (int)(0.3 * MaxWidth), (int)(0.52 * MaxHeight), x[1], y[1] }, (char*)"2");
        if (TrenutniNivo == 3)
            PisiTekst(font[0], { 200, 150, 5, 255 }, { (int)(0.425 * MaxWidth), (int)(0.22 * MaxHeight), x[1], y[1] }, (char*)"3");
        else
            PisiTekst(font[0], { 106,78,23,255 }, { (int)(0.425 * MaxWidth), (int)(0.22 * MaxHeight), x[1], y[1] }, (char*)"3");
        if (TrenutniNivo == 4)
            PisiTekst(font[0], { 200, 150, 5, 255 }, { (int)(0.535 * MaxWidth), (int)(0.52 * MaxHeight), x[1], y[1] }, (char*)"4");
        else
            PisiTekst(font[0], { 106,78,23,255 }, { (int)(0.535 * MaxWidth), (int)(0.52 * MaxHeight), x[1], y[1] }, (char*)"4");
        if (TrenutniNivo == 5)
            PisiTekst(font[0], { 200, 150, 5, 255 }, { (int)(0.72 * MaxWidth), (int)(0.785 * MaxHeight), x[1], y[1] }, (char*)"5");
        else
            PisiTekst(font[0], { 106,78,23,255 }, { (int)(0.72 * MaxWidth), (int)(0.785 * MaxHeight), x[1], y[1] }, (char*)"5");
        if (TrenutniNivo == 6)
            PisiTekst(font[0], { 200, 150, 5, 255 }, { (int)(0.885 * MaxWidth), (int)(0.505 * MaxHeight), x[1], y[1] }, (char*)"6");
        else
            PisiTekst(font[0], { 106,78,23,255 }, { (int)(0.885 * MaxWidth), (int)(0.505 * MaxHeight), x[1], y[1] }, (char*)"6");
        TTF_SetFontSize(font[0], 0.55 * velicina);
        TTF_SizeText(font[0], (const char*)"Sledeca Mapa", &x[1], &y[1]);
        PisiTekst(font[0], { 106,78,23,255 }, { (int)(MaxWidth - x[1]) / 2, (int)(0.82 * MaxHeight), x[1], y[1] }, (char*)"Sledeca Mapa");
    }
    else if (index == 14) {
        SDL_Surface* pozadina = IMG_Load("Slike/Mapa.png");
        SDL_Texture* Pozadina = SDL_CreateTextureFromSurface(renderer, pozadina);
        SDL_Texture* nazad = SDL_CreateTextureFromSurface(renderer, surface[5]);
        NapraviZid(0, 0, MaxWidth, MaxHeight, Pozadina);
        NapraviZid(0.03 * MaxWidth, 0.03 * MaxHeight, velicina, velicina, nazad);
        SDL_DestroyTexture(nazad);
        SDL_FreeSurface(pozadina);
        SDL_DestroyTexture(Pozadina);
        TTF_SetFontSize(font[0], 0.45 * velicina);
        TTF_SizeText(font[0], (const char*)"1", &x[1], &y[1]);
        if (TrenutniNivo == 1)
            PisiTekst(font[0], { 200, 150, 5, 255 }, { (int)(0.03 * MaxWidth), (int)(0.62 * MaxHeight), x[1], y[1] }, (char*)"1");
        else
            PisiTekst(font[0], { 106,78,23,255 }, { (int)(0.03 * MaxWidth), (int)(0.62 * MaxHeight), x[1], y[1] }, (char*)"1");
        if (TrenutniNivo == 2)
            PisiTekst(font[0], { 200, 150, 5, 255 }, { (int)(0.3 * MaxWidth), (int)(0.52 * MaxHeight), x[1], y[1] }, (char*)"2");
        else
            PisiTekst(font[0], { 106,78,23,255 }, { (int)(0.3 * MaxWidth), (int)(0.52 * MaxHeight), x[1], y[1] }, (char*)"2");
        if (TrenutniNivo == 3)
            PisiTekst(font[0], { 200, 150, 5, 255 }, { (int)(0.425 * MaxWidth), (int)(0.22 * MaxHeight), x[1], y[1] }, (char*)"3");
        else
            PisiTekst(font[0], { 106,78,23,255 }, { (int)(0.425 * MaxWidth), (int)(0.22 * MaxHeight), x[1], y[1] }, (char*)"3");
        if (TrenutniNivo == 4)
            PisiTekst(font[0], { 200, 150, 5, 255 }, { (int)(0.535 * MaxWidth), (int)(0.52 * MaxHeight), x[1], y[1] }, (char*)"4");
        else
            PisiTekst(font[0], { 106,78,23,255 }, { (int)(0.535 * MaxWidth), (int)(0.52 * MaxHeight), x[1], y[1] }, (char*)"4");
        if (TrenutniNivo == 5)
            PisiTekst(font[0], { 200, 150, 5, 255 }, { (int)(0.72 * MaxWidth), (int)(0.785 * MaxHeight), x[1], y[1] }, (char*)"5");
        else
            PisiTekst(font[0], { 106,78,23,255 }, { (int)(0.72 * MaxWidth), (int)(0.785 * MaxHeight), x[1], y[1] }, (char*)"5");
        if (TrenutniNivo == 6)
            PisiTekst(font[0], { 200, 150, 5, 255 }, { (int)(0.885 * MaxWidth), (int)(0.505 * MaxHeight), x[1], y[1] }, (char*)"6");
        else
            PisiTekst(font[0], { 106,78,23,255 }, { (int)(0.885 * MaxWidth), (int)(0.505 * MaxHeight), x[1], y[1] }, (char*)"6");
        TTF_SetFontSize(font[0], 0.55 * velicina);
        TTF_SizeText(font[0], (const char*)"Sledeca Mapa", &x[1], &y[1]);
        PisiTekst(font[0], { 106,78,23,255 }, { (int)(MaxWidth - x[1]) / 2, (int)(0.82 * MaxHeight), x[1], y[1] }, (char*)"Sledeca Mapa");
        TTF_SetFontSize(font[0], 0.5 * velicina);
        char BrojMape[10];
        sprintf(BrojMape, "Mapa: %d", mapa + 1);
        TTF_SizeText(font[0], (const char*)BrojMape, &x[1], &y[1]);
        PisiTekst(font[0], { 68,29,12,255 }, { (int)(0.765 * MaxWidth), (int)(0.01 * MaxHeight), x[1], y[1] }, (char*)BrojMape);
        SDL_Surface* zatvorenkovceg = IMG_Load("Slike/Kovceg/0.png");
        SDL_Texture* ZatvorenKovceg = SDL_CreateTextureFromSurface(renderer, zatvorenkovceg);
        SDL_Surface* poluotvorenenkovceg = IMG_Load("Slike/Kovceg/1.png");
        SDL_Texture* PoluOtvorenenKovceg = SDL_CreateTextureFromSurface(renderer, poluotvorenenkovceg);
        SDL_Surface* otvorenenkovceg = IMG_Load("Slike/Kovceg/5.png");
        SDL_Texture* OtvorenenKovceg = SDL_CreateTextureFromSurface(renderer, otvorenenkovceg);
        NapraviZid((int)(0.025 * MaxWidth), (int)(0.44 * MaxHeight), velicina, velicina, (TrenutniNivo > 1) ? OtvorenenKovceg : ((TrenutniNivo == 1) ? PoluOtvorenenKovceg : ZatvorenKovceg));
        NapraviZid((int)(0.255 * MaxWidth), (int)(0.35 * MaxHeight), velicina, velicina, (TrenutniNivo > 2) ? OtvorenenKovceg : ((TrenutniNivo == 2) ? PoluOtvorenenKovceg : ZatvorenKovceg));
        NapraviZid((int)(0.41 * MaxWidth), (int)(0.03 * MaxHeight), velicina, velicina, (TrenutniNivo > 3) ? OtvorenenKovceg : ((TrenutniNivo == 3) ? PoluOtvorenenKovceg : ZatvorenKovceg));
        NapraviZid((int)(0.59 * MaxWidth), (int)(0.46 * MaxHeight), velicina, velicina, (TrenutniNivo > 4) ? OtvorenenKovceg : ((TrenutniNivo == 4) ? PoluOtvorenenKovceg : ZatvorenKovceg));
        NapraviZid((int)(0.7 * MaxWidth), (int)(0.6 * MaxHeight), velicina, velicina, (TrenutniNivo > 5) ? OtvorenenKovceg : ((TrenutniNivo == 5) ? PoluOtvorenenKovceg : ZatvorenKovceg));
        NapraviZid((int)(0.862 * MaxWidth), (int)(0.318 * MaxHeight), velicina, velicina, (TrenutniNivo > 6) ? OtvorenenKovceg : ((TrenutniNivo == 6) ? PoluOtvorenenKovceg : ZatvorenKovceg));
        SDL_FreeSurface(poluotvorenenkovceg);
        SDL_DestroyTexture(PoluOtvorenenKovceg);
        SDL_FreeSurface(otvorenenkovceg);
        SDL_DestroyTexture(OtvorenenKovceg);
        SDL_FreeSurface(zatvorenkovceg);
        SDL_DestroyTexture(ZatvorenKovceg);
    }
}
void Mapa(SDL_Surface** surface, int index1) {
    int x[3], y[3], index = 10;
    TTF_SetFontSize(font[0], 0.45 * velicina);
    TTF_SizeText(font[0], (const char*)"1", &x[1], &y[1]);
    TTF_SetFontSize(font[0], 0.55 * velicina);
    TTF_SizeText(font[0], (const char*)"Sledeca Mapa", &x[2], &y[2]);
    if (index1 == 0) {
        CrtajMapa(surface, 0);
        SDL_RenderPresent(renderer);
    }
    else if (index1 == 1) {
        PustiZvuk(zvukovi[5]);
        CrtajMapa(surface, 8);
    }
    else if (index1 == 2) {
        PustiZvuk(zvukovi[5]);
        CrtajMapa(surface, 11);
    }
    else if (index1 == 3) {
        CrtajMapa(surface, 14);
        SDL_RenderPresent(renderer);
    }
    while (1) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                SDL_Quit();
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN) {
                SDL_GetMouseState(&x[0], &y[0]);
                if (x[0] >= (0.03 * MaxWidth) && x[0] <= (0.03 * MaxWidth + x[1]) && y[0] >= (0.62 * MaxHeight) && y[0] <= ((0.62 * MaxHeight) + y[1]) && TrenutniNivo >= 1) {
                    PustiZvuk(zvukovi[1]);
                    UsoNivo = 1;
                    Igra(surface, SDL_GetTicks());
                }
                else if (x[0] >= (0.3 * MaxWidth) && x[0] <= (0.3 * MaxWidth + x[1]) && y[0] >= (0.52 * MaxHeight) && y[0] <= ((0.52 * MaxHeight) + y[1]) && TrenutniNivo >= 2) {
                    PustiZvuk(zvukovi[1]);
                    UsoNivo = 2;
                    Igra(surface, SDL_GetTicks());
                }
                else if (x[0] >= (0.425 * MaxWidth) && x[0] <= (0.425 * MaxWidth + x[1]) && y[0] >= (0.22 * MaxHeight) && y[0] <= ((0.22 * MaxHeight) + y[1]) && TrenutniNivo >= 3) {
                    PustiZvuk(zvukovi[1]);
                    UsoNivo = 3;
                    Igra(surface, SDL_GetTicks());
                }
                else if (x[0] >= (0.535 * MaxWidth) && x[0] <= (0.535 * MaxWidth + x[1]) && y[0] >= (0.52 * MaxHeight) && y[0] <= ((0.52 * MaxHeight) + y[1]) && TrenutniNivo >= 4) {
                    PustiZvuk(zvukovi[1]);
                    UsoNivo = 4;
                    Igra(surface, SDL_GetTicks());
                }
                else if (x[0] >= (0.72 * MaxWidth) && x[0] <= (0.72 * MaxWidth + x[1]) && y[0] >= (0.785 * MaxHeight) && y[0] <= ((0.785 * MaxHeight) + y[1]) && TrenutniNivo >= 5) {
                    PustiZvuk(zvukovi[1]);
                    UsoNivo = 5;
                    Igra(surface, SDL_GetTicks());
                }
                else if (x[0] >= (0.885 * MaxWidth) && x[0] <= (0.885 * MaxWidth + x[1]) && y[0] >= (0.505 * MaxHeight) && y[0] <= ((0.505 * MaxHeight) + y[1]) && TrenutniNivo >= 6) {
                    PustiZvuk(zvukovi[1]);
                    UsoNivo = 6;
                    Igra(surface, SDL_GetTicks());;
                }
                else if (x[0] >= (0.03 * MaxWidth) && x[0] <= (0.03 * MaxWidth + velicina) && y[0] >= (0.03 * MaxHeight) && y[0] <= (0.03 * MaxHeight + velicina)) {
                    PustiZvuk(zvukovi[1]);
                    PustiZvuk(zvukovi[0]);
                    if (PrviPut1 == 1) {
                        PrviPut1 = 0;
                        n = n1;
                    }
                    PrelazNaGlavniMeni(surface);
                }
                else if (x[0] >= ((MaxWidth - x[2]) / 2) && x[0] <= ((MaxWidth + x[2]) / 2) && y[0] >= (0.82 * MaxHeight) && y[0] <= (0.82 * MaxHeight + y[2]) && (index1 == 2 || index1 == 3)) {
                    PustiZvuk(zvukovi[1]);
                    if (mapa < 2) {
                        mapa++;
                        TrenutniNivo = 1;
                        FILE* file = NULL;
                        fopen_s(&file, (const char*)"Opcije.txt", (const char*)"w");
                        fprintf_s(file, "%d %d\n%d\n%d %d %d\n", MaxWidth, MaxHeight, n, Tezina, Algoritam, BrZombija);
                        fprintf_s(file, "%d %d %d\n", Izgledi[0], Izgledi[1], Izgledi[2]);
                        fprintf_s(file, "%d %d %d\n", TrenutniNivo, mapa, Sacuvaj);
                        fprintf_s(file, "%d %d", Zvukovi, Pozadinska);
                        fclose(file);
                        Mapa(surface, 0);
                    }
                }
            }
            else {
                SDL_GetMouseState(&x[0], &y[0]);
                if (x[0] >= (0.03 * MaxWidth) && x[0] <= (0.03 * MaxWidth + x[1]) && y[0] >= (0.62 * MaxHeight) && y[0] <= ((0.62 * MaxHeight) + y[1]) && TrenutniNivo >= 1) {
                    index = 1;
                    CrtajMapa(surface, index);
                    SDL_RenderPresent(renderer);
                }
                else if (x[0] >= (0.3 * MaxWidth) && x[0] <= (0.3 * MaxWidth + x[1]) && y[0] >= (0.52 * MaxHeight) && y[0] <= ((0.52 * MaxHeight) + y[1]) && TrenutniNivo >= 2) {
                    index = 2;
                    CrtajMapa(surface, index);
                    SDL_RenderPresent(renderer);
                }
                else if (x[0] >= (0.425 * MaxWidth) && x[0] <= (0.425 * MaxWidth + x[1]) && y[0] >= (0.22 * MaxHeight) && y[0] <= ((0.22 * MaxHeight) + y[1]) && TrenutniNivo >= 3) {
                    index = 3;
                    CrtajMapa(surface, index);
                    SDL_RenderPresent(renderer);
                }
                else if (x[0] >= (0.535 * MaxWidth) && x[0] <= (0.535 * MaxWidth + x[1]) && y[0] >= (0.52 * MaxHeight) && y[0] <= ((0.52 * MaxHeight) + y[1]) && TrenutniNivo >= 4) {
                    index = 4;
                    CrtajMapa(surface, index);
                    SDL_RenderPresent(renderer);
                }
                else if (x[0] >= (0.72 * MaxWidth) && x[0] <= (0.72 * MaxWidth + x[1]) && y[0] >= (0.785 * MaxHeight) && y[0] <= ((0.785 * MaxHeight) + y[1]) && TrenutniNivo >= 5) {
                    index = 5;
                    CrtajMapa(surface, index);
                    SDL_RenderPresent(renderer);
                }
                else if (x[0] >= (0.885 * MaxWidth) && x[0] <= (0.885 * MaxWidth + x[1]) && y[0] >= (0.505 * MaxHeight) && y[0] <= ((0.505 * MaxHeight) + y[1]) && TrenutniNivo >= 6) {
                    index = 6;
                    CrtajMapa(surface, index);
                    SDL_RenderPresent(renderer);
                }
                else if (index < 7) {
                    index = 7;
                    CrtajMapa(surface, index);
                    SDL_RenderPresent(renderer);
                }
                else if (x[0] >= (0.03 * MaxWidth) && x[0] <= (0.03 * MaxWidth + velicina) && y[0] >= (0.03 * MaxHeight) && y[0] <= (0.03 * MaxHeight + velicina)) {
                    index = 9;
                    CrtajMapa(surface, index);
                    SDL_RenderPresent(renderer);
                }
                else if (x[0] >= ((MaxWidth - x[2]) / 2) && x[0] <= ((MaxWidth + x[2]) / 2) && y[0] >= (0.82 * MaxHeight) && y[0] <= (0.82 * MaxHeight + y[2]) && (index1 == 2 || index1 == 3)) {
                    index = 12;
                    CrtajMapa(surface, index);
                    SDL_RenderPresent(renderer);
                }
                else if (index == 12 && (index1 == 2 || index1 == 3)) {
                    index = 13;
                    CrtajMapa(surface, index);
                    SDL_RenderPresent(renderer);
                }
                else if (index == 9) {
                    index = 10;
                    CrtajMapa(surface, index);
                    SDL_RenderPresent(renderer);
                }
            }

        }
    }

}
void Tutorial(SDL_Surface** surface, Uint32 Gubitak) {
    PauzaVreme = Gubitak;
    ProsloVreme1 = SDL_GetTicks() - PauzaVreme;
    Uint32 ProsloVreme = SDL_GetTicks(), interval = 1000 / 60, TrenutnoVreme;
    int x, y, x2 = 0, y2 = 0, radi = 1, BrKoraka = 0, Xmis, Ymis, w[8], h[8], LastIndex = -1, velicina1 = Velicina(), index = 9, intervalx = 0, i, j;
    char sekund[5], adresa[50], tutorijal[10][200];
    strcpy(minut, "0:00");
    TTF_SetFontSize(font[1], 6 * velicina / 10);
    int pametna = 0;
    Pozicija izlaz;
    int n1 = n;
    struct Maze* talon = (struct Maze*)malloc((2 * n - 1) * (2 * n - 1) * sizeof(struct Maze));
    struct Maze* maze = (struct Maze*)malloc(n * n * sizeof(struct Maze));
    Pozicija* pozicijeucitane = ucitaj_binarnu_datoteku("Nivo/Tutorial.bin", &talon, &n, &BrZombija, &pametna);
    velicina1 = Velicina();
    velicina = Velicina1();
    Pozicija igracpoz = pozicijeucitane[0];
    izlaz = pozicijeucitane[1];
    izlazx = izlaz.x;
    izlazy = izlaz.y;
    mumije = (Pozicija*)calloc(BrZombija, sizeof(Pozicija));
    trbrojmumija = BrZombija;
    for (i = 0; i < trbrojmumija; i++) {
        mumije[i] = pozicijeucitane[2 + i];
        mumije[i].xm = 12 * MaxWidth / 25 + velicina1 * mumije[i].y;
        mumije[i].ym = 2 * MaxHeight / 15 + mumije[i].x * velicina1;
    }
    for (i = 0; i < n; i++)
        for (j = 0; j < n; j++)
            maze[i * n + j].matrica = 0;
    maze[izlaz.x * n + izlaz.y].matrica = 1;
    talon[2 * izlaz.x * (2 * n - 1) + izlaz.y * 2].matrica = 2;
    x = igracpoz.x - 1;
    y = igracpoz.y;
    trbrojmumija = BrZombija;
    CrtajSve(surface, talon, 0, LastIndex, &x2, &y2);
    IgracX = 0.48 * MaxWidth + velicina1 * y;
    IgracY = 2 * MaxHeight / 15 + x * velicina1;
    sprintf_s(adresa, "Slike/Mumija/Desno/%d/MumijaDesno.png", Izgledi[0]);
    SDL_Surface* mumijad = IMG_Load(adresa);
    sprintf_s(adresa, "Slike/Mumija/Levo/%d/MumijaLevo.png", Izgledi[0]);
    SDL_Surface* mumijal = IMG_Load(adresa);
    sprintf_s(adresa, "Slike/Igrac/Nazad/%d/walk/%d.png", Izgledi[1], 0);
    SDL_Surface* igrac = IMG_Load(adresa);
    mumijadesna = SDL_CreateTextureFromSurface(renderer, mumijad);
    mumijaleva = SDL_CreateTextureFromSurface(renderer, mumijal);
    Igrac = SDL_CreateTextureFromSurface(renderer, igrac);
    SDL_FreeSurface(mumijad);
    SDL_FreeSurface(mumijal);
    SDL_FreeSurface(igrac);
    SDL_Surface* prica[30];
    SDL_Texture* Prica[31];
    for (i = 0; i < 30; i++) {
        sprintf(adresa, "Slike/Igrac/Nazad/1/Prica/%d.png", i);
        prica[i] = IMG_Load(adresa);
        Prica[i] = SDL_CreateTextureFromSurface(renderer, prica[i]);
        SDL_FreeSurface(prica[i]);
    }
    for (i = 0; i < trbrojmumija; i++)
        NapraviZid(mumije[i].xm + velicina1 * 0.1, mumije[i].ym + velicina1 * 0.1, 0.8 * velicina1, 0.8 * velicina1, mumijaleva);
    NapraviZid(IgracX + 0.1 * velicina1, IgracY + 0.1 * velicina1, 0.8 * velicina1, 0.8 * velicina1, Igrac);
    PisiTekst(font[1], { 0,0,0, 255 }, { (int)(MaxWidth - 1.25 * velicina), (int)(0),(int)(velicina * 1.5),(int)(velicina * 0.5) }, (char*)minut);
    SDL_RenderPresent(renderer);
    i = 0;
    prica[0] = IMG_Load("Slike/Igrac/chat/2.png");
    Prica[30] = SDL_CreateTextureFromSurface(renderer, prica[0]);
    SDL_FreeSurface(prica[0]);
    char Poruka[100];
    int BrLinija = 0;
    int TrenutnaLinija = 0;
    int TrenutniKarakter = 0;
    int TrenutniFajl = 0;
    int CekamInput = 0;
    int CekamPomeraj = 0;
    while (radi) {
        TrenutnoVreme = SDL_GetTicks();
        if (TrenutnoVreme - ProsloVreme >= interval) {
            TrenutnoVreme1 = SDL_GetTicks() - PauzaVreme;
            if (TrenutnoVreme1 - ProsloVreme1 >= interval1) {
                ProsloVreme1 = TrenutnoVreme1;
                sprintf_s(minut, "%d", TrenutnoVreme1 / 60000);
                sprintf_s(sekund, "%d", TrenutnoVreme1 / 1000 - (TrenutnoVreme1 / 60000) * 60);
                strcat_s(minut, ":");
                if ((TrenutnoVreme1 / 1000 - (TrenutnoVreme1 / 60000) * 60) < 10)strcat_s(minut, "0");
                strcat_s(minut, sekund);
            }
            Animacije(surface, talon, intervalx, 0, 1);
            if (i < 60)
                NapraviZid(-0.05 * MaxWidth, 0.37 * MaxHeight, 5 * velicina, 5 * velicina, Prica[i / 2]);
            else
                NapraviZid(-0.05 * MaxWidth, 0.37 * MaxHeight, 5 * velicina, 5 * velicina, Prica[0]);
            i++;
            NapraviZid(0.04 * MaxWidth, 0.1 * MaxHeight, 4 * velicina, 4 * velicina, Prica[30]);

            TTF_SetFontSize(font[1], 0.2 * velicina);
            if (TrenutnaLinija < BrLinija) {
                if (tutorijal[TrenutnaLinija][TrenutniKarakter] != '\0') {
                    strncpy(Poruka, tutorijal[TrenutnaLinija], TrenutniKarakter);
                    Poruka[TrenutniKarakter] = '\0';
                    TTF_SizeText(font[1], (const char*)Poruka, &w[6], &h[6]);
                    PisiTekst(font[1], { 0,0,0,255 }, { (int)(0.07 * MaxWidth + 4 * velicina - w[6]) / 2, (int)(0.16 * MaxHeight + 0.05 * MaxHeight * TrenutnaLinija), w[6], h[6] }, (char*)Poruka);
                    TrenutniKarakter++;
                }
                else {
                    tutorijal[TrenutnaLinija][TrenutniKarakter - 1] = '\0';
                    TrenutnaLinija++;
                    TrenutniKarakter = 0;
                }
            }
            else if (TrenutnaLinija == BrLinija && TrenutniFajl < 8 && CekamInput == 0) {
                FILE* file = NULL;
                sprintf(adresa, "Tutorijal/Tutorijal%d.txt", TrenutniFajl);
                fopen_s(&file, adresa, (const char*)"r");
                BrLinija = 0;
                while (fgets(tutorijal[BrLinija], sizeof(tutorijal[BrLinija]), file) != NULL)
                    BrLinija++;
                fclose(file);
                TrenutniFajl++;
                TrenutnaLinija = 0;
                TrenutniKarakter = 0;
            }
            if (TrenutnaLinija == BrLinija && TrenutniFajl < 9 && CekamInput == 0) {
                if (TrenutniFajl >= 5) {
                    CekamPomeraj = 1;
                }
                CekamInput = 1;
            }
            if (TrenutniFajl < 5) {
                SDL_PollEvent(&event);
                if (event.type == SDL_KEYDOWN || event.type == SDL_MOUSEBUTTONDOWN) {
                    PustiZvuk(zvukovi[1]);
                    CekamInput = 0;
                }
            }
            for (int j = 0; j < TrenutnaLinija; j++) {
                TTF_SizeText(font[1], (const char*)tutorijal[j], &w[6], &h[6]);
                PisiTekst(font[1], { 0,0,0,255 }, { (int)(0.07 * MaxWidth + 4 * velicina - w[6]) / 2, (int)(0.16 * MaxHeight + 0.05 * MaxHeight * j), w[6], h[6] }, (char*)tutorijal[j]);
            }
            if (i == 90)
                i = 0;
            intervalx += 3;
            if (intervalx >= (12 * MaxWidth / 25 - velicina))
                intervalx = 0;
            ProsloVreme = TrenutnoVreme;
            SDL_RenderPresent(renderer);
        }
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) radi = 0;
            if (event.type == SDL_KEYDOWN && CekamPomeraj == 1) {
                if (event.key.keysym.sym == SDLK_w || event.key.keysym.sym == SDLK_UP) {
                    CekamPomeraj = 0;
                    CekamInput = 0;
                    if (ProveraPobede(x2, y2, x - 1, y)) {
                        int channel = Mix_PlayChannel(-1, zvukovi[6], 0);
                        Mix_PlayChannelTimed(channel, zvukovi[6], 1, 1200);
                        Pokret(surface, talon, IgracX, IgracY, velicina1, 4, &intervalx, minut);
                        n = n1;
                        for (int i = 0; i < 31; i++)
                            SDL_DestroyTexture(Prica[i]);
                        ModIgre = 2;
                        Pobeda(surface, BrKoraka + 1, TrenutnoVreme1);
                    }
                    if (Validan(x - 1, y) && Zid(x - 1, y, x, y, talon)) {
                        int channel = Mix_PlayChannel(-1, zvukovi[6], 0);
                        Mix_PlayChannelTimed(channel, zvukovi[6], 1, 1200);
                        PrviPut = 0;
                        Pokret(surface, talon, IgracX, IgracY, velicina1, 4, &intervalx, minut);
                        BrKoraka++;
                        i = 90;
                        x -= 1;
                        IgracY -= velicina1;
                        CrtajSve(surface, talon, 1);
                        PisiTekst(font[1], { 0,0,0, 255 }, { (int)(MaxWidth - velicina * 1.25), 0,(int)(velicina * 1.5),(int)(velicina * 0.5) }, (char*)minut);
                        for (int i = 0; i < trbrojmumija; i++)
                            PomeriMumiju(surface, x, y, talon, i, &intervalx, minut);
                        CrtajSve(surface, talon, 1);
                        NapraviZid(IgracX + 0.1 * velicina1, IgracY + 0.1 * velicina1, 0.8 * velicina1, 0.8 * velicina1, Igrac);
                        for (int i = 0; i < trbrojmumija; i++)
                            NapraviZid(mumije[i].xm + velicina1 * 0.1, mumije[i].ym + velicina1 * 0.1, 0.8 * velicina1, 0.8 * velicina1, (StranaMumije[i] == 0) ? mumijaleva : mumijadesna);
                        SDL_RenderPresent(renderer);
                    }
                }
            }
        }
    }
    SDL_Quit();
}
void Biraj(SDL_Surface** surface) {
    int interval = 1000 / 90, smer = 0, x[4], y[4], index = 3;
    float pomerix1 = 0.1 * MaxWidth, pomerix = 0.45 * MaxWidth;
    Uint32 ProsloVreme = SDL_GetTicks(), TrenutnoVreme;
    SDL_Surface* pozadina = IMG_Load("Slike/Biraj.png");
    SDL_Texture* Pozadina = SDL_CreateTextureFromSurface(renderer, pozadina);
    NapraviZid(0, 0, MaxWidth, MaxHeight, Pozadina);
    SDL_FreeSurface(pozadina);
    SDL_Surface* skrol = IMG_Load("Slike/Pobeda.png");
    SDL_Texture* Skrol = SDL_CreateTextureFromSurface(renderer, skrol);
    SDL_FreeSurface(skrol);
    TTF_SetFontSize(font[0], 0.65 * velicina);
    TTF_SizeText(font[0], (const char*)"Tutorijal", &x[1], &y[1]);
    TTF_SizeText(font[0], (const char*)"Avantura", &x[2], &y[2]);
    TTF_SizeText(font[0], (const char*)"Klasik", &x[3], &y[3]);
    while (1) {
        TrenutnoVreme = SDL_GetTicks();
        if (TrenutnoVreme - ProsloVreme >= interval && smer == 0) {
            if (smer == 0) {
                DeoTeksture(0.1 * MaxWidth, 0.05 * MaxHeight, 0.8 * MaxWidth, 0.9 * MaxHeight, 0.1 * 1128.0, 0.05 * 634.0, 0.8 * 1128.0, 0.9 * 634.0, Pozadina);
                NapraviZid(pomerix, 0.05 * MaxHeight, pomerix1, 0.9 * MaxHeight, Skrol);
                SDL_RenderPresent(renderer);
                pomerix -= 0.4 * MaxWidth / 90;
                pomerix1 += 0.8 * MaxWidth / 90;
                if (pomerix <= 0.05 * MaxWidth) {
                    smer = 1;
                    NapraviZid(0, 0, MaxWidth, MaxHeight, Pozadina);
                    NapraviZid(0.05 * MaxWidth, 0.05 * MaxHeight, 0.9 * MaxWidth, 0.9 * MaxHeight, Skrol);
                    PisiTekst(font[0], { 100,80,40,255 }, { (int)(MaxWidth - x[1]) / 2, (int)(0.35 * MaxHeight), x[1],y[1] }, (char*)"Tutorijal");
                    PisiTekst(font[0], { 100,80,40,255 }, { (int)(MaxWidth - x[2]) / 2, (int)(0.48 * MaxHeight), x[2],y[2] }, (char*)"Avantura");
                    PisiTekst(font[0], { 100,80,40,255 }, { (int)(MaxWidth - x[3]) / 2, (int)(0.61 * MaxHeight), x[3],y[3] }, (char*)"Klasik");
                    SDL_RenderPresent(renderer);
                }
            }
            ProsloVreme = TrenutnoVreme;
        }
        while (SDL_PollEvent(&event) != 0 && smer == 1) {
            if (event.type == SDL_QUIT) {
                SDL_Quit();
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN) {
                SDL_GetMouseState(&x[0], &y[0]);
                if (x[0] >= ((MaxWidth - x[1]) / 2) && x[0] <= ((MaxWidth + x[1]) / 2) && y[0] >= (0.35 * MaxHeight) && y[0] <= ((0.35 * MaxHeight) + y[1])) {
                    PustiZvuk(zvukovi[1]);
                    ModIgre = 0;
                    while (1) {
                        TrenutnoVreme = SDL_GetTicks();
                        if (TrenutnoVreme - ProsloVreme >= interval && smer == 1) {
                            if (smer == 1) {
                                DeoTeksture(0.1 * MaxWidth, 0.05 * MaxHeight, 0.8 * MaxWidth, 0.9 * MaxHeight, 0.1 * 1128.0, 0.05 * 634.0, 0.8 * 1128.0, 0.9 * 634.0, Pozadina);
                                NapraviZid(pomerix, 0.05 * MaxHeight, pomerix1, 0.9 * MaxHeight, Skrol);
                                SDL_RenderPresent(renderer);
                                pomerix += 0.4 * MaxWidth / 90;
                                pomerix1 -= 0.8 * MaxWidth / 90;
                                if (pomerix >= 0.4 * MaxWidth) {
                                    break;
                                }
                            }
                            ProsloVreme = TrenutnoVreme;
                        }
                    }
                    SDL_DestroyTexture(Pozadina);
                    SDL_DestroyTexture(Skrol);
                    Tutorial(surface, SDL_GetTicks());
                }
                else if (x[0] >= ((MaxWidth - x[2]) / 2) && x[0] <= ((MaxWidth + x[2]) / 2) && y[0] >= (0.48 * MaxHeight) && y[0] <= ((0.48 * MaxHeight) + y[2])) {
                    PustiZvuk(zvukovi[1]);
                    ModIgre = 1;
                    while (1) {
                        TrenutnoVreme = SDL_GetTicks();
                        if (TrenutnoVreme - ProsloVreme >= interval && smer == 1) {
                            if (smer == 1) {
                                DeoTeksture(0.1 * MaxWidth, 0.05 * MaxHeight, 0.8 * MaxWidth, 0.9 * MaxHeight, 0.1 * 1128.0, 0.05 * 634.0, 0.8 * 1128.0, 0.9 * 634.0, Pozadina);
                                NapraviZid(pomerix, 0.05 * MaxHeight, pomerix1, 0.9 * MaxHeight, Skrol);
                                SDL_RenderPresent(renderer);
                                pomerix += 0.4 * MaxWidth / 90;
                                pomerix1 -= 0.8 * MaxWidth / 90;
                                if (pomerix >= 0.4 * MaxWidth) {
                                    break;
                                }
                            }
                            ProsloVreme = TrenutnoVreme;
                        }
                    }
                    SDL_DestroyTexture(Pozadina);
                    SDL_DestroyTexture(Skrol);
                    if (TrenutniNivo > 6) {
                        Mapa(surface, 3);
                    }
                    else {
                        Mapa(surface, 0);
                    }
                }
                else if (x[0] >= ((MaxWidth - x[3]) / 2) && x[0] <= ((MaxWidth + x[3]) / 2) && y[0] >= (0.61 * MaxHeight) && y[0] <= ((0.61 * MaxHeight) + y[3])) {
                    PustiZvuk(zvukovi[1]);
                    ModIgre = 2;
                    while (1) {
                        TrenutnoVreme = SDL_GetTicks();
                        if (TrenutnoVreme - ProsloVreme >= interval && smer == 1) {
                            if (smer == 1) {
                                DeoTeksture(0.1 * MaxWidth, 0.05 * MaxHeight, 0.8 * MaxWidth, 0.9 * MaxHeight, 0.1 * 1128.0, 0.05 * 634.0, 0.8 * 1128.0, 0.9 * 634.0, Pozadina);
                                NapraviZid(pomerix, 0.05 * MaxHeight, pomerix1, 0.9 * MaxHeight, Skrol);
                                SDL_RenderPresent(renderer);
                                pomerix += 0.4 * MaxWidth / 90;
                                pomerix1 -= 0.8 * MaxWidth / 90;
                                if (pomerix >= 0.4 * MaxWidth) {
                                    break;
                                }
                            }
                            ProsloVreme = TrenutnoVreme;
                        }
                    }
                    SDL_DestroyTexture(Pozadina);
                    SDL_DestroyTexture(Skrol);
                    Igra(surface, SDL_GetTicks());
                }
            }
            else {
                SDL_GetMouseState(&x[0], &y[0]);
                if (x[0] >= ((MaxWidth - x[1]) / 2) && x[0] <= ((MaxWidth + x[1]) / 2) && y[0] >= (0.35 * MaxHeight) && y[0] <= ((0.35 * MaxHeight) + y[1])) {
                    index = 0;
                    PisiTekst(font[0], { 144,110,36,255 }, { (int)(MaxWidth - x[1]) / 2, (int)(0.35 * MaxHeight), x[1],y[1] }, (char*)"Tutorijal");
                    SDL_RenderPresent(renderer);
                }
                else if (x[0] >= ((MaxWidth - x[2]) / 2) && x[0] <= ((MaxWidth + x[2]) / 2) && y[0] >= (0.48 * MaxHeight) && y[0] <= ((0.48 * MaxHeight) + y[2])) {
                    index = 1;
                    PisiTekst(font[0], { 144,110,36,255 }, { (int)(MaxWidth - x[2]) / 2, (int)(0.48 * MaxHeight), x[1],y[1] }, (char*)"Avantura");
                    SDL_RenderPresent(renderer);
                }
                else if (x[0] >= ((MaxWidth - x[3]) / 2) && x[0] <= ((MaxWidth + x[3]) / 2) && y[0] >= (0.61 * MaxHeight) && y[0] <= ((0.61 * MaxHeight) + y[3])) {
                    index = 2;
                    PisiTekst(font[0], { 144,110,36,255 }, { (int)(MaxWidth - x[3]) / 2, (int)(0.61 * MaxHeight), x[1],y[1] }, (char*)"Klasik");
                    SDL_RenderPresent(renderer);
                }
                else if (index < 3) {
                    index = 3;
                    PisiTekst(font[0], { 100,80,40,255 }, { (int)(MaxWidth - x[1]) / 2, (int)(0.35 * MaxHeight), x[1],y[1] }, (char*)"Tutorijal");
                    PisiTekst(font[0], { 100,80,40,255 }, { (int)(MaxWidth - x[2]) / 2, (int)(0.48 * MaxHeight), x[1],y[1] }, (char*)"Avantura");
                    PisiTekst(font[0], { 100,80,40,255 }, { (int)(MaxWidth - x[3]) / 2, (int)(0.61 * MaxHeight), x[1],y[1] }, (char*)"Klasik");
                    SDL_RenderPresent(renderer);
                }
            }
        }
    }
    SDL_Delay(5000);
}
void CrtajGlavniMeni(SDL_Surface** surface, int index, int LastIndex, int frame = 0) {
    int x, y, x1, y1, x2, x3, x4;
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface[frame]);
    NapraviZid(0, 0, MaxWidth, MaxHeight, texture);
    SDL_DestroyTexture(texture);
    TTF_SetFontSize(font[0], velicina);
    TTF_SizeText(font[0], (const char*)"MUMMY MAZE", &x, &y);
    TTF_SetFontSize(font[0], 3 * velicina / 4);
    TTF_SizeText(font[0], (const char*)"Start", &x1, &y1);
    TTF_SizeText(font[0], (const char*)"Opcije", &x2, &y1);
    TTF_SizeText(font[0], (const char*)"Info", &x3, &y1);
    TTF_SizeText(font[0], (const char*)"Izadji", &x4, &y1);
    TTF_SetFontSize(font[0], velicina);
    PisiTekst(font[0], { 0,0,0,255 }, { (MaxWidth - x) / 2, (int)(0.01 * MaxHeight), (int)(0.5 * MaxWidth), (int)(0.2 * MaxHeight) }, (char*)"MUMMY MAZE");
    TTF_SetFontSize(font[0], 3 * velicina / 4);
    if (index == 0 && LastIndex == -1) {
        PisiTekst(font[0], { 0,0,0,255 }, { (MaxWidth - x1) / 2, (int)(0.35 * MaxHeight), (int)(0.1 * MaxWidth), (int)(0.1 * MaxHeight) }, (char*)"Start");
        PisiTekst(font[0], { 0,0,0,255 }, { (MaxWidth - x2) / 2, (int)(0.46 * MaxHeight), (int)(0.1 * MaxWidth), (int)(0.1 * MaxHeight) }, (char*)"Opcije");
        PisiTekst(font[0], { 0,0,0,255 }, { (MaxWidth - x3) / 2, (int)(0.57 * MaxHeight), (int)(0.1 * MaxWidth), (int)(0.1 * MaxHeight) }, (char*)"Info");
        PisiTekst(font[0], { 0,0,0,255 }, { (MaxWidth - x4) / 2, (int)(0.68 * MaxHeight), (int)(0.1 * MaxWidth), (int)(0.1 * MaxHeight) }, (char*)"Izadji");
    }
    else if (index == 1) {
        PisiTekst(font[0], { 255,255,255,255 }, { (MaxWidth - x1) / 2, (int)(0.35 * MaxHeight), (int)(0.1 * MaxWidth), (int)(0.1 * MaxHeight) }, (char*)"Start");
        PisiTekst(font[0], { 0,0,0,255 }, { (MaxWidth - x2) / 2, (int)(0.46 * MaxHeight), (int)(0.1 * MaxWidth), (int)(0.1 * MaxHeight) }, (char*)"Opcije");
        PisiTekst(font[0], { 0,0,0,255 }, { (MaxWidth - x3) / 2, (int)(0.57 * MaxHeight), (int)(0.1 * MaxWidth), (int)(0.1 * MaxHeight) }, (char*)"Info");
        PisiTekst(font[0], { 0,0,0,255 }, { (MaxWidth - x4) / 2, (int)(0.68 * MaxHeight), (int)(0.1 * MaxWidth), (int)(0.1 * MaxHeight) }, (char*)"Izadji");
    }
    else if (index == 2) {
        PisiTekst(font[0], { 0,0,0,255 }, { (MaxWidth - x1) / 2, (int)(0.35 * MaxHeight), (int)(0.1 * MaxWidth), (int)(0.1 * MaxHeight) }, (char*)"Start");
        PisiTekst(font[0], { 255,255,255,255 }, { (MaxWidth - x2) / 2, (int)(0.46 * MaxHeight), (int)(0.1 * MaxWidth), (int)(0.1 * MaxHeight) }, (char*)"Opcije");
        PisiTekst(font[0], { 0,0,0,255 }, { (MaxWidth - x3) / 2, (int)(0.57 * MaxHeight), (int)(0.1 * MaxWidth), (int)(0.1 * MaxHeight) }, (char*)"Info");
        PisiTekst(font[0], { 0,0,0,255 }, { (MaxWidth - x4) / 2, (int)(0.68 * MaxHeight), (int)(0.1 * MaxWidth), (int)(0.1 * MaxHeight) }, (char*)"Izadji");
    }
    else if (index == 3) {
        PisiTekst(font[0], { 0,0,0,255 }, { (MaxWidth - x1) / 2, (int)(0.35 * MaxHeight), (int)(0.1 * MaxWidth), (int)(0.1 * MaxHeight) }, (char*)"Start");
        PisiTekst(font[0], { 0,0,0,255 }, { (MaxWidth - x2) / 2, (int)(0.46 * MaxHeight), (int)(0.1 * MaxWidth), (int)(0.1 * MaxHeight) }, (char*)"Opcije");
        PisiTekst(font[0], { 255,255,255,255 }, { (MaxWidth - x3) / 2, (int)(0.57 * MaxHeight), (int)(0.1 * MaxWidth), (int)(0.1 * MaxHeight) }, (char*)"Info");
        PisiTekst(font[0], { 0,0,0,255 }, { (MaxWidth - x4) / 2, (int)(0.68 * MaxHeight), (int)(0.1 * MaxWidth), (int)(0.1 * MaxHeight) }, (char*)"Izadji");
    }
    else if (index == 4) {
        PisiTekst(font[0], { 0,0,0,255 }, { (MaxWidth - x1) / 2, (int)(0.35 * MaxHeight), (int)(0.1 * MaxWidth), (int)(0.1 * MaxHeight) }, (char*)"Start");
        PisiTekst(font[0], { 0,0,0,255 }, { (MaxWidth - x2) / 2, (int)(0.46 * MaxHeight), (int)(0.1 * MaxWidth), (int)(0.1 * MaxHeight) }, (char*)"Opcije");
        PisiTekst(font[0], { 0,0,0,255 }, { (MaxWidth - x3) / 2, (int)(0.57 * MaxHeight), (int)(0.1 * MaxWidth), (int)(0.1 * MaxHeight) }, (char*)"Info");
        PisiTekst(font[0], { 255,255,255,255 }, { (MaxWidth - x4) / 2, (int)(0.68 * MaxHeight), (int)(0.1 * MaxWidth), (int)(0.1 * MaxHeight) }, (char*)"Izadji");
    }
}
void GlavniMeni(SDL_Surface** surface) {
    Mix_ResumeMusic();
    int x[5], y[5], radi = 1, prosli = 0, frame = 19, smer = 1, interval = 18, i = 21;
    char adresa[65], br[10];
    Uint32 ProsloVreme = SDL_GetTicks(), TrenutnoVreme;
    TTF_SetFontSize(font[0], 0.75 * velicina);
    TTF_SizeText(font[0], (const char*)"Start", &x[1], &y[1]);
    TTF_SizeText(font[0], (const char*)"Opcije", &x[2], &y[1]);
    TTF_SizeText(font[0], (const char*)"Info", &x[3], &y[1]);
    TTF_SizeText(font[0], (const char*)"Izadji", &x[4], &y[1]);
    while (radi) {
        TrenutnoVreme = SDL_GetTicks();
        if (TrenutnoVreme - ProsloVreme >= interval) {
            SDL_GetMouseState(&x[0], &y[0]);
            if (x[0] >= ((MaxWidth - x[1]) / 2) && x[0] <= ((MaxWidth + x[1]) / 2) && y[0] >= (0.35 * MaxHeight) && y[0] <= ((0.35 * MaxHeight) + y[1])) {
                CrtajGlavniMeni(surface, 1, prosli, frame);
                prosli = 1;
            }
            else if (x[0] >= ((MaxWidth - x[2]) / 2) && x[0] <= ((MaxWidth + x[2]) / 2) && y[0] >= (0.46 * MaxHeight) && y[0] <= ((0.46 * MaxHeight) + y[1])) {
                CrtajGlavniMeni(surface, 2, prosli, frame);
                prosli = 2;
            }
            else if (x[0] >= ((MaxWidth - x[3]) / 2) && x[0] <= ((MaxWidth + x[3]) / 2) && y[0] >= (0.57 * MaxHeight) && y[0] <= ((0.57 * MaxHeight) + y[1])) {
                CrtajGlavniMeni(surface, 3, prosli, frame);
                prosli = 3;
            }
            else if (x[0] >= ((MaxWidth - x[4]) / 2) && x[0] <= ((MaxWidth + x[4]) / 2) && y[0] >= (0.68 * MaxHeight) && y[0] <= ((0.68 * MaxHeight) + y[1])) {
                CrtajGlavniMeni(surface, 4, prosli, frame);
                prosli = 4;
            }
            else {
                CrtajGlavniMeni(surface, 0, -1, frame);
                prosli = 0;
            }
            if (smer == 1) frame++;
            else frame--;
            if (frame == 138) smer = 0;
            else if (frame == 19) smer = 1;
            ProsloVreme = TrenutnoVreme;
            SDL_RenderPresent(renderer);
        }
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                SDL_Quit();
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN) {
                SDL_GetMouseState(&x[0], &y[0]);
                if (x[0] >= ((MaxWidth - x[1]) / 2) && x[0] <= ((MaxWidth + x[1]) / 2) && y[0] >= (0.35 * MaxHeight) && y[0] <= ((0.35 * MaxHeight) + y[1])) {
                    PustiZvuk(zvukovi[1]);
                    Biraj(surface);
                }
                else if (x[0] >= ((MaxWidth - x[2]) / 2) && x[0] <= ((MaxWidth + x[2]) / 2) && y[0] >= (0.46 * MaxHeight) && y[0] <= ((0.46 * MaxHeight) + y[1])) {
                    PustiZvuk(zvukovi[1]);
                    PustiZvuk(zvukovi[0]);
                    PrelazNaOpcije(surface, 1, 0);
                }
                else if (x[0] >= ((MaxWidth - x[3]) / 2) && x[0] <= ((MaxWidth + x[3]) / 2) && y[0] >= (0.57 * MaxHeight) && y[0] <= ((0.57 * MaxHeight) + y[1])) {
                    PustiZvuk(zvukovi[1]);
                    PustiZvuk(zvukovi[0]);
                    PrelazNaInfo(surface);
                }
                else if (x[0] >= ((MaxWidth - x[4]) / 2) && x[0] <= ((MaxWidth + x[4]) / 2) && y[0] >= (0.68 * MaxHeight) && y[0] <= ((0.68 * MaxHeight) + y[1])) {
                    PustiZvuk(zvukovi[1]);
                    SDL_Quit();
                }
            }
        }
    }
}
int main(int argc, char* argv[]) {
    int x, y, interval = 1000 / 15, i[4] = { 15, 1, 19, 0 }, skin;
    Uint32 ProsloVreme = SDL_GetTicks(), TrenutnoVreme;
    char adresa[65];
    srand((unsigned int)time(NULL));
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();
    FILE* file = NULL;
    fopen_s(&file, (const char*)"Opcije.txt", (const char*)"r");
    fscanf_s(file, "%d %d\n%d\n", &MaxWidth, &MaxHeight, &n);
    fscanf_s(file, "%d %d %d\n", &Tezina, &Algoritam, &BrZombija);
    fscanf_s(file, "%d %d %d\n", &Izgledi[0], &Izgledi[1], &Izgledi[2]);
    fscanf_s(file, "%d %d %d\n", &TrenutniNivo, &mapa, &Sacuvaj);
    fscanf_s(file, "%d %d", &Zvukovi, &Pozadinska);
    fclose(file);
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    zvukovi[0] = Mix_LoadWAV("Zvukovi/button1.ogg");
    zvukovi[1] = Mix_LoadWAV("Zvukovi/button.ogg");
    zvukovi[2] = Mix_LoadWAV("Zvukovi/button3.ogg");
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    zvukovi[3] = Mix_LoadWAV("Zvukovi/win.ogg");
    zvukovi[4] = Mix_LoadWAV("Zvukovi/lossing.ogg");
    zvukovi[5] = Mix_LoadWAV("Zvukovi/chestopen.ogg");
    zvukovi[6] = Mix_LoadWAV("Zvukovi/footsteps.ogg");
    pozadinska = Mix_LoadMUS("Zvukovi/backround.ogg");
    Mix_VolumeMusic(Pozadinska);
    for (int i = 0; i < 7; i++) {
        Mix_VolumeChunk(zvukovi[i], Zvukovi);
    }
    Mix_PlayMusic(pozadinska, -1);
    Mix_PauseMusic();

    velicina = Velicina1();

    window = SDL_CreateWindow("Mummy maze", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, MaxWidth, MaxHeight, 0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Surface* ikonica = IMG_Load("Slike/Ikonica.png");
    SDL_SetWindowIcon(window, ikonica);
    SDL_FreeSurface(ikonica);
    SDL_Surface* surfaces[140] = {
        (SDL_Surface*)IMG_Load("Slike/Talon/talon6x6.png"),
        (SDL_Surface*)IMG_Load("Slike/Talon/talon7x7.png"),
        (SDL_Surface*)IMG_Load("Slike/Talon/talon8x8.png"),
        (SDL_Surface*)IMG_Load("Slike/Meni.png"),
        (SDL_Surface*)IMG_Load("Slike/Opcije.png"),
        (SDL_Surface*)IMG_Load("Slike/Nazad.png"),
        (SDL_Surface*)IMG_Load("SLike/NazadBelo.png")
    };
    sprintf_s(adresa, "Slike/Zid/%d/LeviTalon.png", Izgledi[2]);
    surfaces[7] = (SDL_Surface*)IMG_Load(adresa);
    sprintf_s(adresa, "Slike/Zid/%d/DesniTalon.png", Izgledi[2]);
    surfaces[8] = (SDL_Surface*)IMG_Load(adresa);
    sprintf_s(adresa, "Slike/Zid/%d/GoreTalon.png", Izgledi[2]);
    surfaces[9] = (SDL_Surface*)IMG_Load(adresa);
    sprintf_s(adresa, "Slike/Zid/%d/DoleTalon.png", Izgledi[2]);
    surfaces[10] = (SDL_Surface*)IMG_Load(adresa);
    surfaces[11] = (SDL_Surface*)IMG_Load("Slike/Zid/IzlazGore.png");
    surfaces[12] = (SDL_Surface*)IMG_Load("Slike/Zid/IzlazDole.png");
    surfaces[13] = (SDL_Surface*)IMG_Load("Slike/Zid/IzlazLevo.png");
    surfaces[14] = (SDL_Surface*)IMG_Load("Slike/Zid/IzlazDesno.png");
    sprintf_s(adresa, "Slike/Zid/%d/LeviTalon.png", Izgledi[2]);
    sprintf_s(adresa, "Slike/Zid/%d/VertikalniZid.png", Izgledi[2]);
    surfaces[15] = (SDL_Surface*)IMG_Load(adresa);
    sprintf_s(adresa, "Slike/Zid/%d/HorizontalniZid.png", Izgledi[2]);
    surfaces[16] = (SDL_Surface*)IMG_Load(adresa);
    surfaces[17] = (SDL_Surface*)IMG_Load("Slike/Info.png");
    SDL_Surface* LoadingScreen[30];
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);
    font[0] = TTF_OpenFont("This Night.ttf", 50);
    font[1] = TTF_OpenFont("FEASFBI_.ttf", 50);
    TTF_SetFontSize(font[0], 0.9 * velicina);
    TTF_SizeText(font[0], (const char*)"Powered By", &x, &y);
    while (1) {
        TrenutnoVreme = SDL_GetTicks();
        if (TrenutnoVreme - ProsloVreme >= interval) {
            if (i[0] != 263) {
                PisiTekst(font[0], { 0, 0, 0, (Uint8)i[0] }, { (int)((MaxWidth - x) / 2), (int)(0.1 * MaxHeight), x, y }, (char*)"Powered By");
                SDL_RenderPresent(renderer);
                i[0] += 8;
                sprintf_s(adresa, "Slike/Tesla/%d.png", i[1]);
                LoadingScreen[i[1] - 1] = (SDL_Surface*)IMG_Load(adresa);
                i[1]++;
                sprintf_s(adresa, "Slike/GlavniMeni/GlavniMeni%d.png", i[2] - 17);;
                surfaces[i[2]] = (SDL_Surface*)IMG_Load(adresa);
                i[2]++;
            }
            else {
                SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, LoadingScreen[i[3]]);
                SDL_FreeSurface(LoadingScreen[i[3]]);
                NapraviZid(0, 0, MaxWidth, MaxHeight, texture);
                SDL_DestroyTexture(texture);
                PisiTekst(font[0], { 0, 0, 0, 255 }, { (int)((MaxWidth - x) / 2), (int)(0.1 * MaxHeight), x, y }, (char*)"Powered By");
                SDL_RenderPresent(renderer);
                sprintf_s(adresa, "Slike/GlavniMeni/GlavniMeni%d.png", i[2] - 17);
                surfaces[i[2]] = (SDL_Surface*)IMG_Load(adresa);
                i[2]++;
                i[3]++;
                if (i[3] == 30)
                    break;
            }
            ProsloVreme = TrenutnoVreme;
        }
    }
    for (int j = i[2]; j < 139; j++) {
        sprintf_s(adresa, "Slike/GlavniMeni/GlavniMeni%d.png", j - 18);
        surfaces[j] = (SDL_Surface*)IMG_Load(adresa);
    }
    GlavniMeni(surfaces);
    return 0;
}