#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "engine.h"

#define NUM 6
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define FISHTANK_WIDTH 100
#define FISHTANK_HEIGHT 200
#define FISH_TYPE_COUNT	4

// 프로그램의 상태 개념 도입
// 게임 상태를 나타내는 열거형 정의
typedef enum {
	STATE_MENU,				// 메뉴 상태
	STATE_PLAYING,				// 게임 플레이 상태
	STATE_GAME_OVER,			// 게임 오버 상태
	STATE_WIN					// 승리 상태
} GameState;
// 물고기 종류 열거형 정의
typedef enum {
	NORMAL,					// 일반 물고기
	FAST,					// 빠른 물고기
	BIG,					// 큰 물고기
	SPECIAL					// 특별한 물고기
} FishType;
// 게임 상태 구조체 정의
typedef struct {
	FishType type; // 물고기 종류
	int fish;				// 물고기 크기 (0~100)
	int water;				// 물 높이 (0~100)
	int isAlive;			// 1: alive, 0: dead
	double growth;				// 성장 단계 (0, 1, 2)
	int waterUse;			// 물 소비량 (레벨과 물고기 크기에 따라 달라짐)
	SDL_Texture* texture;	// 물고기 텍스처
} FishTank;

// 개념 전체 구조체 정의
typedef struct {
	GameState state;			// 현재 게임 상태
	//	FishTank fishTanks[NUM];    // 물고기 어항 배열
	int score;					// 점수
	int level;					// 레벨
	int lives;					// 남은 목숨
} Game;

// 전역 변수 정의
extern Game game;                    // 게임 상태 구조체
extern FishTank fishTanks[NUM];    // 물고기 어항 배열
extern int level;
extern int position;
extern bool running;
extern bool gameOver;
extern bool gameWin;
extern long startTime;
extern long lastUpdateTime;
// SDL 관련 변수
extern SDL_Window* window;          // SDL 창
extern SDL_Renderer* renderer;      // SDL 렌더러
extern TTF_Font* font;              // 폰트
extern SDL_Texture* fishTexture;    // 물고기 텍스처
extern SDL_Texture* fishTextures[FISH_TYPE_COUNT];
// 오디오 관련 변수
extern SDL_AudioDeviceID audioDevice;  // 소리를 출력할 오디오 장치 식별자
extern SDL_AudioSpec wavSpec;              // wav 파일의 오디오 형식 정보
extern Uint8* wavBuffer;            // wav 파일의 실제 소리 데이터
extern Uint32 wavLength;               // 소리 데이터의 길이, 단위는 바이트
extern Mix_Chunk* waterSound; // 물 주는 소리 효과를 위한 Mix_Chunk 구조체 포인터, 초기값은 NULL
extern Mix_Music* playgameBGM; // 배경 음악을 위한 Mix_Music 구조체 포인터, 초기값은 NULL
extern Mix_Chunk* gameOverSound; // 게임 오버 소리 효과를 위한 Mix_Chunk 구조체 포인터, 초기값은 NULL
extern Mix_Music* titleBGM; // 타이틀 화면 배경 음악을 위한 Mix_Music 구조체 포인터, 초기값은 NULL



// 함수 프로토타입 선언
void menuRender();                   // 메뉴 화면 렌더링 함수
bool engine_init();                 // 엔진 초기화 함수
void initGame();                    // 게임 초기화 함수
void renderText(const char* text, int x, int y); // 텍스트 렌더링 함수
void renderFishTanks();             // 어항 렌더링 함수
void updateGame();                  // 게임 상태 업데이트 함수
void renderGame();                  // 게임 렌더링 함수
void cleanupGame();                 // 게임 종료 및 자원 해제 함수
void handleInput(SDL_Event* e);     // 입력 처리 함수
SDL_Texture* loadTexture(const char* path); // 텍스처 로드 함수

bool initAudio();                   // 오디오 초기화 함수
void playWaterSound();              // 물 주는 소리 재생 함수
void playBGM();                     // 배경 음악 재생 함수
void playGameOverSound();           // 게임 오버 소리 재생 함수
void title_BGM();