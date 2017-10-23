//Daniel Turack
//Written: April 27, 2016
//Program will control Enemy Missiles and the resulting collisions
//Additional Special Feature: Radar to only show missiles within radius

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>
#include "danielT.h"
#include "joseR.h"
#include "joseG.h"
extern "C" {
#include "fonts.h"
}

using namespace std;

GLuint emissileTexture;
GLuint c_emissileTexture;

//macros for vectors
#define rnd() (((double)rand())/(double)RAND_MAX)
#define random(a) (rand()%a)
#define VecZero(v) (v)[0]=0.0,(v)[1]=0.0,(v)[2]=0.0
#define MakeVector(x, y, z, v) (v)[0]=(x),(v)[1]=(y),(v)[2]=(z)
#define VecCopy(a,b) (b)[0]=(a)[0];(b)[1]=(a)[1];(b)[2]=(a)[2]
#define VecDot(a,b) ((a)[0]*(b)[0]+(a)[1]*(b)[1]+(a)[2]*(b)[2])

//-----------------------------------------------------------------------------
//Setup timers - Code for timer setup from Gordon Griesel
const double physicsRate = 1.0 / 30.0;
const double oobillion = 1.0 / 1e9;
struct timespec timeStart, timeCurrent;
struct timespec timePause;
double physicsCountdown=0.0;
double timeSpan=0.0;
unsigned int upause=0;
double timeDiff(struct timespec *start, struct timespec *end)
{
	return (double)(end->tv_sec - start->tv_sec ) +
		(double)(end->tv_nsec - start->tv_nsec) * oobillion;
}
void timeCopy(struct timespec *dest, struct timespec *source)
{
	memcpy(dest, source, sizeof(struct timespec));
}
//-----------------------------------------------------------------------------


//constants
const int MAX_EMISSILES = 10;

//int mCount=0;
int smCount=0;
int chCount=0;
float msx=0;
float msy=0;

//defined types
typedef float Flt;
typedef Flt Matrix[4][4];
//

//handles missile explosion physics
void eExplosionPhysics(Game *game)
{
	EExplosion *e;

	if (game->neexplosions <=0) {
		return;
	}
	for (int m=0; m<game->neexplosions; m++) {
		e = &game->eearr[m];
		e->radius += e->radiusInc;
		if ((rand()&1)==0) {
			e->color[0] = 1.0f;
			e->color[1] = 0.1f;
			e->color[2] = 0.1f;
		}
		else if (rand()%3==0) {
			e->color[0] = 1.0f;
			e->color[1] = 1.0f;
			e->color[2] = 0.1f;
		}
		else {
			e->color[0] = 1.0f;
			e->color[1] = 0.549f;
			e->color[2] = 0.0f;
		}
		if (e->radius >= 40.0) {
			e->radiusInc *= -1.25;
			//random spawn of missiles from explosion
			if ((rand()&49)==0 && game->mCount>5 && e->pos.y>WINDOW_HEIGHT/2) {
				std::cout << "special" << endl;
				for (int g=0; g<rand()%5; g++) {
					createEMissiles(game, e->pos.x+g, e->pos.y+g);
				}
			}
		}
		if (e->radius <= 0.0) {
			//delete explosion from array
			game->eearr[m] = game->eearr[game->neexplosions-1];
			game->neexplosions--;	
		}
	}
}

