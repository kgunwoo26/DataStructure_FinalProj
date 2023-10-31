#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <conio.h>
#include<time.h>

// TODO: 몬스터, 총알을 링크드 리스트로 구현해도 될지 고민

// 플레이어 상수
#define PLAYER_YPOS 25
#define PLAYER_INIT_XPOS 1

// 총알 상수
#define INIT_MAX_BULLET 100
#define BULLET_INIT_YPOS 23

// 몬스터 상수
#define INIT_MAX_MONSTER 100
#define MONSTER_INIT_YPOS 2
#define SUMMON_DELAY_TIME 100
#define NUM_MONSTER_TYPE 3
#define MONSTER_SPEED_CONST 10

// 데미지 상수
#define INIT_MAX_DAMAGE 100
#define DAMAGE_XPOS_CONST 3
#define DAMAGE_TIME 2
#define DAMAGE_TIME_COUNT 3

// 공통
#define INIT_XPOS 10
#define XPOS_GAP 14

// 몬스터 종류
enum monsterType
{
	CAT, COW, SPIDER
};

// 구조체 선언
typedef struct player {
	int position;
}Player;

typedef struct monster {
	int position; // 스테이지 위치 1-4
	int yPos;
	monsterType type;
	int life;
	int speed;
	int speedCount;
	int size;
	int damageReceived;
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

// player의 위치로 커서를 이동
void gotoPlayer(Player player) {
	COORD coord;
	coord.X = INIT_XPOS + ((player.position - 1) * XPOS_GAP);
	coord.Y = PLAYER_YPOS;
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
	coord.X = INIT_XPOS + ((monster.position - 1) * XPOS_GAP) - 3;
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

// x,y 위치로 커서를 이동
void gotoxy(int x, int y) {
	COORD coord;
	coord.X = x;
	coord.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

// 프로그램 시작 시 스테이지의 모양을 출력
void printfStage() {
	for (int i = 0; i < 3; i++) {
		printf("\n");
	}

	for (int i = 0; i < 21; i++) {
		printf("   │             │             │             │             │\n");
	}
	printf("   └─────────────┴─────────────┴─────────────┴─────────────┘");
	for (int i = 0; i < 3; i++) {
		printf("\n");
	}
}

// 게임 초기화 함수. 스테이지와 플레이어를 출력
void initGame() {
	printfStage();
	gotoxy(INIT_XPOS, PLAYER_YPOS);
	printf("@");
}

// 키 값을 입력받아 플레이어의 위치를 이동시킴. (좌우 방향키)
void movePlayer(Player* player) {
	static DWORD lastFireTime = 0; // 마지막 이동 시간을 추적

	DWORD currentTime = GetTickCount();

	if (GetAsyncKeyState(0x25) & 0x8000) {
		if (currentTime - lastFireTime >= 100) {
			if (player->position - 1 < 1) return;
			gotoPlayer(*player);
			printf("  ");

			player->position -= 1;
			gotoPlayer(*player);
			printf("@");
			lastFireTime = currentTime;
		}
	}
	if (GetAsyncKeyState(0x27) & 0x8000) {
		if (currentTime - lastFireTime >= 100) {
			if (player->position + 1 > 4) return;
			gotoPlayer(*player);
			printf("  ");

			player->position += 1;
			gotoPlayer(*player);
			printf("@");
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

	if (GetAsyncKeyState(0x20) & 0x8000) {
		// 스페이스 바가 눌린 후 딜레이 이전에는 발사하지 않음
		if (currentTime - lastFireTime >= 500) {
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
* (현재는 가장 첫번째로 소환된 몬스터를 삭제하도록 구현되어 있음)
*
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
	//enum monsterType type = (enum monsterType)(rand() % NUM_MONSTER_TYPE); 
	enum monsterType type = (enum monsterType)0; // 임시로 고양이로 고정

	Monster monster;
	monster.yPos = MONSTER_INIT_YPOS;
	monster.type = type;
	monster.life = 20;
	monster.speed = 1;
	monster.speedCount = MONSTER_SPEED_CONST / monster.speed;
	monster.position = position;
	monster.size = 3;

	if (monsters->count + 1 >= monsters->maxMonster) {
		monsters->maxMonster *= 2;
		monsters->monster = (Monster*)realloc(monsters->monster, sizeof(Monster) * monsters->maxMonster);
	}
	monsters->monster[monsters->count++] = monster;
}

/**
* TODO : 모든 타입의 몬스터를 출력하도록 수정 필요.
* 몬스터를 출력하는 함수
* (현재는 토끼를 출력하도록 구현됨)
*/
void printMonster(Monster monster) {
	gotoMonster(monster, -1);
	printf("       ");
	gotoMonster(monster, 0);
	printf("       ");
	//TODO : 몬스터 체력에 맞춰서 수정 필요
	//체력 바 출력
	gotoMonster(monster, 0);
	printf(" ");
	for (int i = 0; i < monster.life; i += 4) {
		printf("-");
	}
	gotoMonster(monster, 1);
	printf(" /\\_/\\ ");
	gotoMonster(monster, 2);
	printf("( o.o )");
	gotoMonster(monster, 3);
	printf(" > ^ <");
}

/**
* 몬스터 출력을 지우는 함수
* @params monster : 출력을 지울 몬스터 객체
*/
void eraseMonster(Monster monster) {
	for (int i = -1; i < monster.size; i++) {
		gotoMonster(monster, i);
		printf("       ");
	}
}

/**
* 몬스터를 모두 출력하는 함수
*/
void printMonsters(Monsters* monsters) {
	for (int i = monsters->startIndex; i < monsters->count; i++) {
		if (MONSTER_SPEED_CONST / monsters->monster[i].speed == monsters->monster[i].speedCount) {
			if (monsters->monster[i].yPos + 1 >= PLAYER_YPOS - monsters->monster[i].size) {
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
* 총알을 출력하는 함수. 총알의 위치를 업데이트함.
* @params bullets : bullet 배열의 주소 값
* @params monsters : 몬스터에게 총알이 맞는 지 확인하기 위해 monster 배열을 전달
* @params damages : 총알이 몬스터에게 맞으면 데미지 객체를 추가하기 위해 damages 배열 전달
*/
void printBullets(Bullets* bullets, Monsters* monsters, Damages* damages) {

	// 삭제되지 않은 총알들을 순서대로 출력
	for (int i = bullets->startIndex; i < bullets->count; i++) {
		// 총알이 첫 위치가 아니면 출력 삭제
		if (bullets->bullet[i].yPos != BULLET_INIT_YPOS) {
			eraseBullet(bullets->bullet[i]);
		}
		bullets->bullet[i].yPos -= 1;

		// pass 값이 false가 되면 해당 총알 삭제
		boolean pass = true;
		// 총알이 스테이지를 넘어가면 삭제
		if (bullets->bullet[i].yPos < 3) {
			pass = false;
		}
		for (int j = monsters->startIndex; j < monsters->count; j++) {
			// 몬스터의 위치가 총알의 위치와 일치할 경우 데미지 생성, 총알 출력하지 않도록 pass를 false로 변경
			if (monsters->monster[j].position == bullets->bullet[i].position &&
				monsters->monster[j].yPos + monsters->monster[j].size - 1 >= bullets->bullet[i].yPos) {
				int damage = bullets->damage;
				monsters->monster[j].life -= damage;
				monsters->monster[j].damageReceived = damage;
				createDamage(damages, monsters->monster[j], damage);
				if (monsters->monster[j].life <= 0) {
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
	printf("                   ================================================================================\n");
	for (int i = 0; i < 5; i++) {
		printf("\n");
	}
	printf("                                                  ▶   게임 시작\n\n");
	printf("                                                       랭킹 조회\n\n");
	printf("                                                       게임 종료\n\n");
}

int main() {
	//커서 숨기기
	CONSOLE_CURSOR_INFO cursorInfo = { 0, };
	cursorInfo.dwSize = 1;
	cursorInfo.bVisible = FALSE;
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);

	srand(time(NULL));

	// 메뉴 첫 항목으로 초기 값 지정
	int selectedMenuIndex = 0;

	// 플레이어 초기화 
	Player player = { PLAYER_INIT_XPOS };

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

	//initGame();
	int summonCount = SUMMON_DELAY_TIME;

	//while (1) {
	//	movePlayer(&player);
	//	fireWeapon(&player, &bullets);

	//	if (summonCount == SUMMON_DELAY_TIME) {
	//		summonMonster(&monsters);
	//		summonCount = 0;
	//	}
	//	summonCount++;

	//	printBullets(&bullets, &monsters, &damages);
	//	printMonsters(&monsters);
	//	printDamages(&damages);

	//	Sleep(50);
	//}
	printMenu();
	while (1) {
		if (GetAsyncKeyState(0x26) & 0x8000) {
			selectedMenuIndex;
		}
		if (GetAsyncKeyState(0x28) & 0x8000) {

		}
	}



}