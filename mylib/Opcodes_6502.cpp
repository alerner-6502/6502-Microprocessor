//---------------------- Adress Modes ------------------------------

void InterruptPush(uint8_t z){
	switch(cycle){
		case 0: AdBuf = Sreg+0x0100; DtBuf = ((PC+z) >> 8); IoBuf = 0; Sreg--; break;
		case 1: AdBuf = Sreg+0x0100; DtBuf = ((PC+z) & 0x00ff); IoBuf = 0; Sreg--; break;
		case 2: AdBuf = Sreg+0x0100; DtBuf = Freg; IoBuf = 0; Sreg--;                  // push PC and Status to stack
	}
}

void IndirectX(){
	switch(cycle){
		case 0: PC++; AdBuf = PC; IoBuf = 1; break;
		case 1: Buff[0] = DtBuf + Xreg; break;
		case 2: AdBuf = Buff[0]; break;
		case 3: Buff[0] = DtBuf; AdBuf++; break;
		case 4: Buff[1] = DtBuf; AdBuf = (Buff[1] << 8) + Buff[0];           
		// after this cycle DtBuf will conatin the contents of the indirectly accessed memory
		// The effective address of the memory is located in Buff[0] and Buff[1] memory
		// PC will point to the second instruction byte
	}
}

void IndirectY(){
	switch(cycle){
		case 0: PC++; AdBuf = PC; IoBuf = 1; break;
		case 1: AdBuf = DtBuf; break;
		case 2: Buff[0] = DtBuf; AdBuf++; break;
		case 3: AdBuf = (DtBuf << 8) + Buff[0] + Yreg;
		        Buff[1] = (AdBuf >> 8); Buff[0] = (AdBuf & 0x00ff);
		// after this cycle DtBuf will conatin the contents of the indirectly accessed memory
		// The effective address of the memory is located in Buff[0] and Buff[1] memory
		// PC will point to the second instruction byte
	}
}

void ZeroPage(){
	switch(cycle){
		case 0: PC++; AdBuf = PC; IoBuf = 1; break;
		case 1: AdBuf = Buff[0] = DtBuf;
		// after this cycle DtBuf will conatin the contents of the zero page memory
		// The effective address of the memory is located in the Buff[0] memory
		// PC will point to the second instruction byte
	}
}

void Immidiate(){
	switch(cycle){
		case 0: PC++; AdBuf = PC; IoBuf = 1;
		// after this cycle DtBuf will conatin the immidiate value
		// PC will point to the second instruction byte
	}
}

void Absolute(){
	switch(cycle){
		case 0: PC++; AdBuf = PC; IoBuf = 1; break;
		case 1: Buff[0] = DtBuf; PC++; AdBuf = PC; break;
		case 2: Buff[1] = DtBuf; AdBuf = (Buff[1] << 8) + Buff[0];
		// after this cycle DtBuf will conatin the absolute value
		// The effective address of the memory is located in Buff[0] and Buff[1] memory
		// PC will point to the third instruction byte
	}
}

void ZeropageX(){
	switch(cycle){
		case 0: PC++; AdBuf = PC; IoBuf = 1; break;
		case 1: DtBuf += Xreg; AdBuf = Buff[0] = DtBuf;      // zero page wraparound is natural
		// after this cycle DtBuf will conatin the indexed value
		// The effective address of the memory is located in Buff[0] memory
		// PC will point to the second instruction byte
	}
}

void ZeropageY(){
	switch(cycle){
		case 0: PC++; AdBuf = PC; IoBuf = 1; break;
		case 1: DtBuf += Yreg; AdBuf = Buff[0] = DtBuf;      // zero page wraparound is natural
		// after this cycle DtBuf will conatin the indexed value
		// The effective address of the memory is located in Buff[0] memory
		// PC will point to the second instruction byte
	}
}

void AbsoluteX(){
	switch(cycle){
		case 0: PC++; AdBuf = PC; IoBuf = 1; break;
		case 1: Buff[0] = DtBuf; PC++; AdBuf = PC; break;
		case 2: AdBuf = (DtBuf << 8) + Buff[0] + Xreg;
				Buff[1] = (AdBuf >> 8); Buff[0] = (AdBuf & 0x00ff);
		// after this cycle DtBuf will conatin the contents of the indirectly accessed memory
		// The effective address of the memory is located in Buff[0] and Buff[1] memory
		// PC will point to the third instruction byte
	}
}

