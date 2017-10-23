/* 
 * File:    johnC.cpp
 * Author:  John B. Crossley
 * Program: missileCommand (team Project)
 * Purpose: Functions for firing Dmissiles (defense missiles) up to 
 *          destroy/Stop Emissiles (Enemy Missiles)
 *          
  *          (April 2016)
 *          So far I just change the title bar text with mouse left and 
 *          right buttons to prove I am accessing my functions from 
 *          main file "missileCommand.cpp" and visa-versa
 * 
 *          (05/07/2016)
 *          Added "fireDefenseMissile" (used to be "movement" 
 *          inside "missileCommand.cpp")
 *          
 *          removed the extra empty lines+ from within the functions
 *          (5/5/16)
 * 
 *          5/13-14
 *          Added missile firing to mouse coords from 0,0 ONLY!
 *          Still need to make it fire from other locations
 * 
 *          5/14-15/16
 *          added code to make defense missiles lime green and bigger
 *          also made them stop at mouse click location
 * 
 *          5/25/16
 *          Changed explosion coords to mouse location instead of missile 
 *          location to be more accurate. (SEE TODO #1)
 * 
 *          5/26-27/16
 *          - [x] Make missiles stop at mouse coords 
 *          - [x] Nuke 'em  
 *          - [x] Missiles start higher 
 * 
 *          06/01/16 started work on fixing bug 
 *          Def missile remaining shows up as negative numbers
 *          somehow game->defMissilesRemaining is getting past 0 
 *          (-2, etc) fixed by brute force... not sure if its
 *          exactly correct, but the player might not notice
 * 
 */
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>

extern "C" {
#include "fonts.h"
}

#ifndef MISSILECOMMAND_H
#define MISSILECOMMAND_H
#include "missileCommand.h"
#endif



#ifndef _DANIELT_H_
#define _DANIELT_H_
#include "danielT.h"
#endif

using namespace std;
extern void dMissileRemove(Game *game, int dMissilenumber);
extern void createEExplosion(Game *game, float x, float y);
int tempOneTime = 0;
GLuint dmissileTexture;
GLuint c_dmissileTexture;

void renderDefenseMissile(Game *game)
{
            
    // Find end of level to save "defMissilesRemainingAfterLevel" for JG
    if (game->nmissiles == 0 && 
    game->defMissilesRemaining > 0 &&
        game->mCount == 0) {
        game->defMissilesRemainingAfterLevel = 
        game->defMissilesRemaining;
        // cout << "nmissiles: " <<  game->nmissiles << 
          //  "  defMissilesRemainingAfterLevel: " <<  
          // game->defMissilesRemainingAfterLevel << 
          //  "  mCount: " <<  game->mCount <<
          //  endl;
         }
    if (5.0 + game->level*5.0 == game->mCount ) {

        game->defMissilesRemaining = game->level * 10 *1.5;
        // cout << "defMissiles left in IF: " << 
         // game->defMissilesRemaining << endl;
    }
    
    // testing lines (or unit testing... )
    if (game->level * 5 == game->mCount) {
        // cout << "mCount from JBC: " << mCount << endl;
    }
        // cout << "defMissiles left: " << 
        // game->defMissilesRemaining << endl;

     Rect r;
    //glClear(GL_COLOR_BUFFER_BIT);
    r.bot = WINDOW_HEIGHT-100;
    r.left = 50.0;
    r.center = 0;
    
    // JBC Note 06-01-2016 somehow game->defMissilesRemaining is goes past 0 
    // (-2, etc) fixed by brute force... 
    // not sure if its exactly correct, but the player wont know...
    if ( game->defMissilesRemaining < 0 ) {
        game->defMissilesRemaining = 0;
    }
    
    ggprint8b(&r, 16, 0x00005599, "Defense Missiles: %i", 
            game->defMissilesRemaining);
    ggprint8b(&r, 16, 0x00005599, "");
    ggprint8b(&r, 16, 0x00005599, "Keys Menu:");
    ggprint8b(&r, 16, 0x00005599, "-------------");
    ggprint8b(&r, 16, 0x00005599, "'N' = Nuke'em");
    ggprint8b(&r, 16, 0x00005599, "'M' = Menu");
    ggprint8b(&r, 16, 0x00005599, "'R' = Radar");
    ggprint8b(&r, 16, 0x00005599, "'B' = UFO");
    ggprint8b(&r, 16, 0x00005599, "'L' = Last City Standing (Only 1 City)");   
    ggprint8b(&r, 16, 0x00005599, "'C' = Toggle GFX");
    ggprint8b(&r, 16, 0x00005599, "'Z' = Quit");
    ggprint8b(&r, 16, 0x00005599, "'[esc]' = Quit");
    


    DefenseMissile *dMissilePtr;
    float w, h;
    glColor3f(1.0, 1.0, 1.0);
	glPushMatrix();
    for (int i=0; i<game->numberDefenseMissiles; i++) {
        Vec *c = &game->dMissile[i].shape.center;
        w = 10;
        h = 10;
        if (game->gfxMode) {
            glBindTexture(GL_TEXTURE_2D, dmissileTexture);
        } else {
            glBindTexture(GL_TEXTURE_2D, c_dmissileTexture);
        
        }
        //For transparency
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 1.0f); glVertex2i(c->x-w, c->y-h);
        glTexCoord2f(0.0f, 0.0f); glVertex2i(c->x-w, c->y+h);
        glTexCoord2f(1.0f, 0.0f); glVertex2i(c->x+w, c->y+h);
        glTexCoord2f(1.0f, 1.0f); glVertex2i(c->x+w, c->y-h);
        glEnd();
        glDisable(GL_BLEND);
        glPopMatrix();
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    

    if (game->numberDefenseMissiles <= 0)
        return;
    
    for (int i=0; i<game->numberDefenseMissiles; i++) {
        dMissilePtr = &game->dMissile[i];
        
            
        if (dMissilePtr->destinationY >= dMissilePtr->shape.center.y ) {

//            // test location of Missile vs mouse pick coords
//            cout << "X,Y Missile coords just before the next move 
                // (velocity):" << 
//                    dMissilePtr->shape.center.x << 
//                "," << dMissilePtr->shape.center.y << endl; 

            // shape.center refers to the Shape center position
            // IE the position of the center of that particular Shape
            dMissilePtr->shape.center.x += dMissilePtr->velocity.x;
            dMissilePtr->shape.center.y += dMissilePtr->velocity.y;
            
            // Fix to make missiles stop at mouse coords
            // because without this the missile goes 100+ pixels too far
            // I think it has to do with game refresh rate or "render rate"
            if (dMissilePtr->shape.center.y >= dMissilePtr->destinationY) {
                dMissilePtr->shape.center.x = dMissilePtr->destinationX + .01;
                dMissilePtr->shape.center.y = dMissilePtr->destinationY + .01;
                // Added "+ 1" otherwise the missile never explodes 
                        
            }
            
        } else {
            
//            // test location of explosion vs mouse pick coords
//            cout << "X,Y Missile coords:" << 
//                    dMissilePtr->shape.center.x << 
//                "," << dMissilePtr->shape.center.y << endl; 
            
            dMissileRemove(game, i);
            
        }

    }
    
}


