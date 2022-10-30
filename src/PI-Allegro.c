#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/bitmap.h>
#include <allegro5/color.h>
#include <sodium.h>
#include <string.h>
#include "logger.h"
#include "queue.h"
#include "physics.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define FPS 60
#define windowWidth 1366
#define windowHeight 768
#define worldWidth 1000
#define worldHeight 1000

#define projectileVelocity 0.6
#define projectileAccel 0.25
#define projectileOffset 10
#define projectileMax 3
#define projectileDamage 10
#define targetPracticeLife 20
#define enemyMax 10
#define enemyProjectileVelocity 3
#define enemyProjectileOffset 10
#define enemyProjectileMax 25
#define enemyProjectileDamage 10
#define tileSize 32
#define mapSize 100
#define PI 3.14159265
#define Left 0
#define Right 1
#define gravity 0.275
#define SODIUM_STATIC
#define TILE_COUNT 17

typedef struct path {
	char* val;
	int length;
} Path;

enum path_identifier {
	sheets
};

static const Path assets_path[] = {
	{.val = "assets/images/sheets/", .length = 21},
};

enum {
	antiVirus,
	antiBiotic,
	antiMycotic
};
enum {
	backgroundL1,
	backgroundL2,
	backgroundL3
};
enum {
	neutral,
	shooting
};
enum {
	air,
	chaoce,
	chaoc,
	chaocd,
	chao1wt,
	chaome,
	chaom,
	chaomd,
	chao1wm,
	chaobe,
	chaob,
	chaobd,
	chao1wb,
	chao1he,
	chao1hm,
	chao1hd,
	chaounico
};
enum {
	projectileI,
	enemyI
};
enum {
	contact,
	shooter
};
enum {
	friendly,
	foe
};
enum {
	randpos,
	semirand,
	fixed
};
enum {
	gtile,
	ftile
};
enum {
	none,
	espawn,
	pspawn,
	finish
};

typedef struct posicao {
	int x;
	int y;
} pos;

typedef struct entity {
	FPoint pos;
	int tileX, tileY;
	float width, height;
	float life, maxLife;
	int hbWidth, hbHeight;
	float hbX, hbY;
	int spriteChange;
	int shotFC;
	int rotate;
	int selectedWeapon;
	int attack;
	pos spawnTile;
	int currentDir;
	int coord;

	bool isShooting;
	bool onGround;
	bool hitCeiling;
	bool alive;

	int dir;
	int count;
	float vel_x;
	float vel_y;
	bool immortality;
} entity;

typedef struct projectile {
	FPoint pos;
	float width, height;
	float speed, accel;
	float angle, sin, cos;
	int hbWidth, hbHeight;
	float damage;
	int type;
	int dir;
	int r;
	int g;
	int b;
	int origin;
	bool projectileTravel;
	bool projectileHit;
} projectile;

typedef struct tile {
	bool isSolid;
	int id;
} tile;

typedef struct objective {
	int count;
	int type;
} objective;

struct progresso {
	bool m1;
	bool m2;
} adv;

ALLEGRO_DISPLAY* display = NULL;
ALLEGRO_FONT* font = NULL;
ALLEGRO_TIMER* timer = NULL;
ALLEGRO_SAMPLE* bgm1 = NULL;
ALLEGRO_SAMPLE* bgm2 = NULL;
ALLEGRO_SAMPLE* bgm3 = NULL;
ALLEGRO_SAMPLE* sfx_select = NULL;
ALLEGRO_SAMPLE* shot = NULL;
ALLEGRO_SAMPLE* sfx_jump = NULL;
ALLEGRO_SAMPLE* sfx_sp1 = NULL;
ALLEGRO_SAMPLE* sfx_sp2 = NULL;
ALLEGRO_SAMPLE* sfx_sp3 = NULL;
ALLEGRO_SAMPLE* sfx_hit = NULL;
ALLEGRO_SAMPLE_INSTANCE* sampleInstance = NULL;
ALLEGRO_EVENT_QUEUE* evQueue = NULL;
ALLEGRO_BITMAP* playerShotTemplate[2];
ALLEGRO_BITMAP* enemyShotTemplate;
ALLEGRO_BITMAP* stage[3];
ALLEGRO_BITMAP* enemySprite[3];
ALLEGRO_BITMAP* enemysheet;
ALLEGRO_BITMAP* enemyShooterSprite;
ALLEGRO_BITMAP* tileAtlas;
ALLEGRO_BITMAP* playersheet;
ALLEGRO_BITMAP* titulo;
ALLEGRO_BITMAP* btnf1;
ALLEGRO_BITMAP* btnf2;
ALLEGRO_BITMAP* btnle;
ALLEGRO_BITMAP* btnf1S;
ALLEGRO_BITMAP* btnf2S;
ALLEGRO_BITMAP* btnleS;
ALLEGRO_BITMAP* hist;
ALLEGRO_BITMAP* control;
ALLEGRO_BITMAP* antiv;
ALLEGRO_BITMAP* antib;
ALLEGRO_BITMAP* antim;
ALLEGRO_FILE* txtmap;
ALLEGRO_TRANSFORM camera;
FILE* tm;
entity player;
projectile playerShot[projectileMax];
projectile playerTripleShot[projectileMax];
entity enemy[enemyMax];
projectile enemyShot[enemyProjectileMax];
entity enemyShooter;
tile tiles[TILE_COUNT];
objective killCount;
objective endurance;
pos mouse;
pos editorClick;
pos currentTile;

void init_component(int test, const char *description)
{
    if(test) return;

	logger_log(LOG_ERROR, "couldn't initialize %s\n", description);
    exit(EXIT_FAILURE);
}

int initialize() {
	logger_log(LOG_DEBUG, "Initializing sodium");
	init_component(sodium_init() >= 0, "sodium");
	logger_log(LOG_DEBUG, "Initializing allegro ");
	init_component(al_init(), "allegro core");
	logger_log(LOG_DEBUG, "Initializing allegro_image ");
	init_component(al_init_image_addon(), "allegro_image");
	logger_log(LOG_DEBUG, "Initializing allegro_audio ");
	init_component(al_install_audio(), "allegro_audio");
	logger_log(LOG_DEBUG, "Initializing allegro_acodec ");
	init_component(al_init_acodec_addon(), "allegro_acodec");
	logger_log(LOG_DEBUG, "Initializing allegro_font ");
	init_component(al_init_font_addon(), "allegro_font");
	logger_log(LOG_DEBUG, "Initializing allegro_ttf ");
	init_component(al_init_ttf_addon(), "allegro_ttf");
	logger_log(LOG_DEBUG, "Initializing allegro_primitives ");
	init_component(al_init_primitives_addon(), "allegro_primitives");
	
	al_get_default_mixer();

	logger_log(LOG_DEBUG, "Set draw timer to %d fps", FPS);
	timer = al_create_timer(1.0 / FPS);
	al_set_new_display_flags(ALLEGRO_RESIZABLE);
	display = al_create_display(windowWidth, windowHeight);
	evQueue = al_create_event_queue();
	font = al_load_font("assets/fonts/metal-slug.ttf", 13, 0);
	al_set_window_title(display, "Inside Wars");
	logger_log(LOG_DEBUG, "Set window (Inside Wars) %dx%d; props: Rezisable", windowWidth, windowHeight);
	al_reserve_samples(4);

	logger_log(LOG_INFO, "Loading audio assets...");
	sfx_sp1 = al_load_sample("assets/audio/spawn.wav");
	shot = al_load_sample("assets/audio/tiro.ogg");
	sfx_jump = al_load_sample("assets/audio/jump.wav");
	bgm1 = al_load_sample("assets/audio/menu_bg.ogg");
	bgm2 = al_load_sample("assets/audio/fase1.ogg");
	bgm3 = al_load_sample("assets/audio/fase2.ogg");
	sfx_select = al_load_sample("assets/audio/select.ogg");
	playerShotTemplate[0] = al_load_bitmap("assets/images/tiro.png");
	playerShotTemplate[1] = al_load_bitmap("assets/images/tiro2.png");
	al_convert_mask_to_alpha(playerShotTemplate[0], al_map_rgb(255, 0, 255));
	al_convert_mask_to_alpha(playerShotTemplate[1], al_map_rgb(255, 0, 255));

	logger_log(LOG_DEBUG, "Setting up keyboard and mouse");
	al_install_keyboard();
	al_install_mouse();
	logger_log(LOG_DEBUG, "Setting up event queue");
	al_register_event_source(evQueue, al_get_display_event_source(display));
	al_register_event_source(evQueue, al_get_timer_event_source(timer));

	al_register_event_source(evQueue, al_get_mouse_event_source());
	al_register_event_source(evQueue, al_get_keyboard_event_source());

	al_start_timer(timer);

	return 0;
}

ALLEGRO_COLOR get_weapon_color(int sel_weapon) {
	ALLEGRO_COLOR c;
	c.a = 255;
	switch (sel_weapon) {
	case antiVirus:
		c.r = 34;
		c.g = 236;
		c.b = 39;
		break;
	case antiBiotic:
		c.r = 0;
		c.g = 162;
		c.b = 232;
		break;
	case antiMycotic:
		c.r = 226;
		c.g = 18;
		c.b = 29;
		break;
	}
	return c;
}

void setProjectileColor(projectile* a) {
	switch (a->type) {
	case antiVirus:
		a->r = 34;
		a->g = 236;
		a->b = 39;
		break;
	case antiBiotic:
		a->r = 0;
		a->g = 162;
		a->b = 232;
		break;
	case antiMycotic:
		a->r = 226;
		a->g = 18;
		a->b = 29;
		break;
	}
}

void enemyRandomizer(entity* e, int stage) {
	int section;
	int lifeRandomizer = randombytes_uniform(16);

	section = randombytes_uniform(2);

	if (stage == 2) {
		if (section == 1) {
			e->pos.x = player.pos.x + (100 + randombytes_uniform(201));
		}
		else {
			e->pos.x = player.pos.x - (100 + randombytes_uniform(201));
		}
		e->pos.y = player.pos.y + randombytes_uniform(101);
	}

	e->selectedWeapon = randombytes_uniform(3);

	e->life = targetPracticeLife + lifeRandomizer;
	e->maxLife = targetPracticeLife + lifeRandomizer;

	e->alive = true;
}

