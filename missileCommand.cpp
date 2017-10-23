//Author: Jose Garcia, John B. Crossley, Daniel Turack, Jose Reyes
//Program: missileCommand.cpp
//Purpose: This cpp file is the main of our project
//Modified: 5/2/16
//Added comment to prove I can commit several files and merge at once

#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <cstring>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>

#ifndef MISSILECOMMAND_H
#define MISSILECOMMAND_H
#include "missileCommand.h"
#endif


#ifndef JOHNC_H
#define JOHNC_H
#include "johnC.h"
#endif


#include "danielT.h"
#include "joseR.h"
#include "joseG.h"
extern "C" {
#include "fonts.h"
}

//X Windows variables
Display *dpy;
Window win;
GLXContext glc;

//Function prototypes
void initXWindows(void);
void init_opengl(void);
void cleanupXWindows(void);
void check_mouse(XEvent *e, Game *game);
int check_keys(XEvent *e, Game *game);
void movement(Game *game);

// JBC added 5/13
void renderDefenseMissile(Game *game);
void makeDefenseMissile(Game *game, int x, int y);
void nukeEmAll (Game *game);

// JR Prototypes
void render_menu(Game *game);
void render_settings(Game *game);
void render_newgame(Game *game);
void render_gameover(Game *game);

void render(Game *game);

Ppmimage *cityImage=NULL;
Ppmimage *starsImage=NULL;
Ppmimage *streetImage=NULL;
Ppmimage *civilianImage=NULL;
Ppmimage *gameoverImage=NULL;
Ppmimage *mainmenuImage=NULL;
Ppmimage *emissileImage=NULL;
Ppmimage *dmissileImage=NULL;
Ppmimage *dcityImage=NULL;
Ppmimage *c_cityImage=NULL;
Ppmimage *c_emissileImage=NULL;
Ppmimage *c_dmissileImage=NULL;
Ppmimage *c_floorImage=NULL;
Ppmimage *ufoImage=NULL;
Ppmimage *howtoImage=NULL;
GLuint starsTexture;

int main(void)
{
    int done=0;
    srand(time(NULL));
    initXWindows();
    init_opengl();
    //declare game object
    Game game;

    game.numberDefenseMissiles=0;
    
    //JG
    game.nparticles=MAX_PARTICLES;
    game.numberDefenseMissiles=0;

    // JBC 5/19/16
    // added globally accesible defMissileSpeed so that we can 
    // change it dynamically
    game.defMissileSpeed = 10;

    initStruc(&game);
    //Structures sh;

    //Changed call for function prototype 5-17-16 -DT
    createEMissiles(&game, 0, 0);
    initRadar(&game);
    initUFO(&game);
    initHighScores(&game);
    //JR - Menu Object Shapes and Locations
    drawMenu(&game);
    drawSettings(&game);
    game.sounds.loadAudio();
    //start animation
    while (!done) {
        int state = gameState(&game);
        while (XPending(dpy)) {
            XEvent e;
            XNextEvent(dpy, &e);
            check_mouse(&e, &game);
            done = check_keys(&e, &game);
        }
        if (state == 1) {
            render_menu(&game);
        } else if (state == 2) {        	
    		drawSettings(&game);
            render_settings(&game);
        } else if (state == 3) {
            render_newgame(&game);
        } else if (state == 0 || state == 5) {
            if (lvlState(&game) < 0) {
                movement(&game);
                render(&game);
            } 
            if (lvlState(&game) == 1) {
                levelEnd(&game);
            }
        } else {
            render_gameover(&game);
        }
        glXSwapBuffers(dpy, win);
    }
    cleanupXWindows();
    return 0;
}

void set_title(void)
{
    //Set the window title bar.
    XMapWindow(dpy, win);
    XStoreName(dpy, win, "Missile Command / GTNW");
}

void cleanupXWindows(void)
{
    //do not change
    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);
}

