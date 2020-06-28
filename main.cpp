#include <iostream>
#include <cstring>

#define SDL_MAIN_HANDLED
#include "SDL.h"
#include "sdl/minStream4.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include "mylib/DeviceLibrary.cpp"

#define UCHAR unsigned char


using namespace std;

void PrintCpu(CPU_6510 &Z){
	
	const char Names[][10] = { "A", "X", "Y", "S", "F", "Sync", "PC", "Inst", "cycle", "NmiRqs", "IrqRqs", "RstRqs",
	                           "-AdBuf-", "-DtBuf-", "-IoBuf-" };
	const bool bit16[] = {0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0};
	
	printf("\n==== CPU Status ====\n");
	
	printf("A = %02x   X = %02x   Y = %02x\n", Z.GetCpuReg(0), Z.GetCpuReg(1), Z.GetCpuReg(2));
	printf("  S = %02x   PC = %04x\n", Z.GetCpuReg(3), Z.GetCpuReg(6));
	printf("     F = %d%d%d%d%d%d%d%d\n", (Z.GetCpuReg(4)>>7)&0x01, (Z.GetCpuReg(4)>>6)&0x01, (Z.GetCpuReg(4)>>5)&0x01, (Z.GetCpuReg(4)>>4)&0x01,
                                       	  (Z.GetCpuReg(4)>>3)&0x01, (Z.GetCpuReg(4)>>2)&0x01, (Z.GetCpuReg(4)>>1)&0x01, (Z.GetCpuReg(4))&0x01);
	
	printf("====================\n\n\n");
}