//handles missile movement and collisions
void eMissilePhysics(Game *game)
{
	Structures *sh = &game->structures;
	EMissile *e;
	Shape *c;
	EExplosion *d;
	UFO *u;
	Audio *a;
	a = &game->sounds;

	//spawn missiles from UFO
	u = &game->ufo;
	if ((rand()&19)==0 && game->mCount>5 && game->nmissiles<7
			&& u->pos.x > 0.0 && u->pos.x < WINDOW_WIDTH
			&& game->nmissiles>3) {
		for (int p=0; p<(rand()&3); p++) {
			createEMissiles(game, u->pos.x+p, u->pos.y+p);
		}
	}
	if (game->nmissiles < 10) {
		if ((rand()%(80/game->level))==0) {
			createEMissiles(game, 0, 0);
		}
	}
	for (int i=0; i<game->nmissiles; i++) {
		e = &game->emarr[i];

		//update missile position
		e->pos.x += e->vel.x;
		e->pos.y += e->vel.y;

		//update missile trail
		e->trail.end.x = e->pos.x;
		e->trail.end.y = e->pos.y;

		//check for off screen
		if (e->pos.y < 0.0 || e->pos.x <= 0.0 || e->pos.x >= WINDOW_WIDTH) {
			eMissileExplode(game, i);
			std::cout << "misoff" << std::endl;
			continue;
		}

		int k;
		int aliveCount=0;
		//check for collision with cities 
		for (k=0; k<CITYNUM; k++) {
			c = &sh->city[k];
			if (c->alive == 1 && e->pos.y <= c->center.y+c->height && 
					e->pos.x <= c->center.x+c->width && 
					e->pos.x >= c->center.x-c->width) {
				destroyCity(game, k);
				//game->lvl.cCount--;
				makeCivilian(game,c->center.x,c->center.y);
				eMissileExplode(game, i);
				a->playAudio(10);
				chCount++;
				game->score -= 10;
				break;
			}
			else if (c->alive == 1) {
				aliveCount++;
			}
		}

		if (aliveCount==0) {
			game->mCount = 0;
		}

		if (k<CITYNUM)
			continue;

		//check for collision with floor
		c = &sh->floor;
		if (e->pos.y <= c->center.y+c->height) {
			eMissileExplode(game, i);
			a->playAudio(0);
			continue;
		}

		int p;
		//check for explosion collision
		for (p=0; p<game->neexplosions; p++) {
			d = &game->eearr[p];
			float xd = abs(e->pos.x-d->pos.x);
			float yd = abs(e->pos.y-d->pos.y);
			float dist = sqrt(xd*xd+yd*yd);
			if (dist<=d->radius) {
				eMissileExplode(game,i);
				a->playAudio(0);
				game->score += 50*game->level;
				break;
			}
		}
		if (p<game->neexplosions)
			continue;

		//randomly generate new missile branch
		//as long as more missiles are available
		//only generate branches above middle of screen and below top eighth
		if ((rand()&99)==0 && 
				game->nmissiles<10-game->level && 
				game->mCount>10-game->level &&
				e->pos.y>WINDOW_HEIGHT/2 &&
				e->pos.y<((WINDOW_HEIGHT/8)*7)) {
			for (int q=0; q<(rand()%game->level); q++) {
				createEMissiles(game, e->pos.x, e->pos.y);
			}
		}
	}
	//puts in a random delay between missile creation
	if (game->nmissiles < 10) {
		if ((rand()%(100/game->level))==0) {
			createEMissiles(game, 0, 0);
		}
	}
	return;
}



void eMissileExplode(Game *game, int misnum)
{
	EMissile *e = &game->emarr[misnum];
	//create explosion graphic
	createEExplosion(game, e->pos.x, e->pos.y);
	makeCivilian(game, e->pos.x, game->structures.floor.height);
	//delete missile
	game->emarr[misnum] = game->emarr[game->nmissiles-1];
	game->nmissiles--;
}

void displayScore(Game *g) 
{
	if (g->level==1)
		return;
	g->gState = 5;
	smCount = 1;
	if (g->level > 4)
		smCount = 2;
	return;
}