void AbsoluteY(){
	switch(cycle){
		case 0: PC++; AdBuf = PC; IoBuf = 1; break;
		case 1: Buff[0] = DtBuf; PC++; AdBuf = PC; break;
		case 2: AdBuf = (DtBuf << 8) + Buff[0] + Yreg;
				Buff[1] = (AdBuf >> 8); Buff[0] = (AdBuf & 0x00ff);
		// after this cycle DtBuf will conatin the contents of the indirectly accessed memory
		// The effective address of the memory is located in Buff[0] and Buff[1] memory
		// PC will point to the third instruction byte
	}
}

//--------------------------- Shortcuts ---------------------------

void Done(){ PC++; cycle = -1;}

void Branch(bool Z){
	switch(cycle){
		case 0: PC++; AdBuf = PC; PC++; IoBuf = 1; break;
		case 1: if(Z){ PC = PC + char(DtBuf);} cycle = -1;
	}
}

void Push(uint8_t &z){
	switch(cycle){
		case 0: AdBuf = Sreg+0x0100; DtBuf = z; IoBuf = 0; break;
		case 1: Sreg--; break;
		case 2: Done();
	}
}

void Pull(uint8_t &z){
	switch(cycle){
		case 0: Sreg++; AdBuf = Sreg+0x0100; IoBuf = 1; break;
		case 1: z = DtBuf; break;
		case 2: break;
		case 3: Done();
	}
}

//------------------------- Flag modes ----------------------------

void FlagNZ(uint8_t test){
	if(test == 0x00){ Freg = Freg | 0x02;} else{ Freg = Freg & 0xfd;}
	if(test >= 0x80){ Freg = Freg | 0x80;} else{ Freg = Freg & 0x7f;}
}

void FlagAddNZCV(uint8_t A, uint8_t M, uint8_t C){
	 FlagNZ(A+M+C);
	 if((A+M+C) >= 0x0100){ Freg = Freg | 0x01;} else{ Freg = Freg & 0xfe;}
	 A = (~(A^M)) & (A^(A+M+C)) & 0x80;
	 if(A != 0x00){ Freg = Freg | 0x40;} else{ Freg = Freg & 0xbf;}
	 // So overflow is set if two inputs with the same sign produce a result with a different sign.
}

void FlagSubNZCV(uint8_t A, uint8_t M, uint8_t C){
	 FlagNZ(A-M-C);
	 if(A >= M+C){ Freg = Freg | 0x01;} else{ Freg = Freg & 0xfe;}
	 A = (~(A^(0-M))) & (A^(A-M-C)) & 0x80;
	 if(A != 0x00){ Freg = Freg | 0x40;} else{ Freg = Freg & 0xbf;}
	 // So overflow is set if two inputs with the same sign produce a result with a different sign.
}

void FlagTestNZC(uint8_t A, uint8_t M){
	 FlagNZ(A-M);
	 if(A >= M){ Freg = Freg | 0x01;} else{ Freg = Freg & 0xfe;}
}

//------------------------- Op codes ------------------------------

void ORA(){ Areg = Areg | DtBuf; FlagNZ(Areg);}

void ASL(){ Freg = (Freg & 0xfe) | (DtBuf >> 7); 
			DtBuf = (DtBuf << 1); FlagNZ(DtBuf); IoBuf = 0;
}

void ROL(){ Buff[4] = Freg & 0x01; Freg = (Freg & 0xfe) | (DtBuf >> 7);
			DtBuf = (DtBuf << 1) | Buff[4]; FlagNZ(DtBuf); IoBuf = 0;
}

void LSR(){ Freg = (Freg & 0xfe) | (DtBuf & 0x01);
			DtBuf = (DtBuf >> 1); FlagNZ(DtBuf); IoBuf = 0;
}

void ROR(){ Buff[4] = Freg << 7; Freg = (Freg & 0xfe) | (DtBuf & 0x01);
			DtBuf = (DtBuf >> 1) | Buff[4]; FlagNZ(DtBuf); IoBuf = 0;
}