int initplayer(entity* c, ALLEGRO_BITMAP* player, int* eSTCount, int*** m) {
	int aux = 0, i, j;
	player = al_load_bitmap("assets/images/playersheet.png");

	for (i = 0; i < mapSize; i++) {
		for (j = 0; j < mapSize; j++) {
			if (m[ftile][i][j] == pspawn) {
				c->pos.x = j * tileSize;
				c->pos.y = i * tileSize;
			}
			if (m[ftile][i][j] == espawn) {
				*eSTCount += 1;
			}
		}
	}

	c->width = al_get_bitmap_width(player) / 2;
	aux = c->width / tileSize;
	c->hbWidth = aux * tileSize;
	c->height = al_get_bitmap_height(player) / 2;
	aux = c->height / tileSize;
	c->hbHeight = aux * tileSize;
	c->vel_x = 4.5;
	c->dir = 5;
	c->currentDir = Right;
	c->selectedWeapon = 0;
	c->onGround = false;
	c->hitCeiling = false;
	c->alive = true;
	c->immortality = false;
	c->life = 30;

	return 0;
}

int initenemy(entity* e, ALLEGRO_BITMAP* enemy[], int type, int stage) {
	int aux = 0;

	e->attack = type;

	e->shotFC = 0;
	enemyRandomizer(e, stage);

	e->vel_x = 0;
	e->vel_y = 0;
	e->rotate = 0;

	for (aux = 0; aux < 3; aux++) {
		if (aux == e->selectedWeapon) {
			e->width = al_get_bitmap_width(enemy[aux]);
			e->height = al_get_bitmap_height(enemy[aux]);
			break;
		}
	}

	aux = e->width / tileSize;
	e->hbWidth = aux * tileSize;
	aux = e->height / tileSize;
	e->hbHeight = aux * tileSize;

	return 0;
}

/*pos setEnemySpawn(int eSTCount, int*** m) {
	int i, j;

	pos* eSP = malloc(eSTCount * sizeof(pos));;

	for (i = 0; i < mapSize; i++) {
		for (j = 0; j < mapSize; j++) {
			if (m[ftile][i][j] == espawn) {

			}
		}
	}

	return eSP;
}*/

void pShoot(projectile* p, entity* c) {
	p->speed = projectileVelocity;
	p->accel = projectileAccel;

	p->width = 46;
	p->height = 22;

	p->hbWidth = 46;
	p->hbHeight = 22;

	p->origin = friendly;

	p->angle = randombytes_uniform(2);

	p->dir = c->currentDir;

	p->pos.x = c->pos.x + (p->dir == Right ? c->hbWidth : - p->hbWidth);
	p->pos.y = c->pos.y + projectileOffset;

	p->damage = projectileDamage;
	p->type = c->selectedWeapon;

	c->isShooting = true;
	p->projectileTravel = true;
}


void eShoot(projectile* p, entity* e, entity* c, int fc) {
	e->shotFC = fc;
	p->speed = enemyProjectileVelocity;

	p->width = 20;
	p->height = 20;

	p->hbWidth = 20;
	p->hbHeight = 20;

	p->origin = foe;

	p->dir = e->currentDir;

	p->angle = atan2((double)(-1.0 * (e->hbY + e->hbHeight / 2.0)) - (-1.0 * (c->hbY + c->hbHeight / 2.0)), (double)c->hbX + c->hbWidth / 2 - e->hbX + e->hbWidth);
	p->cos = cos(p->angle);
	p->sin = sin(p->angle);

	p->pos.x = e->pos.x;
	p->pos.y = e->pos.y;

	p->damage = 10;
	p->type = 10;

	p->projectileTravel = true;
}

void refreshProjectileState(projectile p[], projectile e[], entity c, int* cpCount, int* epCount, int cx, int cy) {
	int j;

	for (j = 0; j < projectileMax; j++) {
		if (p[j].projectileTravel) {
			if (p[j].pos.x < cx + al_get_display_width(display) && p[j].pos.x > cx&& p[j].pos.y < cy + al_get_display_height(display) && p[j].pos.y > cy) {
				if (p[j].dir == Right)
					p[j].pos.x += p[j].speed;
				else
					p[j].pos.x -= p[j].speed;
			}
			else {
				p[j].projectileTravel = false;
				*cpCount -= 1;
			}
			if (p[j].speed <= 20) {
				p[j].speed += p[j].accel;
			}
		}
	}

	for (j = 0; j < enemyProjectileMax; j++) {
		if (e[j].projectileTravel) {
			if (e[j].pos.x < cx + al_get_display_width(display) && e[j].pos.x > cx&& e[j].pos.y < cy + al_get_display_height(display) && e[j].pos.y > cy) {
				if (e[j].dir == Right) {
					e[j].pos.x += e[j].speed * -e[j].cos;
				}
				else {
					e[j].pos.x += e[j].speed * e[j].cos;
				}
				e[j].pos.y += e[j].speed * e[j].sin;
			}
			else {
				e[j].projectileTravel = false;
				*epCount -= 1;
			}
		}
	}
}

void refreshPlayerMovement(entity* p, tile t[], int*** m) {
	int botTile, btID, upTile, upID, ltTile, ltID, rtTile, rtID, ctID;

	p->hbX = (p->pos.x + (p->width / 2)) - p->hbWidth / 2;
	p->hbY = (p->pos.y + (p->height - p->hbHeight));

	FPoint p0 = {.x = p->pos.x, .y = p->pos.y};

	p->tileX = p->hbX / tileSize;
	p->tileY = p->hbY / tileSize;

	botTile = p->tileY + (p->hbHeight / tileSize);
	btID = m[gtile][botTile][p->tileX];

	upTile = (p->hbY - 1) / tileSize;
	upID = m[gtile][upTile][p->tileX];

	ltTile = (p->hbX - 1) / tileSize;
	ltID = m[gtile][p->tileY][ltTile];

	rtTile = (p->hbX + p->hbWidth + 1) / tileSize;
	rtID = m[gtile][p->tileY][rtTile];

	switch (p->dir) {
	case Right:
		if (!t[rtID].isSolid) {
			p->pos.x += p->vel_x;
		}
		/*else {
			player.pos.x = (rtTile - 1) * tileSize;
		}*/
		break;
	case Left:
		if (!t[ltID].isSolid) {
			p->pos.x -= p->vel_x;
		}
		if (t[m[gtile][p->tileY][(int)p->pos.x / tileSize]].isSolid) {
			p->pos.x = p0.x;
		}
		break;
	}

	if (t[upID].isSolid) {
		p->vel_y = 1;
	}

	if (t[btID].isSolid && p->vel_y >= 0 && p->pos.y >= botTile - tileSize) {
		p->onGround = true;
		p->hitCeiling = false;
		p->vel_y = 0;
		p->hbY -= (int)p->hbY % (tileSize * (p->hbHeight / tileSize));
		p->pos.y = p->hbY - (p->height - p->hbHeight);
	}
	else {
		p->onGround = false;
		p->vel_y += gravity;
		p->pos.y += p->vel_y;
	}
}
//if (t[btID].isSolid && p->vel_y >= 0) {
//	p->onGround = true;
//	p->vel_y = 0;
//	p->pos.y -= ((int)p->hbY % tileSize);
//}
//else {
//	p->onGround = false;
//	p->vel_y += gravity;
//	p->pos.y += p->vel_y;
//}
void refreshPlayerMovement2(entity* p, tile t[], int*** m) {
	int botTile, btID, upTile, upID, ltTile, ltID, rtTile, rtID, ctID;

	p->hbX = (p->pos.x + (p->width / 2)) - p->hbWidth / 2;
	p->hbY = (p->pos.y + (p->height - p->hbHeight));


	FPoint p0 = {.x = p->pos.x, .y = p->pos.y};

	p->tileX = p->hbX / tileSize;
	p->tileY = p->hbY / tileSize;

	botTile = p->tileY + (p->hbHeight / tileSize);
	btID = m[gtile][botTile][p->tileX];

	upTile = p->tileY;
	upID = m[gtile][upTile][p->tileX];

	ltTile = p->tileX;
	ltID = m[gtile][p->tileY][ltTile];

	rtTile = p->tileX + (p->hbWidth / tileSize);
	rtID = m[gtile][p->tileY][rtTile];

	switch (p->dir) {
	case Right:
		if (!t[rtID].isSolid) {
			player.pos.x += player.vel_x;
		}
		break;
	case Left:
		if (!t[ltID].isSolid) {
			currentTile.x = (player.pos.x - player.vel_x) / tileSize;
			currentTile.y = player.pos.y / tileSize;
			ctID = m[gtile][currentTile.y][currentTile.x];
			if (!t[ctID].isSolid) {
				player.pos.x -= player.vel_x;
			}
		}
		break;
	}

	if (t[upID].isSolid && p->pos.y >= upTile + tileSize && !p->hitCeiling && p->vel_y < 0) {
		p->vel_y = 1;
		p->hitCeiling = true;
	}

	if (t[btID].isSolid && p->vel_y >= 0 && p->pos.y >= botTile - tileSize) {
		p->onGround = true;
		p->hitCeiling = false;
		p->vel_y = 0;
		p->hbY -= (int)p->hbY % (tileSize * (p->hbHeight / tileSize));
		p->pos.y = p->hbY - (p->height - p->hbHeight);
	}
	else {
		p->onGround = false;
		p->vel_y += gravity;
		p->pos.y += p->vel_y;
	}
}

void resetEnemy(entity e[], projectile p[]) {
	int i;

	for (i = 0; i < enemyProjectileMax; i++) {
		p[i].projectileTravel = false;
	}
	
	for (i = 0; i < enemyMax; i++) {
		e[i].alive = false;
		e[i].pos.x = 0;
	}
}

