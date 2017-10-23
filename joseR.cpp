// Author:   Jose Reyes
// Created:  April 28, 2016
// Modified: May 9, 2016
// Program:  joseR.cpp
// Purpose:  
//			Draw and create a functioning menu that will allow
//			the user to play, change settings, and exit the game.
// Progress:
//		*May 2nd, 2016*
//			Menu shapes, text, and shape color change on mouse hover.
//			The exit button will close on click.
//		*May 9th, 2016*
//			The play button will toggle menu code off and game code on.
//			The settings button will cout a temp message (filler code).
//				This is determined by the function gameState(...)
//			Added a way of treating the menu as a pseudo pause menu.
//				This is determined by "int inGame" in "struct Game"
//		*May 15th, 2016*
//			OpenAL (sound) functionality has been added
//			Must include joseR.h
//		*May 20th, 2016*
//			Moved OpenAL code into a class
//			Changed how others may call the function to play a sound buffer.
//			Requires &game->sounds->playAudio(int);
//			0 = missile_miss, 10 = missile_hit, 20 = missile_fire
//			30/32 = mouse clicks
//		*May 24th, 2016*
//			Finalized audio code (still need in-line comments)
//			Added settings menu (adjust volume for now)
//			Can call sounds via game->sounds.playAudio(int);
//		*May 29th, 2016*
//			Finalized level-to-level code, still need to correctly
//				setup missile and cities alive count.
//			Starting on Game Over screen and High Scores?
//			
#include <GL/glx.h>
#include "missileCommand.h"
#include "joseR.h"
#include <stdio.h>
#include <time.h>
extern "C" {
	#include "fonts.h"
}

using namespace std;

extern void init_opengl();
extern void initStruc(Game *game);
extern GLuint starsTexture;
extern GLuint cityTexture;
extern void renderBackground(GLuint starsTexture);
extern void renderStruc(Game *game);
extern void renderRadar(Game *game);
extern void renderScores(Game *game);

GLuint gameoverTexture;
GLuint mainmenuTexture;
GLuint howtoplayTexture;

Audio::Audio()
{
	alutInit(0, NULL);
	if (alGetError() != AL_NO_ERROR) {
		printf("ERROR: alutInit()\n");
		return;
	}
	alGetError();
	device = alcOpenDevice(NULL);
	if (!device) {
		printf("ERROR: device\n");
		return;
	}
	alGetError();
	context = alcCreateContext(device, NULL);
	if (!alcMakeContextCurrent(context)) {
		printf("ERROR: context\n");
		return;
	}
	gVolume = 1.0;
	//Setup the listener.
	float vec[6] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f };
	alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);
	alListenerfv(AL_ORIENTATION, vec);
	alListenerf(AL_GAIN, 1.0f);
}

Audio::~Audio()
{
	alDeleteSources(1, &alSource);
	alDeleteBuffers(1, &alBuffer);
	ALCcontext *context = alcGetCurrentContext();
	ALCdevice *device = alcGetContextsDevice(context);
	alcMakeContextCurrent(NULL);
	alcDestroyContext(context);
	alcCloseDevice(device);
	alutExit();
}

