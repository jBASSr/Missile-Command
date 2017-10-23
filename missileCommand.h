/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   missileCommand.h
 * Author: student
 *
 * Created on April 28, 2016, 3:52 PM
 */

// #ifndef MISSILECOMMAND_H
// #define MISSILECOMMAND_H

//INCLUDES
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <cstring>
#include <string>
#include <cmath>
#include <time.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>
#include "ppm.h"

//MACROS
//Number of buttons to show on menu
#define BUTTONS 4
#define BUTTONS_S 3
#define T_BUTTONS BUTTONS + BUTTONS_S
//X Position of all buttons
//Divides WINDOW_WIDTH by the number below
#define BUTTON_X 4.25
#define CITYNUM 5
#define TOTAL_SOUNDS 8
//#define WINDOW_WIDTH  800
//#define WINDOW_HEIGHT 600
#define WINDOW_WIDTH  1024
#define WINDOW_HEIGHT 768
#define MAX_D_MISSILES 100
#define MAX_PARTICLES 20
#define GRAVITY 0.1

//X Windows variables
extern Display *dpy;
extern Window win;
extern GLXContext glc;

struct Vec {
    float x, y, z;
};

struct Shape {
    int alive;
    float width, height;
    float radius;
    Vec center;
};

struct UFO {
	Vec pos;
	Vec vel;
};

// JBC 05/08/16 JBC switched from DefenseMissile to dMissile/DefenseMissile
//struct DefenseMissile {
//  Shape s;
//  Shape s2;
//  Shape s3;
//  Vec velocity;
//};
struct DefenseMissile {
    Shape shape;
//  Shape s2;
//  Shape s3;
    int color[3];
    Vec velocity;
    int destinationX;
    int destinationY;
    int minimumY;
        
};

struct Structures {
    Shape floor;
    Shape city[CITYNUM];
};

struct Particle {
	Shape part;
	Vec velocity;
	Vec pos;
};

struct EMTrail {
    Vec start;
    Vec end;
    float width;
    float color[3];
    EMTrail() {}
};

struct SMissile {
    Vec pos;
    Vec vel;
    float color[3];
    SMissile() { }
};

struct EMissile {
    Vec pos;
    Vec vel;
    double active;
    float angle;
    float color[3];        
    EMTrail trail;
    EMissile() { }
};

struct EExplosion {
    Vec pos;
    float radius; 
    float radiusInc;
    float color[3];
    EExplosion() {}
};

struct Radar {
    Vec pos;
    float radius;
    float radius2;
    float radiusInc;
    Vec tri[3];
    float color[4];
    Radar() {}
};

// defense missile explosion
struct DExplosion {
    Vec pos;
    float radius; 
    float radiusInc;
    float color[3];
    DExplosion() {}
};

// Game sound class for openAL
class Audio 
{
    public:
        //Variables
        ALCdevice *device;
        ALCcontext *context;
        ALuint alSource;
        ALuint alBuffer;
        ALint source_state;
        int source[100];
        int buffer[TOTAL_SOUNDS];
        float gVolume;
        //Constructor & Deconstructor
        Audio();
        ~Audio();
        //Class Prototypes
        void loadAudio();
        void playAudio(int num);
};

struct levelInfo {    
    time_t start, end;
    clock_t gtime;
    float timer, alpha;
    int rCount, cCount, mDone, alertPlayed, prevMCount;
    double diff;
    bool cReset, explMax;
    //Time to stay in function by seconds
    double delay;
    //How fast missiles and cities are tallied
    double m_delay;
    double c_delay;
    int aCities;
    levelInfo() {
        delay = 5.0;
        m_delay = 0.7;
        c_delay = 2.0;
        diff = 0;
        cReset = true, explMax = false;
        gtime = 0.0;
        rCount = 0;
        cCount = 0;
        prevMCount = 0;
        aCities = 0;
        start = 0, end = 0;
        timer = 0.0;
        alpha = 1.0;
        mDone = 1;
        alertPlayed = 0;
    }
};

struct Game {
    int level;
    int prevLevel;
    
    //global variable for score 5-25-16 -JG
    int score;

    Shape box;    
    float defMissileSpeed;
    
    //DT
    EMissile *emarr;                                  
    int nmissiles;
    int mCount;
    EExplosion *eearr;
    int neexplosions;
    SMissile *smarr;
    int nsmissiles;
    Radar radar;
    int radarOn;
    int highScores[5];

    // DefenseMissile section
    int numberDefenseMissiles;
    // array of Defense missile explosions
    DExplosion * defExplArray;
    int numDefExplosions;
    int defMissilesRemaining;
    int defMissilesRemainingAfterLevel;
    
    // JBC 05/08/16 JBC switched from DefenseMissile to dMissile (Defense Missile)
    DefenseMissile dMissile[MAX_D_MISSILES];

    //JR:
    int buttonSpacer[BUTTONS];
    int mouseOnButton[T_BUTTONS];
    int menuExit, gState, inGame, vVolume, gStart, howto, lcm;
    bool gfxMode, expColor;
    Shape mButton[BUTTONS];
    Shape sButton[BUTTONS_S];
    Shape menuBG;
    Shape BonusA[200];
    Shape BonusB[5];
    Shape endExplosion;
    Audio sounds;
    levelInfo lvl;

	//JG
	Particle particle[MAX_PARTICLES];
	int nparticles;
	UFO ufo;
	int ufoOn;
	
    Structures structures;

    //Constructor 
    Game() {
        //DT
        level = 0;
	prevLevel = 0;
        emarr = new EMissile[15];
        smarr = new SMissile[10];
        eearr = new EExplosion[1000];
        numberDefenseMissiles = 0;
        nmissiles = 0;
        mCount = 0;
        neexplosions = 0;
        nsmissiles = 0;
        radarOn = 0;
		ufoOn = 0;
        menuExit = 0;
        gState = 1;
        howto = 0;
        lcm = 0;
        gStart = 1;
        inGame = 0;
        vVolume = 100;
        gfxMode = 1;
        expColor = 0;
        for (int i=0;i<BUTTONS;i++) {
            mouseOnButton[i] = 0;
        }
    }
    //Deconstructor
    ~Game() {
       delete [] emarr;
       delete [] eearr;
       delete [] smarr;
    }    
};


#ifdef __cplusplus
extern "C" {
#endif




#ifdef __cplusplus
}
#endif

// #endif /* MISSILECOMMAND_H */

