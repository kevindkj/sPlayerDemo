/* Copyright (C) 
 * 2018 - DKJ
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * 
 */
#include<stdio.h>
#include<libavutil/avutil.h>
#include<libavcodec/avcodec.h>
#include<libavformat/avformat.h>
#include<SDL2/SDL.h>
//#include<SDL2/SDL_mutex.h>
//#include<pthread.h>

#define REFRESH_EVENT  (SDL_USEREVENT + 1)
#define BREAK_EVENT  (SDL_USEREVENT + 2)
#define SPLAYER_DEMO_DEBUG
#ifdef SPLAYER_DEMO_DEBUG
//#define SPLAYER_PRINT(format, ...) printf(format, ##__VA_ARGS__)
#define SPLAYER_PRINT(format, args...) printf(format, ##args)
#else
#define SPLAYER_PRINT(format, ...)
#endif

typedef struct sPlayerHandle{
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Texture *texture;
	Uint32 pixformat;
	int winWidth;
	int winHeight;
	int pixelWidth;
	int pixelHeight;
	unsigned char *frameBuf;
	unsigned int bufSize;
	int bufDataRdyFlag;
	char *yuvFilePath;
}sPlayerHandle_t;

int g_ThreadExitFlag = 0;
sPlayerHandle_t g_spHandle;
SDL_mutex *g_pSdlMutex = NULL;
SDL_cond *g_pSdlCond = NULL;

