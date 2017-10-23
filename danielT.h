
#ifndef MISSILECOMMAND_H
#define MISSILECOMMAND_H
#include "missileCommand.h"
#endif

#ifndef JOSER_H
#define JOSER_H
#include "joseR.h"
#endif

#ifndef _DANIELT_H_
#define _DANIELT_H_
using namespace std;

extern GLuint emissileTexture;
extern GLuint c_emissileTexture;

extern void eExplosionPhysics(Game *game);
extern void eMissilePhysics(Game *game);
extern void nameInBox(float xpoint, float ypoint);
extern void createEMissiles(Game *game, float x, float y);
extern void renderEMissiles(Game *game);
extern void eMissileExplode(Game *game, int misnum);
extern void renderEExplosions(Game *game);
extern void createEExplosion(Game *game, float x, float y);
extern void renderRadar(Game *game);
extern void radarPhysics(Game *game);
extern void initRadar(Game *game);
extern void createSMissile(Game *game);
extern void sMissilePhysics(Game *game);
extern void renderSMissile(Game *game);
extern void initHighScores(Game *game);
extern void addHighScore(Game *game);

#endif 