void Audio::loadAudio()
{
	//Buffer holds the sound information.
	const string FILE[] = {
		"./sounds/missile_explosion.wav", "./sounds/missile_collision.wav",
		"./sounds/missile_launch2.wav", "./sounds/mouse_click.wav",
		"./sounds/mouse_release.wav", "./sounds/classic_tick.wav",
		"./sounds/classic_lvl_start.wav", "./sounds/classic_gameover.wav" };
	int val = 0;
	//Load and assign sounds
	for (int i = 0; i < TOTAL_SOUNDS; i++) {
		//Load file into buffer
		const char *f = FILE[i].c_str();
		buffer[i] = alutCreateBufferFromFile(f);
		//Check if anything was loaded into the buffer
		if (!buffer[i]) {
			printf("ERROR: Audio File Not Found!\n");
			printf("[%s] - Was not loaded.\n", f);
			break;
		}
		if (val < 30) {
			//Sets explosions and missile sounds to 10 sources each
			for (int n = 0; n < 10; n++) {
				//Generate a source
				alGenSources(1, &alSource);
				//Store source in a buffer
				alSourcei(alSource, AL_BUFFER, buffer[i]);
				//Set volume, pitch, and loop options
				alSourcef(alSource, AL_GAIN, 1.0f);
				alSourcef(alSource, AL_PITCH, 1.0f);
				alSourcei(alSource, AL_LOOPING, AL_FALSE);
				//Store value of that source to call later
				//printf("File: %s stored in buffer[%d].\n", f, val);
				source[val++] = alSource;
			}
		} else if (val >= 30 && val < 34) {
			//Sets menu click sounds
			for (int n = 0; n < 2; n++) {
				alGenSources(1, &alSource);
				alSourcei(alSource, AL_BUFFER, buffer[i]);
				alSourcef(alSource, AL_GAIN, 1.0f);
				alSourcef(alSource, AL_PITCH, 1.0f);
				alSourcei(alSource, AL_LOOPING, AL_FALSE);
				//printf("File: %s stored in buffer[%d].\n", f, val);
				source[val++] = alSource;
			}
		} else if (val >= 34 && val < 41) {
			//Sets score counter and new level sound
			if (val < 39) {
				for (int n = 0; n < 5; n++) {
					alGenSources(1, &alSource);
					alSourcei(alSource, AL_BUFFER, buffer[i]);
					alSourcef(alSource, AL_GAIN, 1.0f);
					alSourcef(alSource, AL_PITCH, 1.0f);
					alSourcei(alSource, AL_LOOPING, AL_FALSE);
					//printf("File: %s stored in buffer[%d].\n", f, val);
					source[val++] = alSource;
				}
			} else {
				alGenSources(1, &alSource);
				alSourcei(alSource, AL_BUFFER, buffer[i]);
				alSourcef(alSource, AL_GAIN, 1.0f);
				alSourcef(alSource, AL_PITCH, 1.0f);
				alSourcei(alSource, AL_LOOPING, AL_FALSE);
				//printf("File: %s stored in buffer[%d].\n", f, val);
				source[val++] = alSource;
			}
		} else {
			printf("Something may have gone wrong...\n");
		}
	}
}

void Audio::playAudio(int num)
{
	int idx = num, max;
	if (idx < 30) {
		max = idx + 9;
	} else if (idx >= 30 && idx < 34) {
		max = idx +1;
	} else if (idx >= 34 && idx < 49) {
		max = idx + 4;
	} else {
		max = idx;
	}
	alSource = source[idx];
	alGetSourcei(alSource, AL_SOURCE_STATE, &source_state);
	while (source_state == AL_PLAYING) {		
		alSource = source[idx++];
		if (idx > max) {
			printf("Max sources for this sound was reached!\n");
			break;
		}
		alGetSourcei(alSource, AL_SOURCE_STATE, &source_state);
		//printf("Sound already playing!\nPlaying source[%d]\n", idx);
	}
	alSourcef(alSource, AL_GAIN, gVolume);
	alSourcePlay(alSource);
}

void drawMenu(Game *game)
{
	for (int j = 0; j < BUTTONS; j++) {
		game->buttonSpacer[j] = (WINDOW_HEIGHT - 200) -(j*95);
		game->mButton[j].width = 120;
		game->mButton[j].height = 25;
		game->mButton[j].center.x = WINDOW_WIDTH / BUTTON_X;
		game->mButton[j].center.y = WINDOW_HEIGHT - game->buttonSpacer[j];
	}
}

void renderMenuObjects(Game *game)
{
	Shape *s;
	float w, h;
	//glBindTexture(GL_TEXTURE_2D, 0);
	for (int i = 0; i < BUTTONS; i++) {
		s = &game->mButton[i];
		glColor3ub(205,92,92);
		//Button colors based on mouse position
		if (game->mouseOnButton[i] == 1) {
			//Button selected color
			glColor3ub(120,120,120);
		}
		glPushMatrix();
		glTranslatef(s->center.x, s->center.y, 0);
		w = s->width;
		h = s->height;
		glEnd();
		glBegin(GL_QUADS);
			glVertex2i(-w,-h);
			glVertex2i(-w, h);
			glVertex2i( w, h);
			glVertex2i( w,-h);
		glEnd();
		glPopMatrix();
		//glFlush();
	}
}