int sPlayerGUI_threadfn(void *args)
{
	sPlayerHandle_t *pHandle = &g_spHandle;
	SDL_Event event;
	SDL_Rect rect;
	
	//SDL init
	if( SDL_Init(SDL_INIT_VIDEO) ){
		printf("couldn't init SDL - %s.\n", SDL_GetError());
		return -1;
	}
	//create window	
	pHandle->window = SDL_CreateWindow("simple player demo", \
							SDL_WINDOWPOS_UNDEFINED, \
							SDL_WINDOWPOS_UNDEFINED, \
							pHandle->winWidth, \
							pHandle->winHeight, \
							SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
	if( !pHandle->window ){
		printf("could not create sdl window - %s.\n", SDL_GetError());
		return -1;
	}

	//create render
	pHandle->renderer = SDL_CreateRenderer(pHandle->window, -1, 0);

	//create texture
	pHandle->texture = SDL_CreateTexture(pHandle->renderer, \
										 pHandle->pixformat,SDL_TEXTUREACCESS_STREAMING, \
										 pHandle->pixelWidth, \
										 pHandle->pixelHeight);


	while(1){
		SDL_WaitEvent(&event);// wait events
		if(event.type == REFRESH_EVENT){

			SDL_LockMutex(g_pSdlMutex);
			while((0 == pHandle->bufDataRdyFlag) && (g_ThreadExitFlag == 0)){
				SPLAYER_PRINT("%s,%d \n",__func__, __LINE__);	
				SDL_CondWait(g_pSdlCond,g_pSdlMutex);
			}
			pHandle->bufDataRdyFlag = 0;	
			SDL_UpdateTexture(pHandle->texture, NULL, pHandle->frameBuf, pHandle->pixelWidth);  
			SDL_UnlockMutex(g_pSdlMutex);

			//FIX: If window is resize
			rect.x = 0;  
			rect.y = 0;  
			rect.w = pHandle->winWidth;  
			rect.h = pHandle->winHeight;  
			
			SDL_RenderClear(pHandle->renderer);   
			SDL_RenderCopy(pHandle->renderer, pHandle->texture, NULL, &rect);  
			SDL_RenderPresent(pHandle->renderer);

		}else if(event.type == SDL_WINDOWEVENT){
			SDL_GetWindowSize(pHandle->window,&(pHandle->winWidth),&(pHandle->winHeight));
		}else if(event.type == SDL_QUIT){
			g_ThreadExitFlag = 1;
			printf("sPlayerGUI_threadfn: sdl quit event occured!\n");
		}else if(event.type == BREAK_EVENT){
			printf("sPlayerGUI_threadfn: sdl break event occured!\n");
			break;
		}
	}

	//quit SDL
	SDL_Quit();
	return 0;
}
int sPlayerDecode_threadfn(void *args)
{
	FILE *fp = NULL;
	SDL_Event eventPost;
	sPlayerHandle_t *pHandle = &g_spHandle;
	size_t tFileRdSize = 0;

	// regitser av api
	// open input av file with avformat lib
	// find stream info
	// find decode
	// open codec
	// cyclic reading frame
		// if(get packet - yes)
			// AVPacket type ?
				// Video packet
			
				// Audio packet
		// else if(get packet - no) quit

	
	
	pHandle->winWidth = 800;
	pHandle->winHeight = 480;
	pHandle->pixelWidth = 640;
	pHandle->pixelHeight = 360;
	pHandle->bufSize = pHandle->pixelWidth * pHandle->pixelHeight * 12 / 8;
	pHandle->frameBuf = (unsigned char *)malloc(pHandle->bufSize);
	pHandle->pixformat = SDL_PIXELFORMAT_IYUV; //IYUV: Y + U + V  (3 planes)    YV12: Y + V + U  (3 planes)
	fp = fopen(pHandle->yuvFilePath,"rb+");//open a raw video pixel data file
	if( NULL == fp ){
		printf("open yuv file failed.\n");
		return -1;
	}


	g_ThreadExitFlag=0;
	while (!g_ThreadExitFlag) {
		
		SDL_LockMutex(g_pSdlMutex);
		tFileRdSize = fread(pHandle->frameBuf, 1, pHandle->bufSize, fp);
		if ( tFileRdSize != pHandle->bufSize){
			fseek(fp, 0, SEEK_SET);
			tFileRdSize = fread(pHandle->frameBuf, 1, pHandle->bufSize, fp);
		}
		SPLAYER_PRINT("%s,%d \n",__func__, __LINE__);	
		pHandle->bufDataRdyFlag = 1;
		SDL_CondSignal(g_pSdlCond);
		SDL_UnlockMutex(g_pSdlMutex);

		eventPost.type = REFRESH_EVENT;
		SDL_PushEvent(&eventPost);
		SDL_Delay(30);
	
	}
	g_ThreadExitFlag=0;
	eventPost.type = BREAK_EVENT;
	SDL_PushEvent(&eventPost);
	
	// free frame buffer
	free(pHandle->frameBuf);

	return 0;
}
int main(int argc, char *argv[])
{
	SDL_Thread *pDecSdlThr,*pGuiSdlThr;
	int iStat = 0;

	//parameters check and using explanation
	if( argc <= 1){
		printf("sPlayerDemo using error : too few parameters!!!\n");
		printf("-----------------------------------------------\n");
		printf("synopsis : \n");
		printf("	sPlayerDemo FILE\n");
		//printf("descriptions : \n ");
		goto error1;
	}else{
		g_spHandle.yuvFilePath = argv[1]; // copy file path
		g_spHandle.bufDataRdyFlag = 0;
	}

	g_pSdlMutex = SDL_CreateMutex();
	g_pSdlCond = SDL_CreateCond();
	pDecSdlThr = SDL_CreateThread(sPlayerDecode_threadfn, "decode", NULL);
	if(NULL == pDecSdlThr){
		printf("mian: create decode thread failed!\n");
		goto error1;
	}
	pGuiSdlThr = SDL_CreateThread(sPlayerGUI_threadfn, "gui", NULL);
	if(NULL == pGuiSdlThr){
		printf("main: create GUI thread failed!\n");
		goto error2;
	}

	SDL_WaitThread(pGuiSdlThr,&iStat);
	SDL_WaitThread(pDecSdlThr,&iStat);
	SDL_DestroyMutex(g_pSdlMutex);
	SDL_DestroyCond(g_pSdlCond);
	return 0;


error2:
	SDL_WaitThread(pDecSdlThr,&iStat);
error1:
	return -1;
}