//initialize enemy missles from top of screen
void createEMissiles(Game *g, float x, float y)
{
	//counts down missiles in each level
	if (g->mCount<=0) {
		//waits at the end of each level
		if (g->nmissiles>0 || g->neexplosions>0 || g->nsmissiles>0)
			return;
		g->level++;
		g->mCount = 5+(g->level*5);
		//after level has increased, return from function 
		//level/score screen will then be displayed
		if (g->prevLevel < g->level) {
			displayScore(g);
			g->prevLevel++;
		}
		return;
	}

	if (g->nmissiles==10) {
		//save x and y value passed in
		if (x!=0 && y!=0) {
			msx = x;
			msy = y;
		}
		return;
	}
	float mRatio;
	EMissile *e = &g->emarr[g->nmissiles];
	if (x==0 && y==0) {
		//if previous location saved, spawn from there
		if (msx>0 || msy>0) {
			e->pos.y = msy;
			e->pos.x = msx;
			msx=0;
			msy=0;
		}
		else {
			e->pos.y = WINDOW_HEIGHT-1;    
			e->pos.x = (rand()%WINDOW_WIDTH-2.0) +1.0;
			e->pos.z = 0;
		}
	}
	else {
		e->pos.y = y;
		e->pos.x = x;
		e->pos.z = 0;
	}
	e->vel.y = -0.1+(g->level*-0.4);
	//find random destination
	float tempX = rand()%WINDOW_WIDTH;
	//calculate angle using start and end points
	mRatio = (e->pos.x-tempX)/WINDOW_HEIGHT;
	e->vel.x = e->vel.y*mRatio;
	e->vel.z = 0;
	e->color[0] = 0.0f;
	e->color[1] = 0.5f;
	e->color[2] = 0.5f;
	e->trail.start.x = e->pos.x;
	e->trail.start.y = e->pos.y;
	e->trail.start.z = 0.0;
	e->trail.end.x = e->pos.x;
	e->trail.end.y = e->pos.y;
	e->trail.end.z = 0.0;
	e->trail.width = 1.5;
	e->trail.color[0] = 0.75;
	e->trail.color[1] = 0.2;
	e->trail.color[2] = 0.2;
	g->mCount--;
	g->nmissiles++;
}

void createEExplosion(Game *g, float x, float y)
{
	EExplosion *e = &g->eearr[g->neexplosions];
	e->pos.y = y;
	e->pos.x = x;
	e->pos.z = 0;
	e->radius = 4.0;
	e->radiusInc = 0.75;
	e->color[0] = 1.0f;
	e->color[1] = 0.0f;
	e->color[2] = 0.0f;
	g->neexplosions++;
}

void renderEMissiles(Game *g)
{
	if (g->radarOn==0) {
		for (int i=0; i<g->nmissiles; i++) {
			EMissile *e = &g->emarr[i];

			//render missile trails using start and end point of missile
			glPushMatrix();
			glColor3f(e->trail.color[0], e->trail.color[1], e->trail.color[2]);
			glLineWidth(e->trail.width);
			glBegin(GL_LINES);
			glVertex2f(e->trail.start.x, e->trail.start.y);
			glVertex2f(e->trail.end.x, e->trail.end.y);
			glEnd();
			glPopMatrix();

			//render Missiles using texture
			glColor3f(1.0,1.0,1.0);
			glPushMatrix();
			if (g->gfxMode)
				glBindTexture(GL_TEXTURE_2D, emissileTexture);
			else
				glBindTexture(GL_TEXTURE_2D, c_emissileTexture);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
			glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 1.0f); glVertex2i(e->pos.x-8, e->pos.y-15);
			glTexCoord2f(0.0f, 0.0f); glVertex2i(e->pos.x-10, e->pos.y+15);
			glTexCoord2f(1.0f, 0.0f); glVertex2i(e->pos.x+10, e->pos.y+15);
			glTexCoord2f(1.0f, 1.0f); glVertex2i(e->pos.x+8, e->pos.y-15);
			glEnd();
			glDisable(GL_BLEND);
			glPopMatrix();
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}
	else {
		for (int i=0; i<g->nmissiles; i++) {
			EMissile *e = &g->emarr[i];

			//render Missiles with radar
			//only simple missile is rendered with radar
			float xd = abs(e->pos.x-g->radar.pos.x);
			float yd = abs(e->pos.y-g->radar.pos.y);
			float dist = sqrt(xd*xd+yd*yd);
			if (dist<=g->radar.radius || e->pos.y <=100.0) {
				glPushMatrix();
				glColor3f(e->color[0], e->color[1], e->color[2]);
				glBegin(GL_QUADS);
				glVertex2i(e->pos.x-1, e->pos.y-6);
				glVertex2i(e->pos.x-3, e->pos.y+6);
				glVertex2i(e->pos.x+3, e->pos.y+6);
				glVertex2i(e->pos.x+1, e->pos.y-6);
				glEnd();
				glPopMatrix();
			}
		}
	}
	//count up cities still alive
	int cCount = 0;
	for (int c=0; c<CITYNUM; c++) {
		if (g->structures.city[c].alive==1)
			cCount++;
	}
	if (cCount==0) {
		g->mCount=0;
	}
	//print out game info
	Rect r;
	r.bot = WINDOW_HEIGHT-30;
	r.left = 50.0;
	r.center = 0;
	ggprint8b(&r, 16, 0x00005599, "LEVEL: %i", g->level);
	ggprint8b(&r, 16, 0x00005599, "High Score 1: %i", g->highScores[0]);
	ggprint8b(&r, 16, 0x00005599, "High Score 2: %i", g->highScores[1]);
	//ggprint8b(&r, 16, 0x00005599, "City Hit Count: %i", chCount);
	//ggprint8b(&r, 16, 0x00005599, "Remaining Cities: %i", cCount);
}