void renderMenuText(Game *game)
{
	Rect rt;
	int j = 0;
	rt.bot = WINDOW_HEIGHT - game->buttonSpacer[j] - 10;
	rt.left = WINDOW_WIDTH / BUTTON_X;
	//std::cout << rt.bot << " " << rt.left << std::endl;
	rt.center = 1;
	ggprint16(&rt, 16, 0x00ffffff, "Quit");
	j++;
	rt.bot = WINDOW_HEIGHT - game->buttonSpacer[j] - 10;
	ggprint16(&rt, 16, 0x00ffffff, "Settings");
	j++;
	rt.bot = WINDOW_HEIGHT - game->buttonSpacer[j] - 10;
	ggprint16(&rt, 16, 0x00ffffff, "How To Play");
	j++;
	rt.bot = WINDOW_HEIGHT - game->buttonSpacer[j] - 10;
	if (game->inGame == 0) {
		ggprint16(&rt, 16, 0x00ffffff, "Play");
	} else {
		ggprint16(&rt, 16, 0x00ffffff, "Resume");
	}
}

//Settings buttons draw function goes here

void drawSettings(Game *game)
{
	//
	Shape *s;
	s = &game->menuBG;
	s->width = WINDOW_WIDTH - 650;
	s->height = WINDOW_HEIGHT - 550;
	s->center.x = WINDOW_WIDTH / 2;
	s->center.y = WINDOW_HEIGHT / 2;
	//Back
	s = &game->sButton[0];
	s->width = 125;
	s->height = 25;
	s->center.x = WINDOW_WIDTH / 2;
	s->center.y = WINDOW_HEIGHT - 685;
	//Plus and Minus
	if (game->howto == 1) {
		for (int i = 1; i < BUTTONS_S; i++) {
			s = &game->sButton[i];
			s->width = 25;
			s->height = 25;
			s->center.x = -50;
		}
	} else {
		for (int i = 1; i < BUTTONS_S; i++) {
			s = &game->sButton[i];
			s->width = 25;
			s->height = 25;
			if (i == 1)
				s->center.x = WINDOW_WIDTH / 2 + 100;
			if (i == 2)
				s->center.x = WINDOW_WIDTH / 2 - 100;
			s->center.y = WINDOW_HEIGHT - 250;
		}
	}
}

void renderSettings(Game *game)
{
	Shape *s;
	//glClearColor(0.15, 0.15, 0.15, 0.15);
	//glClear(GL_COLOR_BUFFER_BIT);
	//Render Settings Menu BG
	s = &game->menuBG;
	float w, h;
	//glBindTexture(GL_TEXTURE_2D, 0);
	glColor3ub(35,35,35);
	glPushMatrix();
	glTranslatef(s->center.x, s->center.y, 0);
	w = s->width;
	h = s->height;
	//Attach texture
	if (game->howto == 1) {
		glBindTexture(GL_TEXTURE_2D, howtoplayTexture);
		w = s->width + 145;
		h = s->height + 160;		
		glColor3ub(255,255,255);
	}
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 1.0f); glVertex2i(-w,-h);
		glTexCoord2f(0.0f, 0.0f); glVertex2i(-w, h);
		glTexCoord2f(1.0f, 0.0f); glVertex2i( w, h);
		glTexCoord2f(1.0f, 1.0f); glVertex2i( w,-h);
	glEnd();
	glPopMatrix();
	glBindTexture(GL_TEXTURE_2D, 0);
//glFlush();
	//Render Settings Buttons	
	int numSButtons = BUTTONS_S;
	if (game->howto == 1)
		numSButtons = 1;
	for (int i = 0; i < numSButtons; i++) {
		s = &game->sButton[i];
		float w, h;
		glColor3ub(205,92,92);
		if (game->mouseOnButton[i] == 1) {
			//Button selected color
			glColor3ub(120,120,120);
		}
		glPushMatrix();
		glTranslatef(s->center.x, s->center.y, 0);
		w = s->width;
		h = s->height;
		glBegin(GL_QUADS);
			glVertex2i(-w,-h);
			glVertex2i(-w, h);
			glVertex2i( w, h);
			glVertex2i( w,-h);
		glEnd();
		glPopMatrix();
		//glFlush();
	}
}