void initXWindows(void)
{
    //do not change
    GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
    int w=WINDOW_WIDTH, h=WINDOW_HEIGHT;
    dpy = XOpenDisplay(NULL);
    if (dpy == NULL) {
        std::cout << "\n\tcannot connect to X server\n" << std::endl;
        exit(EXIT_FAILURE);
    }
    Window root = DefaultRootWindow(dpy);
    XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
    if (vi == NULL) {
        std::cout << "\n\tno appropriate visual found\n" << std::endl;
        exit(EXIT_FAILURE);
    } 
    Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
    XSetWindowAttributes swa;
    swa.colormap = cmap;
    // JBC just moved code over <- to stay with the "80 lines" requirement
    swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
        ButtonPress | ButtonReleaseMask |
        PointerMotionMask |
        StructureNotifyMask | SubstructureNotifyMask;
    win = XCreateWindow(dpy, root, 0, 0, w, h, 0, vi->depth,
            InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
    set_title();
    glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
    glXMakeCurrent(dpy, win, glc);
}

void init_opengl(void)
{
    //OpenGL initialization
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    //Initialize matrices
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    //Set 2D mode (no perspective)
    glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_FOG);
    glDisable(GL_CULL_FACE);
    //Set the screen background color
    glClearColor(0.0, 0.0, 0.0, 1.0);
    //Initialize Fonts
    glEnable(GL_TEXTURE_2D);
    initialize_fonts();
    //system convert
    system("convert ./images/city.png ./images/city.ppm");
    system("convert ./images/street.jpg ./images/street.ppm");
    system("convert ./images/stars.png ./images/stars.ppm");
    system("convert ./images/civilian.jpg ./images/civilian.ppm");
    system("convert ./images/gameover.png ./images/gameover.ppm");
    system("convert ./images/mainmenu.png ./images/mainmenu.ppm");
    system("convert ./images/dcity.png ./images/dcity.ppm");
    system("convert ./images/emissile.png ./images/emissile.ppm");
    system("convert ./images/dmissile.png ./images/dmissile.ppm");
    system("convert ./images/ufo.png ./images/ufo.ppm");
    system("convert ./images/c_city.png ./images/c_city.ppm");
    system("convert ./images/c_emissile.png ./images/c_emissile.ppm");
    system("convert ./images/c_dmissile.png ./images/c_dmissile.ppm");
    system("convert ./images/c_floor.png ./images/c_floor.ppm");
    system("convert ./images/howtoplay.png ./images/howtoplay.ppm");
    //system("convert ./images/c_bomber.png ./images/c_bomber.ppm");
    //system("convert ./images/c_satellite.png ./images/c_satellite.ppm");
    //system("convert ./images/c_silo.png ./images/c_silo.ppm");
    //system("convert ./images/c_sbomb.png ./images/c_sbomb.ppm");

    //load images into a ppm structure
    cityImage = ppm6GetImage("./images/city.ppm");
    starsImage = ppm6GetImage("./images/stars.ppm");
    streetImage = ppm6GetImage("./images/street.ppm");
    civilianImage = ppm6GetImage("./images/civilian.ppm");
    gameoverImage =ppm6GetImage("./images/gameover.ppm");
    mainmenuImage = ppm6GetImage("./images/mainmenu.ppm");
    dcityImage = ppm6GetImage("./images/dcity.ppm");
    emissileImage = ppm6GetImage("./images/emissile.ppm");
    dmissileImage = ppm6GetImage("./images/dmissile.ppm");
    ufoImage = ppm6GetImage("./images/ufo.ppm");
    howtoImage = ppm6GetImage("./images/howtoplay.ppm");
    //classic images
    c_cityImage = ppm6GetImage("./images/c_city.ppm");
    c_emissileImage = ppm6GetImage("./images/c_emissile.ppm");
    c_dmissileImage = ppm6GetImage("./images/c_dmissile.ppm");
    c_floorImage = ppm6GetImage("./images/c_floor.ppm");

    //create opengl texture elements
    //stars
    starsTexture = makeTexture(starsTexture, starsImage);
    //street
    streetTexture = makeTexture(streetTexture, streetImage);
    //city
    cityTexture = makeTransparentTexture(cityTexture, cityImage);
    //civilian
    civilianTexture = makeTransparentTexture(civilianTexture, civilianImage);
    //dcity
    dcityTexture = makeTransparentTexture(cityTexture, dcityImage);
    //emissile
    emissileTexture = makeTransparentTexture(cityTexture, emissileImage);
    //dmissile
    dmissileTexture = makeTransparentTexture(cityTexture, dmissileImage);
    //ufo
    ufoTexture = makeTransparentTexture(cityTexture, ufoImage);
    //Others
    gameoverTexture = makeTransparentTexture(gameoverTexture, gameoverImage);
    mainmenuTexture = makeTexture(mainmenuTexture, mainmenuImage);
    howtoplayTexture = makeTexture(howtoplayTexture, howtoImage);
    //Classic
    c_floorTexture = makeTexture(c_floorTexture, c_floorImage);
    c_cityTexture = makeTransparentTexture(c_cityTexture, c_cityImage);
    c_emissileTexture = makeTransparentTexture(c_emissileTexture, c_emissileImage);
    c_dmissileTexture = makeTransparentTexture(c_dmissileTexture, c_dmissileImage);
    
    //remove ppm's
    remove("./images/city.ppm");
    remove("./images/stars.ppm");
    remove("./images/street.ppm");
    remove("./images/civilian.ppm");
    remove("./images/gameover.ppm");
    remove("./images/mainmenu.ppm");
    remove("./images/dcity.ppm");
    remove("./images/emissile.ppm");
    remove("./images/dmissile.ppm");
    remove("./images/ufo.ppm");
    remove("./images/howtoplay.ppm");
    //remove classic stuff  
    remove("./images/c_city.ppm");
    remove("./images/c_emissile.ppm");
    remove("./images/c_dmissile.ppm");
    remove("./images/c_floor.ppm");
}


