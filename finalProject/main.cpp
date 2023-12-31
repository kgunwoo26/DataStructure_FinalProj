﻿#define _CRT_SECURE_NO_WARNINGS

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

#define FILE_NAME "Rank_List.txt"

// 배경 음악
#define BGM_0 "./src/menu_bgm.mp3"
#define BGM_1 "./src/stage_bgm.mp3"
#define NAVI_MP3 "./src/navi.mp3"
#define SELECT_MP3 "./src/select.mp3"

// 레벨업 선택 메뉴 상수
#define LEVEL_UP_NUM 6
#define LEVEL_UP_SELECT_XPOS 10
#define LEVEL_UP_SELECT_YPOS 3
#define LEVEL_UP_SELECT_XPOS_GAP 35
#define LEVEL_UP_SELECT_TITLE_YPOS 15
#define LEVEL_UP_SELECT_LEVEL_YPOS  17
#define LEVEL_UP_SELECT_CONTENT_YPOS 19

// 메뉴 위치 상수
#define MENU_CURSOR_XPOS 51
#define MENU_STRING_XPOS 55

// 플레이어 상수
#define PLAYER_YPOS 26
#define PLAYER_INIT_XPOS 31

// 총알 상수
#define INIT_MAX_BULLET 100
#define BULLET_INIT_YPOS 24

// 몬스터 상수
#define INIT_MAX_MONSTER 100
#define MONSTER_INIT_YPOS 3
#define SUMMON_DELAY_TIME 60
#define NUM_MONSTER_TYPE 3
#define MONSTER_SPEED_CONST 10

// 데미지 상수
#define INIT_MAX_DAMAGE 100
#define DAMAGE_XPOS_CONST 3
#define DAMAGE_TIME 2
#define DAMAGE_TIME_COUNT 3

// 공통
#define INIT_XPOS 40
#define XPOS_GAP 14

enum color {
	RED = 31,
	WHITE = 37,
};

// 플레이어 색상 설정
color playerColor = WHITE;

// 몬스터 종류        
typedef enum monsterType
{
	CAT, COW, SPIDER
}MonsterType;

enum levelUpSelecttype 
{
	LIFE = 0,
	DAMAGE = 1,
	DEFENCE = 2,
	RELOAD = 3
};

// 효과음 종류
enum sound
{
	BGM, NAVI, SELECT, ATTACK, HIT
};

typedef enum weapons {
	PISTOL, RIFLE , SHOTGUN , SNIPER
}Weapon;

// 사용자 정보
struct User {
	char name[50];
	int score;
};


// 구조체 선언
struct LevelUpOptions {
	char title[20];
	int level;
	int size[2];
	char content[50];
	char picture[20][40];
};


typedef struct player {
	Weapon weapon;
	int armor;
	int fullHp;
	int hp;
	int position;
	int level;
	int xp;
}Player;

typedef struct monsterInfo {
	int life;
	int speed;
	int size;
	int damage;
	int xp;
	int point;
}MonsterInfo;