void renderSettingsText(Game *game)
{
	Rect rt;
	int j = 0;
	if (game->howto == 0) {
		rt.bot = WINDOW_HEIGHT - 250 - (j * 100) + 25;
		rt.left = WINDOW_WIDTH / 2;
		//std::cout << rt.bot << " " << rt.left << std::endl;
		rt.center = 1;
		ggprint16(&rt, 16, 0x00ffffff, "Volume");
		rt.bot = WINDOW_HEIGHT - 250 - (j * 100) - 10;
		rt.left = WINDOW_WIDTH / 2 - 100;
		ggprint16(&rt, 16, 0x00ffffff, " - ");
		rt.bot = WINDOW_HEIGHT - 250 - (j * 100) - 10;
		rt.left = WINDOW_WIDTH / 2 + 100;
		ggprint16(&rt, 16, 0x00ffffff, " + ");
		rt.bot = WINDOW_HEIGHT - 250 - (j * 100) - 10;
		rt.left = WINDOW_WIDTH / 2;
		ggprint16(&rt, 16, 0x00ffffff, "%d", game->vVolume);
	}
	j++;
	rt.bot = WINDOW_HEIGHT - 250 - (j * 100) - 345;
	rt.left = WINDOW_WIDTH / 2;
	ggprint16(&rt, 16, 0x00ffffff, "Back");
}

int lvlState(Game *game)
{
	//printf("lvlState()\n");
	if (gameState(game) != 5)
		return -1;
	int rMissiles = 0;
	rMissiles = game->defMissilesRemainingAfterLevel;
	int rCities   = 0;
	//TODO: Attach to correct struct variables
	for (int i = 0; i < CITYNUM; i++) {
		if (game->structures.city[i].alive == 1)
			rCities++;
	}
	//Check for Game Over
	//rMissiles = 0;
	//rCities = 0;
	if (rCities == 0 && rMissiles == 0) {
		//Set state to some unused value
		game->gState = 99;
		//renderGameOver(game);
		return 2;
	}
	game->lvl.aCities = rCities;
	game->lvl.prevMCount = rMissiles;
	return 1;
}

void resetMainGame(Game *game)
{
	game->level = 1;
	game->prevLevel = 1;
	game->score = 0;
	game->inGame = 0;
	game->radarOn = 0;
	game->ufoOn = 0;
	game->mCount = 10;
	//for (int i = 0; i < CITYNUM; i++) {
	//	game->structures.city[i].alive = 1;
	//}
	initStruc(game);
}

void resetLevelEnd(Game *game)
{
	game->lvl.diff = 0;
	game->lvl.cReset = true;
	game->lvl.explMax = false;
	game->lvl.gtime = 0.0;
	game->lvl.rCount = 0;
	game->lvl.cCount = 0;
	game->lvl.start = 0;
	game->lvl.end = 0;
	game->lvl.timer = 0.0;
	game->lvl.alpha = 1.0;
	game->lvl.mDone = 1;
	game->lvl.alertPlayed = 0;
	game->lvl.aCities = 0;
}

void levelEnd(Game *game)
{
	//Variables stored in struct levelInfo
	time_t start = game->lvl.start;
	time_t end   = game->lvl.end;
	double delay = game->lvl.delay;
	float timer = game->lvl.timer;
	int rCount = game->lvl.rCount;
	int rMissiles = game->lvl.prevMCount;
	int cCount = game->lvl.cCount;
	int rCities = game->lvl.aCities;
	double diff = game->lvl.diff;
	double m_delay = game->lvl.m_delay;
	double c_delay = game->lvl.c_delay;
	bool clockReset = game->lvl.cReset;
	bool type = 0;
	Audio *a;
	a = &game->sounds;
	if (rCount == rMissiles && game->lvl.mDone == 1)  {
		clockReset = true;
		sleep(1);
	}
	if (clockReset) {
		//printf("Clock Reset\n");
		time(&start);
		timer = 0.0;
		clockReset = false;
	}
	//Calculate Score
	if (rCount != rMissiles || cCount != rCities || game->lvl.alertPlayed == 0) {
		timer += 0.1;
		if (rCount != rMissiles) {
			if (timer > m_delay) {
				if (rMissiles > 14)
					time(&start);
				++rCount;
				a->playAudio(34);			
				glClear(GL_COLOR_BUFFER_BIT);
				renderBackground(starsTexture);
				renderStruc(game);
				renderBonusA(game, rCount, cCount, type);
				timer = 0.0;
			}
		} else if (rCities == 0) {
			game->gState = 99;
			resetLevelEnd(game);
			return;
		} else if (cCount != rCities) {
			game->lvl.mDone = 0;
			type = 1;
			if (timer > c_delay) {
				++cCount;
				a->playAudio(34);			
				glClear(GL_COLOR_BUFFER_BIT);
				renderBackground(starsTexture);
				renderStruc(game);
				renderBonusA(game, rCount, cCount, type);
				timer = 0.0;
			}
		} else {
			if (game->lvl.alertPlayed == 0 && difftime(end, start) > 2.0) {
				glClear(GL_COLOR_BUFFER_BIT);
				renderBackground(starsTexture);
				renderStruc(game);
				//renderRadar(game);
				renderNewLevelMsg(game);
				renderScores(game);
				game->lvl.alertPlayed = 1;
			}
		}
	}
	time(&end);
	//Reset Game State once delay is reached
	if (difftime(end, start) >= delay+1.0) {
		printf("Level: %d\n", game->level);
		resetLevelEnd(game);
		game->gState = 0;
	} else {
		//Store calculated data
		game->lvl.start = start;
		game->lvl.end = end;
		game->lvl.rCount = rCount;
		game->lvl.cCount = cCount;
		game->lvl.diff = diff;
		game->lvl.cReset = clockReset;
		game->lvl.timer = timer;
	}
}