//function to display explosions
void renderEExplosions(Game *g) 
{
	int tris = 20;
	float twicePi = 2.0f * 3.14159265359;
	for (int i=0; i<g->neexplosions; i++) {
		EExplosion *e = &g->eearr[i];
		glPushMatrix();
		glTranslatef(0.0, 0.0, -1.0);
		glEnable (GL_BLEND);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glColor4f(e->color[0], e->color[1], e->color[2], 0.75);
		glBegin(GL_TRIANGLE_FAN);
		glVertex2f(e->pos.x, e->pos.y);
		for (int i=0; i<=tris; i++) {
			glVertex2f(
					e->pos.x + (e->radius * 
						cos(i * twicePi/tris)),
					e->pos.y + (e->radius * 
						sin(i * twicePi/tris))
					);
		}
		glDisable(GL_BLEND);
		glEnd();
		glPopMatrix();
	}
}

//Smart Missile
void createSMissile(Game *g)
{
	//keep track of available smart missiles
	if (smCount==0)
		return;
	smCount--;
	SMissile *s = &g->smarr[g->nsmissiles];
	s->pos.y = WINDOW_HEIGHT-1.0;
	s->pos.x = (rand()%(WINDOW_WIDTH)-1.0);
	s->pos.z = 0.0;
	s->vel.y = -0.1+(g->level*-0.4);
	//find random destination
	float tempX = rand()%WINDOW_WIDTH;
	//calculate angle using start and end points
	float mRatio = (s->pos.x-tempX)/WINDOW_HEIGHT;
	s->vel.x = s->vel.y*mRatio;
	s->vel.z = 0.0;
	s->color[0] = 0.4f;
	s->color[1] = 0.6f;
	s->color[2] = 1.0f;
	g->nsmissiles++;
}

