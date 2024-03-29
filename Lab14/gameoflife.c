#include "gameoflife.h"
#include <stdlib.h>
#include <string.h>

//#include <stdio.h>
////Temp
//void printField(GameField* field) {
//    for (int i = 0; i < field->height; i++) {
//        for (int j = 0; j < field->width; j++) {
//            if (field->data[i][j])
//                printf("*");
//            else
//                printf(" ");
//        }
//        printf("\n");
//    }
//}
//

char** createField(size_t x, size_t y) {
    char** field = malloc(y * sizeof(char*));
    for (int i = 0; i < y; i++) {
        field[i] = malloc(x * sizeof(char));
    }
    return field;
}

GAME* newGame(BMP* image, unsigned maxGenCount, unsigned outputFreq, char* outputDir) {
    GAME* game = malloc(sizeof(GAME));
    game->field = getGameField(image);
    game->image = image;
    game->prevField.data = createField(game->field.width, game->field.height);
    game->prevField.width = game->field.width;
    game->prevField.height = game->field.height;
    game->maxGenCount = maxGenCount;
    game->currentGen = 0;
    game->outputFreq = outputFreq;
    game->outputDir = outputDir;
    return game;
}

int getCellState(GameField* field, int x, int y) {
    return field->data[(field->height + y) % field->height][(field->width + x) % field->width] ? 1 : 0;
}

void setCellState(GameField* field, int x, int y, char state) {
    field->data[(field->height + y) % field->height][(field->width + x) % field->width] = state ? 1 : 0;
}

int getNeighboursCount(GameField* field, int x, int y) {
    return  getCellState(field, x - 1, y) +
            getCellState(field, x + 1, y) +
            getCellState(field, x, y - 1) +
            getCellState(field, x, y + 1) +
            getCellState(field, x - 1, y - 1) +
            getCellState(field, x - 1, y + 1) +
            getCellState(field, x + 1, y - 1) +
            getCellState(field, x + 1, y + 1);
}

void nextGen(GAME* game) {
    free(game->prevField.data);
    game->prevField.data = game->field.data;
    game->prevField.height = game->field.height;
    game->prevField.width = game->field.width;
    game->field.data = createField(game->field.width, game->field.height);
    for (int i = 0; i < game->prevField.height; i++) {
        for (int j = 0; j < game->prevField.width; j++) {
            if ((!getCellState(&game->prevField, i, j) && getNeighboursCount(&game->prevField, i, j) == 3) ||
            (getCellState(&game->prevField, i, j) && (getNeighboursCount(&game->prevField, i, j) == 2 ||
            getNeighboursCount(&game->prevField, i, j) == 3)))
                setCellState(&game->field, i, j, 1);
            else
                setCellState(&game->field, i, j, 0);
        }
    }
}

char checkChanged(GAME* game) {
    for (int i = 0; i < game->field.height; i++) {
        for (int j = 0; j < game->field.width; j++) {
            if (game->prevField.data[i][j] != game->field.data[i][j])
                return 1;
        }
    }
    return 0;
}

void start(GAME* game) {
    while (game->currentGen++ < game->maxGenCount) {
        nextGen(game);
        if (game->currentGen % game->outputFreq == 0) {
            editImage(game->image, game->field);
            char filename[200];
            strcpy(filename, game->outputDir);
            char num_str[50];
            itoa(game->currentGen, num_str, 10);
            strcat(filename, "/");
            strcat(filename, num_str);
            strcat(filename, ".bmp\0");
            writeBMP(game->image, filename);
        }
        if (game->prevField.data != NULL && !checkChanged(game))
            break;
    }
}

GameField getGameField(BMP* image) {
    GameField field;
    char** pixelData = getPixelData(image);
    field.height = image->bmpInfo->biHeight;
    field.width = image->bmpInfo->biWidth;
    field.data = malloc(field.height * sizeof(char*));
    for (int i = 0; i < field.height; i++)
        field.data[i] = malloc(field.width * sizeof(char));

    for (int i = 0; i < field.height; i++) {
        for (int j = 0; j < field.width; j++) {
            if (pixelData[i][j])
                field.data[field.height - i - 1][j] = 0;
            else
                field.data[field.height - i - 1][j] = 1;
        }
    }
    return field;
}

void editImage(BMP* image, GameField field) {
    char** pixelData = malloc(image->bmpInfo->biHeight * sizeof(char*));
    for (int i = 0; i < image->bmpInfo->biHeight; i++)
        pixelData[i] = malloc(image->bmpInfo->biWidth * sizeof(char));

    for (int i = 0; i < field.height; i++) {
        for (int j = 0; j < field.width; j++) {
            if (field.data[i][j])
                pixelData[field.height - i - 1][j] = 0;
            else
                pixelData[field.height - i - 1][j] = -1;
        }
    }
    setPixelData(image, pixelData);
}
