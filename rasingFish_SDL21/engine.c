#include <stdio.h>
#include <SDL.h>
#include <SDL_ttf.h>   
#include <SDL_mixer.h>
#include <stdbool.h>			
#include "engine.h"

// 함수 정의

bool engine_init()
{
    // SDL 초기화, 비디오와 오디오 서브시스템 초기화
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
        return 0;
    // 창 생성 및 렌더러 초기화
    window = SDL_CreateWindow("Raising Fishes", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!window)
        return 0;
    // TTF 초기화 및 폰트 로드
    if (TTF_Init() != 0)
        return 0;
    font = TTF_OpenFont("C:\\Windows\\Fonts\\arial.ttf", 20);       // 폰트 파일 경로(절대 혹은 상대) 필요
    if (!font) {
        printf("폰트 로드 실패: %s\n", TTF_GetError());
        SDL_Quit();
        return 0;
    }
    // 물고기 텍스처 로드
    //fishTexture = loadTexture("fish.bmp");          // 물고기 이미지 파일 경로 필요
    fishTextures[NORMAL] = loadTexture("normalfish.bmp");       // 일반 물고기 텍스처 로드
    fishTextures[FAST] = loadTexture("fastfish.bmp");           // 빠른 물고기 텍스처 로드
    fishTextures[BIG] = loadTexture("bigfish.bmp");             // 큰 물고기    텍스처 로드
    fishTextures[SPECIAL] = loadTexture("specialfish.bmp");     // 특별한 물고기 텍스처 로드
    for (int i = 0; i < FISH_TYPE_COUNT; i++) {
        if (!fishTextures[i]) {                     // 텍스처 로드 실패 시 에러 메시지 출력
            printf("물고기 텍스처 로드 실패: %s\n", SDL_GetError());
            SDL_Quit();
            return 0;
        }
    }
    /*
    if (!fishTexture) {                             // 텍스처 로드 실패 시 에러 메시지 출력
        SDL_Quit();
        return 0;
    }
    */
    // 오디오 초기화
    if (!initAudio()) {                            // 오디오 초기화 실패 시 에러 메시지 출력
        SDL_Quit();
        return 0;
    }

    title_BGM();

    return 1;
}

// 오디오 초기화 함수 정의
bool initAudio()
{
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("SDL_mixer 초기화 실패: %s\n", Mix_GetError());
        return false;
    }

    waterSound = Mix_LoadWAV("water.wav");
    gameOverSound = Mix_LoadWAV("gameover.wav");

	playgameBGM = Mix_LoadMUS("playgame_bgm.wav");
	titleBGM = Mix_LoadMUS("title_bgm.wav");

    if (!waterSound || !playgameBGM || !gameOverSound) {
        printf("사운드 파일 로드 실패: %s\n", Mix_GetError());
        return false;
    }

    Mix_VolumeMusic(24);
    return true;
}
// 물 주는 소리 재생 함수 정의
void playWaterSound()
{
    if (waterSound != NULL)
    {
        Mix_PlayChannel(-1, waterSound, 0);
    }
}

void playGameOverSound()
{
    if (gameOverSound != NULL)
    {
        Mix_PlayChannel(-1, gameOverSound, 0); // -1: 가능한 모든 채널에서 재생, 0: 반복 없음
    }
}

void playBGM()
{
    if (playgameBGM != NULL)
    {
        Mix_PlayMusic(playgameBGM, -1); // -1: 가능한 모든 채널에서 재생, -1: 무한 반복
    }
}

void title_BGM()
{
    if (titleBGM != NULL)
    {
        Mix_PlayMusic(titleBGM, -1); // -1: 가능한 모든 채널에서 재생, -1: 무한 반복
    }
}

