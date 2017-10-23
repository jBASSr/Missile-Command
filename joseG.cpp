//Author: Jose Garcia
//Program: joseG.cpp
//Purpose: This program is my source code for rendering shapes
//that are supposed to represent the floor and cities
//Written: 4/28/16
//Modified: 6/1/16

#include "joseG.h"
#include "danielT.h"
#include "joseR.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <vector>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>
extern "C" {
#include "fonts.h"
}

using namespace std;

#define MakeVector(x, y, z, v) (v)[0]=(x),(v)[1]=(y),(v)[2]=(z)

GLuint cityTexture;
GLuint streetTexture;
GLuint civilianTexture;
GLuint dcityTexture;
GLuint ufoTexture;
//JR
GLuint c_cityTexture;
GLuint c_floorTexture;

void initStruc(Game *game)
{

	Structures *shape = &game->structures;
	//floor shape
	shape->floor.width = WINDOW_WIDTH;
	shape->floor.height = 50;
	shape->floor.center.x = 400;
	shape->floor.center.y = 10;

	//city shape
	for (int i=0; i < CITYNUM; i++) {
		shape->city[i].alive = 1;
		shape->city[i].width = 60;
		shape->city[i].height = 30;
		shape->city[i].center.x = 100 + i*200;
		shape->city[i].center.y = 80;
	}
}

void renderUFO(Game *game)
{
    if (game->ufoOn == 0)
	return;
	float w;
	UFO *u = &game->ufo;
	glColor3f(1.0, 1.0, 1.0);
	glPushMatrix();
	glTranslatef(u->pos.x, u->pos.y, 0);
	w = 20;
	//h = c->height + 10;
	glBindTexture(GL_TEXTURE_2D, ufoTexture);
	//For transparency
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glBegin(GL_QUADS);
	if (u->vel.x > 0.0) {
		glTexCoord2f(0.0f, 1.0f); glVertex2i(-w,-w);
		glTexCoord2f(0.0f, 0.0f); glVertex2i(-w, w);
		glTexCoord2f(1.0f, 0.0f); glVertex2i( w, w);
		glTexCoord2f(1.0f, 1.0f); glVertex2i( w,-w);
	}
	else
	{
		glTexCoord2f(1.0f, 1.0f); glVertex2i(-w,-w);
		glTexCoord2f(1.0f, 0.0f); glVertex2i(-w, w);
		glTexCoord2f(0.0f, 0.0f); glVertex2i( w, w);
		glTexCoord2f(0.0f, 1.0f); glVertex2i( w,-w);
	}
	glEnd();
	glDisable(GL_BLEND);
	glPopMatrix();
	glBindTexture(GL_TEXTURE_2D, 0);
}

void initUFO(Game *g)
{
	UFO *u = &g->ufo;
	//MakeVector(-150.0,180.0,0.0, u->pos);
	//MakeVector(6.0,0.0,0.0, u->vel);
	u->pos.x = -150.0;
	//spawns UFO between 380 and 680
	u->pos.y = (rand()%300)+480.0;
	u->pos.z = 0.0;
	u->vel.x = 4.0;
	u->vel.y = 0.0;
	u->vel.z = 0.0;
}

void ufoPhysics(Game *game)
{
    if (game->ufoOn == 0)
	return;
	UFO *u = &game->ufo;
	//UFO *u;
	//move ufo...
	int addgrav = 1;
	//Update position
	u->pos.x += u->vel.x;
	u->pos.y += u->vel.y;
	//Check for collision with window edges
	if ((u->pos.x < -140.0 && u->vel.x < 0.0) ||
				(u->pos.x >= (float)WINDOW_WIDTH+140.0 && u->vel.x > 0.0)) {
		u->vel.x = -u->vel.x;
		addgrav = 0;
	}
	if ((u->pos.y < 470.0 && u->vel.y < 0.0) ||
				(u->pos.y >= (float)WINDOW_HEIGHT && u->vel.y > 0.0)) {
		u->vel.y = -u->vel.y;
		addgrav = 0;
	}
	//Gravity
	if (addgrav)
		u->vel.y -= 0.05;
}