void LDX(){ Xreg = DtBuf; FlagNZ(DtBuf);}

void LDY(){ Yreg = DtBuf; FlagNZ(DtBuf);}

void LDA(){ Areg = DtBuf; FlagNZ(Areg);}

void CPX(){ FlagTestNZC(Xreg, DtBuf);}

void CPY(){ FlagTestNZC(Yreg, DtBuf);}

void CMP(){ FlagTestNZC(Areg, DtBuf);}

void AND(){ Areg = Areg & DtBuf; FlagNZ(Areg);}

void EOR(){ Areg = Areg ^ DtBuf; FlagNZ(Areg);}

void ADC(){ Buff[4] = Areg + DtBuf + (Freg & 0x01);
			FlagAddNZCV(Areg, DtBuf, (Freg & 0x01)); 
			Areg = Buff[4];
}

void SBC(){ Buff[4] = Areg - DtBuf - (~Freg & 0x01);
			FlagSubNZCV(Areg, DtBuf, (~Freg & 0x01)); 
			Areg = Buff[4];
}

void BIT(){ FlagNZ(Areg & DtBuf); 
			Freg = (Freg & 0x7f) | (DtBuf & 0x80); // N bit (overwrite)
			Freg = (Freg & 0xbf) | (DtBuf & 0x40);
}

void STA(){ DtBuf = Areg; IoBuf = 0;}

void STY(){ DtBuf = Yreg; IoBuf = 0;}

void STX(){ DtBuf = Xreg; IoBuf = 0;}

void DEC(){ DtBuf--; FlagNZ(DtBuf); IoBuf = 0;}

void INC(){ DtBuf++; FlagNZ(DtBuf); IoBuf = 0;}

//------------------------- Listing -------------------------------

void RST__(){
	switch(cycle){
		case 0: Sreg = 0xff; Freg = 0x30;
				Areg = Xreg = Yreg = 0;
				AdBuf = 0xfffd; IoBuf = 1; break;
		case 1: PC = DtBuf; PC = PC << 8;
				AdBuf = 0xfffc; break;
		case 2: PC = PC + DtBuf; cycle = -1; RstRqs = false;
	}
}
		
void NMI__(){
	InterruptPush(0);
	switch(cycle){
		case 3: Freg = Freg | 0x04; break;
		case 4: AdBuf = 0xfffb; IoBuf = 1; break;
		case 5: PC = DtBuf; PC = PC << 8;
				AdBuf = 0xfffa; break;
		case 6: PC = PC + DtBuf; Freg = Freg & 0xfb;
				NMI_Pending = IRQ_Pending = false; cycle = -1;        
	}
}

void IRQ__(){
	InterruptPush(0);
	switch(cycle){
		case 0: Freg = Freg & 0xef; break;          // hot injection (Freg hasn't been pushed yet)
		case 3: Freg = Freg | 0x04; break;          // disable irq
		case 4: AdBuf = 0xffff; IoBuf = 1; break;
		case 5: PC = DtBuf; PC = PC << 8;
				AdBuf = 0xfffe; break;
		case 6: PC = PC + DtBuf; Freg = Freg & 0xfb;    // reenable irq
				NMI_Pending = IRQ_Pending = false; cycle = -1;      
	}
}

void BRK__(){
	InterruptPush(2);
	switch(cycle){
		case 0: Freg = Freg | 0x10; break;           // hot injection (Freg hasn't been pushed yet)
		case 3: Freg = Freg | 0x04; break;
		case 4: AdBuf = 0xffff; IoBuf = 1; break;
		case 5: PC = DtBuf; PC = PC << 8;
				AdBuf = 0xfffe; break;
		case 6: PC = PC + DtBuf; cycle = -1;      
	}
}

void JSR__(){
	switch(cycle){
		case 0: AdBuf = Sreg+0x0100; DtBuf = ((PC+2) >> 8); IoBuf = 0; Sreg--; break;
		case 1: AdBuf = Sreg+0x0100; DtBuf = ((PC+2) & 0x00ff); IoBuf = 0; Sreg--; break;
		case 2: break;
		case 3: PC++; AdBuf = PC; IoBuf = 1; break;
		case 4: Buff[0] = DtBuf; AdBuf++; break;
		case 5: PC = (DtBuf << 8) + Buff[0]; cycle = -1;
	}
}