void initGame() {
    game.state = STATE_MENU;                      // 게임 상태 초기화
    srand((unsigned int)time(NULL));
    for (int i = 0; i < NUM; i++) {
        //fishTanks[i].texture = fishTexture;
        // 랜덤하게 물고기 타입 할당,물고기 타입에 따라 물의 소비량도 달라지도록 설정
        FishType type = (FishType)(rand() % FISH_TYPE_COUNT); // 0~3 사이의 랜덤한 타입 생성, (FishType)로 형변환하여 FishType 열거형으로 사용     
        fishTanks[i].type = type; // 물고기 타입 설정
        fishTanks[i].fish = 30;              // 초기 물고기 크기 설정
        fishTanks[i].water = 75;             // 초기 물 높이 설정
        fishTanks[i].isAlive = 1;            // 물고기 생존 상태 초기화
        fishTanks[i].growth = 0;             // 성장 단계 초기화
        fishTanks[i].waterUse = (type + 1) * 2; // 물 소비량 설정 (타입에 따라 2, 4, 6, 8)
        fishTanks[i].texture = fishTextures[type];  // 해당 타입에 맞는 텍스처 할당
    }
    startTime = SDL_GetTicks();             // 게임 시작 시간 기록
    lastUpdateTime = startTime;             // 마지막 업데이트 시간 초기화
}

void renderGame() {                         // 게임 화면 렌더링
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);     // 배경을 검은색으로 설정
    SDL_RenderClear(renderer);                          // 화면 초기화
    renderFishTanks();                      // 어항과 물고기 렌더링

    char levelText[64];                     // 레벨 텍스트 렌더링
    sprintf_s(levelText, sizeof(levelText), "Level %d", level);     // 현재 레벨 표시
    renderText(levelText, 10, 10);          // 게임 상태 텍스트 렌더링

    SDL_RenderPresent(renderer);            // 렌더링 업데이트
}

void cleanupGame() {
    // 종료 메시지 화면
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);             // 배경을 검은색으로 설정
    SDL_RenderClear(renderer);                                  // 화면 초기화
    if (gameWin) {
        renderText("You Win! All levels completed!", 200, 200); // 승리 메시지 렌더링
    }
    else if (gameOver) {
        playGameOverSound(); // 게임 오버 사운드 재생
        renderText("Game Over! All fish are dead!", 200, 200); // 게임 오버 메시지 렌더링
    }
    else {
        renderText("Game Over", 200, 200);      // 일반 게임 오버 메시지 렌더링
    }
    SDL_RenderPresent(renderer);                // 렌더링 업데이트   
    SDL_Delay(3000);                            // 메시지 표시 후 3초 대기

    //SDL_DestroyTexture(fishTexture);            // 물고기 텍스처 메모리 해제
    for (int i = 0; i < FISH_TYPE_COUNT; i++) {
        if (fishTextures[i] != NULL) {
            SDL_DestroyTexture(fishTextures[i]); // 물고기 타입별 텍스처 메모리 해제
        }
    }
    //if (audioDevice != 0) {
    //    SDL_CloseAudioDevice(audioDevice);      // 오디오 장치 닫기
    //}

    //if (wavBuffer != NULL) {
    //    SDL_FreeWAV(wavBuffer);                 // WAV 버퍼 메모리 해제
    //}

    if (waterSound) Mix_FreeChunk(waterSound);
    if (gameOverSound) Mix_FreeChunk(gameOverSound); // 해제
    if (playgameBGM) Mix_FreeMusic(playgameBGM);                 // 음악 해제
    if (titleBGM) Mix_FreeMusic(titleBGM);                 // 음악 해제


	Mix_CloseAudio();                            // 오디오 시스템 종료
    TTF_CloseFont(font);                        // 폰트 메모리 해제
    SDL_DestroyRenderer(renderer);              // 렌더러 메모리 해제
    SDL_DestroyWindow(window);                  // 창 메모리 해제
    TTF_Quit();                                 // TTF 종료
    SDL_Quit();                                 // SDL 종료   
}

