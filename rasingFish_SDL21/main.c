#include "engine.h"

// 전역 변수 정의
Game game;                    // 게임 상태 구조체
FishTank fishTanks[NUM];    // 물고기 어항 배열
int level = 1;
int position = 0;
bool running = true;
bool gameOver = false;
bool gameWin = false;
long startTime = 0;
long lastUpdateTime = 0;
// SDL 관련 변수
SDL_Window* window = NULL;          // SDL 창
SDL_Renderer* renderer = NULL;      // SDL 렌더러
TTF_Font* font = NULL;              // 폰트
//SDL_Texture* fishTexture = NULL;    // 물고기 텍스처
SDL_Texture* fishTextures[FISH_TYPE_COUNT] = { NULL };
// 오디오 관련 변수
SDL_AudioDeviceID audioDevice = 0;  // 소리를 출력할 오디오 장치 식별자
SDL_AudioSpec wavSpec;              // wav 파일의 오디오 형식 정보
Uint8* wavBuffer = NULL;            // wav 파일의 실제 소리 데이터
Uint32 wavLength = 0;               // 소리 데이터의 길이, 단위는 바이트

extern Mix_Chunk* waterSound = NULL; // 물 주는 소리 효과를 위한 Mix_Chunk 구조체 포인터, 초기값은 NULL
extern Mix_Chunk* gameOverSound = NULL; // 게임 오버 소리 효과를 위한 Mix_Chunk 구조체 포인터, 초기값은 NULL
extern Mix_Music* playgameBGM = NULL; // 배경 음악을 위한 Mix_Music 구조체 포인터, 초기값은 NULL
extern Mix_Music* titleBGM = NULL; // 타이틀 화면 배경 음악을 위한 Mix_Chunk 구조체 포인터, 초기값은 NULL

// 메인 함수
int main(int argc, char* argv[]) {
    // 엔진 초기화
    if (!engine_init()) {
        printf("Error initializing engine: %s\n", SDL_GetError());  // 초기화 실패 시 에러 메시지 출력
        return 1;                   // 초기화 실패 시 프로그램 종료
    }

    //initGame();                     // 게임 초기화, 게임을 시작하기전에 시간이 진행되는 문제 방지 위해 게임 초기화는 메뉴 화면에서 엔터키를 눌렀을 때로 변경
    //title_BGM();
    while (running) {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = false;

            handleInput(&event);
        }

        if (game.state == STATE_MENU) {
            menuRender();
        }
        else if (game.state == STATE_PLAYING) {
            updateGame();
            renderGame();
        }
        else if (game.state == STATE_GAME_OVER) {
            cleanupGame();
            running = false;
        }

        SDL_Delay(100);
    }
    cleanupGame();

    return 0;
}