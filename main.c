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

const int bpp=12;
int screen_w=800,screen_h=480;
const int pixel_w=640,pixel_h=360;
unsigned char buffer[345600];

//Refresh Event
#define REFRESH_EVENT  (SDL_USEREVENT + 1)
#define BREAK_EVENT  (SDL_USEREVENT + 2)

int thread_exit=0;

int refresh_video(void *opaque){
	thread_exit=0;
	while (!thread_exit) {
		SDL_Event event;
		event.type = REFRESH_EVENT;
		SDL_PushEvent(&event);
		SDL_Delay(40);
	}
	thread_exit=0;
	//Break
	SDL_Event event;
	event.type = BREAK_EVENT;
	SDL_PushEvent(&event);

	return 0;
}
int sPlayerGuiInit()
{

	return 0;
}
int sPlayerRefreshTread()
{

}
int sPlayerGuiExit()
{

}
int main(int argc, char *argv[])
{
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Texture *texture;
	Uint32 pixformat;
	FILE *fp = NULL;
	SDL_Rect rect;
	SDL_Thread *refreshThread = NULL;
	SDL_Event event;
	char *pfilePath = NULL;

	//parameters check and using explanation
	if( argc <= 1){
		printf("sPlayerDemo using error : too few parameters!!!\n");
		printf("-----------------------------------------------\n");
		printf("synopsis : \n");
		printf("	sPlayerDemo FILE\n");
		//printf("descriptions : \n ");
		return -1;
	}else{
		pfilePath = argv[1]; // copy file path
	}

	printf("sPlayerDemo begin run.\n");

	// regitser av api
	// open input av file with avformat lib
	// find stream info
	// find decode
	// open codec
	// cyclic reading frame
		// if(get packet - yes)
			// AVPacket ?
				// Video packet
				// Audio packet
		// else if(get packet - no) quit
	

	//SDL init
	if( SDL_Init(SDL_INIT_VIDEO) ){
		printf("couldn't init SDL - %s.\n", SDL_GetError());
		return -1;
	}
	//create window	
	window = SDL_CreateWindow("simple player demo",
							SDL_WINDOWPOS_UNDEFINED,
							SDL_WINDOWPOS_UNDEFINED,
							screen_w,screen_h,
							SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
	if( !window ){
		printf("could not create window - %s.\n", SDL_GetError());
		return -1;
	}

	//create render
	renderer = SDL_CreateRenderer(window, -1, 0);

	//create texture
	pixformat= SDL_PIXELFORMAT_IYUV; //IYUV: Y + U + V  (3 planes)    YV12: Y + V + U  (3 planes)
	texture = SDL_CreateTexture(renderer,pixformat,SDL_TEXTUREACCESS_STREAMING,pixel_w,pixel_h);

	//open a raw video pixel data file
	fp = fopen(pfilePath,"rb+");
	if( NULL == fp ){
		printf("open yuv file failed.\n");
		return -1;
	}

	//create a sdl refresh thread
	refreshThread = SDL_CreateThread(refresh_video,NULL,NULL);

	//while(1) wait refresh event
	while(1){
		SDL_WaitEvent(&event);

		if(event.type == REFRESH_EVENT){
			if (fread(buffer, 1, pixel_w*pixel_h*bpp/8, fp) != pixel_w*pixel_h*bpp/8){
				// Loop
				fseek(fp, 0, SEEK_SET);
				fread(buffer, 1, pixel_w*pixel_h*bpp/8, fp);
			}

			SDL_UpdateTexture(texture, NULL, buffer, pixel_w);  

			//FIX: If window is resize
			rect.x = 0;  
			rect.y = 0;  
			rect.w = screen_w;  
			rect.h = screen_h;  
			
			SDL_RenderClear(renderer);   
			SDL_RenderCopy(renderer, texture, NULL, &rect);  
			SDL_RenderPresent(renderer);
		}else if(event.type == SDL_WINDOWEVENT){
			SDL_GetWindowSize(window,&screen_w,&screen_h);
		}else if(event.type == SDL_QUIT){
			thread_exit = 1;
			printf("sdl quit event occured!\n");
		}else if( event.type == BREAK_EVENT){
			printf("sdl break event occured!\n");
			break;
		}
	}

	//quit SDL
	SDL_Quit();
	printf("sPlayerDemo exit!\n");
	return 0;
}