void destroyCity(Game *game, int citynum)
{
	Shape *dcity = &game->structures.city[citynum];
	dcity->alive = 0;
	makeCivilian(game,dcity->center.x,dcity->center.y);
	return;
}

void renderStruc(Game *game)
{
	Structures *shape = &game->structures;

	//Draw shapes...
	float w, h, w2, h2;

	//draw floor
	glColor3ub(105,105,105);
	Shape *s;
	s = &shape->floor;
	glPushMatrix();
	glTranslatef(s->center.x, s->center.y, 0);
	w = s->width;
	h = s->height;
	if (game->gfxMode)
		glBindTexture(GL_TEXTURE_2D, streetTexture);
	else
		glBindTexture(GL_TEXTURE_2D, c_floorTexture);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 1.0f); glVertex2i(-w,-h);
	glTexCoord2f(0.0f, 0.0f); glVertex2i(-w, h);
	glTexCoord2f(1.0f, 0.0f); glVertex2i( w, h);
	glTexCoord2f(1.0f, 1.0f); glVertex2i( w,-h);
	glEnd();
	glPopMatrix();
	glBindTexture(GL_TEXTURE_2D, 0);

	//draw cities
	Shape *c;
	for (int i = 0; i < CITYNUM; i++) {
		c = &shape->city[i];
		if (c->alive==1) {
		glColor3f(1.0, 1.0, 1.0);
		glPushMatrix();
		glTranslatef(c->center.x, c->center.y, 0);
		w2 = c->width;
		h2 = c->height;
		if (game->gfxMode)
			glBindTexture(GL_TEXTURE_2D, cityTexture);
		else
			glBindTexture(GL_TEXTURE_2D, c_cityTexture);
		//For transparency
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		//glColor4ub(255,255,255,255);
		glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 1.0f); glVertex2i(-w2,-h2);
			glTexCoord2f(0.0f, 0.0f); glVertex2i(-w2, h2);
			glTexCoord2f(1.0f, 0.0f); glVertex2i( w2, h2);
			glTexCoord2f(1.0f, 1.0f); glVertex2i( w2,-h2);
		glEnd();
		glDisable(GL_BLEND);
		glPopMatrix();
		glBindTexture(GL_TEXTURE_2D, 0);
		}
		else {
		glColor3f(1.0, 1.0, 1.0);
		glPushMatrix();
		glTranslatef(c->center.x, c->center.y, 0);
		w2 = c->width;
		h2 = c->height + 10;
		glBindTexture(GL_TEXTURE_2D, dcityTexture);
		//For transparency
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 1.0f); glVertex2i(-w2,-h2);
			glTexCoord2f(0.0f, 0.0f); glVertex2i(-w2, h2);
			glTexCoord2f(1.0f, 0.0f); glVertex2i( w2, h2);
			glTexCoord2f(1.0f, 1.0f); glVertex2i( w2,-h2);
		glEnd();
		glDisable(GL_BLEND);
		glPopMatrix();
		glBindTexture(GL_TEXTURE_2D, 0);
		}
	}
	//draw civilians
	glColor3f(1.0,1.0,1.0);
	glPushMatrix();
	for (int i=0; i<game->nparticles; i++) {
		Vec *g = &game->particle[i].part.center;
		w = 20;
		h = 20;
		glBindTexture(GL_TEXTURE_2D, civilianTexture);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 1.0f); glVertex2i(g->x-w, g->y-h);
			glTexCoord2f(0.0f, 0.0f); glVertex2i(g->x-w, g->y+h);
			glTexCoord2f(1.0f, 0.0f); glVertex2i(g->x+w, g->y+h);
			glTexCoord2f(1.0f, 1.0f); glVertex2i(g->x+w, g->y-h);
		glEnd();
		glDisable(GL_BLEND);
		glPopMatrix();
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

