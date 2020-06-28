/*This source code copyrighted by Lazy Foo' Productions (2004-2019)
and may not be redistributed without written permission.*/

#define SDL_MAIN_HANDLED

#include <SDL.h>
#include <stdio.h>
#include <string>

#include "minStream4.h"


void initSDL(SDL_Window** gWindow, int ScrW, int ScrH, SDL_Texture** mTexture, int TexW, int TexH, SDL_Renderer** gRenderer, float scale){
	
	SDL_Init( SDL_INIT_VIDEO );
	SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" );
	*gWindow = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, ScrW, ScrH, SDL_WINDOW_SHOWN );
	*gRenderer = SDL_CreateRenderer( *gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
	SDL_RenderSetScale( *gRenderer, scale, scale );  // <------ Everything (even the Xpos/Ypos coordinates will be enlarged)
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0"); // <------ Pixalated enlargement
	SDL_SetRenderDrawColor( *gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
	*mTexture = SDL_CreateTexture( *gRenderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, TexW, TexH);
}


void closeSDL(SDL_Window** gWindow, SDL_Texture** mTexture, SDL_Renderer** gRenderer){
	
	SDL_DestroyTexture( *mTexture );	
	SDL_DestroyRenderer( *gRenderer );
	SDL_DestroyWindow( *gWindow );
	SDL_Quit();
}


void clearwinSDL(SDL_Renderer** gRenderer, Uint32 Col){ // color as RGBA

	SDL_SetRenderDrawColor( *gRenderer, char(Col >> 24), char(Col >> 16), char(Col >> 8), char(Col) );
	SDL_RenderClear( *gRenderer );  // clears the screen with the color above
}


bool limitfpsSDL(unsigned int fps){
	
	static Uint32 Sta = 0;              // done only once
	
	bool ret = false;                   // frame rate was limited flag
	Uint32 End = SDL_GetTicks();
	unsigned int Mpf = 1000/fps;                 // milliseconds per frame
	
	if((End-Sta) < Mpf && Sta != 0){    // FPS max control
		SDL_Delay(Mpf-(End-Sta));
		ret = true;
	}
	
	Sta = SDL_GetTicks();
	return ret;
}


void* getptrSDL(SDL_Texture** mTexture){
	
	void* mPixels = NULL; int mPitch;
	
	SDL_LockTexture( *mTexture, NULL, &mPixels, &mPitch );
	// After this, mPixel will point to memory area I can write my data to!
		
	return mPixels;   // mPixels points to Uchar32: ABGR color order
}


void renderSDL(SDL_Texture** mTexture, int Xpos, int Ypos, int Xlen, int Ylen, SDL_Renderer** gRenderer){
	
	SDL_UnlockTexture( *mTexture );    //mPixels must NOT be NULL

	//Render frame
	SDL_Rect renderQuad = { Xpos, Ypos, Xlen, Ylen };
	SDL_RenderCopyEx( *gRenderer, *mTexture, NULL, &renderQuad, 0.0, NULL, SDL_FLIP_NONE);

	//Update screen
	SDL_RenderPresent( *gRenderer );
}









