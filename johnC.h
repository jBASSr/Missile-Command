
/* 
 * File:   johnC.h for -> johnBC.cpp
 * Author:  John B. Crossley
 * Program: missileCommand (team Project)
 * Purpose: Functions for firing Dmissiles (defense missiles) up to 
 *          destroy/Stop Emissiles (Enemy Missiles)
 *      
 *         
 */

#ifndef MISSILECOMMAND_H
#define MISSILECOMMAND_H
#endif


extern GLuint dmissileTexture;
extern GLuint c_dmissileTexture;
void fireDefenseMissile();
// void changeTitle();

extern void createDefenseMissileExplosion(Game *game, float x, float y);
void renderDefExplosions(Game *game);


