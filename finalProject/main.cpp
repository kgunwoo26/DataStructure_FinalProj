#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <conio.h>
#include<time.h>

#include <Mmsystem.h>
#include <mciapi.h>
#include <process.h>
//these two headers are already included in the <Windows.h> header
#pragma comment(lib, "Winmm.lib")

HANDLE hMusicThread;
HANDLE hMenuThread;

// TODO: ����, �Ѿ��� ��ũ�� ����Ʈ�� �����ص� ���� ���

// ��� ����
#define BGM_0 "./src/menu_bgm.mp3"
#define BGM_1 "./src/stage_bgm.mp3"
#define NAVI_MP3 "./src/navi.mp3"
#define SELECT_MP3 "./src/select.mp3"

// �޴� ��ġ ���
#define MENU_CURSOR_XPOS 51
#define MENU_STRING_XPOS 55

// �÷��̾� ���
#define PLAYER_YPOS 26
#define PLAYER_INIT_XPOS 31

// �Ѿ� ���
#define INIT_MAX_BULLET 100
#define BULLET_INIT_YPOS 24

// ���� ���
#define INIT_MAX_MONSTER 100
#define MONSTER_INIT_YPOS 3
#define SUMMON_DELAY_TIME 100
#define NUM_MONSTER_TYPE 3
#define MONSTER_SPEED_CONST 10

// ������ ���
#define INIT_MAX_DAMAGE 100
#define DAMAGE_XPOS_CONST 3
#define DAMAGE_TIME 2
#define DAMAGE_TIME_COUNT 3

// ����
#define INIT_XPOS 40
#define XPOS_GAP 14

// ���� ��� 0: �޴� 1: ���� �÷��� 2: ��ŷ
int currentMode = 0;

// �޴� ù �׸����� �ʱ� �� ����
int selectedMenuIndex = 0;

// ���� ����
enum monsterType
{
	CAT, COW, SPIDER
};

// ȿ���� ����
enum sound
{
	BGM, NAVI, SELECT, ATTACK, HIT
};

// ����ü ����
typedef struct player {
	int fullHp;
	int hp;
	int position;
	int level;
	int xp;
}Player;

typedef struct monster {
	int position; // �������� ��ġ 1-4
	int yPos;
	monsterType type;
	int life;
	int speed;
	int speedCount;
	int size;
	int damageReceived;
	int damage;
	int xp;
}Monster;

typedef struct monsters {
	int startIndex;
	int count;
	int maxMonster;
	Monster* monster;
}Monsters;

typedef struct bullet {
	int position;
	int yPos;
}Bullet;

typedef struct bullets {
	int startIndex;
	int count;
	int maxBullet;
	int damage;
	Bullet* bullet;
}Bullets;

typedef struct damage {
	int durationCount;
	int damageReceived;
	int position;
	int yPos;
	int count;
}Damage;

typedef struct damages {
	int count;
	int maxCount;
	int startIndex;
	Damage* damage;
}Damages;