void refreshEnemyMovement(entity* e, entity* p) {
	e->hbX = (e->pos.x + (e->width / 2)) - e->hbWidth / 2;
	e->hbY = (e->pos.y + (e->height - e->hbHeight));

	if (e->attack == contact) {
		if (e->pos.x != p->hbX && e->alive) {
			if (e->pos.x > p->hbX) {
				if (e->vel_x >= -1.8) {
					e->vel_x -= 0.2;
				}
			}
			else {
				if (e->vel_x <= 1.8) {
					e->vel_x += 0.2;
				}
			}
			e->pos.x += e->vel_x;
		}

		if (e->pos.y != p->hbY && e->alive) {
			if (e->pos.y > p->hbY) {
				if (e->vel_y >= -1.8) {
					e->vel_y -= 0.2;
				}
			}
			else {
				if (e->vel_y <= 1.8) {
					e->vel_y += 0.2;
				}
			}
			e->pos.y += e->vel_y; 
		}
	}
}

void refreshCamera(float* cx, float* cy, entity p) {
	*cx = (player.pos.x + player.width / 2) - al_get_display_width(display) / 2;
	*cy = (player.pos.y + player.height / 2) - al_get_display_height(display) / 2;

	if (*cx < 0) {
		*cx = 0;
	}
	if (*cy < 0) {
		*cy = 0;
	}
}

void hitboxDetection(projectile* a, entity e[], entity* p, objective* kc, int* hitI, int* pCount, int* iFC, int* edFC, int* fc, bool* sc) {
	float xAxisPivotA, yAxisPivotA, xAxisPivotB, yAxisPivotB, rightA, leftA, downA, upA, rightB, leftB, downB, upB;
	int i, j;
	if (a[0].origin == friendly) {
		for (j = 0; j < enemyMax; j++) {
			for (i = 0; i < projectileMax; i++) {
				if (a[i].projectileTravel) {
					xAxisPivotA = a[i].pos.x + a[i].width / 2;
					yAxisPivotA = a[i].pos.y + a[i].height / 2;
					xAxisPivotB = e[j].pos.x + e[j].width / 2;
					yAxisPivotB = e[j].pos.y + e[j].height / 2;

					rightA = xAxisPivotA + a[i].hbWidth / 2;
					leftA = xAxisPivotA - a[i].hbWidth / 2;
					downA = yAxisPivotA + a[i].hbHeight / 2;
					upA = yAxisPivotA - a[i].hbHeight / 2;

					rightB = xAxisPivotB + e[j].hbWidth / 2;
					leftB = xAxisPivotB - e[j].hbWidth / 2;
					downB = yAxisPivotB + e[j].hbHeight / 2;
					upB = yAxisPivotB - e[j].hbHeight / 2;

					if ((rightA > leftB&& rightA < rightB) || (leftA > leftB&& leftA < rightB)) {
						if ((upA < downB && upA > upB) || (downA > upB&& downA < downB)) {
							a[i].projectileTravel = false;
							a[i].pos.x = 0;
							a[i].pos.y = 0;

							if (e[j].selectedWeapon == playerShot[i].type) {
								e[j].life -= playerShot[i].damage + playerShot[i].damage * (randombytes_uniform(16) / 100.0);
							}

							if (e[j].life <= 0) {
								e[j].alive = false;
								e[j].pos.y = 0;
								e[j].vel_x = 0;
								e[j].vel_y = 0;
								e[j].life = 0;
								sc[e[j].coord] = false;
								edFC[j] = *fc;
								if (e[j].attack == kc->type) {
									kc->count -= 1;
								}
							}
							*pCount -= 1;
							hitI[projectileI] = i;
							hitI[enemyI] = j;
						}
					}
				}
			}
		}
	}
	else {
		for (i = 0; i < enemyProjectileMax; i++) {
			if (a[i].projectileTravel) {
				xAxisPivotA = a[i].pos.x + a[i].width / 2;
				yAxisPivotA = a[i].pos.y + a[i].height / 2;
				xAxisPivotB = p->pos.x + p->width / 2;
				yAxisPivotB = p->pos.y + p->height / 2;

				rightA = xAxisPivotA + a[i].hbWidth / 2;
				leftA = xAxisPivotA - a[i].hbWidth / 2;
				downA = yAxisPivotA + a[i].hbHeight / 2;
				upA = yAxisPivotA - a[i].hbHeight / 2;

				rightB = xAxisPivotB + p->hbWidth / 2;
				leftB = xAxisPivotB - p->hbWidth / 2;
				downB = yAxisPivotB + p->hbHeight / 2;
				upB = yAxisPivotB - p->hbHeight / 2;
				if (!p->immortality) {
					if ((rightA > leftB&& rightA < rightB) || (leftA > leftB&& leftA < rightB)) {
						if ((upA < downB && upA > upB) || (downA > upB&& downA < downB)) {
							a[i].projectileTravel = false;
							a[i].pos.x = 0;
							a[i].pos.y = 0;

							*iFC = *fc;
							p->life -= a[i].damage;
							p->immortality = true;
						}
					}
				}
			}
		}
	}
}

void colisionDetection(entity* e, entity* p, int* iFC, int* fc) {
	float xAxisPivotP, yAxisPivotP, xAxisPivotE, yAxisPivotE, rightP, leftP, downP, upP, rightE, leftE, downE, upE;
	int i, j;

	for (j = 0; j < enemyMax; j++) {
		for (i = 0; i < projectileMax; i++) {

			xAxisPivotP = p[i].pos.x + p[i].width / 2;
			yAxisPivotP = p[i].pos.y + p[i].height / 2;
			xAxisPivotE = e[j].pos.x + e[j].width / 2;
			yAxisPivotE = e[j].pos.y + e[j].height / 2;

			rightP = xAxisPivotP + p[i].hbWidth / 2;
			leftP = xAxisPivotP - p[i].hbWidth / 2;
			downP = yAxisPivotP + p[i].hbHeight / 2;
			upP = yAxisPivotP - p[i].hbHeight / 2;

			rightE = xAxisPivotE + e[j].hbWidth / 2;
			leftE = xAxisPivotE - e[j].hbWidth / 2;
			downE = yAxisPivotE + e[j].hbHeight / 2;
			upE = yAxisPivotE - e[j].hbHeight / 2;

			if (!p->immortality) {
				if ((rightP > leftE&& rightP < rightE) || (leftP > leftE&& leftP < rightE)) {
					if ((upP < downE && upP > upE) || (downP > upE&& downP < downE)) {

						*iFC = *fc;

						for (i = 0; i > -8; i--) {
							e[j].vel_y--;
						}

						p->life -= projectileDamage;
						p->immortality = true;
					}
				}
			}
		}
	}
}

void exitGame(ALLEGRO_EVENT ev, bool* loop, bool* exit_control) {
	switch (ev.type) {
	case ALLEGRO_EVENT_DISPLAY_CLOSE:
		*loop = false;
		*exit_control = true;
		break;
	case ALLEGRO_EVENT_KEY_DOWN:
		if (ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
			*loop = false;
			*exit_control = true;
		}
		break;
	}
}

void set_tiles() {
	int i = 0;

	for (; i < TILE_COUNT; i++) {
		tiles[i].isSolid = i != air;
		tiles[i].id = i;
	}
	
}

void load_atlas(ALLEGRO_BITMAP* atlas, char* filename) {
	Path file = {.val = filename, .length = strlen(filename)};
	Path path = assets_path[sheets];
	char* fullname = malloc((file.length + path.length + 1) * sizeof(char));
	strncat(fullname, path.val, path.length);
	strncat(fullname, file.val, file.length);
	atlas = al_load_bitmap(fullname);
	al_convert_mask_to_alpha(atlas, al_map_rgb(255, 0, 255));
}