void RTI__(){
	switch(cycle){
		case 0: Sreg++; AdBuf = Sreg+0x0100; IoBuf = 1; break;
		case 1: Freg = DtBuf; Sreg++; AdBuf = Sreg+0x0100; break;
		case 2: Buff[0] = DtBuf; Sreg++; AdBuf = Sreg+0x0100; break;
		case 3: PC = (DtBuf << 8) + Buff[0]; break;
		case 4: break;
		case 5: cycle = -1;
	}
}

void RTS__(){
	switch(cycle){
		case 0: Sreg++; AdBuf = Sreg+0x0100; IoBuf = 1; break;
		case 1: Buff[0] = DtBuf; Sreg++; AdBuf = Sreg+0x0100; break;
		case 2: PC = (DtBuf << 8) + Buff[0]; break;
		case 3: break;
		case 4: PC++; cycle = -1;
	}
}

void JMPab(){
	switch(cycle){
		case 0: PC++; AdBuf = PC; IoBuf = 1; break;
		case 1: Buff[0] = DtBuf; AdBuf++; break;
		case 2: PC = (DtBuf << 8) + Buff[0]; cycle = -1;
	}
}

void JMPin(){
	switch(cycle){
		case 0: PC++; AdBuf = PC; IoBuf = 1; break;
		case 1: Buff[0] = DtBuf; AdBuf++; break;
		case 2: AdBuf = (DtBuf << 8) + Buff[0]; break; 
		case 3: Buff[0] = DtBuf; AdBuf++; break;
		case 4: PC = (DtBuf << 8) + Buff[0]; cycle = -1;
	}
}

//-----------------------------------------------

void PHP__(){
	Push(Freg);
}

void PHA__(){
	Push(Areg);
}

void PLP__(){
	Pull(Freg);
}

void PLA__(){
	Pull(Areg);
	FlagNZ(Areg);  // <--- Ihis implementation is messy, but it works
}

void INX__(){
	switch(cycle){
		case 0: Xreg++; FlagNZ(Xreg); break;
		case 1: Done();
	}
}

void INY__(){
	switch(cycle){
		case 0: Yreg++; FlagNZ(Yreg); break;
		case 1: Done();
	}
}

void DEY__(){
	switch(cycle){
		case 0: Yreg--; FlagNZ(Yreg); break;
		case 1: Done();
	}
}

void DEX__(){
	switch(cycle){
		case 0: Xreg--; FlagNZ(Xreg); break;
		case 1: Done();
	}
}

void TYA__(){
	switch(cycle){
		case 0: Areg = Yreg; FlagNZ(Areg); break;
		case 1: Done();
	}
}

void TXA__(){
	switch(cycle){
		case 0: Areg = Xreg; FlagNZ(Areg); break;
		case 1: Done();
	}
}

void TAX__(){
	switch(cycle){
		case 0: Xreg = Areg; FlagNZ(Xreg); break;
		case 1: Done();
	}
}

void TXS__(){
	switch(cycle){
		case 0: Sreg = Xreg; break;
		case 1: Done();
	}
}

void TSX__(){
	switch(cycle){
		case 0: Xreg = Sreg; FlagNZ(Xreg); break;
		case 1: Done();
	}
}

void TAY__(){
	switch(cycle){
		case 0: Yreg = Areg; FlagNZ(Yreg); break;
		case 1: Done();
	}
}

void CLI__(){
	switch(cycle){
		case 0: Freg = Freg & 0xfb; break;
		case 1: Done();
	}
}

void SEI__(){
	switch(cycle){
		case 0: Freg = Freg | 0x04; break;
		case 1: Done();
	}
}

void CLV__(){
	switch(cycle){
		case 0: Freg = Freg & 0xbf; break;
		case 1: Done();
	}
}

void CLD__(){
	switch(cycle){
		case 0: Freg = Freg & 0xf7; break;
		case 1: Done();
	}
}

void SED__(){
	switch(cycle){
		case 0: Freg = Freg | 0x08; break;
		case 1: Done();
	}
}