void sMissilePhysics(Game *g)
{
	SMissile *s;
	Structures *sh = &g->structures;
	Shape *c;
	EExplosion *d;
	Audio *a;
	a = &g->sounds;

	//create more smissiles if available
	if (g->nsmissiles==0 && (rand()&99)==0
			&& g->mCount>=5 && g->nmissiles >=10) {
		createSMissile(g);
	}
	else if (smCount<=0 && g->nsmissiles==0)
		return;

	for (int i=0; i<g->nsmissiles; i++) {

		s = &g->smarr[i];

		//update missile position
		s->pos.x += s->vel.x;
		s->pos.y += s->vel.y;

		//add gravity only if missile is going up
		if (s->vel.y >= -0.5) {
			s->vel.y += -0.5;
		}
		//add friction if missile being pushed sideways
		if (abs(s->vel.x) >= 1.0) {
			s->vel.x *= 0.998;
		}

		//check for off screen
		//if missile hits the side of the screen it explodes
		if (s->pos.x <= 0.0 || s->pos.x >= WINDOW_WIDTH) {
			createEExplosion(g, s->pos.x, s->pos.y);
			g->smarr[i] = g->smarr[g->nsmissiles-1];
			g->nsmissiles--;
			a->playAudio(10);	    
			//s->vel.x *= -1.0;
			continue;
		}

		//if missile pushed to the top of the screen it switches direction
		if (s->pos.y >= WINDOW_HEIGHT) {
			s->vel.y += -1.0;
		}

		int k;

		//check for collision with cities 
		for (k=0; k<CITYNUM; k++) {
			c = &sh->city[k];
			if (c->alive == 1 && 
					s->pos.y <= c->center.y+c->height &&
					s->pos.x <= c->center.x+c->width &&
					s->pos.x >= c->center.x-c->width) {
				destroyCity(g, k);
				makeCivilian(g, c->center.x, c->center.y);
				createEExplosion(g, s->pos.x, s->pos.y);
				g->smarr[i] = g->smarr[g->nsmissiles-1];
				g->nsmissiles--;
				a->playAudio(10);
				chCount++;
				g->score -= 10;
				break;
			}
		}

		if (k<CITYNUM)
			continue;

		//check for collision with floor
		c = &sh->floor;
		if (s->pos.y <= c->center.y+c->height) {
			createEExplosion(g, s->pos.x, s->pos.y);
			g->smarr[i] = g->smarr[g->nsmissiles-1];
			g->nsmissiles--;
			a->playAudio(0);
			continue;
		}

		int p;
		//check for explosion collision
		for (p=0; p<g->neexplosions; p++) {
			d = &g->eearr[p];
			float xd = abs(s->pos.x-d->pos.x);
			float yd = abs(s->pos.y-d->pos.y);
			float dist = sqrt(xd*xd+yd*yd);
			//smissile explodes if caught within radius - direct hit
			if (dist<=d->radius) {
				createEExplosion(g, s->pos.x, s->pos.y);
				g->smarr[i] = g->smarr[g->nsmissiles-1];
				g->nsmissiles--;
				a->playAudio(0);
				g->score += 500;
				break;
			}
			//if smissile is close to explosion it goes the other way
			else if (dist<=d->radius+20.0) {
				s->vel.y += 0.75;
				s->vel.x += ((rand()&9)-5)*0.01;
				//s->vel.x *= -0.99;
				break;
			}
		}
		if (p<g->neexplosions)
			continue;
	}
}

void renderSMissile(Game *g)
{
	int tris = 20;
	float twicePi = 2.0f * 3.14159265359;
	for (int i=0; i<g->nsmissiles; i++) {
		SMissile *s = &g->smarr[i];
		glPushMatrix();
		glColor3f(s->color[0], s->color[1], s->color[2]);
		glBegin(GL_TRIANGLE_FAN);
		glVertex2f(s->pos.x, s->pos.y);
		for (int i=0; i<=tris; i++) {
			glVertex2f(
					s->pos.x + (5.0 * cos(i * twicePi/tris)),
					s->pos.y + (5.0 * sin(i * twicePi/tris))
					);
		}
		glEnd();
		glPopMatrix();
	}
}

//DT special feature - radar