void check_mouse(XEvent *e, Game *game)
{
    static int savex = 0;
    static int savey = 0;
    static int n = 0;
    Audio *a;
    a = &game->sounds;


    if (e->type == ButtonRelease) {
            return;
    }
    if (e->type == ButtonPress && lvlState(game) < 0) {
        //LEFT-CLICK
        if (e->xbutton.button==1) {
            //Left button was pressed
            int y = WINDOW_HEIGHT - e->xbutton.y;
            //Check game state when LEFT-clicking
            if (gameState(game) == 1 || gameState(game) == 2) {
                a->playAudio(30);
                menuClick(game);
                a->playAudio(32);
            } else if (gameState(game) == 0) {
                // JBC Added 5/30 to only make defense 
                // missiles and play sound when enemy 
                // missiles are present
                if ((game->nmissiles > 0 ||
               game->nsmissiles > 0) &&
            game->defMissilesRemaining > 0) {
                        makeDefenseMissile(game, e->xbutton.x, y);
                        a->playAudio(20);
                        game->defMissilesRemainingAfterLevel = 
                            game->defMissilesRemaining;
                }
            }
            return;
        }
        //RIGHT-CLICK
        if (e->xbutton.button==3) {
            //Check game state when RIGHT-clicking
            if (gameState(game) == 1) {
                //Menu functions
            } else if (gameState(game) == 0) {
                //Game Functions
                // fireDefenseMissile(game);
                // JBC idea to add menu pop up for right-click
                game->gState ^= 1;
            }
            return;
        }
    }
    //Did the mouse move?
    if (savex != e->xbutton.x || savey != e->xbutton.y) {
            savex = e->xbutton.x;
            savey = e->xbutton.y;
            int y = WINDOW_HEIGHT - e->xbutton.y;
            if (++n < 10)
                    return;
            if (gameState(game) == 1 || gameState(game) == 2) {
                //Menu Functions
                mouseOver(savex, y, game);
            } else if (gameState(game) == 0) {
                //Game Functions
                if (game->lcm)
                    lastCityMode(savex, y, game);
                // JBC note 5/13
                // moved the "particle" stuff out of here 
                // makeParticle(game, e->xbutton.x, y);
            }
    }
}

int check_keys(XEvent *e, Game *game)
{
    //Was there input from the keyboard?
    if (e->type == KeyPress) {
        int key = XLookupKeysym(&e->xkey, 0);
        if (key == XK_Escape) {
            return 1;
        }

        // Added line for checking "z" key (just closes for now)
        if (key == XK_z) {
            return 1;
        }

        // Added line for checking "n" key (nukeEm/ Kill 'em all)
        if (key == XK_n) {
            nukeEmAll(game);
        }

        //JR: Allows pause menu if play has been clicked
        if (key == XK_m && game->inGame == 1 && game->gState <= 1) {
            game->gState ^= 1;
        }
        if (key == XK_c) {
            classicMode(game);
        }
        //JR Last City Standing
        if (key == XK_l) {
            game->lcm ^= 1;
        }

        //DT special feature - radar
        if (key == XK_r) {
            game->radarOn ^= 1;
        }

        //JG: ufo
        if (key == XK_b) {
            game->ufoOn ^= 1;
            initUFO(game);
        }
        
        //You may check other keys here.
    }
    //JR: Check if exit button was clicked
    // This is a temp work around for my exit code and should
    // NOT interfere with anyone elses code or main functions
    if (game->menuExit == 1) {
        return 1;
    } else {
        return 0;
    }
}

// JBC note 5/13
// moved the "particle" stuff out of here 
void movement(Game *game)
{    
    radarPhysics(game);
    eMissilePhysics(game);
    sMissilePhysics(game);
    //dMissilePhysics(game);
    eExplosionPhysics(game);
    civilianPhysics(game);
    ufoPhysics(game);
}

void render_menu(Game *game)
{
    renderBackground(mainmenuTexture);
    renderMenuObjects(game);
    renderMenuText(game);
    game_credits(game);
}

void render_settings(Game *game)
{
    renderBackground(mainmenuTexture);
    renderSettings(game);
    renderSettingsText(game);
}

void render_newgame(Game *game)
{
    renderBackground(starsTexture);
    renderStruc(game);
    renderNewLevelMsg(game);
}

void render_gameover(Game *game)
{
    //renderBackground(gameoverTexture);
    gameOver(game);
}

void render(Game *game)
{
    glClear(GL_COLOR_BUFFER_BIT);
    //glPushMatrix();
    //glColor3ub(150,160,220);
    // JBC removed "particle" stuff that is no longer in use   
    // DT
    // JBC commented out... please keep for my testing
    //        if (game->nmissiles < 10) {
    //      createEMissiles(game);
    //  }
    renderBackground(starsTexture);
    //endLevel(game);
    renderRadar(game);
    renderUFO(game);
    renderEMissiles(game);
    renderEExplosions(game);
    renderDefenseMissile(game);
    renderStruc(game);
    renderScores(game);
    renderSMissile(game);
    // renderDefExplosions(game);
}