/// JBC Note somehow game->defMissilesRemaining is getting past 0 (-2, etc)
// need to find a fix...
void dMissileRemove(Game *game, int dMissilenumber)
{
    DefenseMissile *dMissilePtr = &game->dMissile[dMissilenumber];
    createEExplosion(game,  dMissilePtr->destinationX,
                            dMissilePtr->destinationY);

    //delete defense missile
    game->dMissile[dMissilenumber] = 
        game->dMissile[game->numberDefenseMissiles - 1];
    game->numberDefenseMissiles--;
    game->defMissilesRemaining--;
    
}

void nukeEmAll (Game *game)
{
    EMissile *enemyMissile;
    
    for (int i=0; i<game->nmissiles; i++) {
        enemyMissile = &game->emarr[i];

	//Use Enemy missile position to create explosion just below it
        if (enemyMissile->vel.x>0) {
            // dowmn and to the right enemy missile
            createEExplosion(game,  
                enemyMissile->pos.x + enemyMissile->vel.x, 
                enemyMissile->pos.y + enemyMissile->vel.y);
            
        } else { 
            // dowmn and to the left enemy missile
            createEExplosion(game,  
                enemyMissile->pos.x - fabs(enemyMissile->vel.x), 
                enemyMissile->pos.y + enemyMissile->vel.y);
            
        }
    }
    
    
}


// 5/14 changes to make missile firing work
// seems OK now... :-)
void makeDefenseMissile(Game *game, int x, int y)
{
    if (game->nmissiles > 0 &&  game->defMissilesRemaining > 0) {
        //a->playAudio(20);
    } 

    
    if (game->numberDefenseMissiles >= MAX_D_MISSILES || 
            game->defMissilesRemaining <1) {
        return;
    }
        DefenseMissile *dMissilePtr = 
                &game->dMissile[game->numberDefenseMissiles];
        dMissilePtr->shape.width = 10;
        dMissilePtr->shape.height = 10;
        dMissilePtr->shape.radius = 10;

        dMissilePtr->color[0] = 0;
        dMissilePtr->color[1] = 255;
        dMissilePtr->color[2] = 0;

        // set target of missile from mouse coords
        dMissilePtr->destinationX = x;
        // do not allow shooting below a certain point 
        dMissilePtr->minimumY = 200;
        if ( y <  dMissilePtr->minimumY ) {
            dMissilePtr->destinationY = dMissilePtr->minimumY;
        } else {
            dMissilePtr->destinationY = y;
            
        }
        
        
        
        
        // test location of explosion vs mouse pick coords
//        cout << "X,Y Mouse coords:" << dMissilePtr->destinationX << 
//                "," << dMissilePtr->destinationY << endl;
        
        // set speed of missile
        // 0.5 is a good start, 0.25 seemed a bit to slow & 5.0 
        // seemed insanely fast (BEFORE setting my NVidia card to 
        // "Sync to VBlank")
        
        // Moved to main game struct
        // float defMissileSpeed = 40;
        // game->defMissileSpeed = 40;
        
        
        
        // set start position of missile
        // Works now @ 2pm 05/14/16
        float xStart = 500.0;
        float yStart = 50.0;
        dMissilePtr->shape.center.x = xStart;
        dMissilePtr->shape.center.y = yStart;
        
        // do the math to find Velocity values to show missile at next
        // location upon screen refresh
        float dx = dMissilePtr->destinationX - xStart; // delta "x" (dx)
        float dy = dMissilePtr->destinationY - yStart; // delta "y" (dy)
        float dist = sqrt(dx*dx + dy*dy);
        dx /= dist;
        dy /= dist;
        float missileVelocityX = 0;
        float missileVelocityY = 0;

        missileVelocityX = game->defMissileSpeed * dx;
        missileVelocityY = missileVelocityY + game->defMissileSpeed * dy;

        // Velocity is a vector quantity that refers to 
        // the rate at which an object changes its position.
        // The diff between X & Y determines the angle
        dMissilePtr->velocity.y = missileVelocityY;
        dMissilePtr->velocity.x = missileVelocityX;

        game->numberDefenseMissiles++;
}