void renderText(const char* text, int x, int y) {           // 텍스트 렌더링 함수
    SDL_Color color = { 255, 255, 255 };                    // 흰색 텍스트 색상
    SDL_Surface* surface = TTF_RenderText_Solid(font, text, color);         // 텍스트를 표면으로 렌더링
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface); // 표면에서 텍스처 생성
    SDL_Rect dest = { x, y, surface->w, surface->h };       // 텍스처를 화면에 렌더링, 위치와 크기 설정
    SDL_RenderCopy(renderer, texture, NULL, &dest);         // 텍스처 렌더링
    SDL_FreeSurface(surface);                               // 표면 메모리 해제
    SDL_DestroyTexture(texture);                            // 텍스처 메모리 해제
}

void renderFishTanks() {                        // 어항과 물고기 렌더링 함수
    for (int i = 0; i < NUM; i++) {
        int x = 50 + i * (FISHTANK_WIDTH + 10); 
        SDL_Rect bowl = { x, 300, FISHTANK_WIDTH, FISHTANK_HEIGHT };
        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); // 파란색 테두리
        SDL_RenderDrawRect(renderer, &bowl);

        // 물 높이 표시
        int waterHeight = fishTanks[i].water * FISHTANK_HEIGHT / 100;
        SDL_Rect water = { x, 300 + FISHTANK_HEIGHT - waterHeight, FISHTANK_WIDTH, waterHeight };
        SDL_SetRenderDrawColor(renderer, 0, 128, 255, 255);
        SDL_RenderFillRect(renderer, &water);

        //// 물고기 이미지 표시
        //if (fishTanks[i].isAlive) {
        //    SDL_Rect fishRect = { x + 20, 300 + FISHTANK_HEIGHT - waterHeight - 30, 80, 80 };   //
        //    //물고기 그림이 어항에 꽉 차도록 크기 조절, 물 높이에 따라 물고기 위치도 조절
        //    //SDL_RenderCopy(renderer, fishTexture, NULL, &fishRect);
        //    SDL_RenderCopy(renderer, fishTanks[i].texture, NULL, &fishRect);// 물고기 타입에 따라 해당 텍스처 렌더링
        //    // 물고기가 성장하면 물고기가 커지는 효과 추가, 성장 단계에 따라 물고기 크기 조절
        //}

        if (fishTanks[i].isAlive) {
            // 물고기 크기: 최소 30 ~ 최대 100 픽셀, fish(0..100)에 비례
            int size = 30 + (fishTanks[i].fish * 70) / 100;
            SDL_Rect fishRect = { x + (FISHTANK_WIDTH - size) / 2, 300 + FISHTANK_HEIGHT - waterHeight - size, size , size };
            SDL_SetTextureColorMod(fishTanks[i].texture, 255, 255, 255); // 정상 색상
			SDL_RenderCopy(renderer, fishTanks[i].texture, NULL, &fishRect); // 물고기 타입에 따라 해당 텍스처 렌더링, 물고기가 성장하면 물고기가 커지는 효과 추가, 성장 단계에 따라 물고기 크기 조절
        }
        else {
            // 사망 시: 작게 표시하고 회색톤 적용
            int size = 40;
            SDL_Rect fishRect = { x + (FISHTANK_WIDTH - size) / 2, 300 + FISHTANK_HEIGHT - waterHeight - size, size, size };
            SDL_SetTextureColorMod(fishTanks[i].texture, 140, 140, 140); // 회색화
            SDL_RenderCopy(renderer, fishTanks[i].texture, NULL, &fishRect);
            SDL_SetTextureColorMod(fishTanks[i].texture, 255, 255, 255); // 복원
        }

        // 물고기 크기 텍스트 출력
        char status[64];
        if (fishTanks[i].isAlive) {
            const char* typeStr;
            switch (fishTanks[i].type) {
            case NORMAL: typeStr = "Normal"; break;
            case FAST: typeStr = "Fast"; break;
            case BIG: typeStr = "Big"; break;
			case SPECIAL: typeStr = "Special"; break;
            default: typeStr = "Unknown"; break;
            }
            sprintf_s(status, sizeof(status), "T:%s", typeStr);
            renderText(status, x + 10, 520);
            sprintf_s(status, sizeof(status), "F:%d W:%d", fishTanks[i].fish, fishTanks[i].water);
        }
        else
            sprintf_s(status, sizeof(status), "DEAD");
        renderText(status, x + 10, 540);

        // 커서 표시
        if (i == position) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // 노란색
            SDL_RenderDrawRect(renderer, &bowl);
        }
    }
}