void renderRadar(Game *g) 
{
	if (g->radarOn == 0)
		return;
	Radar *r = &g->radar;
	//Begin by creating an outline of the radar
	int tris = 100;
	float twicePi = 2.0f * 3.14159265359;
	glPushMatrix();
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(r->color[0], r->color[1], r->color[2], 0.2);
	glLineWidth(1.0);
	glBegin(GL_LINE_LOOP);
	for (int i=0; i<=tris; i++) {
		glVertex2f(
				r->pos.x + (r->radius * cos(i * twicePi/tris)),
				r->pos.y + (r->radius * sin(i * twicePi/tris))
				);
	}
	glDisable(GL_BLEND);
	glEnd();
	glPopMatrix();


	//create a triangle fan to illuminate radar
	glPushMatrix();
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBegin(GL_TRIANGLE_FAN);
	glColor4f(r->color[0], r->color[1], r->color[2], 0.1);
	glVertex2f(r->tri[0].x, r->tri[0].y);
	float transp = r->color[3];
	for (int i = r->radius2; transp>-0.2; i--) {
		glColor4f(r->color[0], r->color[1], r->color[2], transp);
		glVertex2f(
				r->pos.x + (r->radius * cos(i * twicePi/tris)),
				r->pos.y + (r->radius * sin(i * twicePi/tris))
				);
		transp -= 0.005;
	}
	glEnd();
	glDisable(GL_BLEND);
	glPopMatrix();
}

void radarPhysics(Game *g) 
{
	if (g->radarOn == 0)
		return;
	Radar *r = &g->radar;

	//pick fan for illumination
	if (r->radiusInc==5.0) {
		r->radius2++;
		r->radiusInc=0.0;
	}
	else
		r->radiusInc +=1.0;

	if (r->radius2 >= 100)
		r->radius2 = 0;

	//update radar triangle
	int tris = 100;
	float twicePi = 2.0f * 3.14159265359;
	r->tri[0].x = r->pos.x;
	r->tri[0].y = r->pos.y;
	r->tri[0].z = r->pos.z;
	r->tri[1].x = r->pos.x + (r->radius * 
			cos(r->radius2 * twicePi/tris));
	r->tri[1].y = r->pos.y + (r->radius * 
			sin(r->radius2 * twicePi/tris));
	r->tri[1].z = r->pos.z;
	r->tri[2].x = r->pos.x + (r->radius * 
			cos((r->radius2 * twicePi/tris)+twicePi/tris*8));
	r->tri[2].y = r->pos.y + (r->radius * 
			sin((r->radius2 * twicePi/tris)+twicePi/tris*8));
	r->tri[2].z = r->pos.z;
}

void initRadar(Game *g) 
{
	int tris = 100;
	float twicePi = 2.0f * 3.14159265359;
	Radar *r = &g->radar;
	r->pos.x = g->structures.city[2].center.x;
	r->pos.y = 50.0;
	r->pos.z = 0.0;
	r->radius = 600.0;
	r->radius2 = 0;
	r->radiusInc = 0.0;
	r->tri[0].x = r->pos.x;
	r->tri[0].y = r->pos.y;
	r->tri[0].z = r->pos.z;
	r->tri[1].x = r->pos.x + (r->radius * 
			cos(r->radius2 * twicePi/tris));
	r->tri[1].y = r->pos.y + (r->radius * 
			sin(r->radius2 * twicePi/tris));
	r->tri[1].z = r->pos.z;
	r->tri[2].x = r->pos.x + (r->radius * 
			cos((r->radius2 * twicePi/tris)+twicePi/tris));
	r->tri[2].y = r->pos.y + (r->radius * 
			sin((r->radius2 * twicePi/tris)+twicePi/tris));
	r->tri[2].z = r->pos.z;
	r->color[0] = 0.091;
	r->color[1] = 0.973;
	r->color[2] = 0.016;
	r->color[3] = 0.3;
}

//initializes high scores to defaults 
void initHighScores(Game *g)
{
	int basescore = 7500;
	for (int i=0;i<5;i++) {
		g->highScores[i] = basescore;
		basescore -= 500;
	}

}

//keeps track of highest five scores while game is running
void addHighScore(Game *g)
{
	int hscore = g->score;
	for (int i=0;i<5;i++) {
		if (hscore >= g->highScores[i]) {
			for (int k=4;k>i;k--) {
				g->highScores[k] = g->highScores[k-1];
			}
			g->highScores[i] = hscore;
			return;
		}
	}
}


