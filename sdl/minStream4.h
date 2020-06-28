#ifndef MINSTREAM4_H
#define MINSTREAM4_$

void initSDL(SDL_Window** gWindow, int ScrW, int ScrH, SDL_Texture** mTexture, int TexH, int TexW, SDL_Renderer** gRenderer, float scale);

void closeSDL(SDL_Window** gWindow, SDL_Texture** mTexture, SDL_Renderer** gRenderer);

void clearwinSDL(SDL_Renderer** gRenderer, Uint32 Col);

bool limitfpsSDL(unsigned int fps);

void* getptrSDL(SDL_Texture** mTexture);

void renderSDL(SDL_Texture** mTexture, int Xpos, int Ypos, int Xlen, int Ylen, SDL_Renderer** gRenderer);

#endif

/*
======================= EXAMPLE =======================


#define SDL_MAIN_HANDLED
#include <SDL.h>
#include "minStream4.h"

int main(){
	
	SDL_Window* gWindow = NULL;
	SDL_Renderer* gRenderer = NULL;
	SDL_Texture* mTexture;
	SDL_Event e; bool quit;

	unsigned char MyFrame[65536];
	
	initSDL(&gWindow, 640, 480, &mTexture, 128, 128, &gRenderer, 2.0);
	
	clearwinSDL( &gRenderer, 0xffffffff );            // Clear screen
	
	quit = false;	                                      // Main loop flag
	while(!quit){                                         // While application is running
	
		while(SDL_PollEvent(&e) != 0){                    // Handle events on queue
			if( e.type == SDL_QUIT ){ quit = true;}       // If user requests quit
		}

		void* vi = getptrSDL( &mTexture );                // Lock texture and get data pointer
		//char *ui = (char*)vi;                           // In case you want to write to memory directly
		
		MyFrame[hk] = 0x50; hk++;	
		memcpy( vi, MyFrame, 65536 );
		
		renderSDL( &mTexture, 50, 50, 128, 128, &gRenderer );    // unlock and render
		
		limitfpsSDL(30);      // fps limit, returns limit successful flag
			
	}

	closeSDL(&gWindow, &mTexture, &gRenderer);

	return 0;
}

=======================================================
*/