void gameOver(Game *game)
{
	//resetLevelEnd(game);
	time_t start = game->lvl.start;
	time_t end   = game->lvl.end;
	double counter = game->lvl.diff;
	bool max = game->lvl.explMax;
	Audio *a;
	a = &game->sounds;

	if (game->lvl.cReset) {
		time(&start);
		//Play explosion
		a->playAudio(40);
		counter = 1;
		game->lvl.cReset = false;
	}
	//
	if (counter > 335.0) {
		game->lvl.mDone = 0;
		game->lvl.explMax = 1;
	}
	if (game->lvl.mDone == 1)
		counter += 1.55;
	else
		if (counter >= 0.0)
			counter -= 0.75;
	glClear(GL_COLOR_BUFFER_BIT);
	if (max)
		renderBackground(gameoverTexture);
	renderGameOverExpl(game, counter);
	time(&end);
	if (difftime(end, start) > 8.0) {
		//RESET level, score, cities, etc
		//printf("8 seconds have passed...\n");
		addHighScore(game);
		resetMainGame(game);
		resetLevelEnd(game);
		createEMissiles(game, 0, 0);
		game->gState = 1;
		//highscore
		//reset everything (score, levels, cities, etc...)
		//render exit button?
	} else {
		//Store calculated data
		game->lvl.start = start;
		game->lvl.end = end;
		game->lvl.diff = counter;
	}

}

void renderGameOverExpl(Game *game, double n)
{
	//Draw
	Shape *c;
	bool max = game->lvl.explMax;
	float alpha = game->lvl.alpha;
	if (max) {
		alpha -= 0.0045;
		game->lvl.alpha = alpha;
	}
	//if (!game->expColor) {
	glColor4f(1.0, 0.35, 0.0, alpha);
	game->expColor = true;
	//} else {
	//	glColor4f(1.0, 0.35, 0.0, alpha);
	//	game->expColor = false;
	//}
	c = &game->endExplosion;
	c->radius = 0;
	c->radius += n;
	c->center.x = WINDOW_WIDTH / 2;
	c->center.y = WINDOW_HEIGHT / 2;
	//Render
	//clear
	glPushMatrix();
	glTranslatef(c->center.x, c->center.y, 0);
	float r = c->radius;
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glBegin(GL_TRIANGLE_FAN);
	for (int i = 0; i < 360; i++) {
		float deg2rad = i * (3.14159/180);
		glTexCoord2f(cos(deg2rad)*0.5+0.5, 5.0-sin(deg2rad));
		glVertex3d(cos(deg2rad)*r, sin(deg2rad)*r, -0.5);
	}
	glEnd();
	glPopMatrix();
	glDisable(GL_BLEND);
}