void makeCivilian(Game *game, int x, int y)
{
	if (game->nparticles >= MAX_PARTICLES)
		return;
	//std::cout << "makeParticle()" << x << " " << y << std::endl;
	//position of civilian
	Particle *p = &game->particle[game->nparticles];
	p->part.center.x = x;
	p->part.center.y = y;
	p->velocity.y = 0.05;
	p->velocity.x = 0.5;
	game->nparticles++;
}

void civilianPhysics(Game *game)
{
	Particle *p;
	Structures *sh = &game->structures;
	Shape *c;

	for (int i=0; i<game->nparticles; i++) {
		p = &game->particle[i];
		//civilian position
		p->part.center.x += p->velocity.x;
		p->part.center.y += p->velocity.y;

		//gravity
		p->velocity.y -= GRAVITY;

	//check for collision with floor
	c = &sh->floor;
	if (p->pos.y <= c->center.y+c->height) {
		p->velocity.y *= -0.1;
	}

	//check for off-screen
	if (p->part.center.y < 0.0 || p->part.center.x > WINDOW_WIDTH) {
			//std::cout << "off screen" << std::endl;
			game->particle[i] = game->particle[game->nparticles-1];
			game->nparticles--;
	}
	}
}

void renderBackground(GLuint starsTexture)
{
	glPushMatrix();	
	glColor3f(1.0,1.0,1.0);
    	glBindTexture(GL_TEXTURE_2D, starsTexture);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 1.0f); glVertex2i(0, 0);
	glTexCoord2f(0.0f, 0.0f); glVertex2i(0, WINDOW_HEIGHT);
	glTexCoord2f(1.0f, 0.0f); glVertex2i(WINDOW_WIDTH, WINDOW_HEIGHT);
	glTexCoord2f(1.0f, 1.0f); glVertex2i(WINDOW_WIDTH, 0);
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
	glPopMatrix();
}

void renderScores(Game *game)
{
    Rect r;
    r.bot = 30;
    r.left = 50.0;
    r.center = 0;
    ggprint16(&r, 16, 0x00005599, "Score: %i", game->score);
}

unsigned char *buildAlphaData(Ppmimage *img)
{
	//add 4th component to RGB stream...
	int i;
	int a,b,c;
	unsigned char *newdata, *ptr;
	unsigned char *data = (unsigned char *)img->data;
	newdata = (unsigned char *)malloc(img->width * img->height * 4);
	ptr = newdata;
	for (i=0; i<img->width * img->height * 3; i+=3) {
		a = *(data+0);
		b = *(data+1);
		c = *(data+2);
		*(ptr+0) = a;
		*(ptr+1) = b;
		*(ptr+2) = c;
		//get largest color component...
		//*(ptr+3) = (unsigned char)((
		//              (int)*(ptr+0) +
		//              (int)*(ptr+1) +
		//              (int)*(ptr+2)) / 3);
		//d = a;
		//if (b >= a && b >= c) d = b;
		//if (c >= a && c >= b) d = c;
		//*(ptr+3) = d;
		*(ptr+3) = (a|b|c);
		ptr += 4;
		data += 3;
	}
	return newdata;
}

GLuint makeTexture(GLuint texture, Ppmimage * image)
{

	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &texture);

	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 3,
							image->width, image->height,
							0, GL_RGB, GL_UNSIGNED_BYTE, image->data);
	return texture;
}

GLuint makeTransparentTexture(GLuint texture, Ppmimage * image)
{

	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &texture);

	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 3,
							image->width, image->height,
							0, GL_RGB, GL_UNSIGNED_BYTE, image->data);
	GLuint transparentTexture;
	glGenTextures(1, &transparentTexture);

	glBindTexture(GL_TEXTURE_2D, transparentTexture);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	unsigned char *transparentData = buildAlphaData(image);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->width, image->height, 0,
							GL_RGBA, GL_UNSIGNED_BYTE, transparentData);
	free(transparentData);
	return transparentTexture;
}