typedef struct monster {
	int position; // 스테이지 위치 1-4
	int yPos;
	MonsterType type;
	int speedCount;
	int damageReceived;
	int life;
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

MonsterInfo monsterInfoArray[NUM_MONSTER_TYPE] = {
	{16,1,3,2,20,100}, // CAT
	{28,1,4,4,15,200}, // COW
	{40,1,4,6,10,300}  // SPIDER
};

LevelUpOptions levelUpOptionsArray[LEVEL_UP_NUM] = {
	{" 라이프 증가",0,{25,7}," 라이프 +1",{
	"      ┌───┐",
	"      │   │",
	"  ┌───┘   └───┐",
	"  │           │",
	"  └───┐   ┌───┘",
	"      │   │",
	"      └───┘"
	}},
		{" 데미지 증가",0,{25,8}," 데미지 +1",{
	"    _________",
	"   [_________]",
	"    |  .-.  |",
	"    |,(o.o).|",
	"    | >|n|< |",
	"    |` `\"` `|",
	"    |DAMAGE!|",
	"    `\"\"\"\"\"\"\"`"
	}},
		{" 방어력 증가",0,{25,8}," 방어력 +1",{
	"     ________",
	"    / ______ \\",
	"   / /  ||  \\ \\",
	"  / /___||___\\ \\",
	"  \\ \\---||---/ /",
	"   \\ \\  ||  / / ",
	"    \\ \\_||_/ / ",
	"     \\______/ "
	}},
	{"공격 속도 증가",0,{25,9},"공격 속도 +1",{
	"  ____\\   ______",
	" /     \\ |      \\",
	"/______/ |____/  \\",
	"\\   \\ /     \\/   /",
	" \\  /        \\_ /_",
	"  \\/ ____   /\\",
	"    /  \\   /  \\",
	"   /\\   \\ /  /",
	"     \\___\\__/"
	}},
	{"무기 교체",5,{25,6},"  스나이퍼",{
	"",
	"",
	" ____)==/______,_",
	"/__.-^-|_|''`",
	"",
	""
	}},
	{"무기 교체",5,{25,6},"  라이플",{
	"",
	"",
	" ____)=======____,_",
	"/__.-^-|_|''`",
	"",
	""
	}}
};

HANDLE hMusicThread;
HANDLE hMenuThread;

// TODO: 몬스터, 총알을 링크드 리스트로 구현해도 될지 고민
int score = 0;

//파일
int maxUsers = 500;

// 현재 모드 0: 메뉴 1: 게임 플레이 2: 랭킹
int currentMode = 0;

// 메뉴 첫 항목으로 초기 값 지정
int selectedMenuIndex = 0;

// 랜덤으로 지정된 레벨업 선택 번호
int levelUpNum[3] = { 0,1,2 };

// 몬스터 종류 제한
int monsterTypeNumLimit = 1;

// 무기 선택 활성화 여부
bool weaponActivated = false;

// x,y 위치로 커서를 이동
void gotoxy(int x, int y) {
	COORD coord;
	coord.X = x;
	coord.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

// 사용자 정보를 파일에서 읽어오는 함수
int readFromFile(struct User* users, const char* filename) {
	FILE* file = fopen(filename, "r");
	if (file == NULL) {
		perror("Error opening file");
		exit(EXIT_FAILURE);
	}

	int numUsers = 0;
	while (fscanf(file, "%s %d", users[numUsers].name, &users[numUsers].score) == 2 && numUsers<=10) {
		numUsers++;
	}

	fclose(file);
	return numUsers;
}

// 사용자 정보를 파일에 저장하는 함수
void saveToFile(struct User* users, int numUsers, const char* filename) {
	FILE* file = fopen(filename, "w");
	if (file == NULL) {
		perror("Error opening file");
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < numUsers; ++i) {
		fprintf(file, "%s %d\n", users[i].name, users[i].score);
	}

	fclose(file);
}

// 사용자 정보를 점수가 높은 순으로 정렬하는 함수
void sortByScore(struct User* users, int numUsers) {
	for (int i = 0; i < numUsers - 1; ++i) {
		for (int j = 0; j < numUsers - i - 1; ++j) {
			if (users[j].score < users[j + 1].score) {
				// 현재 원소가 다음 원소보다 작으면 교환
				struct User temp = users[j];
				users[j] = users[j + 1];
				users[j + 1] = temp;
			}
		}
	}
}

// 사용자 정보 입력 및 정렬 및 랭킹 출력 함수
void showRanking(struct User* users, int maxUsers) {
	// 파일에서 기존 사용자 정보 읽어오기
	int numUsers = readFromFile(users, FILE_NAME);

	// 새로운 사용자 정보 입력 받기
	/*printf("이름을 입력해주세요! (점수): ");
	scanf("%s %d", users[numUsers].name, &users[numUsers].score);
	numUsers++;*/

	// 점수를 기준으로 사용자 정보 정렬
	sortByScore(users, numUsers);

	// 파일에 사용자 정보 저장
	saveToFile(users, numUsers, FILE_NAME);

	// 정렬된 사용자 정보 및 등수 출력
	printf("                                    ______   ___   _   _  _   __ _____  _   _  _____ \n");
	printf("                                    | ___ \\ / _ \\ | \\ | || | / /|_   _|| \\ | ||  __ \\ \n");
	printf("                                    | |_/ // /_\\ \\|  \\| || |/ /   | |  |  \\| || |  \\/ \n");
	printf("                                    |    / |  _  || . ` ||    \\   | |  | . ` || | __ \n");
	printf("                                    | |\\ \\ | | | || |\\  || |\\  \\ _| |_ | |\\  || |_\\ \\ \n ");
	printf("                                   \\_| \\_|\\_| |_/\\_| \\_/\\_| \\_/ \\___/ \\_| \\_/ \\____/ \n ");
	printf("\n\n");

	for (int i = 0; i < numUsers; ++i) {
		gotoxy(42, 8 + (2*i));
		printf("%d등 : %s", i+1, users[i].name);
		gotoxy(70, 8 + (2 * i));
		printf("%d점", users[i].score);
	}
	for (int i = numUsers; i < 10; i++) {
		gotoxy(42, 8 + (2 * i));
		printf("%d등 : OOOO", i);
		gotoxy(70, 8 + (2 * i));
		printf("OOOO점");
	}
	printf("\n\n\n< BackSpace   메인 화면으로");
}

void showRankingEnd(struct User* users, int maxUsers) {
	// 파일에서 기존 사용자 정보 읽어오기
	int numUsers = readFromFile(users, FILE_NAME);

	// 새로운 사용자 정보 입력 받기
	/*printf("이름을 입력해주세요! (점수): ");
	scanf("%s %d", users[numUsers].name, &users[numUsers].score);
	numUsers++;*/

	// 점수를 기준으로 사용자 정보 정렬
	sortByScore(users, numUsers);

	// 파일에 사용자 정보 저장
	saveToFile(users, numUsers, FILE_NAME);

	// 정렬된 사용자 정보 및 등수 출력
	printf("                                    ______   ___   _   _  _   __ _____  _   _  _____ \n");
	printf("                                    | ___ \\ / _ \\ | \\ | || | / /|_   _|| \\ | ||  __ \\ \n");
	printf("                                    | |_/ // /_\\ \\|  \\| || |/ /   | |  |  \\| || |  \\/ \n");
	printf("                                    |    / |  _  || . ` ||    \\   | |  | . ` || | __ \n");
	printf("                                    | |\\ \\ | | | || |\\  || |\\  \\ _| |_ | |\\  || |_\\ \\ \n ");
	printf("                                   \\_| \\_|\\_| |_/\\_| \\_/\\_| \\_/ \\___/ \\_| \\_/ \\____/ \n ");
	printf("\n\n");


	for (int i = 0; i < numUsers; ++i) {
		gotoxy(42, 8 + (2 * i));
		printf("%d등 : %s", i + 1, users[i].name);
		gotoxy(70, 8 + (2 * i));
		printf("%d점", users[i].score);
	}
	for (int i = numUsers; i < 10; i++) {
		gotoxy(42, 8 + (2 * i));
		printf("%d등 : OOOO", i);
		gotoxy(70, 8 + (2 * i));
		printf("OOOO점");
	}

	printf("\n\n\n< BackSpace 버튼을 입력하여 종료");
}

// player의 위치로 커서를 이동
void gotoPlayer(Player player, int xpos, int ypos) {
	COORD coord;
	coord.X = INIT_XPOS + xpos + ((player.position - 1) * XPOS_GAP);
	coord.Y = PLAYER_YPOS + ypos;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

// bullet의 위치로 커서를 이동
void gotoBullet(Bullet bullet) {
	COORD coord;
	coord.X = INIT_XPOS + ((bullet.position - 1) * XPOS_GAP);
	coord.Y = bullet.yPos;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

// monster의 위치에서 index 값 만큼 y 위치를 이동시킨 곳으로 커서를 이동
void gotoMonster(Monster monster, int index) {
	COORD coord;
	coord.X = INIT_XPOS + ((monster.position - 1) * XPOS_GAP) - 5;
	coord.Y = monster.yPos + index;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

// damage의 위치로 커서를 이동
void gotoDamage(Damage damage) {
	COORD coord;
	coord.X = INIT_XPOS + ((damage.position - 1) * XPOS_GAP) + DAMAGE_XPOS_CONST;
	coord.Y = damage.yPos - damage.count;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

// MP3 파일(filepath)을 재생
void PlayMP3(const char* filePath) {
	char command[256];
	int delay;
	sprintf_s(command, "open \"%s\" type mpegvideo alias mp3", filePath);
	// 재생하는 효과음에 따라 유지 시간 조정
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

// 음악 재생을 위한 쓰레드
unsigned __stdcall MusicThread(void* filePath) {
	PlayMP3((const char*)filePath);
	// 스레드 종료 후 핸들을 닫음
	_endthreadex(0);
	return 0;
}

// 배경 음악을 계속 재생하도록 하는 함수
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

// 배경 음악을 종료하는 함수
void stopBgm() {
	mciSendString("close mp3", NULL, 0, NULL);
}

// 메뉴 탐색 시 효과음 재생
void playNaviSound(void) {
	const char* musicFilePath = "./src/navi.mp3";
	hMusicThread = (HANDLE)_beginthreadex(NULL, 0, &MusicThread, (void*)musicFilePath, 0, NULL);
	// 스레드 종료 대기
	WaitForSingleObject(hMusicThread, INFINITE);
	// 스레드 핸들 닫기
	CloseHandle(hMusicThread);
}

// 메뉴 선택 시 글자 깜빡 거리도록 하는 쓰레드 함수
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

// 현재 선택된 메뉴를 구별하여 쓰레드 함수로 전달
void blinkSelectedMenu() {
	const char* menuString = "";
	switch (currentMode) {
	case 1: menuString = "게임 시작"; break;
	case 2: menuString = "랭킹 조회"; break;
	case 3: menuString = "게임 종료"; break;
	}
	hMenuThread = (HANDLE)_beginthreadex(NULL, 0, &MenuBlinkThread, (void*)menuString, 0, NULL);
	// 스레드 종료 대기
	WaitForSingleObject(hMusicThread, INFINITE);
	// 스레드 핸들 닫기
	CloseHandle(hMusicThread);
}


// 메뉴 선택시 효과음 재생하는 함수
void playSelectSound(void) {
	const char* musicFilePath = "./src/select.mp3";
	blinkSelectedMenu();
	hMusicThread = (HANDLE)_beginthreadex(NULL, 0, &MusicThread, (void*)musicFilePath, 0, NULL);
	// 스레드 종료 대기
	WaitForSingleObject(hMusicThread, INFINITE);
	stopBgm();
	// 스레드 핸들 닫기
	CloseHandle(hMusicThread);
}

// 상황에 맞는 음악 파일을 재생하도록 하는 함수
void playSound(sound type) {
	switch (type) {
	case BGM: playBgm(); break;
	case NAVI: playNaviSound(); break;
	case SELECT: playSelectSound();  break;
	case ATTACK: break;
	case HIT: break;
	}
}

void setRandomOption() {
	int num[3] = { NULL, };
	for (int i = 0; i < 3; i++) {
		int position = NULL;
		bool exist = true;
		while (exist) {
			position = rand() % LEVEL_UP_NUM;
			bool found = false;
			for (int j = 0; j < i; j++) {
				if (num[j] == position) {
					found = true;
					break;
				}
			}
			if (levelUpOptionsArray[position].level >= 5) {
				found = true;
			}
			if (!found) {
				exist = false;
				num[i] = position;
			}
		}
		levelUpNum[i] = position;
	}
}

void printLevelUpSelect() {
	// 레벨업 강화 랜덤 배정
	setRandomOption();
	// 프레임 출력
	for (int i = 0; i < 3; i++) {
		gotoxy(LEVEL_UP_SELECT_XPOS + LEVEL_UP_SELECT_XPOS_GAP * (i), LEVEL_UP_SELECT_YPOS);
		printf("┌──────────────────────────────┐\n");
		for (int j = 0; j < 21; j++) {
			gotoxy(LEVEL_UP_SELECT_XPOS + LEVEL_UP_SELECT_XPOS_GAP * (i), LEVEL_UP_SELECT_YPOS+j+1);
			printf("│                              │\n");
		}
		gotoxy(LEVEL_UP_SELECT_XPOS + LEVEL_UP_SELECT_XPOS_GAP * (i), LEVEL_UP_SELECT_YPOS+21);
		printf("└──────────────────────────────┘\n");

		// 컨텐츠 내용 출력

		gotoxy(LEVEL_UP_SELECT_XPOS + 9 + LEVEL_UP_SELECT_XPOS_GAP * (i), LEVEL_UP_SELECT_TITLE_YPOS);
		printf("%s ", levelUpOptionsArray[levelUpNum[i]].title);
		gotoxy(LEVEL_UP_SELECT_XPOS + 12 + LEVEL_UP_SELECT_XPOS_GAP * (i), LEVEL_UP_SELECT_LEVEL_YPOS);
		printf("레벨 %d", levelUpOptionsArray[levelUpNum[i]].level+1);
		gotoxy(LEVEL_UP_SELECT_XPOS + 10 + LEVEL_UP_SELECT_XPOS_GAP * (i), LEVEL_UP_SELECT_CONTENT_YPOS);
		printf("%s", levelUpOptionsArray[levelUpNum[i]].content);
		for (int j = 0; j < levelUpOptionsArray[levelUpNum[i]].size[1]; j++) {
			gotoxy(LEVEL_UP_SELECT_XPOS + 7 + LEVEL_UP_SELECT_XPOS_GAP * (i), LEVEL_UP_SELECT_YPOS + 2 + j);
			printf("%s", levelUpOptionsArray[levelUpNum[i]].picture[j]);
		}
	}
	gotoxy(88, 28);
	printf("레벨업 보상 선택: 방향키 ← ↓ →");
}

void printScore() {
	gotoxy(1, 29);
	printf("점수: %d점",score);
}

void printInfo() {
	int damage = levelUpOptionsArray[1].level;
	int armor = levelUpOptionsArray[2].level;
	int attackSpeed = levelUpOptionsArray[3].level;
	gotoxy(5, 8);
	printf("데미지  ");
	for (int i = 0; i < damage; i++) {
		printf("■");
	}
	for (int i = damage; i < 5; i++) {
		printf("□");
	}
	gotoxy(2, 10);
	printf("공격 속도  ");
	for (int i = 0; i < attackSpeed; i++) {
		printf("■");
	}
	for (int i = attackSpeed; i < 5; i++) {
		printf("□");
	}

	gotoxy(5, 12);
	printf("방어력  ");
	for (int i = 0; i < armor; i++) {
		printf("■");
	}
	for (int i = armor; i < 5; i++) {
		printf("□");
	}
}

// 유저의 hp를 출력하는 함수
void printHP(Player* player) {
	gotoxy(6, 6);
	printf("HP  ");
	for (int i = 0; i < player->fullHp + levelUpOptionsArray[0].level; i++) {
		if (player->hp > i) {
			printf("■");
		}
		else {
			printf("□");
		}
		
	}
}

// 유저의 레벨을 출력
void printLevel(Player* player) {
	gotoxy(3, 4);
	printf("레벨: %d", player->level);
}

// 유저의 경험치를 출력
void printXP(Player* player) {
	gotoxy(0, 0);
	printf("XP ");
	printf("|");
	for (int i = 1; i < 116; i++) {
		if (player->xp < i) {
			printf(" ");
		}
		else {
			printf("■");
		}
	}
	printf("|");
}

// 데미지를 받을 경우 플레이어의 HP를 감소시키고 HP를 새로고침
void getDamage(Player *player, int damageReceived) {
	player->hp -= damageReceived;
	printHP(player);
}

// 몬스터를 처치해 경험치를 얻으면 값을 변경하고 경험치 바를 새로고침
void getXP(Player* player, int xpReceived) {
	player->xp += xpReceived;
	if (player->xp > 115) {
		player->xp -= 115;
		player->level++;
		printLevel(player);
		printLevelUpSelect();
		currentMode = 4;
	}
	printXP(player);
}

// 프로그램 시작 시 스테이지의 모양을 출력
void printfStage() {
	gotoxy(0, 0);
	for (int i = 0; i < 4; i++) {
		printf("\n");
	}

	for (int i = 0; i < 21; i++) {
		printf("                                 │             │             │             │             │\n");
	}
	printf("                                 └─────────────┴─────────────┴─────────────┴─────────────┘");
	for (int i = 0; i < 3; i++) {
		printf("\n");
	}
}

void printWeapon(Weapon weapon) {
	gotoxy(106, 3);
	printf("현재 무기");
	if (weapon == PISTOL) {
		gotoxy(105, 4);
		printf("   ______.");
		gotoxy(105, 5);
		printf(" ~(_]----'");
		gotoxy(105, 6);
		printf(" /_( ");
		gotoxy(107, 8);
		printf("권총");
	}
	else if(weapon == RIFLE) {
		gotoxy(102, 5);
		printf(" ____)=======____,_");
		gotoxy(102, 6);
		printf("/__.-^-|_|''`");
		gotoxy(107, 8);
		printf("라이플");
	}
	else if (weapon == SNIPER) {
		gotoxy(102, 5);
		printf(" ____)==/______,_");
		gotoxy(102, 6);
		printf("/__.-^-|_|''`");
		gotoxy(107, 8);
		printf("저격총");
	}
	
}


// 게임 초기화 함수. 스테이지와 플레이어를 출력
void startGame(Player *player) {
	system("cls");
	playBgm();
	printfStage();
	printWeapon(player->weapon);
	gotoxy(INIT_XPOS, PLAYER_YPOS);
	printf("|\n");
	gotoxy(INIT_XPOS, PLAYER_YPOS+1);
	printf("o\n");
	gotoxy(INIT_XPOS-1, PLAYER_YPOS+2);
	printf("/_\\\n");
}

void reprintGame(Player* player) {
	system("cls");
	printHP(player);
	printfStage();
	printWeapon(player->weapon);
	gotoPlayer(*player, 0, 0);
	printf("\x1b[%dm", playerColor);
	printf("|\n");
	gotoPlayer(*player, 0, 1);
	printf("o\n");
	gotoPlayer(*player, -1, 2);
	printf("/_\\\n");
}

// 키 값을 입력받아 플레이어의 위치를 이동시킴. (좌우 방향키)
void movePlayer(Player* player) {
	static DWORD lastFireTime = 0; // 마지막 이동 시간을 추적

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
			printf("\x1b[%dm", playerColor);
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
* 총알을 생성하여 Bullets.bullet에 추가
* @params bullets : bullet 배열의 주소 값
* @params playerPos : 플레이어의 위치 (1-4)
*/
void createBullet(Bullets* bullets, int playerPos) {
	Bullet bullet = { playerPos, BULLET_INIT_YPOS };
	bullets->bullet[bullets->count++] = bullet;
}

/**
* TODO: bullets->bullet을 큐로 바꿀 예정
* bullets를 삭제
* ( 현재는 bullets->startIndex의 값을 +1 하여 가장 먼저 생성된 bullet을 삭제)
*/
void removeBullet(Bullets* bullets, int index) {
	bullets->startIndex++;
}

/**
* 출력된 총알을 출력 제거
*/
void eraseBullet(Bullet bullet) {
	gotoBullet(bullet);
	printf("  ");
}

/**
* 총알을 발사하는 함수. 스페이스 바를 누를 시 총알이 생성
* @params player : 플레이어의 주소 값
* @params bullets : bullet 배열의 주소 값
*/
void fireWeapon(Player* player, Bullets* bullets) {
	static DWORD lastFireTime = 0; // 마지막 발사 시간을 추적

	if (bullets->count + 1 >= bullets->maxBullet) {
		bullets->maxBullet *= 2;
		bullets->bullet = (Bullet*)realloc(bullets->bullet, sizeof(Bullet) * bullets->maxBullet);
	}

	// 현재 시간을 가져옴
	DWORD currentTime = GetTickCount();

	int weaponSpeed = 0;
	switch (player->weapon)
	{
	case 0:
		weaponSpeed = 500;
		break;
	case 1:
		weaponSpeed = 150;
		break;
	case 3:
		weaponSpeed = 500;
		break;
	default :
		weaponSpeed = 500;
		break;
	}

	if (GetAsyncKeyState(0x20) & 0x8000) {
		// 스페이스 바가 눌린 후 딜레이 이전에는 발사하지 않음
		if (currentTime - lastFireTime >= weaponSpeed - levelUpOptionsArray[3].level * 50) {
			createBullet(bullets, player->position);
			lastFireTime = currentTime; // 마지막 발사 시간 업데이트
		}
	}
}
/**
*  새로운 데미지 변수를 몬스터의 위치 바로 위로 생성
*  @params damages : damage 배열의 주소 값
*  @params monster : 데미지를 받은 monster
*  @params damageReceived : 받은 데미지 값
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
* TODO: monsters->monster을 큐로 바꿀 예정
* 몬스터가 플레이어에게 닿았거나 데미지를 받아 죽은 경우 삭제하는 함수
* @params monster : 몬스터 배열의 주소 값
* @params index : monster의 인덱스 값
*/
void removeMonster(Monsters* monsters, int index) {
	monsters->startIndex++;
}

/**
* 몬스터를 추가하는 함수
* @params monsters : 몬스터 배열
*/
void summonMonster(Monsters* monsters) {
	int position = rand() % 4 + 1;
	enum monsterType type = (enum monsterType)(rand() % monsterTypeNumLimit); 
	

	Monster monster;
	monster.yPos = MONSTER_INIT_YPOS;
	monster.type = type;
	monster.speedCount = MONSTER_SPEED_CONST / monsterInfoArray[type].speed;
	monster.position = position;
	monster.life = monsterInfoArray[type].life;

	if (monsters->count + 1 >= monsters->maxMonster) {
		monsters->maxMonster *= 2;
		monsters->monster = (Monster*)realloc(monsters->monster, sizeof(Monster) * monsters->maxMonster);
	}
	monsters->monster[monsters->count++] = monster;
}

/**
* TODO : 모든 타입의 몬스터를 출력하도록 수정 필요.
* 몬스터를 출력하는 함수
*/
void printMonster(Monster monster) {
	gotoMonster(monster, -1);
	printf("            ");
	gotoMonster(monster, 0);
	printf("            ");
	//TODO : 몬스터 체력에 맞춰서 수정 필요
	//체력 바 출력
	gotoMonster(monster, 0);
	if (monster.type == CAT) {
		printf("   ");
	}
	else if (monster.type == COW) {
		printf("  ");
	}
	else if (monster.type == SPIDER) {
		printf(" ");
	}
	for (int i = 0; i < monster.life; i += 4) {
		printf("-");
	}
	if (monster.type == CAT) {
		gotoMonster(monster, 1);
		printf("   /\\_/\\ ");
		gotoMonster(monster, 2);
		printf("  ( ");
		printf("\x1b[31m");  // 빨간색 텍스트
		printf("o");
		printf("\x1b[0m");   // 색상을 원래 상태(기본값)로 되돌림
		printf(".");
		printf("\x1b[31m");  // 빨간색 텍스트
		printf("o");
		printf("\x1b[0m");   // 색상을 원래 상태(기본값)로 되돌림
		printf(" )");
		gotoMonster(monster, 3);
		printf("   > ^ <");
	}
	else if (monster.type == COW) {
		gotoMonster(monster, 1);
		printf("((_,...,_))");
		gotoMonster(monster, 2);
		printf("   |");
		printf("\x1b[31m");  // 빨간색 텍스트
		printf("o");
		printf(" o");
		printf("\x1b[0m");   // 색상을 원래 상태(기본값)로 되돌림
		printf("|");
		gotoMonster(monster, 3);
		printf("   \\   /");
		gotoMonster(monster, 4);
		printf("    ^_^ ");
	}
	else if (monster.type == SPIDER) {
		gotoMonster(monster, 1);
		printf(" /\\ \\  / /\\ ");
		gotoMonster(monster, 2);
		printf("//\\\\");
		printf("\x1b[31m");  // 빨간색 텍스트
		printf(" ..");
		printf("\x1b[0m");   // 색상을 원래 상태(기본값)로 되돌림
		printf(" //\\\\");
		gotoMonster(monster, 3);
		printf("//\\((  ))/\\\\");
		gotoMonster(monster, 4);
		printf("/  < `' >  \\");
	}
	
}

void printRankInput() {
	char name[100] = { '0', };
	struct User* users = (struct User*)malloc(maxUsers * sizeof(struct User));
	int numUsers = readFromFile(users, FILE_NAME);
	int x = 45;
	int y = 10;
	gotoxy(x, y + 0);
	printf("┌─────────────────────────────────────┐\n");
	gotoxy(x, y + 1);
	printf("│                                     │\n");
	gotoxy(x, y + 2);
	printf("│                                     │\n");
	gotoxy(x, y + 3);
	printf("│                                     │\n");
	gotoxy(x, y + 4);
	printf("│                                     │\n");
	gotoxy(x, y + 5);
	printf("│                                     │\n");
	gotoxy(x, y + 6);
	printf("│                                     │\n");
	gotoxy(x, y + 7);
	printf("│                                     │\n");
	gotoxy(x, y + 8);
	printf("│                                     │\n");
	gotoxy(x, y + 9);
	printf("└─────────────────────────────────────┘\n");
	gotoxy(x + 3, y + 3);
	printf("랭킹에 등록할 이름을 입력하세요.");
	gotoxy(x + 3, y + 6);
	printf("이름 : ");
	scanf("%s", &name);
	strcpy(users[numUsers].name, name);
	users[numUsers].score = score;
	numUsers++;
	saveToFile(users, numUsers, FILE_NAME);
	if (users == NULL) {
		perror("error");
		exit(EXIT_FAILURE);
	}
	system("cls");
	showRankingEnd(users, maxUsers);
	while (1) {
		if (GetAsyncKeyState(0x08) & 0x8000) {
			exit(0);
			break;
		}
	}
}

void printGameOver() {
	int value = 35;
	int y = 7;
	gotoxy(value, y);
	printf(" ________  ________  _____ ______   _______");
	gotoxy(value, y + 1);
	printf("|\\   ____\\|\\   __  \\|\\   _ \\  _   \\|\\  ___ \\");
	gotoxy(value, y + 2);
	printf("\\ \\  \\___|\\ \\  \\|\\  \\ \\  \\\\\\__\\ \\  \\ \\   __/|");
	gotoxy(value, y + 3);
	printf(" \\ \\  \\  __\\ \\   __  \\ \\  \\\\|__| \\  \\ \\  \\_|/");
	gotoxy(value, y + 4);
	printf("  \\ \\  \\|\\  \\ \\  \\ \\  \\ \\  \\    \\ \\  \\ \\  \\_|\\ \\");
	gotoxy(value, y + 5);
	printf("   \\ \\_______\\ \\__\\ \\__\\ \\__\\    \\ \\__\\ \\_______\\");
	gotoxy(value, y + 6);
	printf("    \\|_______|\\|__|\\|__|\\|__|     \\|__|\\|_______|");
	gotoxy(value, y + 7);
	printf("       ________  ___      ___ _______   ________");
	gotoxy(value, y + 8);
	printf("      |\\   __  \\|\\  \\    /  /|\\  ___ \\ |\\   __  \\");
	gotoxy(value, y + 9);
	printf("      \\ \\  \\|\\  \\ \\  \\  /  / | \\   __/|\\ \\  \\|\\  \\");
	gotoxy(value, y + 10);
	printf("       \\ \\  \\\\\\  \\ \\  \\/  / / \\ \\  \\_|/_\\ \\   _  _\\");
	gotoxy(value, y + 11);
	printf("        \\ \\  \\\\\\  \\ \\    / /   \\ \\  \\_|\\ \\ \\  \\\\  \\|");
	gotoxy(value, y + 12);
	printf("         \\ \\_______\\ \\__/ /     \\ \\_______\\ \\__\\\\ _\\");
	gotoxy(value, y + 13);
	printf("          \\|_______|\\|__|/       \\|_______|\\|__|\\|__|");
	gotoxy(value, y + 14);
	printf("                                                                 ");
	gotoxy(value, y + 15);
	printf("                                Lctrl : 랭킹 이름 입력 ");
	while (1) {
		 if (GetAsyncKeyState(VK_LCONTROL) & 0x8000) {
			// 오른쪽 컨트롤 키가 눌렸을 때 수행할 작업
			 printRankInput();
			break;
		}
	}
	
}

/**
* 몬스터 출력을 지우는 함수
* @params monster : 출력을 지울 몬스터 객체
*/
void eraseMonster(Monster monster) {
	for (int i = -1; i < monsterInfoArray[monster.type].size; i++) {
		gotoMonster(monster, i);
		printf("            ");
	}
}

/**
* 몬스터를 모두 출력하는 함수
*/
void printMonsters(Player *player,Monsters* monsters) {
	for (int i = monsters->startIndex; i < monsters->count; i++) {
		if ((MONSTER_SPEED_CONST) / (monsterInfoArray[monsters->monster[i].type].speed )== monsters->monster[i].speedCount) {
			if (monsters->monster[i].yPos + 1 >= PLAYER_YPOS - monsterInfoArray[monsters->monster[i].type].size) {
				getDamage(player, monsterInfoArray[monsters->monster[i].type].damage - levelUpOptionsArray[2].level);
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
	if (player->hp <= 0) {
		printGameOver();
	}
}


/**
* 총알을 출력하는 함수. 총알의 위치를 업데이트함.
* @params bullets : bullet 배열의 주소 값
* @params monsters : 몬스터에게 총알이 맞는 지 확인하기 위해 monster 배열을 전달
* @params damages : 총알이 몬스터에게 맞으면 데미지 객체를 추가하기 위해 damages 배열 전달
*/
void printBullets(Player *player, Bullets* bullets, Monsters* monsters, Damages* damages) {

	// 삭제되지 않은 총알들을 순서대로 출력
	for (int i = bullets->startIndex; i < bullets->count; i++) {
		// 총알이 첫 위치가 아니면 출력 삭제
		if (bullets->bullet[i].yPos != BULLET_INIT_YPOS) {
			eraseBullet(bullets->bullet[i]);
		}
		bullets->bullet[i].yPos -= 1;

		// pass 값이 false가 되면 해당 총알 삭제
		boolean pass = true;
		int levelUp = 0;
		// 총알이 스테이지를 넘어가면 삭제
		if (bullets->bullet[i].yPos < 3) {
			pass = false;
		}
		for (int j = monsters->startIndex; j < monsters->count; j++) {
			// 몬스터의 위치가 총알의 위치와 일치할 경우 데미지 생성, 총알 출력하지 않도록 pass를 false로 변경
			if (monsters->monster[j].position == bullets->bullet[i].position &&
				((monsters->monster[j].yPos + monsterInfoArray[monsters->monster[j].type].size - 1 >= bullets->bullet[i].yPos)&&
					(monsters->monster[j].yPos <= bullets->bullet[i].yPos))) {
				int damage;
				damage = bullets->damage + levelUpOptionsArray[1].level * 2;
				
				monsters->monster[j].life -= damage;
				monsters->monster[j].damageReceived = damage;
				createDamage(damages, monsters->monster[j], damage );
				if (monsters->monster[j].life <= 0) {
					removeMonster(monsters, j);
					eraseMonster(monsters->monster[j]);
					//몬스터 처치 시 몬스터의 점수 + 플레이어와 몬스터 간의 거리 차 x 10를 점수로 얻음
					score += monsterInfoArray[monsters->monster[j].type].point + (PLAYER_YPOS - monsters->monster[j].yPos) * 10;
					levelUp = monsterInfoArray[monsters->monster[j].type].xp;
				}
				if (player->weapon != SNIPER) {
					pass = false;
				}
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

		// 총알 때문에 출력이 지워져서 맨 뒤로 뺌
		if (levelUp > 0) {
			getXP(player, levelUp);
		}
	}
}

void removeDamages(Damages* damages, int index) {
	damages->startIndex++;
}

void eraseDamage(Damage damage) {
	gotoDamage(damage);
	printf("  ");
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
		// 데미지가 몇 칸 올라갈 때까지 유지될 지 결정
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
	printf("                   ∥=============================================================================∥\n");
	printf("                   ∥       _          _                 _       _       __                       ∥\n");
	printf("                   ∥      / \\   _ __ (_)_ __ ___   __ _| |   __| | ___ / _| ___ _ __   ___ ___   ∥\n");
	printf("                   ∥     / _ \\ | '_ \\| | '_ ` _ \\ / _` | |  / _` |/ _ \\ |_ / _ \\ '_ \\ / __/ _ \\  ∥ \n");
	printf("                   ∥    / ___ \\| | | | | | | | | | (_| | | | (_| |  __/  _|  __/ | | | (_|  __/  ∥\n");
	printf("                   ∥   /_/   \\_\\_| |_|_|_| |_| |_|\\__,_|_|  \\__,_|\\___|_|  \\___|_| |_|\\___\\___|  ∥\n");
	printf("                   ∥                                                                             ∥\n");
	printf("                   ===============================================================================\n");
	for (int i = 0; i < 6; i++) {
		printf("\n");
	}
	printf("                                                   ▶   게임 시작\n\n");
	printf("                                                       랭킹 조회\n\n");
	printf("                                                       게임 종료\n\n");
	for (int i = 0; i < 3; i++) {
		printf("\n");
	}
	if (selectedMenuIndex != 0) {
		gotoxy(MENU_CURSOR_XPOS, 19 );
		printf(" ");
		gotoxy(MENU_CURSOR_XPOS, 19 + 2 * (selectedMenuIndex));
		printf("▶");
	}
}


void selectMenu(Player *player) {
	currentMode = selectedMenuIndex + 1;
	playSound(SELECT);
	if (currentMode == 1) {
		startGame(player);
	}
	else if (currentMode == 2) {
		playBgm();
		system("cls");
		// 동적 할당
		struct User* users = (struct User*)malloc(maxUsers * sizeof(struct User));
		if (users == NULL) {
			perror("error");
			exit(EXIT_FAILURE);
		}
		showRanking(users, maxUsers);
		free(users);
	}
	else if (currentMode == 3) {
		exit(0);
	}
}

void moveMenuCursor(Player *player) {
	if (GetAsyncKeyState(0x28) & 0x8000) {
			if (selectedMenuIndex == 2) return;
			playNaviSound();
			gotoxy(MENU_CURSOR_XPOS, 19 + 2 * (selectedMenuIndex));
			printf(" ");
			selectedMenuIndex += 1;
			gotoxy(MENU_CURSOR_XPOS, 19 + 2 * (selectedMenuIndex));
			printf("▶");
	}
	else if (GetAsyncKeyState(0x26) & 0x8000) {
			if (selectedMenuIndex == 0) return;
			playNaviSound();
			gotoxy(MENU_CURSOR_XPOS, 19 + 2 * (selectedMenuIndex));
			printf(" ");
			selectedMenuIndex -= 1;
			gotoxy(MENU_CURSOR_XPOS, 19 + 2 * (selectedMenuIndex));
			printf("▶");
	}
	else if (GetAsyncKeyState(0x0D) & 0x8000) {
		selectMenu(player);
	}
}

void moveLevelUpSelectCursor(Player *player) {
	// 왼쪽
	if (GetAsyncKeyState(0x25) & 0x8000) {
		if (levelUpNum[0] == 5) {
			levelUpOptionsArray[5].level = 5;
			player->weapon = RIFLE;
			levelUpOptionsArray[1].level = 0;
			levelUpOptionsArray[3].level = 0;
		}
		else if (levelUpNum[0] == 4) {
			levelUpOptionsArray[4].level = 5;
			player->weapon = SNIPER;
			levelUpOptionsArray[1].level = 0;
			levelUpOptionsArray[3].level = 0;
		}
		else {
			levelUpOptionsArray[levelUpNum[0]].level++;
		}
		if (levelUpOptionsArray[levelUpNum[0]].level >= 5) {
			if (weaponActivated == false) {
				levelUpOptionsArray[4].level = 0;
				levelUpOptionsArray[5].level = 0;
				weaponActivated = true;
			}
		}
		currentMode = 1;
		reprintGame(player);
	}
	// 아래
	else if (GetAsyncKeyState(0x28) & 0x8000) {
		if (levelUpNum[1] == 5 ) {
			levelUpOptionsArray[5].level = 5;
			player->weapon = RIFLE;
			levelUpOptionsArray[1].level = 0;
			levelUpOptionsArray[3].level = 0;
		}
		else if (levelUpNum[1] == 4 ) {
			levelUpOptionsArray[4].level = 5;
			player->weapon = SNIPER;
			levelUpOptionsArray[1].level = 0;
			levelUpOptionsArray[3].level = 0;
		}
		else {
			levelUpOptionsArray[levelUpNum[1]].level++;
		}
		if (levelUpOptionsArray[levelUpNum[1]].level >= 5) {
			if (weaponActivated == false) {
				levelUpOptionsArray[4].level = 0;
				levelUpOptionsArray[5].level = 0;
				weaponActivated = true;
			}
		}
		currentMode = 1;
		reprintGame(player);
	}
	// 오른쪽
	else if (GetAsyncKeyState(0x27) & 0x8000) {
		if (levelUpNum[2] == 5) {
			levelUpOptionsArray[5].level = 5;
			player->weapon = RIFLE;
			levelUpOptionsArray[1].level = 0;
			levelUpOptionsArray[3].level = 0;
		}
		else if (levelUpNum[2] == 4) {
			levelUpOptionsArray[4].level = 5;
			player->weapon = SNIPER;
			levelUpOptionsArray[1].level = 0;
			levelUpOptionsArray[3].level = 0;
		}
		else {
			levelUpOptionsArray[levelUpNum[2]].level++;
		}
		if (levelUpOptionsArray[levelUpNum[2]].level >= 5) {
			if (weaponActivated == false) {
				levelUpOptionsArray[4].level = 0;
				levelUpOptionsArray[5].level = 0;
				weaponActivated = true;
			}
		}
		currentMode = 1;
		reprintGame(player);
	}
}

int main() {
	//커서 숨기기
	CONSOLE_CURSOR_INFO cursorInfo = { 0, };
	cursorInfo.dwSize = 1;
	cursorInfo.bVisible = FALSE;
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
	srand(time(NULL));

	// 게임 시작 시간 저장
	DWORD startTime = GetTickCount();

	// 몬스터 타입 증가 시간 간격
	const DWORD monsterTypeIncreaseInterval = 40000; // 40초


	// 플레이어 초기화 
	Player player = { PISTOL,0,10,10,1,1,0 };

	// 불렛 초기화
	Bullets bullets;
	bullets.startIndex = 0;
	bullets.count = 0;
	bullets.maxBullet = 100;
	bullets.damage = 4;
	bullets.bullet = (Bullet*)malloc(sizeof(Bullet) * INIT_MAX_BULLET);

	// 몬스터 초기화
	Monsters monsters;
	monsters.startIndex = 0;
	monsters.count = 0;
	monsters.maxMonster = 100;
	monsters.monster = (Monster*)malloc(sizeof(Monster) * INIT_MAX_MONSTER);

	// 데미지 배열 초기화
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
			moveMenuCursor(&player);
			break;

		case 1:
			movePlayer(&player);
			fireWeapon(&player, &bullets);
			if (summonCount == SUMMON_DELAY_TIME) {
				summonMonster(&monsters);
				summonCount = 0;
			}
			summonCount++;
			printMonsters(&player, &monsters);
			printDamages(&damages);
			printHP(&player);
			printScore();
			printXP(&player);
			printInfo();
			printLevel(&player);
			printBullets(&player, &bullets, &monsters, &damages);
			Sleep(50);
			break;

		case 2:
			if (GetAsyncKeyState(0x08) & 0x8000) {
				currentMode = 0;
				system("cls");
				printMenu();
				playBgm();
			}
			break;
		case 4:
			moveLevelUpSelectCursor(&player);
			break;
		}
		if (currentMode == 1) {
			DWORD currentTime = GetTickCount();
			if (currentTime - startTime >= monsterTypeIncreaseInterval && monsterTypeNumLimit < 3) {
				if (monsterTypeNumLimit < 3) {
					// 몬스터 타입 증가
					monsterTypeNumLimit++;
				}
				for (int i = 0; i < 3; i++) {
					monsterInfoArray[i].speed++;
				}
				// 다음 증가를 위한 시간 업데이트
				startTime = currentTime;
				gotoxy(0, 1);
				printf("%d", monsterTypeNumLimit); 
			}
		}
	}

}