void renderBonusA(Game *game, int rCount, int cCount, bool type)
{
	int n = rCount;
	int m = cCount;
	//printf("%d | %d\n", n, m);
	Rect rt;
	//std::cout << rt.bot << " " << rt.left << std::endl;
	rt.center = 1;
	rt.bot = (WINDOW_HEIGHT / 2) + 200;
	rt.left = (WINDOW_WIDTH / 2);
	ggprint16(&rt, 16, 0x00ffffff, "BONUS   POINTS");
	rt.bot = (WINDOW_HEIGHT / 2) + 100;
	rt.left = (WINDOW_WIDTH / 2) - 100;
	ggprint16(&rt, 16, 0x00ffffff, "%d", n*5);
	if (n > 0)
		game->score += 5;
	//
	Shape *s;	
	//Missiles
	if (n >= 20) {
		n = 20;
	}
	for (int i = 0; i < n; i++) {
		s = &game->BonusA[i];
		s->width = 8;
		s->height = 10;
		s->center.x = (WINDOW_WIDTH / 2) - 50 + i*15;
		s->center.y = (WINDOW_HEIGHT / 2) + 110;
	}
	for (int i = 0; i < n; i++) {
		s = &game->BonusA[i];
		float w, h;
		glColor3f(1.0, 1.0, 1.0);
		glPushMatrix();
		glTranslatef(s->center.x, s->center.y, 0);
		w = s->width;
		h = s->height;
		//Attach texture
		if (game->gfxMode)
			glBindTexture(GL_TEXTURE_2D, dmissileTexture);
		else
			glBindTexture(GL_TEXTURE_2D, c_dmissileTexture);
		//For transparency
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 1.0f); glVertex2i(-w,-h);
			glTexCoord2f(0.0f, 0.0f); glVertex2i(-w, h);
			glTexCoord2f(1.0f, 0.0f); glVertex2i( w, h);
			glTexCoord2f(1.0f, 1.0f); glVertex2i( w,-h);
		glEnd();
		glDisable(GL_BLEND);
		glPopMatrix();
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	//Cities
	if (type) {
		Rect rt;
		rt.bot = (WINDOW_HEIGHT / 2) + 50;
		rt.left = (WINDOW_WIDTH / 2) - 105;
		rt.center = 1;
		ggprint16(&rt, 16, 0x00ffffff, "%d", m*100);
		if (m > 0)
			game->score += 100;
		for (int i = 0; i < m; i++) {
			s = &game->BonusB[i];
			s->width = 24;
			s->height = 12;
			s->center.x = (WINDOW_WIDTH / 2) - 30 + i*60;
			s->center.y = (WINDOW_HEIGHT / 2) + 60;
		}
		for (int i = 0; i < m; i++) {
			s = &game->BonusB[i];
			float w, h;
			glColor3f(1.0, 1.0, 1.0);
			glPushMatrix();
			glTranslatef(s->center.x, s->center.y, 0);
			w = s->width;
			h = s->height;
			//Attach texture
			if (game->gfxMode)
				glBindTexture(GL_TEXTURE_2D, cityTexture);
			else
				glBindTexture(GL_TEXTURE_2D, c_cityTexture);
			//For transparency
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
			glBegin(GL_QUADS);
				glTexCoord2f(0.0f, 1.0f); glVertex2i(-w,-h);
				glTexCoord2f(0.0f, 0.0f); glVertex2i(-w, h);
				glTexCoord2f(1.0f, 0.0f); glVertex2i( w, h);
				glTexCoord2f(1.0f, 1.0f); glVertex2i( w,-h);
			glEnd();
			glDisable(GL_BLEND);
			glPopMatrix();
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}
}

void renderNewLevelMsg(Game *game)
{
	Rect rt;
	rt.bot = (WINDOW_HEIGHT / 2);
	rt.left = (WINDOW_WIDTH / 2);
	time_t start = game->lvl.start;
	time_t end   = game->lvl.end;
	double delay = game->lvl.delay;
	//std::cout << rt.bot << " " << rt.left << std::endl;
	rt.center = 1;
	Audio *a;
	a = &game->sounds;
	if (game->lvl.alertPlayed == 0) {
		a->playAudio(39);
		time(&start);
	}	
	ggprint16(&rt, 16, 0x00ffffff, "%d  X  POINTS", game->level);
	rt.bot = (WINDOW_HEIGHT / 2) + 100;
	ggprint16(&rt, 16, 0x00ffffff, "LEVEL   %d", game->level);
	rt.bot = (WINDOW_HEIGHT / 2) - 100;
	ggprint16(&rt, 16, 0x00ffffff, "DEFEND               CITIES");
	//
	game->lvl.alertPlayed = 1;
	time(&end);
	if (difftime(end, start) >= delay - 2 && gameState(game) == 3) {
		printf("First Level\n");
		game->gState = 0;
		resetLevelEnd(game);
	} else {
		//Store calculated data
		game->lvl.start = start;
		game->lvl.end = end;
	}
}

void mouseOver(int savex, int savey, Game *game)
{
	Shape *s;
	if (gameState(game) == 1) {
		for (int j = 0; j < BUTTONS; j++) {			
			s = &game->mButton[j];
			if (savey >= s->center.y - (s->height) &&
				savey <= s->center.y + (s->height) &&
				savex >= s->center.x - (s->width) &&
				savex <= s->center.x + (s->width)) {
					game->mouseOnButton[j] = 1;
			} else {
				game->mouseOnButton[j] = 0;
			}
		}
	} else if (gameState(game) == 2) {
		for (int j = 0; j < BUTTONS_S; j++) {			
			s = &game->sButton[j];
			if (savey >= s->center.y - (s->height) &&
				savey <= s->center.y + (s->height) &&
				savex >= s->center.x - (s->width) &&
				savex <= s->center.x + (s->width)) {
					game->mouseOnButton[j] = 1;
			} else {
				game->mouseOnButton[j] = 0;
			}
		}
	}
}

void menuClick(Game *game)
{
	if (gameState(game) == 1) {
		//Play Button (3)
		if (game->mouseOnButton[3] == 1 && game->inGame == 0) {
			game->gState = 3;
			game->inGame = 1;
		} else if (game->mouseOnButton[3] == 1 && game->inGame == 1) {
			game->gState = 0;
		}
		//How To Play/Instructions (2)
		if (game->mouseOnButton[2] == 1) {
			game->gState = 2;
			game->howto = 1;
		}
		//Settings Button (1)
		if (game->mouseOnButton[1] == 1) {
			game->gState = 2;
		}
		//Exit Button (0)
		if (game->mouseOnButton[0] == 1) {
			//std::cout << "Quitting..." << std::endl;
			game->menuExit = 1;
		}
	} else if (gameState(game) == 2) {
		float adjust = 0.1;
		int iadjust = 10;
		//Volume -
		if (game->mouseOnButton[2] == 1) {
			if (game->sounds.gVolume > 0.0 && game->sounds.gVolume <= 1.0) {
				game->sounds.gVolume -= adjust;
				game->vVolume -= iadjust;
			}
			//printf("%f\n", game->sounds.gVolume);
		}
		//Volume +
		if (game->mouseOnButton[1] == 1) {
			if (game->sounds.gVolume > -1.0 && game->sounds.gVolume < 1.0) {
				game->sounds.gVolume += adjust;
				game->vVolume += iadjust;
			}
			//printf("%f\n", game->sounds.gVolume);
		}
		//Back
		if (game->mouseOnButton[0] == 1) {
			game->gState = 1;
			game->howto = 0;
		}
	}
}

int gameState(Game *game)
{
	int state = 0;
	int g;
	g = game->gState;
	if (g > state) {
		return g;
	}
	return state;
}

float gameVolume(Game *game)
{
	return game->sounds.gVolume;
}

void classicMode(Game *game)
{
	//Toggle Between Project and Classic Images
	game->gfxMode ^= 1;
	//Unload Audio
	//Reload Audio
}

int isLastCity(Game *game)
{
	int counter = 0;
	int x;
	for (int i = 0; i < CITYNUM; i++) {
		if (game->structures.city[i].alive == 1) {
			counter++;
			if (counter > 1)
				return -1;
			else
				x = i;
		}
	}
	return x;
}

void lastCityMode(int x, int y, Game *game)
{
	//Last Surviving City Moves on X-Axis based on mouse
	int idx;
	idx = isLastCity(game);
	if (idx < 0) {
		printf("There are still others...\n");
		game->lcm ^= 1;
		return;
	} else {
		Shape *c;
		c = &game->structures.city[idx];
		if (c->alive == 0)
			game->lcm ^= 1;
		c->center.x = x;
		//c->center.y = y;
	}
}

void game_credits(Game *game)
{
	Rect rt;
	rt.bot = 20;
	rt.left = WINDOW_WIDTH / 2;
	rt.center = 1;
	ggprint8b(&rt, 16, 0x00ffff00, 
		"Created by: Daniel T, John C, Jose G, "
		"and Jose R - CS335 [Software Engineering] Spring 2016");
}