void NOP__(){ 
	switch(cycle){
		case 0: break;
		case 1: Done();
	}; 
}

void ASLzp(){
	ZeroPage();
	switch(cycle){
		case 2: ASL(); break; // address is already setup! (One instruction ago though, so be carefull)
		case 3: break;
		case 4: Done();
	}   
}

void ASLzx(){
	ZeropageX();
	switch(cycle){
		case 2: ASL(); break;
		case 3: break;
		case 4: break;
		case 5: Done();
	}
}

void ASL__(){
	switch(cycle){
		case 0: DtBuf = Areg; ASL(); Areg = DtBuf; IoBuf = 1; break; 
		case 1: Done();    // ^---- It's important to flip OpBuf afrer ASL()!
	}
}

void ROL__(){
	switch(cycle){
		case 0: DtBuf = Areg; ROL(); Areg = DtBuf; IoBuf = 1; break; 
		case 1: Done();    // ^---- It's important to flip OpBuf afrer ASL()!
	}
}

void LSR__(){
	switch(cycle){
		case 0: DtBuf = Areg; LSR(); Areg = DtBuf; IoBuf = 1; break; 
		case 1: Done();    // ^---- It's important to flip OpBuf afrer ASL()!
	}
}

void ROR__(){
	switch(cycle){
		case 0: DtBuf = Areg; ROR(); Areg = DtBuf; IoBuf = 1; break; 
		case 1: Done();    // ^---- It's important to flip OpBuf afrer ASL()!
	}
}

void ASLab(){
	Absolute();
	switch(cycle){
		case 3: ASL(); break;
		case 4: break; 
		case 5: Done();
	}
}

void ASLax(){
	AbsoluteX();
	switch(cycle){
		case 3: ASL(); break;
		case 4: break;
		case 5: break;
		case 6: Done();
	}
}

void STXzp(){
	ZeroPage();
	switch(cycle){
		case 1: STX(); break;
		case 2: Done(); // No flags
	}
}

void LDXim(){
	Immidiate();
	switch(cycle){
		case 1: LDX(); Done();
	}
}

void ORAim(){
	Immidiate();
	switch(cycle){
		case 1: ORA(); Done();
	}
}

void ORAab(){
	Absolute();
	switch(cycle){
		case 3: ORA(); Done();
	}
}

void ORAix(){
	IndirectX();
	switch(cycle){
		case 5: ORA(); Done();
	}
}

void ORAzp(){
	ZeroPage();
	switch(cycle){
		case 2: ORA(); Done();
	}
}

void BPLre(){
	Branch((Freg & 0x80) == 0);
}

void ORAiy(){
	IndirectY();
	switch(cycle){
		case 4: ORA(); Done();
	}
}

void ORAzx(){
	ZeropageX();
	switch(cycle){
		case 2: ORA(); break;
		case 3: Done();
	}
}

void LDAim(){
	Immidiate();
	switch(cycle){
		case 1: LDA(); Done();
	}
}

void CLC__(){
	switch(cycle){
		case 0: Freg = Freg & 0xfe; break;
		case 1: Done();
	}
}

void SEC__(){
	switch(cycle){
		case 0: Freg = Freg | 0x01; break;
		case 1: Done();
	}
}

void ORAay(){
	AbsoluteY();
	switch(cycle){
		case 3: ORA(); Done();
	}
}

void ORAax(){
	AbsoluteX();
	switch(cycle){
		case 3: ORA(); Done();
	}
}

void LDAab(){
	Absolute();
	switch(cycle){
		case 3: LDA(); Done();
	}
}

void LDAax(){
	AbsoluteX();
	switch(cycle){
		case 3: LDA(); Done();
	}
}

void LDYab(){
	Absolute();
	switch(cycle){
		case 3: LDY(); Done();
	}
}

void STAab(){
	Absolute();
	switch(cycle){
		case 2: STA(); break;    // AdBuf is already calculated 
		case 3: Done();
	}
}

void STXab(){
	Absolute();
	switch(cycle){
		case 2: STX(); break;    // AdBuf is already calculated 
		case 3: Done();
	}
}

