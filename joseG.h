//Author: Jose Garcia
//Program: joseG.cpp
//Purpose: This program will render the background/foreground of our project
//

#ifndef MISSILECOMMAND_H
#define MISSILECOMMAND_H
#include "missileCommand.h"
#endif

#ifndef _JOSEG_H_
#define _JOSEG_H_
using namespace std;

extern GLuint cityTexture;
extern GLuint streetTexture;
extern GLuint civilianTexture;
extern GLuint dcityTexture;
extern GLuint ufoTexture;
extern GLuint c_cityTexture;
extern GLuint c_floorTexture;

extern void initStruc(Game *game);
extern void renderStruc(Game *game);
extern void destroyCity(Game *game, int citynum);
extern void renderBackground(GLuint starsTexture);
extern void renderScores(Game *game);
extern void makeCivilian(Game *game, int x, int y);
extern void civilianPhysics(Game *game);
extern void renderUFO(Game *game);
extern void ufoPhysics(Game *game);
extern void initUFO(Game *game);
extern unsigned char *buildAlphaData(Ppmimage *img);
extern GLuint makeTexture(GLuint texture, Ppmimage * image);
extern GLuint makeTransparentTexture(GLuint texture, Ppmimage * image);

#endif