int main() {
	logger_set_output_level(LOG_DEBUG);

	int entcount = 0, spawntimeout = 0, i, j, k, l = 0, projectileCount = 0, stageSelect = 1, enemyProjectileCount = 0, enemySpawnTileCount = 0, enemyDmgGauge = 0, hit = 0, hitI[2] = { 0, 0 }, hitII = 0, tilefunc = 0, frameCount = 0, immortalityFC = 0, enemyDeadFC[enemyMax] = { 0, 0 }, runCycle = 0, spawn;
	int *eSTx, *eSTy;
	int*** tileset = NULL;
	float cx = 0, cy = 0;
	char mousePos[25] = "", debugTest[6] = "debug", enemyLifeGauge[5], ptx[8], pty[8], objText[25];
	bool gameLoop = false, menuLoop = true, stageLoop = false, toggleStartText = false, exit_control = false, devMode = false, modDown = false, levelEditor = false, exitStage = false;
	bool* checkspawn;
	queue devChecker;

	initialize();
	logger_log(LOG_INFO, "Finished initializing core systems");
	
	queue_init(&devChecker, 5);

	set_tiles();

	logger_log(LOG_DEBUG, "Loading stage images");

	stage[backgroundL1] = al_load_bitmap("assets/images/backgroundL1.png");
	stage[backgroundL2] = al_load_bitmap("assets/images/backgroundL2.png");
	al_convert_mask_to_alpha(stage[backgroundL2], al_map_rgb(255, 0, 255));
	stage[backgroundL3] = al_load_bitmap("assets/images/backgroundL3.png");
	al_convert_mask_to_alpha(stage[backgroundL3], al_map_rgb(255, 0, 255));

	logger_log(LOG_DEBUG, "Loading tilesheets");
	load_atlas(tileAtlas, "tilesheet.png");
	load_atlas(playersheet, "playersheet.png");
	load_atlas(enemysheet, "enemysheet.png");

	logger_log(LOG_DEBUG, "Loading menu assets");
	titulo = al_load_bitmap("assets/images/Menu/title.png");
	btnf1 = al_load_bitmap("assets/images/Menu/Nselect/lvl1.png");
	btnf2 = al_load_bitmap("assets/images/Menu/Nselect/lvl2.png");
	btnle = al_load_bitmap("assets/images/Menu/Nselect/editor.png");
	btnf1S = al_load_bitmap("assets/images/Menu/Select/lvl1_select.png");
	btnf2S = al_load_bitmap("assets/images/Menu/Select/lvl2_select.png");
	btnleS = al_load_bitmap("assets/images/Menu/Select/editor_select.png");
	hist = al_load_bitmap("assets/images/Menu/TelasProntas/TelaContexto.png");
	control = al_load_bitmap("assets/images/Menu/TelasProntas/TelaControle.png");
	antiv = al_load_bitmap("assets/images/Menu/TelasProntas/antiv.png");
	antib = al_load_bitmap("assets/images/Menu/TelasProntas/antib.png");
	antim = al_load_bitmap("assets/images/Menu/TelasProntas/antim.png");

	logger_log(LOG_DEBUG, "Loading bacteria sprites...");
	enemySprite[antiBiotic] = al_load_bitmap("assets/images/bacteria.png");
	logger_log(LOG_DEBUG, "Getting pixel");
	ALLEGRO_COLOR c = al_get_pixel(enemySprite[antiBiotic], 0, 0);
	fprintf(stderr, "r %.2f g %.2f b %.2f \n", c.r, c.g, c.b);
	logger_log(LOG_DEBUG, "Masking bacteria sprites...");
	al_convert_mask_to_alpha(enemySprite[antiBiotic], al_map_rgb(255, 0, 255));

	logger_log(LOG_DEBUG, "Loading fungi sprites...");
	enemySprite[antiMycotic] = al_load_bitmap("assets/images/fungo.png");
	logger_log(LOG_DEBUG, "Masking fungi sprites...");
	al_convert_mask_to_alpha(enemySprite[antiMycotic], al_map_rgb(255, 0, 255));

	enemySprite[antiVirus] = al_load_bitmap("assets/images/virus.png");
	al_convert_mask_to_alpha(enemySprite[antiVirus], al_map_rgb(255, 0, 255));

	al_clear_to_color(al_map_rgb(255, 255, 255));
	al_play_sample(bgm1, 1, 0.0, 1.0, ALLEGRO_PLAYMODE_LOOP, NULL);
	while (!exit_control) {
		if (exitStage)	{
			al_stop_samples();
			al_play_sample(bgm1, 1, 0.0, 1.0, ALLEGRO_PLAYMODE_LOOP, NULL);
			resetEnemy(&enemy, &enemyShot);
			exitStage = !exitStage;
			spawntimeout = 0;
		}

		while (menuLoop) {
			ALLEGRO_EVENT event;
			al_wait_for_event(evQueue, &event);

			if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
				if (event.keyboard.keycode == ALLEGRO_KEY_ENTER) {
					entcount++;

					if (entcount == 2) {
						stageLoop = true;
						menuLoop = false;
					}
				}
			}

			al_clear_to_color(al_map_rgb(0, 0, 0));

			if (entcount == 0 ) {
				al_draw_bitmap(titulo, (al_get_display_width(display) / 2) - al_get_bitmap_width(titulo) / 2, al_get_display_height(display) / 5, 0);
				al_draw_text(font, al_map_rgb(255, 255, 255), (al_get_display_width(display) / 2), 2 * (al_get_display_height(display) / 5), ALLEGRO_ALIGN_CENTER, "Aperte Enter");
			}
			if (entcount == 1) {
				//al_draw_bitmap(hist, (al_get_display_width(display) / 2) - al_get_bitmap_width(hist) / 2, al_get_display_height(display) / 2 - al_get_bitmap_width(hist) / 2, 0);
				al_draw_bitmap(hist,  120, 0, 0);
			}
			al_flip_display();
		}

		while (stageLoop) {
			ALLEGRO_EVENT event;
			al_wait_for_event(evQueue, &event);

			if (event.type == ALLEGRO_EVENT_KEY_CHAR) {

				if (queue_is_full(&devChecker)) {
					queue_dequeue(&devChecker);
				}					
				queue_enqueue(&devChecker, event.keyboard.unichar);
				

				i = 0;
				while (devChecker.queue[(devChecker.start + i) % devChecker.capacity] == debugTest[i]) {
					i++;
				}
				devMode = i == 4;
			}
			if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
				switch (event.keyboard.keycode) {
				case ALLEGRO_KEY_LSHIFT:
				case ALLEGRO_KEY_RSHIFT:
					modDown = true;
					break;
				case ALLEGRO_KEY_ENTER:
					switch (stageSelect) {
					case 0:
						levelEditor = !levelEditor;
						break;
					case 1:
						spawn = semirand;
						killCount.count = 10;
						killCount.type = shooter;
						stageLoop = false;
						if (!levelEditor) {
							gameLoop = true;	
						}
						break;
					case 2:
						spawn = randpos;
						endurance.count = 2400;
						stageLoop = false;
						if (!levelEditor) {
							gameLoop = true;
						}
						break;
					}
					break;
				case ALLEGRO_KEY_UP:
					stageSelect--;
					if (stageSelect < 0) {
						stageSelect = 2;
					}
					break;
				case ALLEGRO_KEY_DOWN:
					stageSelect++;
					if (stageSelect > 2) {
						stageSelect = 0;
					}
					break;
				}
			}

			if (event.type == ALLEGRO_EVENT_KEY_UP) {
				if (event.keyboard.keycode == ALLEGRO_KEY_LSHIFT || event.keyboard.keycode == ALLEGRO_KEY_RSHIFT) {
					modDown = false;
				}
			}

			if (event.type == ALLEGRO_EVENT_TIMER) {
				frameCount++;
			}
			exitGame(event, &stageLoop, &exit_control);

			if (event.type == ALLEGRO_EVENT_DISPLAY_RESIZE) {
				al_acknowledge_resize(display);
			}

			if (al_is_event_queue_empty(evQueue)) {
				al_clear_to_color(al_map_rgb(0, 0, 0));
				
				al_draw_bitmap(titulo, (al_get_display_width(display) / 2) - al_get_bitmap_width(titulo) / 2, al_get_display_height(display) / 5, 0);

				/*if (devMode) {
					al_draw_text(font, al_map_rgb(255, 255, 255), windowWidth / 2, 0, ALLEGRO_ALIGN_CENTER, "Dev mode!!");
				}*/

				switch (stageSelect) {
				case 0:
					al_draw_bitmap(btnleS, (al_get_display_width(display) / 2) - al_get_bitmap_width(btnleS) / 2, 2 * (al_get_display_height(display) / 5), 0);
					if (!adv.m1) {
						al_draw_bitmap(btnf1, (al_get_display_width(display) / 2) - al_get_bitmap_width(btnf1) / 2, 3 * (al_get_display_height(display) / 5), 0);
					}
					else {
						al_draw_tinted_bitmap(btnf1, al_map_rgb(0, 255, 0), (al_get_display_width(display) / 2) - al_get_bitmap_width(btnf1) / 2, 3 * (al_get_display_height(display) / 5), 0);
					}
					if (!adv.m2) {
						al_draw_bitmap(btnf2, (al_get_display_width(display) / 2) - al_get_bitmap_width(btnf2) / 2, 4 * (al_get_display_height(display) / 5), 0);
					}
					else {
						al_draw_tinted_bitmap(btnf2, al_map_rgb(0, 255, 0), (al_get_display_width(display) / 2) - al_get_bitmap_width(btnf2) / 2, 3 * (al_get_display_height(display) / 5), 0);
					}
					//al_draw_text(font, al_map_rgb(255, 255, 255), (al_get_display_width(display) / 2) - al_get_bitmap_width(btnf2S) / 2, 2 * (al_get_display_height(display) / 5), 0, "->");
					break;
				case 1:
					if (!levelEditor) {
						al_draw_bitmap(btnle, (al_get_display_width(display) / 2) - al_get_bitmap_width(btnle) / 2, 2 * (al_get_display_height(display) / 5), 0);
					}
					else {
						al_draw_tinted_bitmap(btnle, al_map_rgb(255, 200, 0), (al_get_display_width(display) / 2) - al_get_bitmap_width(btnle) / 2, 2 * (al_get_display_height(display) / 5), 0);
					}
					if (!adv.m1) {
						al_draw_bitmap(btnf1S, (al_get_display_width(display) / 2) - al_get_bitmap_width(btnf1S) / 2, 3 * (al_get_display_height(display) / 5), 0);
					}
					else {
						al_draw_tinted_bitmap(btnf1, al_map_rgb(0, 255, 0), (al_get_display_width(display) / 2) - al_get_bitmap_width(btnf1) / 2, 3 * (al_get_display_height(display) / 5), 0);
					}
					if (!adv.m2) {
						al_draw_bitmap(btnf2, (al_get_display_width(display) / 2) - al_get_bitmap_width(btnf2) / 2, 4 * (al_get_display_height(display) / 5), 0);
					}
					else {
						al_draw_tinted_bitmap(btnf2, al_map_rgb(0, 255, 0), (al_get_display_width(display) / 2) - al_get_bitmap_width(btnf2) / 2, 3 * (al_get_display_height(display) / 5), 0);
					}
					//al_draw_text(font, al_map_rgb(255, 255, 255), (al_get_display_width(display) / 2) - al_get_bitmap_width(btnf2S) / 2, 3 * (al_get_display_height(display) / 5), 0, "->");
					break;
				case 2:
					if (!levelEditor) {
						al_draw_bitmap(btnle, (al_get_display_width(display) / 2) - al_get_bitmap_width(btnle) / 2, 2 * (al_get_display_height(display) / 5), 0);
					}
					else {
						al_draw_tinted_bitmap(btnle, al_map_rgb(255, 200, 0), (al_get_display_width(display) / 2) - al_get_bitmap_width(btnle) / 2, 2 * (al_get_display_height(display) / 5), 0);
					}
					if (!adv.m1) {
						al_draw_bitmap(btnf1, (al_get_display_width(display) / 2) - al_get_bitmap_width(btnf1) / 2, 3 * (al_get_display_height(display) / 5), 0);
					}
					else {
						al_draw_tinted_bitmap(btnf1, al_map_rgb(0, 255, 0), (al_get_display_width(display) / 2) - al_get_bitmap_width(btnf1) / 2, 3 * (al_get_display_height(display) / 5), 0);
					}
					if (!adv.m2) {
						al_draw_bitmap(btnf2S, (al_get_display_width(display) / 2) - al_get_bitmap_width(btnf2S) / 2, 4 * (al_get_display_height(display) / 5), 0);
					}
					else {
						al_draw_tinted_bitmap(btnf2, al_map_rgb(0, 255, 0), (al_get_display_width(display) / 2) - al_get_bitmap_width(btnf2) / 2, 3 * (al_get_display_height(display) / 5), 0);
					}
					//al_draw_text(font, al_map_rgb(255, 255, 255), (al_get_display_width(display) / 2) - al_get_bitmap_width(btnf2S) / 2, 4 * (al_get_display_height(display) / 5), 0, "->");
					break;
				}

				al_draw_bitmap(antiv, 10, 350, 0);

				al_draw_bitmap(antib, 10, 480, 0);

				al_draw_bitmap(antim, 940, 350, 0);

				al_flip_display();
			}
		}

		tileset = (int***)malloc(2 * sizeof(int**));
		
		if (!tileset) {
			logger_log(LOG_ERROR, "Error declaring tileset matrix");
			exit(EXIT_FAILURE);
		}

		for (i = 0; i < 2; i++) {
			tileset[i] = (int**)malloc(mapSize * sizeof(int*));
			for (j = 0; j < mapSize; j++) {
				tileset[i][j] = (int*)malloc(mapSize * sizeof(int));
			}
		}

		switch (stageSelect) {
		case 1:
			al_stop_samples();
			tm = fopen("assets/tilemaps/tilemap1.txt", "r");
			al_play_sample(bgm2, 0.25, 0.0, 1.0, ALLEGRO_PLAYMODE_LOOP, NULL);
			break;
		case 2:
			al_stop_samples();
			tm = fopen("assets/tilemaps/tilemap2.txt", "r");
			al_play_sample(bgm3, 0.25, 0.0, 1.0, ALLEGRO_PLAYMODE_LOOP, NULL);
			break;
		}

		for (i = 0; i < 2; i++) {
			for (j = 0; j < mapSize; j++) {
				for (k = 0; k < mapSize; k++) {
					fscanf(tm, "%d", &tileset[i][j][k]);
				}
			}
		}

		fclose(tm);

		initplayer(&player, playersheet, &enemySpawnTileCount, tileset);

		if (levelEditor) {
			player.vel_x = 0;
		}
		else {
			player.vel_x = 4.5;
		}

		eSTx = malloc(enemySpawnTileCount * sizeof(int));
		eSTy = malloc(enemySpawnTileCount * sizeof(int));
		checkspawn = malloc(enemySpawnTileCount * sizeof(int));

		for (i = 0; i < enemySpawnTileCount; i++) {
			checkspawn[i] = false;
		}

		k = 0;
		for (i = 0; i < mapSize; i++) {
			for (j = 0; j < mapSize; j++) {
				if (tileset[ftile][i][j] == espawn) {
					eSTx[k] = j;
					eSTy[k] = i;
					k++;
				}
			}
		}

		while (levelEditor) {
			ALLEGRO_EVENT event;
			al_wait_for_event(evQueue, &event);

			if (event.type == ALLEGRO_EVENT_MOUSE_AXES) {
				mouse.x = event.mouse.x + cx;
				mouse.y = event.mouse.y + cy;

				player.tileX = mouse.x / tileSize;
				player.tileY = mouse.y / tileSize;
			}

			if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
				if (event.mouse.button == 1) {
					if (!modDown) {
						tileset[gtile][player.tileY][player.tileX] = player.selectedWeapon;
						editorClick.x = player.tileX;
						editorClick.y = player.tileY;
					}
					else {
						if (editorClick.x == player.tileX) {
							if (player.tileY > editorClick.y) {
								for (i = editorClick.y; i <= player.tileY; i++) {
									tileset[gtile][i][player.tileX] = player.selectedWeapon;
								}
							}
							if (player.tileY < editorClick.y) {
								for (i = editorClick.y; i >= player.tileY; i--) {
									tileset[gtile][i][player.tileX] = player.selectedWeapon;
								}
							}
						}
						else if (editorClick.y == player.tileY) {
							if (player.tileX > editorClick.x) {
								for (i = editorClick.x; i <= player.tileX; i++) {
									tileset[gtile][player.tileY][i] = player.selectedWeapon;
								}
							}
							if (player.tileX < editorClick.x) {
								for (i = editorClick.x; i >= player.tileX; i--) {
									tileset[gtile][player.tileY][i] = player.selectedWeapon;
								}
							}
						}
						editorClick.x = player.tileX;
						editorClick.y = player.tileY;
					}
				}
				else if (event.mouse.button == 2) {
					if (!modDown) {
						tileset[ftile][player.tileY][player.tileX] = tilefunc;
						editorClick.x = player.tileX;
						editorClick.y = player.tileY;
					}
					else {
						if (editorClick.x == player.tileX) {
							if (player.tileY > editorClick.y) {
								for (i = editorClick.y; i <= player.tileY; i++) {
									tileset[ftile][i][player.tileX] = tilefunc;
								}
							}
							if (player.tileY < editorClick.y) {
								for (i = editorClick.y; i >= player.tileY; i--) {
									tileset[ftile][i][player.tileX] = tilefunc;
								}
							}
						}
						else if (editorClick.y == player.tileY) {
							if (player.tileX > editorClick.x) {
								for (i = editorClick.x; i <= player.tileX; i++) {
									tileset[ftile][player.tileY][i] = tilefunc;
								}
							}
							if (player.tileX < editorClick.x) {
								for (i = editorClick.x; i >= player.tileX; i--) {
									tileset[ftile][player.tileY][i] = tilefunc;
								}
							}
						}
						editorClick.x = player.tileX;
						editorClick.y = player.tileY;
					}
				}
			}

			if (event.type == ALLEGRO_EVENT_TIMER) {
				if (player.pos.x <= 0 + (al_get_display_width(display) / 2)) {
					player.vel_x = 0;
					player.pos.x = 1 + (al_get_display_width(display) / 2);
				}
				if (player.pos.x >= (mapSize * tileSize) - (al_get_display_width(display) / 2)) {
					player.vel_x = 0;
					player.pos.x = (mapSize * tileSize) - (1 + (al_get_display_width(display) / 2));
				}
				if (player.pos.y <= 0 + (al_get_display_height(display) / 2)) {
					player.vel_y = 0;
					player.pos.y = 1 + (al_get_display_height(display) / 2);
				}
				if (player.pos.y >= (mapSize * tileSize) - (al_get_display_height(display) / 2)) {
					player.vel_y = 0;
					player.pos.y = (mapSize * tileSize) - (1 + (al_get_display_height(display) / 2));
				}

				player.pos.x += player.vel_x;
				player.pos.y += player.vel_y;

				refreshCamera(&cx, &cy, player);

				frameCount++;
			}

			exitGame(event, &levelEditor, &exit_control);

			if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
				switch (event.keyboard.keycode) {
				case ALLEGRO_KEY_UP:
				case ALLEGRO_KEY_W:
					player.vel_y += -15;
					break;

				case ALLEGRO_KEY_LEFT:
				case ALLEGRO_KEY_A:
					player.vel_x += -15;
					break;

				case ALLEGRO_KEY_DOWN:
				case ALLEGRO_KEY_S:
					player.vel_y += 15;
					break;

				case ALLEGRO_KEY_RIGHT:
				case ALLEGRO_KEY_D:
					player.vel_x += 15;
					break;
				case ALLEGRO_KEY_LSHIFT:
				case ALLEGRO_KEY_RSHIFT:
					modDown = true;
					break;
				case ALLEGRO_KEY_Q:
					player.selectedWeapon--;
					if (player.selectedWeapon < 0) {
						player.selectedWeapon = 16;
					}
					break;
				case ALLEGRO_KEY_E:
					player.selectedWeapon++;
					if (player.selectedWeapon > 16) {
						player.selectedWeapon = 0;
					}
					break;
				case ALLEGRO_KEY_Z:
					tilefunc--;
					if (tilefunc < 0) {
						tilefunc = 3;
					}
					break;
				case ALLEGRO_KEY_C:
					tilefunc++;
					if (tilefunc > 3) {
						tilefunc = 0;
					}
					break;
				case ALLEGRO_KEY_ENTER:
					switch (stageSelect) {
					case 1:
						tm = fopen("assets/tilemaps/tilemap1.txt", "w");
						break;
					case 2:
						tm = fopen("assets/tilemaps/tilemap2.txt", "w");
						break;
					}

					for (i = 0; i < 2; i++) {
						for (j = 0; j < mapSize; j++) {
							for (k = 0; k < mapSize; k++) {
								if (k == mapSize - 1) {
									fprintf(tm, "%d\n", tileset[i][j][k]);
								}
								else {
									fprintf(tm, "%d ", tileset[i][j][k]);
								}
							}
						}
					}

					fclose(tm);

					break;
				}
			}

			if (event.type == ALLEGRO_EVENT_KEY_UP) {
				switch (event.keyboard.keycode) {
				case ALLEGRO_KEY_UP:
				case ALLEGRO_KEY_W:
				case ALLEGRO_KEY_DOWN:
				case ALLEGRO_KEY_S:
					player.vel_y = 0;
					break;

				case ALLEGRO_KEY_LEFT:
				case ALLEGRO_KEY_A:
				case ALLEGRO_KEY_RIGHT:
				case ALLEGRO_KEY_D:
					player.vel_x = 0;
					break;
				case ALLEGRO_KEY_LSHIFT:
				case ALLEGRO_KEY_RSHIFT:
					modDown = false;
					break;
				}
			}

			exitGame(event, &gameLoop, &exit_control);

			if (event.type == ALLEGRO_EVENT_DISPLAY_RESIZE) {
				al_acknowledge_resize(display);
			}

			if (al_is_event_queue_empty(evQueue)) {
				al_clear_to_color(al_map_rgb(255, 255, 255));

				al_identity_transform(&camera);
				al_translate_transform(&camera, -cx * 0.05, 0);
				al_use_transform(&camera);

				al_draw_bitmap(stage[backgroundL1], 0, 0, 0);
				al_draw_bitmap(stage[backgroundL1], al_get_bitmap_width(stage[backgroundL1]), 0, ALLEGRO_FLIP_HORIZONTAL);

				al_identity_transform(&camera);
				al_translate_transform(&camera, -cx * 0.25, 0);
				al_use_transform(&camera);

				al_draw_bitmap(stage[backgroundL2], 0, 0, 0);
				al_draw_bitmap(stage[backgroundL2], al_get_bitmap_width(stage[backgroundL2]), 0, 0);

				al_identity_transform(&camera);
				al_translate_transform(&camera, -cx * 0.85, 0);
				al_use_transform(&camera);

				al_draw_bitmap(stage[backgroundL3], 0, 0, 0);
				al_draw_bitmap(stage[backgroundL3], al_get_bitmap_width(stage[backgroundL3]), 0, 0);
				al_draw_bitmap(stage[backgroundL3], 2 * al_get_bitmap_width(stage[backgroundL3]), 0, 0);

				al_identity_transform(&camera);
				al_translate_transform(&camera, -cx, -cy);
				al_use_transform(&camera);

				for (i = 0; i < 2; i++) {
					for (j = 0; j < mapSize; j++) {
						for (k = 0; k < mapSize; k++) {
							if (i == gtile)	{
								switch (tileset[gtile][j][k]) {
								case chaoce:
									al_draw_bitmap_region(tileAtlas, 0, 0, tileSize, tileSize, k * tileSize, j * tileSize, 0);
									break;
								case chaoc:
									al_draw_bitmap_region(tileAtlas, 1 * tileSize, 0, tileSize, tileSize, k * tileSize, j * tileSize, 0);
									break;
								case chaocd:
									al_draw_bitmap_region(tileAtlas, 2 * tileSize, 0, tileSize, tileSize, k * tileSize, j * tileSize, 0);
									break;
								case chao1wt:
									al_draw_bitmap_region(tileAtlas, 3 * tileSize, 0, tileSize, tileSize, k * tileSize, j * tileSize, 0);
									break;
								case chaome:
									al_draw_bitmap_region(tileAtlas, 0 * tileSize, 1 * tileSize, tileSize, tileSize, k * tileSize, j * tileSize, 0);
									break;
								case chaom:
									al_draw_bitmap_region(tileAtlas, 1 * tileSize, 1 * tileSize, tileSize, tileSize, k * tileSize, j * tileSize, 0);
									break;
								case chaomd:
									al_draw_bitmap_region(tileAtlas, 2 * tileSize, 1 * tileSize, tileSize, tileSize, k * tileSize, j * tileSize, 0);
									break;
								case chao1wm:
									al_draw_bitmap_region(tileAtlas, 3 * tileSize, 1 * tileSize, tileSize, tileSize, k * tileSize, j * tileSize, 0);
									break;
								case chaobe:
									al_draw_bitmap_region(tileAtlas, 0 * tileSize, 2 * tileSize, tileSize, tileSize, k * tileSize, j * tileSize, 0);
									break;
								case chaob:
									al_draw_bitmap_region(tileAtlas, 1 * tileSize, 2 * tileSize, tileSize, tileSize, k * tileSize, j * tileSize, 0);
									break;
								case chaobd:
									al_draw_bitmap_region(tileAtlas, 2 * tileSize, 2 * tileSize, tileSize, tileSize, k * tileSize, j * tileSize, 0);
									break;
								case chao1wb:
									al_draw_bitmap_region(tileAtlas, 3 * tileSize, 2 * tileSize, tileSize, tileSize, k * tileSize, j * tileSize, 0);
									break;
								case chao1he:
									al_draw_bitmap_region(tileAtlas, 0 * tileSize, 3 * tileSize, tileSize, tileSize, k * tileSize, j * tileSize, 0);
									break;
								case chao1hm:
									al_draw_bitmap_region(tileAtlas, 1 * tileSize, 3 * tileSize, tileSize, tileSize, k * tileSize, j * tileSize, 0);
									break;
								case chao1hd:
									al_draw_bitmap_region(tileAtlas, 2 * tileSize, 3 * tileSize, tileSize, tileSize, k * tileSize, j * tileSize, 0);
									break;
								case chaounico:
									al_draw_bitmap_region(tileAtlas, 3 * tileSize, 3 * tileSize, tileSize, tileSize, k * tileSize, j * tileSize, 0);
									break;
								}
							}
							else {
								switch (tileset[ftile][j][k]) {
								case espawn:
									al_draw_text(font, al_map_rgb(0, 0, 0), k * tileSize + tileSize / 2, j * tileSize, 0, "E");
									break;
								case pspawn:
									al_draw_text(font, al_map_rgb(0, 0, 0), k * tileSize + tileSize / 2, j * tileSize, 0, "P");
									break;
								case finish:
									al_draw_text(font, al_map_rgb(0, 0, 0), k * tileSize + tileSize / 2, j * tileSize, 0, "F");
									break;
								}
							}
						}
					}
				}

				al_identity_transform(&camera);
				al_use_transform(&camera);

				switch (stageSelect) {
				case 1:
					sprintf(objText, "Stage %d", stageSelect);
					al_draw_text(font, al_map_rgb(255, 255, 255), 10, 5, 0, objText);
					break;
				case 2:
					sprintf(objText, "Stage %d", stageSelect);
					al_draw_text(font, al_map_rgb(255, 255, 255), 10, 5, 0, objText);
					break;
				}

				switch (player.selectedWeapon) {
				case 0:
					al_draw_text(font, al_map_rgb(255, 255, 255), 10, 28, 0, "Selected Tile = air");
					break;
				case 1:
					al_draw_text(font, al_map_rgb(255, 255, 255), 10, 28, 0, "Selected Tile = chao superior esquerdo");
					break;
				case 2:
					al_draw_text(font, al_map_rgb(255, 255, 255), 10, 28, 0, "Selected Tile = chao superior");
					break;
				case 3:
					al_draw_text(font, al_map_rgb(255, 255, 255), 10, 28, 0, "Selected Tile = chao superior direito");
					break;
				case 4:
					al_draw_text(font, al_map_rgb(255, 255, 255), 10, 28, 0, "Selected Tile = chao superior (1 wide)");
					break;
				case 5:
					al_draw_text(font, al_map_rgb(255, 255, 255), 10, 28, 0, "Selected Tile = chao meio esquerdo");
					break;
				case 6:
					al_draw_text(font, al_map_rgb(255, 255, 255), 10, 28, 0, "Selected Tile = chao meio");
					break;
				case 7:
					al_draw_text(font, al_map_rgb(255, 255, 255), 10, 28, 0, "Selected Tile = chao meio direito");
					break;
				case 8:
					al_draw_text(font, al_map_rgb(255, 255, 255), 10, 28, 0, "Selected Tile = chao meio (1 wide)");
					break;
				case 9:
					al_draw_text(font, al_map_rgb(255, 255, 255), 10, 28, 0, "Selected Tile = chao inferior esquerdo");
						break;
				case 10:
					al_draw_text(font, al_map_rgb(255, 255, 255), 10, 28, 0, "Selected Tile = chao inferior");
					break;
				case 11:
					al_draw_text(font, al_map_rgb(255, 255, 255), 10, 28, 0, "Selected Tile = chao inferior direito");
					break;
				case 12:
					al_draw_text(font, al_map_rgb(255, 255, 255), 10, 28, 0, "Selected Tile = chao inferior (1 wide)");
					break;
				case 13:
					al_draw_text(font, al_map_rgb(255, 255, 255), 10, 28, 0, "Selected Tile = chao esquerdo (1 high)");
					break;
				case 14:
					al_draw_text(font, al_map_rgb(255, 255, 255), 10, 28, 0, "Selected Tile = chao meio (1 high)");
					break;
				case 15:
					al_draw_text(font, al_map_rgb(255, 255, 255), 10, 28, 0, "Selected Tile = chao direito (1 high)");
					break;
				case 16:
					al_draw_text(font, al_map_rgb(255, 255, 255), 10, 28, 0, "Selected Tile = chao 1x1");
					break;
				}

				switch (tilefunc) {
				case none:
					al_draw_text(font, al_map_rgb(255, 255, 255), 10, 44, 0, "Selected function = none");
					break;
				case espawn:
					al_draw_text(font, al_map_rgb(255, 255, 255), 10, 44, 0, "Selected function = enemy spawn");
						break;
				case pspawn:
					al_draw_text(font, al_map_rgb(255, 255, 255), 10, 44, 0, "Selected function = player spawn");
						break;
				case finish:
					al_draw_text(font, al_map_rgb(255, 255, 255), 10, 44, 0, "Selected function = finish tile");
						break;
				}

				if (devMode) {
					al_draw_text(font, al_map_rgb(255, 255, 255), windowWidth / 2, 0, ALLEGRO_ALIGN_CENTER, "Dev mode!!");
				}

				al_draw_text(font, al_map_rgb(255, 255, 255), 10, 73, 0, mousePos);
				sprintf(ptx, "X = %d", player.tileX);
				al_draw_text(font, al_map_rgb(255, 255, 255), 10, 60, 0, ptx);
				sprintf(pty, "Y = %d", player.tileY);
				al_draw_text(font, al_map_rgb(255, 255, 255), 100, 60, 0, pty);

				al_flip_display();
			}
		}

		while (gameLoop) {
			ALLEGRO_EVENT event;
			al_wait_for_event(evQueue, &event);

			if (event.type == ALLEGRO_EVENT_TIMER) {
				refreshPlayerMovement(&player, tiles, tileset);
				refreshProjectileState(playerShot, enemyShot, player, &projectileCount, &enemyProjectileCount, cx, cy);

				refreshCamera(&cx, &cy, player);

				if (player.isShooting) {
					if (player.spriteChange >= FPS / 2) {
						player.spriteChange = 0;
						player.isShooting = false;
					}
					else player.spriteChange++;
				}
				
				for (j = 0; j < enemyMax; j++) {
					refreshEnemyMovement(&enemy[j], &player);

					if (enemy[j].selectedWeapon == antiMycotic && frameCount % 3 == 0) {
						enemy[j].rotate++;
					}

					if (enemy[j].attack == shooter && enemy[j].alive) {
						for (i = 0; i < enemyProjectileMax; i++) {
							if (frameCount - enemy[j].shotFC >= FPS * 2) {
								if (!enemyShot[i].projectileTravel) {
									enemyProjectileCount++;
									eShoot(&enemyShot[i], &enemy[j], &player, frameCount);
									break;
								}
							}
						}
					}

					// Bullet hell lmao
					/*if (enemy[j].attack == shooter) {
						for (i = 0; i < enemyProjectileMax; i++) {
							if (frameCount - enemy[i].shotFC >= FPS) {
								if (!enemyShot[i].projectileTravel) {
									enemyProjectileCount++;
									eShoot(&enemyShot[i], &enemy[j], &player, frameCount);
									break;
								}
							}
						}
					}*/
				}

				hitboxDetection(playerShot, enemy, &player, &killCount, hitI, &projectileCount, &immortalityFC, enemyDeadFC, &frameCount, checkspawn);
				hitboxDetection(enemyShot, enemy, &player, &killCount, hitI, &projectileCount, &immortalityFC, enemyDeadFC, &frameCount, checkspawn);
				colisionDetection(enemy, &player, &immortalityFC, &frameCount);

				if (player.life <= 0) {
					player.alive = false;
					stageLoop = true;
					gameLoop = false;
					exitStage = true;
				}
				if (player.immortality && frameCount - immortalityFC >= (int)(1.5 * FPS)) {
					player.immortality = false;
				}

				if (enemyDmgGauge < enemy[hitI[enemyI]].maxLife - enemy[hitI[enemyI]].life && frameCount % 2 == 0) {
					enemyDmgGauge++;
				}

				switch (stageSelect) {
				case 1:
					for (i = 0; i < enemyMax; i++) {
						if (!enemy[i].alive && frameCount - enemyDeadFC[i] >= FPS && frameCount % 90 == 0) {
							initenemy(&enemy[i], &enemySprite, shooter, 1);
							
							spawntimeout = 0;

							j = randombytes_uniform(enemySpawnTileCount);
							
							while (checkspawn[j]) {
								j = randombytes_uniform(enemySpawnTileCount);
								if (spawntimeout == enemyMax) {
									break;
								}
								spawntimeout++;
							}

							if (spawntimeout == enemyMax) {
								break;
							}

							enemy[i].coord = j;
							enemy[i].pos.y = eSTy[j] * tileSize;
							enemy[i].pos.x = eSTx[j] * tileSize;
							checkspawn[j] = true;

							for (k = 0; k < enemyMax; k++) {
								if (k == i) {
									k++;
								}
								if (enemy[i].pos.x != enemy[k].pos.x && enemy[i].pos.y != enemy[k].pos.y) {
									break;
								}
							}
							enemyDmgGauge = 0;
							enemy[i].alive = true;
							break;
						}
					}
					if (killCount.count == 0) {
						adv.m1 = true;
						stageLoop = true;
						gameLoop = false;
						exitStage = true;
					}
					break;
				case 2:
					endurance.count--;
					for (i = 0; i < enemyMax; i++) {
						if (!enemy[i].alive && frameCount - enemyDeadFC[i] >= FPS && frameCount % 90 == 0) {
							initenemy(&enemy[i], &enemySprite, contact, 2);
							enemyDmgGauge = 0;
							break;
						}
					}
					if (endurance.count == 0) {
						adv.m2 = true;
						stageLoop = true;
						gameLoop = false;
						exitStage = true;
					}
					break;
				}

				if (player.dir != 5) {
					runCycle++;
				}
				frameCount++;
				al_flip_display();
			}

			if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
				switch (event.keyboard.keycode) {
				case ALLEGRO_KEY_UP:
					if (player.onGround) {
						al_play_sample(sfx_jump, 0.25, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
						player.vel_y = -8;
						player.onGround = false;
					}
					break;
				case ALLEGRO_KEY_LEFT:
					runCycle = 0;
					player.dir = Left;
					player.currentDir = Left;
					break;
				case ALLEGRO_KEY_RIGHT:
					runCycle = 0;
					player.dir = Right;
					player.currentDir = Right;
					break;
				case ALLEGRO_KEY_A:
					player.selectedWeapon--;
					if (player.selectedWeapon < 0) {
						player.selectedWeapon = 2;
					}
					break;
				case ALLEGRO_KEY_D:
					player.selectedWeapon++;
					if (player.selectedWeapon > 2) {
						player.selectedWeapon = 0;
					}
					break;
				case ALLEGRO_KEY_SPACE:
					al_play_sample(shot, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
					if (projectileCount < projectileMax) {
						for (i = 0; i < projectileMax; i++) {
							if (!playerShot[i].projectileTravel) {
								projectileCount++;
								player.spriteChange = 0;
								pShoot(&playerShot[i], &player);
								break;
							}
						}
					}
					break;
				}
			}

			if (event.type == ALLEGRO_EVENT_KEY_UP) {
				switch (event.keyboard.keycode) {
				case ALLEGRO_KEY_RIGHT:
					if (player.dir == Right) {
						runCycle = 0;
						player.dir = 5;
					}
					break;
				case ALLEGRO_KEY_LEFT:
					if (player.dir == Left) {
						runCycle = 0;
						player.dir = 5;
					}
					break;
				}
			}

			exitGame(event, &gameLoop, &exit_control);

			if (event.type == ALLEGRO_EVENT_DISPLAY_RESIZE) {
				al_acknowledge_resize(display);
			}

			if (al_is_event_queue_empty(evQueue)) {
				al_clear_to_color(al_map_rgb(255, 255, 255));

				al_identity_transform(&camera);
				al_translate_transform(&camera, -cx * 0.05, 0);
				al_use_transform(&camera);

				al_draw_bitmap(stage[backgroundL1], 0, 0, 0);
				al_draw_bitmap(stage[backgroundL1], al_get_bitmap_width(stage[backgroundL1]), 0, ALLEGRO_FLIP_HORIZONTAL);

				al_identity_transform(&camera);
				al_translate_transform(&camera, -cx * 0.25, 0);
				al_use_transform(&camera);

				al_draw_bitmap(stage[backgroundL2], 0, 0, 0);
				al_draw_bitmap(stage[backgroundL2], al_get_bitmap_width(stage[backgroundL2]), 0, 0);
				
				al_identity_transform(&camera);
				al_translate_transform(&camera, -cx * 0.85, 0);
				al_use_transform(&camera);

				al_draw_bitmap(stage[backgroundL3], 0, 0, 0);
				al_draw_bitmap(stage[backgroundL3], al_get_bitmap_width(stage[backgroundL3]), 0, 0);
				al_draw_bitmap(stage[backgroundL3], 2 * al_get_bitmap_width(stage[backgroundL3]), 0, 0);

				al_identity_transform(&camera);
				al_translate_transform(&camera, -cx, -cy);
				al_use_transform(&camera);
				
				for (i = 0; i < 2; i++) {
					for (j = 0; j < mapSize; j++) {
						for (k = 0; k < mapSize; k++) {
							if (i == gtile) {
								switch (tileset[gtile][j][k]) {
								case chaoce:
									al_draw_bitmap_region(tileAtlas, 0, 0, tileSize, tileSize, k * tileSize, j * tileSize, 0);
									break;
								case chaoc:
									al_draw_bitmap_region(tileAtlas, 1 * tileSize, 0, tileSize, tileSize, k * tileSize, j * tileSize, 0);
									break;
								case chaocd:
									al_draw_bitmap_region(tileAtlas, 2 * tileSize, 0, tileSize, tileSize, k * tileSize, j * tileSize, 0);
									break;
								case chao1wt:
									al_draw_bitmap_region(tileAtlas, 3 * tileSize, 0, tileSize, tileSize, k * tileSize, j * tileSize, 0);
									break;
								case chaome:
									al_draw_bitmap_region(tileAtlas, 0 * tileSize, 1 * tileSize, tileSize, tileSize, k * tileSize, j * tileSize, 0);
									break;
								case chaom:
									al_draw_bitmap_region(tileAtlas, 1 * tileSize, 1 * tileSize, tileSize, tileSize, k * tileSize, j * tileSize, 0);
									break;
								case chaomd:
									al_draw_bitmap_region(tileAtlas, 2 * tileSize, 1 * tileSize, tileSize, tileSize, k * tileSize, j * tileSize, 0);
									break;
								case chao1wm:
									al_draw_bitmap_region(tileAtlas, 3 * tileSize, 1 * tileSize, tileSize, tileSize, k * tileSize, j * tileSize, 0);
									break;
								case chaobe:
									al_draw_bitmap_region(tileAtlas, 0 * tileSize, 2 * tileSize, tileSize, tileSize, k * tileSize, j * tileSize, 0);
									break;
								case chaob:
									al_draw_bitmap_region(tileAtlas, 1 * tileSize, 2 * tileSize, tileSize, tileSize, k * tileSize, j * tileSize, 0);
									break;
								case chaobd:
									al_draw_bitmap_region(tileAtlas, 2 * tileSize, 2 * tileSize, tileSize, tileSize, k * tileSize, j * tileSize, 0);
									break;
								case chao1wb:
									al_draw_bitmap_region(tileAtlas, 3 * tileSize, 2 * tileSize, tileSize, tileSize, k * tileSize, j * tileSize, 0);
									break;
								case chao1he:
									al_draw_bitmap_region(tileAtlas, 0 * tileSize, 3 * tileSize, tileSize, tileSize, k * tileSize, j * tileSize, 0);
									break;
								case chao1hm:
									al_draw_bitmap_region(tileAtlas, 1 * tileSize, 3 * tileSize, tileSize, tileSize, k * tileSize, j * tileSize, 0);
									break;
								case chao1hd:
									al_draw_bitmap_region(tileAtlas, 2 * tileSize, 3 * tileSize, tileSize, tileSize, k * tileSize, j * tileSize, 0);
									break;
								case chaounico:
									al_draw_bitmap_region(tileAtlas, 3 * tileSize, 3 * tileSize, tileSize, tileSize, k * tileSize, j * tileSize, 0);
									break;
								}
							}
						}
					}
				}
				if (!player.immortality || (frameCount - immortalityFC) / 12 % 2 == 0) {
					if (player.currentDir == Right) {
						if (!player.isShooting) {
							if ((runCycle / 7) % 2 == 0) {
								al_draw_bitmap_region(playersheet, 0, 0, 48, 48, player.pos.x, player.pos.y, ALLEGRO_FLIP_HORIZONTAL);
							}
							else {
								al_draw_bitmap_region(playersheet, 48, 0, 48, 48, player.pos.x, player.pos.y, ALLEGRO_FLIP_HORIZONTAL);
							}
						}
						else {
							if ((runCycle / 7) % 2 == 0) {
								al_draw_bitmap_region(playersheet, 0, 48, 48, 48, player.pos.x, player.pos.y, ALLEGRO_FLIP_HORIZONTAL);
							}
							else {
								al_draw_bitmap_region(playersheet, 48, 48, 48, 48, player.pos.x, player.pos.y, ALLEGRO_FLIP_HORIZONTAL);
							}
						}
						//al_draw_filled_rectangle(player.hbX, player.hbY, player.hbX + player.hbWidth, player.hbY + player.hbHeight, al_map_rgba(0, 0, 255, 50));
					}
					else {
						if (!player.isShooting) {
							if ((runCycle / 7) % 2 == 0) {
								al_draw_bitmap_region(playersheet, 0, 0, 48, 48, player.pos.x, player.pos.y, 0);
							}
							else {
								al_draw_bitmap_region(playersheet, 48, 0, 48, 48, player.pos.x, player.pos.y, 0);
							}
						}
						else {
							if ((runCycle / 7) % 2 == 0) {
								al_draw_bitmap_region(playersheet, 0, 48, 48, 48, player.pos.x, player.pos.y, 0);
							}
							else {
								al_draw_bitmap_region(playersheet, 48, 48, 48, 48, player.pos.x, player.pos.y, 0);
							}
						}
						//al_draw_filled_rectangle(player.hbX, player.hbY, player.hbX + player.hbWidth, player.hbY + player.hbHeight, al_map_rgba(0, 0, 255, 50));
					}
				}

				for (i = 0; i < enemyMax; i++) {
					if (enemy[i].alive) {
						if (enemy[i].selectedWeapon == antiMycotic) {
							if (enemy[i].pos.x < player.pos.x)
								al_draw_tinted_scaled_rotated_bitmap_region(enemysheet, enemy[i].selectedWeapon * enemy[i].width, 0, enemy[i].width, enemy[i].height, al_map_rgb_f(1, 1, 1), enemy[i].width / 2, enemy[i].height / 2, enemy[i].pos.x, enemy[i].pos.y, 1, 1, enemy[i].rotate, ALLEGRO_FLIP_HORIZONTAL);
							else
								al_draw_tinted_scaled_rotated_bitmap_region(enemysheet, enemy[i].selectedWeapon * enemy[i].width, 0, enemy[i].width, enemy[i].height, al_map_rgb_f(1, 1, 1), enemy[i].width / 2, enemy[i].height / 2, enemy[i].pos.x, enemy[i].pos.y, 1, 1, enemy[i].rotate, 0);
						}
						else {
							if (enemy[i].pos.x < player.pos.x) {
								al_draw_bitmap_region(enemysheet, enemy[i].selectedWeapon * enemy[i].width + 1, enemy[i].selectedWeapon * enemy[i].height, enemy[i].width, enemy[i].height, enemy[i].pos.x, enemy[i].pos.y, ALLEGRO_FLIP_HORIZONTAL);
							}
							else {
								al_draw_bitmap_region(enemysheet, enemy[i].selectedWeapon * enemy[i].width + 1, enemy[i].selectedWeapon * enemy[i].height, enemy[i].width, enemy[i].height, enemy[i].pos.x, enemy[i].pos.y, 0);
							}
						}
					}
				}

				for (i = 0; i < projectileMax; i++) {
					if (playerShot[i].projectileTravel) {
						setProjectileColor(&playerShot[i]);
						if (playerShot[i].dir == Right) {
							if (playerShot[i].angle == 0) {
								al_draw_tinted_bitmap(playerShotTemplate[0], al_map_rgb(playerShot[i].r, playerShot[i].g, playerShot[i].b), playerShot[i].pos.x, playerShot[i].pos.y, ALLEGRO_FLIP_HORIZONTAL);
							}
							else {
								al_draw_tinted_bitmap(playerShotTemplate[1], al_map_rgb(playerShot[i].r, playerShot[i].g, playerShot[i].b), playerShot[i].pos.x, playerShot[i].pos.y, ALLEGRO_FLIP_HORIZONTAL);
							}
							
						}
						else {
							if (playerShot[i].angle == 0) {
								al_draw_tinted_bitmap(playerShotTemplate[0], al_map_rgb(playerShot[i].r, playerShot[i].g, playerShot[i].b), playerShot[i].pos.x, playerShot[i].pos.y, 0);
							}
							else {
								al_draw_tinted_bitmap(playerShotTemplate[1], al_map_rgb(playerShot[i].r, playerShot[i].g, playerShot[i].b), playerShot[i].pos.x, playerShot[i].pos.y, 0);
							}
						}
					}
				}

				for (i = 0; i < enemyProjectileMax; i++) {
					if (enemyShot[i].projectileTravel) {
						al_draw_filled_circle(enemyShot[i].pos.x, enemyShot[i].pos.y, enemyShot[i].width / 2, al_map_rgb(255, 165, 0));
						al_draw_filled_circle(enemyShot[i].pos.x, enemyShot[i].pos.y, (enemyShot[i].width / 2) - 2, al_map_rgb(255, 255, 255));
					}
				}

				al_identity_transform(&camera);
				al_use_transform(&camera);

				switch ((int)player.life / 10) {
				case 3:
					al_draw_filled_rectangle(10, 35, 20, 45, al_map_rgb(255, 0, 0));
					al_draw_filled_rectangle(30, 35, 40, 45, al_map_rgb(255, 0, 0));
					al_draw_filled_rectangle(50, 35, 60, 45, al_map_rgb(255, 0, 0));
					break;
				case 2:
					al_draw_filled_rectangle(10, 35, 20, 45, al_map_rgb(255, 0, 0));
					al_draw_filled_rectangle(30, 35, 40, 45, al_map_rgb(255, 0, 0));
					al_draw_rectangle(50, 35, 60, 45, al_map_rgb(255, 0, 0), 2);
					break;
				case 1:
					al_draw_filled_rectangle(10, 35, 20, 45, al_map_rgb(255, 0, 0));
					al_draw_rectangle(30, 35, 40, 45, al_map_rgb(255, 0, 0), 2);
					al_draw_rectangle(50, 35, 60, 45, al_map_rgb(255, 0, 0), 2);
					break;
				}

				al_draw_filled_rectangle(windowWidth - (2 * (enemy[hitI[enemyI]].maxLife - enemyDmgGauge) + 50), 50, windowWidth - (enemy[hitI[enemyI]].life + 50), 62, al_map_rgb(255, 0, 0));
				al_draw_filled_rectangle(windowWidth - 50, 50, windowWidth - (2 * enemy[hitI[enemyI]].life + 50), 62, al_map_rgb(0, 128, 0));
				sprintf(enemyLifeGauge, "%.0f", enemy[hitI[enemyI]].life);
				al_draw_text(font, al_map_rgb(255, 255, 255), windowWidth - 48, 42, 0, enemyLifeGauge);

				switch (stageSelect) {
				case 1:
					sprintf(objText, "Shooters left = %d", killCount.count);
					al_draw_text(font, al_map_rgb(255, 255, 255), 10, 5, 0, objText);
					break;
				case 2:
					sprintf(objText, "survive = %d", endurance.count);
					al_draw_text(font, al_map_rgb(255, 255, 255), 10, 5, 0, objText);
					break;
				}

				if (devMode) {
					al_draw_text(font, al_map_rgb(255, 255, 255), windowWidth / 2, 0, ALLEGRO_ALIGN_CENTER, "Dev mode!!");
				}

				sprintf(ptx, "X = %d", player.tileX);
				al_draw_text(font, al_map_rgb(255, 255, 255), 10, 50, 0, ptx);
				sprintf(pty, "Y = %d", player.tileY);
				al_draw_text(font, al_map_rgb(255, 255, 255), 100, 50, 0, pty);

				ALLEGRO_COLOR weapon_color = get_weapon_color(player.selectedWeapon);
				switch (player.selectedWeapon) {
				case 0:
					al_draw_text(font, al_map_rgb(weapon_color.r, weapon_color.g, weapon_color.b), 70, 25, 0, "Antivirus");
					break;
				case 1:
					al_draw_text(font, al_map_rgb(weapon_color.r, weapon_color.g, weapon_color.b), 70, 25, 0, "Antibiotic");
					break;
				case 2:
					al_draw_text(font, al_map_rgb(weapon_color.r, weapon_color.g, weapon_color.b), 70, 25, 0, "Antimycotic");
					break;
				}

				al_flip_display();
			}
		}
	}

	al_destroy_display(display);
	al_destroy_event_queue(evQueue);
	al_uninstall_keyboard();
	al_uninstall_mouse();
	al_destroy_bitmap(playersheet);
	al_destroy_bitmap(enemysheet);
	al_destroy_bitmap(tileAtlas);
	al_destroy_bitmap(enemyShooterSprite);
	al_destroy_bitmap(enemySprite[antiBiotic]);
	al_destroy_bitmap(enemySprite[antiMycotic]);
	al_destroy_bitmap(enemySprite[antiVirus]);
	al_destroy_bitmap(enemyShooterSprite);
	al_destroy_bitmap(stage[backgroundL1]);
	al_destroy_bitmap(stage[backgroundL2]);
	al_destroy_bitmap(stage[backgroundL3]);
	al_destroy_bitmap(playerShotTemplate[0]);
	al_destroy_bitmap(playerShotTemplate[1]);
	al_destroy_bitmap(enemyShotTemplate);
	al_destroy_bitmap(titulo);
	al_destroy_bitmap(btnf1);
	al_destroy_bitmap(btnf2);
	al_destroy_bitmap(btnle);
	al_destroy_bitmap(btnf1S);
	al_destroy_bitmap(btnf2S);
	al_destroy_bitmap(btnleS);
	al_destroy_bitmap(hist);
	al_destroy_bitmap(control);
	al_destroy_timer(timer);
	al_destroy_sample(bgm1);
	al_destroy_sample(bgm2);
	al_destroy_sample(bgm3);
	al_destroy_sample(sfx_select);
	al_destroy_sample(shot);
	al_destroy_sample(sfx_jump);
	al_destroy_sample(sfx_sp1);
	al_destroy_sample(sfx_sp2);
	al_destroy_sample(sfx_sp3);
	al_destroy_sample(sfx_hit);
	al_destroy_sample_instance(sampleInstance);
	al_destroy_font(font);
	free(tileset);
	
	return 0;
}

//font by everiux365