void STAax(){
	AbsoluteX();
	switch(cycle){
		case 2: STA(); break;    // AdBuf is already calculated 
		case 3: break;
		case 4: Done();
	}
}

void CPXim(){
	Immidiate();
	switch(cycle){
		case 1: CPX(); Done();
	}
}

void CPYim(){
	Immidiate();
	switch(cycle){
		case 1: CPY(); Done();
	}
}

void CPXzp(){
	ZeroPage();
	switch(cycle){
		case 2: CPX(); Done();
	}
}

void CPXab(){
	Absolute();
	switch(cycle){
		case 3: CPX(); Done();
	}
}

void BNEre(){
	Branch((Freg & 0x02) == 0);
}

void BEQre(){
	Branch((Freg & 0x02) != 0);
}

void ANDix(){
	IndirectX();
	switch(cycle){
		case 5: AND(); Done();
	}
}

void BMIre(){
	Branch((Freg & 0x80) != 0);
}

void ANDiy(){
	IndirectY();
	switch(cycle){
		case 4: AND(); Done();
	}
}

void EORix(){
	IndirectX();
	switch(cycle){
		case 5: EOR(); Done();
	}
}

void EORiy(){
	IndirectY();
	switch(cycle){
		case 4: EOR(); Done();
	}
}

void BVCre(){
	Branch((Freg & 0x40) == 0);
}

void BVSre(){
	Branch((Freg & 0x40) != 0);
}

void ADCix(){
	IndirectX();
	switch(cycle){
		case 5: ADC(); Done();
				
	}
}

void SBCix(){
	IndirectX();
	switch(cycle){
		case 5: SBC(); Done();
				
	}
}

void ADCim(){
	Immidiate();
	switch(cycle){
		case 1: ADC(); Done();			
	}
}

void ADCiy(){
	IndirectY();
	switch(cycle){
		case 4: ADC(); Done();
	}
}

void SBCiy(){
	IndirectY();
	switch(cycle){
		case 4: SBC(); Done();
	}
}

void STAix(){
	IndirectX();
	switch(cycle){
		case 4: STA(); break;   // AdBuf is already set up
		case 5: Done();			
	}
}

void STAiy(){
	IndirectY();
	switch(cycle){
		case 3: STA(); break;  // AdBuf is already set up
		case 4: break;
		case 5: Done();
	}
}

void BCCre(){
	Branch((Freg & 0x01) == 0);
}

void BCSre(){
	Branch((Freg & 0x01) != 0);
}

void LDYim(){
	Immidiate();
	switch(cycle){
		case 1: LDY(); Done();			
	}
}

void LDAix(){
	IndirectX();
	switch(cycle){
		case 5: LDA(); Done();			
	}
}

void LDAiy(){
	IndirectY();
	switch(cycle){
		case 4: LDA(); Done();
	}
}

void CMPix(){
	IndirectX();
	switch(cycle){
		case 5: CMP(); Done();			
	}
}

void CMPiy(){
	IndirectY();
	switch(cycle){
		case 4: CMP(); Done();
	}
}

void SBCim(){
	Immidiate(); //R
	switch(cycle){
		case 1: SBC(); Done();			
	}
}

void BITzp(){
	ZeroPage(); //R
	switch(cycle){
		case 2: BIT(); Done();
	}
}

void STYzp(){
	ZeroPage();
	switch(cycle){
		case 1: STY(); break;  // AdBuf is already set up
		case 2: Done();
	}
}

void STYzx(){
	ZeropageX();
	switch(cycle){
		case 1: STY(); break;  // AdBuf is already set up
		case 2: break;
		case 3: Done();
	}
}

void LDYzp(){
	ZeroPage(); //R
	switch(cycle){
		case 2: LDY(); Done();
	}
}

void LDXzp(){
	ZeroPage(); //R
	switch(cycle){
		case 2: LDX(); Done();
	}
}

void LDYzx(){
	ZeropageX(); //R
	switch(cycle){
		case 2: LDY(); break; 
		case 3: Done();
	}
}

void CPYzp(){
	ZeroPage(); //R
	switch(cycle){
		case 2: CPY(); Done();
	}
}