void menuRender() {
    // 메뉴 화면 렌더링
    renderText("Menu", 200, 200);
    //title_BGM(); // 타이틀 화면 배경 음악 재생
    // 메뉴에서 엔터 키를 누르면 게임 시작
    SDL_RenderPresent(renderer);
}

//void gameOverRender() {
//    // 게임 오버 화면 렌더링
//    renderText("Game Over", 200, 200);
//}
//
//void gameWinRender() {
//    // 게임 승리 화면 렌더링
//    renderText("You Win!", 200, 200);
//}

void updateGame() {
    long currentTime = SDL_GetTicks();
    long elapsed = (currentTime - lastUpdateTime) / 1000; // 초 단위
    if (elapsed > 0) {
        int aliveCount = 0;
        for (int i = 0; i < NUM; i++) {
            if (fishTanks[i].isAlive == 1) {
                // 물 증발 및 소비
                //fishTanks[i].water -= (level * (fishTanks[i].fish / 20 + 1) * elapsed);
                fishTanks[i].water -= (fishTanks[i].waterUse * elapsed); // 물 소비량에 따라 물 감소
                fishTanks[i].water -= (level * elapsed); // 레벨에 따라 추가로 물 감소 
                if (fishTanks[i].water < 0) {
                    fishTanks[i].water = 0;
                    fishTanks[i].isAlive = 0;
                }

                // 물고기 성장
                /*
                if (fishTanks[i].water > 0)
                    fishTanks[i].fish += ((fishTanks[i].water / 100 + 1) * elapsed);
                if (fishTanks[i].fish > 100) fishTanks[i].fish = 100;
                */
                if (fishTanks[i].water > 0) {
                    fishTanks[i].growth += (fishTanks[i].water / 100.0) * elapsed; // 성장 단계 증가
                    //#printf("Fish %d growth: %f\n", i, fishTanks[i].growth);
                    if (fishTanks[i].growth >= 1) {
                        fishTanks[i].fish += 10; // 성장 단계가 1 이상이면 물고기 크기 증가
                        fishTanks[i].growth = 0; // 성장 단계 초기화
                    }
                }
                if (fishTanks[i].fish > 100) fishTanks[i].fish = 100;
                aliveCount++;
            }
        }

        if (aliveCount == 0) {
            gameOver = true;
            running = false;
        }

        // 레벨 업 조건: 시간 경과
        long totalElapsed = (currentTime - startTime) / 1000;
        if (totalElapsed / 20 > level - 1) {
            level++;
            if (level > 5) {
                level = 5;
                gameWin = true;
                running = false;
            }
        }

        lastUpdateTime = currentTime;
    }
}

void handleInput(SDL_Event* e) {
    if (e->type == SDL_KEYDOWN) {
        switch (e->key.keysym.sym) {
        case SDLK_j:
            if (position > 0) position--;
            break;
        case SDLK_l:
            if (position < NUM - 1) position++;
            break;
        case SDLK_k:
            if (fishTanks[position].water >= 0 && fishTanks[position].water < 100)  // 물이 0 이상 100 미만일 때만 물을 줄 수 있도록 조건 추가
                fishTanks[position].water += 5;
            playWaterSound(); // 물 주는 소리 재생 
            if (fishTanks[position].water > 100) fishTanks[position].water = 100; // 물이 100을 초과하지 않도록 제한
            break;
        case SDLK_ESCAPE:
            running = false;
            break;
        case SDLK_RETURN:
            if (game.state == STATE_MENU) {
                initGame();
                game.state = STATE_PLAYING;

				playBGM(); // 게임 시작 시 배경 음악 재생 
            }
            break;
        }
    }
}

SDL_Texture* loadTexture(const char* path) {
    SDL_Surface* surface = SDL_LoadBMP(path);
    if (!surface) {
        printf("이미지 로드 실패: %s\n", SDL_GetError());
        return NULL;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}