int main(){
	
	//----------- Variables ----------


	StandardBus<uint16_t>  CpuAddr;                                                  // (PRESENTATION NOTE): no argument constructor
	StandardBus<uint8_t>   CpuData;
	StandardBus<bool>      CpuIO, CpuSync, Nmi;
	Clock                  Clk(1,1);                                                 // (PRESENTATION NOTE): two argument constructor
	
	StandardBus<bool>      RamE, RomE, GpioE, Port0E, Port1E, Port2E;                // enable lines
	StandardBus<bool>      ShftData, ShftClr;
	
	StandardBus<bool>      LedData[8];                                               // (PRESENTATION NOTE): array of LED lines

	StandardBus<uint8_t>   PalData, GpioData, Port0Data, Port1Data, Port2Data;       // Pal data bus
	
	CollectorBitBus        Irq, Null;       // unatached outputs can go here! It's a collector bus, therefore no errors
	
	VccSource Vcc;
	GndSource Gnd;
	
	
	//------------ Connection Setup --------------
	
	int MouseX, MouseY, Code;                         // mouse click location and button code
	
	int img_width, img_height, img_channels;
	unsigned char *VBuff = stbi_load("resources/blank2.png", &img_width, &img_height, &img_channels, 4);
	
	CpuIO = true;                  // (PRESENTATION NOTE): Assignment (equal sign) overload. Chaining.
	Null.Reset(); Irq.Reset();
	
	// (PRESENTATION NOTE): You can still assign "collector bus" pointers to "Standard Bus" pointers due to inheritance
	//                      virtual functions will do the job
	
	
	CPU_6510 Cpu(0, &CpuAddr, &CpuData, &CpuSync, &CpuIO, &Gnd, &Clk, &Irq, &Nmi);
	
	MemoryDevice<uint16_t, uint8_t> Ram(1, &RamE, 15, &CpuAddr, 8, &CpuData, &CpuIO);         // (PRESENTATION NOTE): using the template classes
	
	MemoryDevice<uint16_t, uint8_t> Rom(2, &RomE, 14, &CpuAddr, 8, &CpuData);                 // ROM functionality 
	
	MemoryDevice<uint16_t, uint8_t> Pal(3, &Gnd, 16, &CpuAddr, 8, &PalData);                  // ROM functionality
	
	Splitter8 Splt(&PalData, &RomE, &RamE, &GpioE, &Port0E, &Port1E, &Port2E, &Null, &Null);  // collector bus compatibility with standard bus
	
	LatchReg<uint8_t> Gpio(&CpuData, &GpioData, &GpioE);
	
	TriGate<uint8_t> Port0(&Port0Data, &CpuData, &Port0E);
	
	LatchReg<uint8_t> Port1(&CpuData, &Port1Data, &Port1E);
	
	LatchReg<uint8_t> Port2(&CpuData, &Port2Data, &Port2E);
	
	Keyboard_6502kit Key(&Port1Data, &Port0Data, 18, 91, &MouseX, &MouseY, &Code);
	
	Segment8D Disp(&Port2Data, &Port1Data, &Port2E, VBuff, 2208, 0, 0);
	
	ShftReg8<bool> Shft(&Vcc, &ShftData, &CpuSync, &ShftClr);
	
	NotGate Not(&ShftData, &Nmi);
	
	Splitter8to1 Splt2(&Port1Data, &ShftClr, 6);
	
	Splitter8 LedSplt( &GpioData,   &LedData[0], &LedData[1], &LedData[2], &LedData[3], 
					   &LedData[4], &LedData[5], &LedData[6], &LedData[7]);
	
	SquareLed *LedP[8];                                                           // array of pointers to LED objects
	for(int j = 315, i = 0; i < 8; i++, j+=19){
		if(i == 4){ j += 16;}
		LedP[i] = new SquareLed(&LedData[i], VBuff, 2208, j, 34);                 // (PRESENTATION NOTE): Initializing dynamically allocated objects!!!
	}
			
	
	Device *System[14] = { &Cpu, &Pal, &Splt, &Rom, &Ram, &Port1, &Key, &Port0,    // (PRESENTATION NOTE): Emulation pointer list
					       &Port2, &Disp, &Gpio, &Splt2, &Shft, &Not };
	

	//------ Memory Initialization ------
	
	
	FileToMemory(Rom, Rom[0], 16384, 0, "resources/ROM", 0, 16384);               // (PRESENTATION NOTE): using a template function
	
	FileToMemory(Pal, Pal[0], 65536, 0, "resources/PAL", 0, 65536);
	
	FileToMemory(Ram, Ram[0], 65536, 512, "resources/PROG_BINCOUNT", 0, 16);
	
	//FileToMemory(Ram, Ram[0], 65536, 512, "resources/PROG_SEG7", 0, 176);

	
	//---------- SDL setup -----------
	
	
	SDL_Window* gWindow = NULL;
	SDL_Renderer* gRenderer = NULL;
	SDL_Texture* mTexture;
	SDL_Event e; bool quit, first;
	
	initSDL(&gWindow, 590, 335, &mTexture, 552, 300, &gRenderer, 1.0);
	
	clearwinSDL( &gRenderer, 0x0a392fff );            // Clear screen
	
	unsigned int iy, ix = 0;
	
	first = true;
	quit = false;	                                             // Main loop flag
	while(!quit){                                                // While application is running
	
		while(SDL_PollEvent(&e) != 0){                           // Handle events on queue
			if( e.type == SDL_QUIT ){ quit = true;}              // If user requests quit
			if( e.type == SDL_MOUSEBUTTONDOWN )
			{
				MouseX = e.motion.x;                             // Get the mouse offsets
				MouseY = e.motion.y;
			}
			if( e.type == SDL_MOUSEBUTTONUP ){
				MouseX = 0;
				MouseY = 0;
				if(Code == 8){ Cpu.ResetRequest();}
			}
		}
		
		//---- begin evaluation ----
		
		for(ix = 0; ix < 200000; ix++){
			
			/*
			Cpu.Evaluate();	
			Pal.Evaluate();
			Splt.Evaluate();
			Rom.Evaluate();
			Ram.Evaluate();
			Port1.Evaluate();
			Key.Evaluate();
			Port0.Evaluate();
			Port2.Evaluate();
			Disp.Evaluate();
			Gpio.Evaluate();
			Splt2.Evaluate();
			Shft.Evaluate();
			Not.Evaluate();
			*/
			
			for(iy = 0; iy < 14; iy++){ System[iy]->Evaluate();} 
				
			Clk++;
		}
		
		LedSplt.Evaluate();
		for(iy = 0; iy < 8; iy++){ LedP[iy]->Evaluate();}        // LED object evaluation
		
		//---- end evaluation ----
		
		
		void* vi = getptrSDL( &mTexture );                       // Lock texture and get data pointer
			
		if(first){ 
			memcpy( vi, VBuff, 662400);                          // full frame copy
			first = false;
		}
		else{ 
			memcpy( vi, VBuff, 132480);                          // the display section copy
		}
		
		renderSDL( &mTexture, 18, 17, 552, 300, &gRenderer );    // unlock and render
		
		limitfpsSDL(10);                                         // fps limit, returns limit successful flag
			
	}

	closeSDL(&gWindow, &mTexture, &gRenderer);
	stbi_image_free(VBuff);
	
	PrintCpu(Cpu);
	
	//MemoryToFile(M, 0x10000, 0x0200, "resources/OUT", 256);
	
	//-------------- End --------------
	
	getchar();
	return 0;
}