void ANDzp(){
	ZeroPage();
	switch(cycle){
		case 2: AND(); Done();
	}
}

void ANDzx(){
	ZeropageX();
	switch(cycle){
		case 2: AND(); break;
		case 3: Done();
	}
}

void EORzp(){
	ZeroPage();
	switch(cycle){
		case 2: EOR(); Done();
	}
}

void EORzx(){
	ZeropageX();
	switch(cycle){
		case 2: EOR(); break;
		case 3: Done();
	}
}

void ADCzp(){
	ZeroPage();
	switch(cycle){
		case 2: ADC(); Done();
	}
}

void ADCzx(){
	ZeropageX();
	switch(cycle){
		case 2: ADC(); break;
		case 3: Done();
	}
}

void STAzp(){
	ZeroPage();
	switch(cycle){
		case 1: STA(); break;  // AdBuf is already set up
		case 2: Done();
	}
}

void STAzx(){
	ZeropageX();
	switch(cycle){
		case 1: STA(); break;  // AdBuf is already set up
		case 2: break;
		case 3: Done();
	}
}

void LDAzp(){
	ZeroPage();
	switch(cycle){
		case 2: LDA(); Done();
	}
}

void LDAzx(){
	ZeropageX();
	switch(cycle){
		case 2: LDA(); break;
		case 3: Done();
	}
}

void CMPzp(){
	ZeroPage();
	switch(cycle){
		case 2: CMP(); Done();
	}
}

void CMPzx(){
	ZeropageX();
	switch(cycle){
		case 2: CMP(); break;
		case 3: Done();
	}
}

void SBCzp(){
	ZeroPage();
	switch(cycle){
		case 2: SBC(); Done();
	}
}

void SBCzx(){
	ZeropageX();
	switch(cycle){
		case 2: SBC(); break;
		case 3: Done();
	}
}

void STXzy(){
	ZeropageY();
	switch(cycle){
		case 1: STX(); break;
		case 2: break;
		case 3: Done();
	}
}

void ROLzp(){
	ZeroPage();
	switch(cycle){
		case 2: ROL(); break;
		case 3: break;
		case 4: Done();
	}   
}

void ROLzx(){
	ZeropageX();
	switch(cycle){
		case 2: ROL(); break;
		case 3: break;
		case 4: break;
		case 5: Done();
	}
}

void LSRzp(){
	ZeroPage();
	switch(cycle){
		case 2: LSR(); break;
		case 3: break;
		case 4: Done();
	}   
}

void LSRzx(){
	ZeropageX();
	switch(cycle){
		case 2: LSR(); break;
		case 3: break;
		case 4: break;
		case 5: Done();
	}
}

void RORzp(){
	ZeroPage();
	switch(cycle){
		case 2: ROR(); break;
		case 3: break;
		case 4: Done();
	}   
}

void RORzx(){
	ZeropageX();
	switch(cycle){
		case 2: ROR(); break;
		case 3: break;
		case 4: break;
		case 5: Done();
	}
}

void LDXzy(){
	ZeropageY();
	switch(cycle){
		case 2: LDX(); Done();
	}
}

void DECzp(){
	ZeroPage();
	switch(cycle){
		case 2: DEC(); break;
		case 3: break;
		case 4: Done();
	}   
}

void DECzx(){
	ZeropageX();
	switch(cycle){
		case 2: DEC(); break;
		case 3: break;
		case 4: break;
		case 5: Done();
	}
}

//

void INCzp(){
	ZeroPage();
	switch(cycle){
		case 2: INC(); break;
		case 3: break;
		case 4: Done();
	}   
}

void INCzx(){
	ZeropageX();
	switch(cycle){
		case 2: INC(); break;
		case 3: break;
		case 4: break;
		case 5: Done();
	}
}

void ANDim(){
	Immidiate();
	switch(cycle){
		case 1: AND(); Done();
	}
}

void ANDay(){
	AbsoluteY();
	switch(cycle){
		case 3: AND(); Done();
	}
}

void EORim(){
	Immidiate();
	switch(cycle){
		case 1: EOR(); Done();
	}
}

void EORay(){
	AbsoluteY();
	switch(cycle){
		case 3: EOR(); Done();
	}
}