// player�� ��ġ�� Ŀ���� �̵�
void gotoPlayer(Player player, int xpos, int ypos) {
	COORD coord;
	coord.X = INIT_XPOS + xpos + ((player.position - 1) * XPOS_GAP);
	coord.Y = PLAYER_YPOS + ypos;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

// bullet�� ��ġ�� Ŀ���� �̵�
void gotoBullet(Bullet bullet) {
	COORD coord;
	coord.X = INIT_XPOS + ((bullet.position - 1) * XPOS_GAP);
	coord.Y = bullet.yPos;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

// monster�� ��ġ���� index �� ��ŭ y ��ġ�� �̵���Ų ������ Ŀ���� �̵�
void gotoMonster(Monster monster, int index) {
	COORD coord;
	coord.X = INIT_XPOS + ((monster.position - 1) * XPOS_GAP) - 3;
	coord.Y = monster.yPos + index;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

// damage�� ��ġ�� Ŀ���� �̵�
void gotoDamage(Damage damage) {
	COORD coord;
	coord.X = INIT_XPOS + ((damage.position - 1) * XPOS_GAP) + DAMAGE_XPOS_CONST;
	coord.Y = damage.yPos - damage.count;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

// x,y ��ġ�� Ŀ���� �̵�
void gotoxy(int x, int y) {
	COORD coord;
	coord.X = x;
	coord.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

// MP3 ����(filepath)�� ���
void PlayMP3(const char* filePath) {
	char command[256];
	int delay;
	sprintf_s(command, "open \"%s\" type mpegvideo alias mp3", filePath);
	// ����ϴ� ȿ������ ���� ���� �ð� ����
	if (strcmp(filePath, NAVI_MP3) == 0) {
		delay = 150;
	}
	else if (strcmp(filePath, SELECT_MP3) == 0) {
		delay = 2000;
	}
	else {
		delay = 150;
	}
	mciSendString(command, NULL, 0, NULL);
	mciSendString("play mp3", NULL, 0, NULL);
	Sleep(delay);
	mciSendString("close mp3", NULL, 0, NULL);
}

// ���� ����� ���� ������
unsigned __stdcall MusicThread(void* filePath) {
	PlayMP3((const char*)filePath);
	// ������ ���� �� �ڵ��� ����
	_endthreadex(0);
	return 0;
}

// ��� ������ ��� ����ϵ��� �ϴ� �Լ�
void playBgm() {
	mciSendString("close mp3", NULL, 0, NULL);
	char command[256];
	switch (currentMode) {
	case 0: sprintf_s(command, "open \"%s\" type mpegvideo alias mp3", BGM_0); break;
	case 1: sprintf_s(command, "open \"%s\" type mpegvideo alias mp3", BGM_1); break;
	case 2: break;
	}
	mciSendString(command, NULL, 0, NULL);
	mciSendString("play mp3", NULL, 0, NULL);
}

// ��� ������ �����ϴ� �Լ�
void stopBgm() {
	mciSendString("close mp3", NULL, 0, NULL);
}

// �޴� Ž�� �� ȿ���� ���
void playNaviSound(void) {
	const char* musicFilePath = "./src/navi.mp3";
	hMusicThread = (HANDLE)_beginthreadex(NULL, 0, &MusicThread, (void*)musicFilePath, 0, NULL);
	// ������ ���� ���
	WaitForSingleObject(hMusicThread, INFINITE);
	// ������ �ڵ� �ݱ�
	CloseHandle(hMusicThread);
}

// �޴� ���� �� ���� ���� �Ÿ����� �ϴ� ������ �Լ�
unsigned __stdcall MenuBlinkThread(void* str) {
	for (int i = 0; i < 4; i++) {
		gotoxy(MENU_STRING_XPOS, 19 + 2 * (selectedMenuIndex));
		printf("            ");
		Sleep(250);
		gotoxy(MENU_STRING_XPOS, 19 + 2 * (selectedMenuIndex));
		printf("%s", str);
		Sleep(250);
	}
	_endthreadex(0);
	return 0;
}

// ���� ���õ� �޴��� �����Ͽ� ������ �Լ��� ����
void blinkSelectedMenu() {
	const char* menuString = "";
	switch (currentMode) {
	case 1: menuString = "���� ����"; break;
	case 3: menuString = "���� ����"; break;
	}
	hMenuThread = (HANDLE)_beginthreadex(NULL, 0, &MenuBlinkThread, (void*)menuString, 0, NULL);
	// ������ ���� ���
	WaitForSingleObject(hMusicThread, INFINITE);
	// ������ �ڵ� �ݱ�
	CloseHandle(hMusicThread);
}

// �޴� ���ý� ȿ���� ����ϴ� �Լ�
void playSelectSound(void) {
	const char* musicFilePath = "./src/select.mp3";
	blinkSelectedMenu();
	hMusicThread = (HANDLE)_beginthreadex(NULL, 0, &MusicThread, (void*)musicFilePath, 0, NULL);
	// ������ ���� ���
	WaitForSingleObject(hMusicThread, INFINITE);
	stopBgm();
	// ������ �ڵ� �ݱ�
	CloseHandle(hMusicThread);
}

// ��Ȳ�� �´� ���� ������ ����ϵ��� �ϴ� �Լ�
void playSound(sound type) {
	switch (type) {
	case BGM: playBgm(); break;
	case NAVI: playNaviSound(); break;
	case SELECT: playSelectSound();  break;
	case ATTACK: break;
	case HIT: break;
	}
}

// ������ hp�� ����ϴ� �Լ�
void printHP(Player* player) {
	gotoxy(3, 5);
	printf("HP  ");
	for (int i = 0; i < player->fullHp; i++) {
		if (player->hp > i) {
			printf("��");
		}
		else {
			printf("��");
		}
		
	}
}

// ������ ������ ���
void printLevel(Player* player) {
	gotoxy(3, 3);
	printf("����: %d", player->level);
}

// ������ ����ġ�� ���
void printXP(Player* player) {
	gotoxy(0, 0);
	printf("XP ");
	printf("|");
	for (int i = 1; i < 116; i++) {
		if (player->xp < i) {
			printf(" ");
		}
		else {
			printf("��");
		}
	}
	printf("|");
}

// �������� ���� ��� �÷��̾��� HP�� ���ҽ�Ű�� HP�� ���ΰ�ħ
void getDamage(Player *player, int damageReceived) {
	player->hp -= damageReceived;
	printHP(player);
}

// ���͸� óġ�� ����ġ�� ������ ���� �����ϰ� ����ġ �ٸ� ���ΰ�ħ
void getXP(Player* player, int xpReceived) {
	player->xp += xpReceived;
	if (player->xp > 115) {
		player->xp -= 115;
		player->level++;
		printLevel(player);
	}
	printXP(player);
}

// ���α׷� ���� �� ���������� ����� ���
void printfStage() {
	gotoxy(0, 0);
	for (int i = 0; i < 4; i++) {
		printf("\n");
	}

	for (int i = 0; i < 21; i++) {
		printf("                                 ��             ��             ��             ��             ��\n");
	}
	printf("                                 ������������������������������������������������������������������������������������������������������������������");
	for (int i = 0; i < 3; i++) {
		printf("\n");
	}
}


// ���� �ʱ�ȭ �Լ�. ���������� �÷��̾ ���
void startGame() {
	system("cls");
	playBgm();
	printfStage();
	gotoxy(INIT_XPOS, PLAYER_YPOS);
	printf("|\n");
	gotoxy(INIT_XPOS, PLAYER_YPOS+1);
	printf("o\n");
	gotoxy(INIT_XPOS-1, PLAYER_YPOS+2);
	printf("/_\\\n");
}

// Ű ���� �Է¹޾� �÷��̾��� ��ġ�� �̵���Ŵ. (�¿� ����Ű)
void movePlayer(Player* player) {
	static DWORD lastFireTime = 0; // ������ �̵� �ð��� ����

	DWORD currentTime = GetTickCount();

	if (GetAsyncKeyState(0x25) & 0x8000) {
		if (currentTime - lastFireTime >= 100) {
			if (player->position - 1 < 1) return;
			gotoPlayer(*player, 0, 0);
			printf("     ");
			gotoPlayer(*player, -2, 1);
			printf("     ");
			gotoPlayer(*player, -1, 2);
			printf("     ");

			player->position -= 1;
			gotoPlayer(*player,0,0);
			printf("|\n");
			gotoPlayer(*player, 0, 1);
			printf("o\n");
			gotoPlayer(*player, -1, 2);
			printf("/_\\\n");
			lastFireTime = currentTime;
		}
	}
	if (GetAsyncKeyState(0x27) & 0x8000) {
		if (currentTime - lastFireTime >= 100) {
			if (player->position + 1 > 4) return;
			gotoPlayer(*player, 0, 0);
			printf("     ");
			gotoPlayer(*player, -2, 1);
			printf("     ");
			gotoPlayer(*player, -1, 2);
			printf("     ");

			player->position += 1;
			gotoPlayer(*player, 0, 0);
			printf("|\n");
			gotoPlayer(*player, 0, 1);
			printf("o\n");
			gotoPlayer(*player, -1, 2);
			printf("/_\\\n");
			lastFireTime = currentTime;
		}
	}
}

/**
* �Ѿ��� �����Ͽ� Bullets.bullet�� �߰�
* @params bullets : bullet �迭�� �ּ� ��
* @params playerPos : �÷��̾��� ��ġ (1-4)
*/
void createBullet(Bullets* bullets, int playerPos) {
	Bullet bullet = { playerPos, BULLET_INIT_YPOS };
	bullets->bullet[bullets->count++] = bullet;
}

/**
* TODO: bullets->bullet�� ť�� �ٲ� ����
* bullets�� ����
* ( ����� bullets->startIndex�� ���� +1 �Ͽ� ���� ���� ������ bullet�� ����)
*/
void removeBullet(Bullets* bullets, int index) {
	bullets->startIndex++;
}

/**
* ��µ� �Ѿ��� ��� ����
*/
void eraseBullet(Bullet bullet) {
	gotoBullet(bullet);
	printf("  ");
}

/**
* �Ѿ��� �߻��ϴ� �Լ�. �����̽� �ٸ� ���� �� �Ѿ��� ����
* @params player : �÷��̾��� �ּ� ��
* @params bullets : bullet �迭�� �ּ� ��
*/
void fireWeapon(Player* player, Bullets* bullets) {
	static DWORD lastFireTime = 0; // ������ �߻� �ð��� ����

	if (bullets->count + 1 >= bullets->maxBullet) {
		bullets->maxBullet *= 2;
		bullets->bullet = (Bullet*)realloc(bullets->bullet, sizeof(Bullet) * bullets->maxBullet);
	}

	// ���� �ð��� ������
	DWORD currentTime = GetTickCount();

	if (GetAsyncKeyState(0x20) & 0x8000) {
		// �����̽� �ٰ� ���� �� ������ �������� �߻����� ����
		if (currentTime - lastFireTime >= 500) {
			createBullet(bullets, player->position);
			lastFireTime = currentTime; // ������ �߻� �ð� ������Ʈ
		}
	}
}
/**
*  ���ο� ������ ������ ������ ��ġ �ٷ� ���� ����
*  @params damages : damage �迭�� �ּ� ��
*  @params monster : �������� ���� monster
*  @params damageReceived : ���� ������ ��
*/
void createDamage(Damages* damages, Monster monster, int damageReceived) {
	Damage damage;
	damage.count = 0;
	damage.damageReceived = damageReceived;
	damage.position = monster.position;
	damage.yPos = monster.yPos;
	damage.durationCount = 0;

	damages->damage[damages->count++] = damage;
}

/**
* TODO: monsters->monster�� ť�� �ٲ� ����
* ���Ͱ� �÷��̾�� ��Ұų� �������� �޾� ���� ��� �����ϴ� �Լ�
* (����� ���� ù��°�� ��ȯ�� ���͸� �����ϵ��� �����Ǿ� ����)
*
* @params monster : ���� �迭�� �ּ� ��
* @params index : monster�� �ε��� ��
*/
void removeMonster(Monsters* monsters, int index) {
	monsters->startIndex++;
}

/**
* ���͸� �߰��ϴ� �Լ�
* @params monsters : ���� �迭
*/
void summonMonster(Monsters* monsters) {
	int position = rand() % 4 + 1;
	//enum monsterType type = (enum monsterType)(rand() % NUM_MONSTER_TYPE); 
	enum monsterType type = (enum monsterType)0; // �ӽ÷� ����̷� ����

	Monster monster;
	monster.yPos = MONSTER_INIT_YPOS;
	monster.type = type;
	monster.life = 20;
	monster.speed = 1;
	monster.speedCount = MONSTER_SPEED_CONST / monster.speed;
	monster.position = position;
	monster.size = 3;
	monster.damage = 2;
	monster.xp = 10;

	if (monsters->count + 1 >= monsters->maxMonster) {
		monsters->maxMonster *= 2;
		monsters->monster = (Monster*)realloc(monsters->monster, sizeof(Monster) * monsters->maxMonster);
	}
	monsters->monster[monsters->count++] = monster;
}

/**
* TODO : ��� Ÿ���� ���͸� ����ϵ��� ���� �ʿ�.
* ���͸� ����ϴ� �Լ�
* (����� �䳢�� ����ϵ��� ������)
*/
void printMonster(Monster monster) {
	gotoMonster(monster, -1);
	printf("       ");
	gotoMonster(monster, 0);
	printf("       ");
	//TODO : ���� ü�¿� ���缭 ���� �ʿ�
	//ü�� �� ���
	gotoMonster(monster, 0);
	printf(" ");
	for (int i = 0; i < monster.life; i += 4) {
		printf("-");
	}
	gotoMonster(monster, 1);
	printf(" /\\_/\\ ");
	gotoMonster(monster, 2);
	printf("( ");
	printf("\x1b[31m");  // ������ �ؽ�Ʈ
	printf("o");
	printf("\x1b[0m");   // ������ ���� ����(�⺻��)�� �ǵ���
	printf(".");
	printf("\x1b[31m");  // ������ �ؽ�Ʈ
	printf("o");
	printf("\x1b[0m");   // ������ ���� ����(�⺻��)�� �ǵ���
	printf(" )");
	gotoMonster(monster, 3);
	printf(" > ^ <");
	
}

/**
* ���� ����� ����� �Լ�
* @params monster : ����� ���� ���� ��ü
*/
void eraseMonster(Monster monster) {
	for (int i = -1; i < monster.size; i++) {
		gotoMonster(monster, i);
		printf("       ");
	}
}

/**
* ���͸� ��� ����ϴ� �Լ�
*/
void printMonsters(Player *player,Monsters* monsters) {
	for (int i = monsters->startIndex; i < monsters->count; i++) {
		if (MONSTER_SPEED_CONST / monsters->monster[i].speed == monsters->monster[i].speedCount) {
			if (monsters->monster[i].yPos + 1 >= PLAYER_YPOS - monsters->monster[i].size) {
				getDamage(player,monsters->monster[i].damage);
				removeMonster(monsters, i);
				eraseMonster(monsters->monster[i]);
			}
			else {
				printMonster(monsters->monster[i]);
				monsters->monster[i].speedCount = 0;
				monsters->monster[i].yPos++;
			}
		}
		else {
			monsters->monster[i].speedCount++;
		}
	}
}


/**
* �Ѿ��� ����ϴ� �Լ�. �Ѿ��� ��ġ�� ������Ʈ��.
* @params bullets : bullet �迭�� �ּ� ��
* @params monsters : ���Ϳ��� �Ѿ��� �´� �� Ȯ���ϱ� ���� monster �迭�� ����
* @params damages : �Ѿ��� ���Ϳ��� ������ ������ ��ü�� �߰��ϱ� ���� damages �迭 ����
*/
void printBullets(Player *player, Bullets* bullets, Monsters* monsters, Damages* damages) {

	// �������� ���� �Ѿ˵��� ������� ���
	for (int i = bullets->startIndex; i < bullets->count; i++) {
		// �Ѿ��� ù ��ġ�� �ƴϸ� ��� ����
		if (bullets->bullet[i].yPos != BULLET_INIT_YPOS) {
			eraseBullet(bullets->bullet[i]);
		}
		bullets->bullet[i].yPos -= 1;

		// pass ���� false�� �Ǹ� �ش� �Ѿ� ����
		boolean pass = true;
		// �Ѿ��� ���������� �Ѿ�� ����
		if (bullets->bullet[i].yPos < 3) {
			pass = false;
		}
		for (int j = monsters->startIndex; j < monsters->count; j++) {
			// ������ ��ġ�� �Ѿ��� ��ġ�� ��ġ�� ��� ������ ����, �Ѿ� ������� �ʵ��� pass�� false�� ����
			if (monsters->monster[j].position == bullets->bullet[i].position &&
				monsters->monster[j].yPos + monsters->monster[j].size - 1 >= bullets->bullet[i].yPos) {
				int damage = bullets->damage;
				monsters->monster[j].life -= damage;
				monsters->monster[j].damageReceived = damage;
				createDamage(damages, monsters->monster[j], damage);
				if (monsters->monster[j].life <= 0) {
					getXP(player, monsters->monster[j].xp);
					removeMonster(monsters, j);
					eraseMonster(monsters->monster[j]);
				}
				pass = false;
			}
		}
		if (pass == false) {
			eraseBullet(bullets->bullet[i]);
			removeBullet(bullets, i);
		}
		else {
			gotoBullet(bullets->bullet[i]);
			printf("o");
		}
	}
}

void removeDamages(Damages* damages, int index) {
	damages->startIndex++;
}

void eraseDamage(Damage damage) {
	gotoDamage(damage);
	printf(" ");
}

void printDamages(Damages* damages) {
	for (int i = damages->startIndex; i < damages->count; i++) {

		damages->damage[i].durationCount++;
		if (damages->damage[i].durationCount > DAMAGE_TIME_COUNT) {
			eraseDamage(damages->damage[i]);
			damages->damage[i].count++;
			gotoDamage(damages->damage[i]);
			printf("%d", damages->damage[i].damageReceived);
			damages->damage[i].durationCount = 0;
		}
		// �������� �� ĭ �ö� ������ ������ �� ����
		if (damages->damage[i].count > DAMAGE_TIME) {
			removeDamages(damages, i);
			eraseDamage(damages->damage[i]);
		}
	}
}

void printMenu() {
	for (int i = 0; i < 5; i++) {
		printf("\n");
	}
	printf("                   ��=============================================================================��\n");
	printf("                   ��       _          _                 _       _       __                       ��\n");
	printf("                   ��      / \\   _ __ (_)_ __ ___   __ _| |   __| | ___ / _| ___ _ __   ___ ___   ��\n");
	printf("                   ��     / _ \\ | '_ \\| | '_ ` _ \\ / _` | |  / _` |/ _ \\ |_ / _ \\ '_ \\ / __/ _ \\  �� \n");
	printf("                   ��    / ___ \\| | | | | | | | | | (_| | | | (_| |  __/  _|  __/ | | | (_|  __/  ��\n");
	printf("                   ��   /_/   \\_\\_| |_|_|_| |_| |_|\\__,_|_|  \\__,_|\\___|_|  \\___|_| |_|\\___\\___|  ��\n");
	printf("                   ��                                                                             ��\n");
	printf("                   ================================================================================\n");
	for (int i = 0; i < 6; i++) {
		printf("\n");
	}
	printf("                                                   ��   ���� ����\n\n");
	printf("                                                       ��ŷ ��ȸ\n\n");
	printf("                                                       ���� ����\n\n");
	for (int i = 0; i < 3; i++) {
		printf("\n");
	}
}


void selectMenu() {
	currentMode = selectedMenuIndex + 1;
	playSound(SELECT);
	if (currentMode == 1) {
		startGame();
	}
	else if (currentMode == 3) {
		exit(0);
	}
}

void moveMenuCursor() {
	if (GetAsyncKeyState(0x28) & 0x8000) {
			if (selectedMenuIndex == 2) return;
			playNaviSound();
			gotoxy(MENU_CURSOR_XPOS, 19 + 2 * (selectedMenuIndex));
			printf(" ");
			selectedMenuIndex += 1;
			gotoxy(MENU_CURSOR_XPOS, 19 + 2 * (selectedMenuIndex));
			printf("��");
	}
	else if (GetAsyncKeyState(0x26) & 0x8000) {
			if (selectedMenuIndex == 0) return;
			playNaviSound();
			gotoxy(MENU_CURSOR_XPOS, 19 + 2 * (selectedMenuIndex));
			printf(" ");
			selectedMenuIndex -= 1;
			gotoxy(MENU_CURSOR_XPOS, 19 + 2 * (selectedMenuIndex));
			printf("��");
	}
	else if (GetAsyncKeyState(0x0D) & 0x8000) {
		selectMenu();
	}
}

int main() {
	//Ŀ�� �����
	CONSOLE_CURSOR_INFO cursorInfo = { 0, };
	cursorInfo.dwSize = 1;
	cursorInfo.bVisible = FALSE;
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
	srand(time(NULL));

	// �÷��̾� �ʱ�ȭ 
	Player player = { 10,10,1,1,0 };

	// �ҷ� �ʱ�ȭ
	Bullets bullets;
	bullets.startIndex = 0;
	bullets.count = 0;
	bullets.maxBullet = 100;
	bullets.damage = 4;
	bullets.bullet = (Bullet*)malloc(sizeof(Bullet) * INIT_MAX_BULLET);

	// ���� �ʱ�ȭ
	Monsters monsters;
	monsters.startIndex = 0;
	monsters.count = 0;
	monsters.maxMonster = 100;
	monsters.monster = (Monster*)malloc(sizeof(Monster) * INIT_MAX_MONSTER);

	// ������ �迭 �ʱ�ȭ
	Damages damages;
	damages.count = 0;
	damages.startIndex = 0;
	damages.maxCount = 100;
	damages.damage = (Damage*)malloc(sizeof(Damage) * INIT_MAX_DAMAGE);

	int summonCount = SUMMON_DELAY_TIME;

	printMenu();
	playBgm();
	while (1) {
		switch (currentMode) {
		case 0:
			moveMenuCursor();
			break;

		case 1:
			movePlayer(&player);
			fireWeapon(&player, &bullets);
			if (summonCount == SUMMON_DELAY_TIME) {
				summonMonster(&monsters);
				summonCount = 0;
			}
			summonCount++;
			printBullets(&player,&bullets, &monsters, &damages);
			printMonsters(&player,&monsters);
			printDamages(&damages);
			printHP(&player);
			printXP(&player);
			printLevel(&player);
			Sleep(50);
			break;

		case 2:
			break;
		}
		//Sleep(100);
	}

}