void ADCay(){
	AbsoluteY();
	switch(cycle){
		case 3: ADC(); Done();
	}
}

void STAay(){
	AbsoluteY();
	switch(cycle){
		case 2: STA(); break;
		case 3: break;
		case 4: Done();
	}
}

void LDAay(){
	AbsoluteY();
	switch(cycle){
		case 3: LDA(); Done();
	}
}

void CMPim(){
	Immidiate();
	switch(cycle){
		case 1: CMP(); Done();
	}
}

void CMPay(){
	AbsoluteY();
	switch(cycle){
		case 3: CMP(); Done();
	}
}

void SBCay(){
	AbsoluteY();
	switch(cycle){
		case 3: SBC(); Done();
	}
}

void BITab(){
	Absolute();
	switch(cycle){
		case 3: BIT(); Done();
	}
}

void STYab(){
	Absolute();
	switch(cycle){
		case 2: STY(); break;
		case 3: Done();
	}
}

void LDYax(){
	AbsoluteX();
	switch(cycle){
		case 3: LDY(); Done();
	}
}

void CPYab(){
	Absolute();
	switch(cycle){
		case 3: CPY(); Done();
	}
}

void ANDab(){
	Absolute();
	switch(cycle){
		case 3: AND(); Done();
	}
}

void ANDax(){
	AbsoluteX();
	switch(cycle){
		case 3: AND(); Done();
	}
}

void EORab(){
	Absolute();
	switch(cycle){
		case 3: EOR(); Done();
	}
}

void EORax(){
	AbsoluteX();
	switch(cycle){
		case 3: EOR(); Done();
	}
}

void ADCab(){
	Absolute();
	switch(cycle){
		case 3: ADC(); Done();
	}
}

void ADCax(){
	AbsoluteX();
	switch(cycle){
		case 3: ADC(); Done();
	}
}

void CMPab(){
	Absolute();
	switch(cycle){
		case 3: CMP(); Done();
	}
}

void CMPax(){
	AbsoluteX();
	switch(cycle){
		case 3: CMP(); Done();
	}
}

void SBCab(){
	Absolute();
	switch(cycle){
		case 3: SBC(); Done();
	}
}

void SBCax(){
	AbsoluteX();
	switch(cycle){
		case 3: SBC(); Done();
	}
}

void ROLab(){
	Absolute();
	switch(cycle){
		case 3: ROL(); break;
		case 4: break; 
		case 5: Done();
	}
}

void ROLax(){
	AbsoluteX();
	switch(cycle){
		case 3: ROL(); break;
		case 4: break;
		case 5: break;
		case 6: Done();
	}
}

void LSRab(){
	Absolute();
	switch(cycle){
		case 3: LSR(); break;
		case 4: break; 
		case 5: Done();
	}
}

void LSRax(){
	AbsoluteX();
	switch(cycle){
		case 3: LSR(); break;
		case 4: break;
		case 5: break;
		case 6: Done();
	}
}

void RORab(){
	Absolute();
	switch(cycle){
		case 3: ROR(); break;
		case 4: break; 
		case 5: Done();
	}
}

void RORax(){
	AbsoluteX();
	switch(cycle){
		case 3: ROR(); break;
		case 4: break;
		case 5: break;
		case 6: Done();
	}
}

void LDXab(){
	Absolute();
	switch(cycle){
		case 3: LDX(); Done();
	}
}

void LDXay(){
	AbsoluteY();
	switch(cycle){
		case 3: LDX(); Done();
	}
}

void DECab(){
	Absolute();
	switch(cycle){
		case 3: DEC(); break;
		case 4: break; 
		case 5: Done();
	}
}

void DECax(){
	AbsoluteX();
	switch(cycle){
		case 3: DEC(); break;
		case 4: break;
		case 5: break;
		case 6: Done();
	}
}

void INCab(){
	Absolute();
	switch(cycle){
		case 3: INC(); break;
		case 4: break; 
		case 5: Done();
	}
}

void INCax(){
	AbsoluteX();
	switch(cycle){
		case 3: INC(); break;
		case 4: break;
		case 5: break;
		case 6: Done();
	}
}

//--------------------------- The table ------------------------



		
//--------------------------- End ------------------------






