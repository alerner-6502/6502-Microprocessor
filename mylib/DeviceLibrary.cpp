#include <iostream>
#include <fstream>
#include <cstring>
#include <stdlib.h>

using namespace std;

//======================================== Busses =========================================

//PRESENTATION NOTE: Carrier can by any type that can handle integer assignments

//--- Standard class template ---
template <class Carrier = uint8_t>                // (PRESENTATION NOTE)
class StandardBus {
	protected:                                    // (PRESENTATION NOTE) PROTECTED
		Carrier value;                            // BUS carrier (bool, uint8_t, uint16_t, uint32_t)
		Carrier mask;
		int WriteCount;
		
	public:
		StandardBus(){                            // no argument constructor
			mask = 0; mask = mask - 1;
			value = 0;
		}
		
		explicit StandardBus(int bits){           // The explicit keyword prohibits initialization like "StdBus Z = 1;"
			mask = (1 << bits) - 1;               // ^---- that's kinda pointless (PRESENTATION NOTE)
			WriteCount = 0; value = 0;
		}
		
		virtual Carrier Read() const{             // child classes may ovveride these (PRESENTATION NOTE)
			return value;
		}
		
		virtual void Write(Carrier data){          
			value = data & mask;
			WriteCount++;
		}
		
		virtual void Reset(){                     // every cycle resets busses
			value = 0; WriteCount = 0;
		}
		
		virtual bool Error() const{
			return (WriteCount > 1 ? true : false);     // multiple writes to a bus counts as an error
		}
		
		operator Carrier() const{                       // Bus can always be converted to it's carrier
			return value;                               // (PRESENTATION NOTE) Inherited functions will get to use this
		}
		                                                // (PRESENTATION NOTE)
		Carrier operator = (Carrier data){      // Assignment (equal sign) overload. Passing by reference (faster)
			value = data & mask;                        // WITH CHAINING! ( A = B = C = 1)
			WriteCount++;
			return value;
		}                                               // Don't confuse this with the initializing constructor!
		// NOTES: "StandardBus A = B;" is not an assignment! It's initialization. Equivalent: "StandardBus A(B);"
        // !THE ASSIGNMENT OPERATOR CANNOT BE INHERITED!
		
		// This is especially useful in these cases:  "IO = Col = true;". Where there aren't any pointers.
		// For pointers I'd like to use this: "IO->Write(false);" , rather than "*(IO) = true;"
		// In any case, it's all about user preference
		
		StandardBus& operator = (StandardBus &Z){       // (PRESENTATION NOTE) replaces: A.Write(B.Read());
			value = Z.value & mask;                     // WITH CHAINING! ( A = B = C = D)
			WriteCount++;
			return *(this);
		} 
		
		// !!!(PRESENTATION NOTE)!!! I can also do stuff like this: "*A = 5;" instead of "A->Read();"
		
};

// PRESENTATION NOTE: My Devices typically accept StandardBus pointers. However, special buss pointers such as
//                    Vcc, Gnd, Clock and collector bus can also be passed if they inherited StandardBus in a public 
//                    manner. There is a problem though. If I do that, the device won't use special buss's functions.
//                    It will access the standard functions within the base class. To make my overloads work, I must
//                    make standard functions virtual! They are overwriten only if the special bus decides to do so.

/*
An abstract function cannot have functionality. You're basically saying, any child class MUST give their own version of this 
method, however it's too general to even try to implement in the parent class.

A virtual function, is basically saying look, here's the functionality that may or may not be good enough for the child class. 
So if it is good enough, use this method, if not, then override me, and provide your own functionality.
*/                    

//--- special open collector bus for interrupts ---
class CollectorBitBus : public StandardBus<bool> {            // (PRESENTATION NOTE) Not a template anymore
	public:
		CollectorBitBus() : StandardBus<bool>() {}
		
		void Write(bool data){ value = value & data;}         // (PRESENTATION NOTE) overloaded Write member function
			
		void Reset(){ value = 1;}
		
		bool Error() const{ return false;}
		
		// (PRESENTATION NOTE)
		// CollectorBitBus and the functions bellow can only do the following: Type z = OBJECT;
		// The assignment operator functions are not available!  "A = B = C" and "A = B = 1" cannot be performed!
		
};


class VccSource : public StandardBus<bool> {                 // virtual functions like Read and Write are not overloaded
	public:                                                  // They won't have any effect anyway
		VccSource() : StandardBus<bool>() { value = 1;}      // Don't forger to initialize. Overloading the read() isn't good enough
		                                                     // because the inherited "int = OBJECT" operator uses the "value" variable!
		bool Read() const { return true;}
		
		void Write() { WriteCount = 10;}                     // overwritten write that tracks errors
};                                                           // (can't write to power line)


class GndSource : public StandardBus<bool> {
	public:
		GndSource() : StandardBus<bool>() { value = 0;}
		
		bool Read() const { return false;}
		
		void Write() { WriteCount = 10;}
};


class Clock : public StandardBus<bool>{
	private:
		unsigned long devision;
		unsigned long count;
		
	public:
		Clock() : StandardBus<bool>() {         // zero argument constructor
			devision = 1;
			count = 0;
		}
		
		Clock(bool init, unsigned long dev) : StandardBus<bool>(){            // one argument constructor
			value = init;
			devision = dev;                                                   // once can never change clock properties after it
			count = 0;                                                        // has been initialized	
		}
		
		void Write(bool data){ WriteCount = 10;}     // can't do anything
			
		void Reset(){}                               // reset is prohibited
		
		void operator ++ (int){                      // (PRESENTATION NOTE) the only way to update the clock
			count++;
			if(count == devision){                   // clock incrementation
				value = !value;
				count = 0;
			}
		}		
};

//======================================== Device Base ====================================

// PRESENTATION NOTE: Digital circuit simulator with the emphasis based on simple computing.
//                    Flexibility is the primary objective of this project. As such, high
//                    performance emulation is not possible.

class Device {                          // abstract class (pure virtual function is present)
	private:
		int ID;
		
	public:
		Device(int i){                  // overloaded constructor
			ID = i;
		}
		
		int GetID(){                    // ID cannot be modefied after the object has been created
			return ID;
		}
		
		virtual void Evaluate() = 0;    // (PRESENTATION NOTE) pure virtual function, Input->Output device evaluation
};

//======================================== Other Device Templates =========================

//-------------------------------------

// (PRESENTATION NOTE): Anstract class, used for constructing logic gates (AND, OR, XOR)
// PURPOSE: demostrating abstract classes, template->template inheritance, this pointer

template <class Type>
class LogicGate : public Device {
	protected:
		StandardBus<Type> *A, *B, *C;

	public:
		LogicGate( StandardBus<Type> *ap, StandardBus<Type> *bp, 
		           StandardBus<Type> *cp ) : Device(0) { A = ap; B = bp; C = cp;}
};


/*
Within D<T>::g(), the name f does not depend on template parameter T, so f is known as a nondependent name. 
On the other hand, B<T> is dependent on template parameter T so B<T> is called a dependent name.

Here’s the rule: the compiler does not look in dependent base classes (like B<T>) when looking up nondependent 
names (like f).

"This" makes it clear to the compiler that the member name depends on the template parameters so that 
it searches for the definition of that name in the right places. For more information also see 
"this entry in the C++ Faq Lite"

Since "this" is always implicitly dependent in a template, this->f is dependent and the lookup is 
therefore deferred until the template is actually instantiated, at which point all base classes 
are considered.
*/


template <class Type>
class AndGate : public LogicGate<Type> {         // (PRESENTATION NOTE): inhereting constructors! (Since C++11)
	using LogicGate<Type>::LogicGate;            // Equivalent to: Derived(int a) : Base(a), m() {} 
	
	public:	
		void Evaluate(){                                   // (PRESENTATION NOTE): this class is no longer abstract
			*(this->C) = *(this->A) & *(this->B);          // (PRESENTATION NOTE): why this-> is neccessary
		}	                                               // this->C->Write( this->A->Read() & this->B->Read() );
};


template <class Type>
class OrGate : public LogicGate<Type> {
	using LogicGate<Type>::LogicGate;
	
	public:
		void Evaluate(){
			*(this->C) = *(this->A) | *(this->B);
		}
};


template <class Type>
class XorGate : public LogicGate<Type> {
	using LogicGate<Type>::LogicGate;
	
	public:
		void Evaluate(){ 
			*(this->C) = *(this->A) ^ *(this->B);      // //this->C->Write( this->A->Read() ^ this->B->Read() );
		}
};


template <class Type>
class NorGate : public LogicGate<Type> {
	using LogicGate<Type>::LogicGate;
	
	public:
		void Evaluate(){ 
			*(this->C) = ~(*(this->A) | *(this->B));
		}
};


template <class Type>
class NandGate : public LogicGate<Type> {
	using LogicGate<Type>::LogicGate;
	
	public:
		void Evaluate(){ 
			*(this->C) = ~(*(this->A) & *(this->B));
		}
};


template <class Type>
class AddGate : public LogicGate<Type> {
	using LogicGate<Type>::LogicGate;
	
	public:
		void Evaluate(){ 
			*(this->C) = *(this->A) + *(this->B);
		}
};


template <class Type>
class SubGate : public LogicGate<Type> {
	using LogicGate<Type>::LogicGate;
	
	public:
		void Evaluate(){ 
			*(this->C) = *(this->A) - *(this->B);
		}
};


//-------------------------------------

// Emulates basic registers, and shift registers.
template <class Type>
class ShftReg8 : public Device {
	private:
		StandardBus<Type> *I, *O;
		StandardBus<bool> *Clk, *Clr;  // clock and clear (active low)
		Type MemP[16];
		bool LstClkState;
		
	public:
		ShftReg8( StandardBus<Type> *ip, StandardBus<Type> *op,
		          StandardBus<bool> *clk, StandardBus<bool> *clr) : Device(0) { 
				 
				 I = ip; O = op; Clk = clk; Clr = clr;
				 LstClkState = *Clk; 

		}
		
		Type& operator [](int n){      // (PRESENTATION NOTE) overloaded [] operator (I/O functionality)
			return MemP[n%16];
		}
		
		void Evaluate(){ 
			if(LstClkState == 0 && LstClkState != *Clk){ 
				for(int i = 15; i > 0; i--){
					MemP[i] = MemP[i-1];
				}
				MemP[0] = *I;                  // *MemP = I->Read();
				*O = MemP[15];                  // O->Write( *(MemP+(Len-1)) );
			}
			LstClkState = *Clk;                // conversion operator is being called
			
			if(*Clr == 0){
				for(int i = 0; i < 16; i++){
					MemP[i] = 0;
				}
				*O = 0;
			}
		}
};

//-------------------------------------

template <class Type>
class DuoMux : public Device {
	private:
		StandardBus<Type> *A, *B, *C;
		StandardBus<bool> *S;

	public:
		DuoMux( StandardBus<Type> *ap, StandardBus<Type> *bp, StandardBus<Type> *cp,
		        StandardBus<bool> *sp) : Device(0) { A = ap; B = bp; C = cp; S = sp;}
		
		void Evaluate(){ 
			if(*S == false){ *C = *A;}    // if(S->Read() == false){ C->Write( A->Read() );}
			else{ *C = *B;}               //else{ C->Write( B->Read() );}
		}
};


template <class Type>
class DuoDemux : public Device {
	private:
		StandardBus<Type> *A, *B, *C;
		StandardBus<bool> *S;

	public:
		DuoDemux( StandardBus<Type> *ap, StandardBus<Type> *bp, StandardBus<Type> *cp,
		        StandardBus<bool> *sp) : Device(0) { A = ap; B = bp; C = cp; S = sp;}
		
		void Evaluate(){ 
			if(*S == false){ *A = *C; *B = 0;}    // if(S->Read() == false){ C->Write( A->Read() );}
			else{ *B = *C; *A = 0;}               //else{ C->Write( B->Read() );}
		}
};


template <class Type>
class TriGate : public Device {
	private:
		StandardBus<Type> *A, *B;
		StandardBus<bool> *E;
	public:
		TriGate( StandardBus<Type> *ap, StandardBus<Type> *bp, StandardBus<bool> *ep
		       ) : Device(0) { A = ap; B = bp; E = ep;}
		
		void Evaluate(){ 
			if(*E == 0){ *B = *A;}
		}
};


class NotGate : public Device {
	private:
		StandardBus<bool> *A, *B;
	public:
		NotGate( StandardBus<bool> *ap, StandardBus<bool> *bp) : Device(0) { A = ap; B = bp;}
		
		void Evaluate(){ 
			*B = !(bool(*A));
		}
};


template <class Type>
class Mapper : public Device {
	private:
		StandardBus<Type> *A, *B;        // Input address and output address
		StandardBus<bool> *E;            // enable 
		Type NumL, NumH;                 // max and low address numbers
	public:
		Mapper( StandardBus<Type> *ap, StandardBus<Type> *bp, Type numl, Type numh, StandardBus<bool> *ep
		        ) : Device(0) { A = ap; B = bp; NumL = numl; NumH = numh; E = ep;}
		
		void Evaluate(){ 
			if(*A >= NumL && *A <= NumH){ *E = 0; *B = *A - NumL;}
			else{ *E = 1; *B = *A;}
		}
};


template <class Type>
class RndNum : public Device {
	private:
		StandardBus<Type> *D;            // Input data bus
		StandardBus<bool> *E;            // enable 
	public:
		RndNum( StandardBus<Type> *dp, StandardBus<bool> *ep) : Device(0) { D = dp; E = ep;}
		
		void Evaluate(){ 
			if(*E == 0){ *D = Type(rand());}
		}
};


template <class Type>
class LatchReg : public Device {
	private:
		StandardBus<Type> *ID, *OD;      // Input and output data bus
		StandardBus<bool> *E, *TRI;         // write enable (active low) and bus enable 
		Type value;
	public:
		LatchReg( StandardBus<Type> *idp, StandardBus<Type> *odp, 
				  StandardBus<bool> *ep, StandardBus<bool> *tp = NULL) : Device(0) {

				  ID = idp; OD = odp; E = ep; TRI = tp;
		}
		
		void Evaluate(){ 
			if(*E == 0){ value = *ID;}          // nice and simple 
			
			if(TRI == NULL){ *OD = value;}
			else if(*TRI == 0){ *OD = value;}
		}
};



//======================================== Splitters ====================================

class Splitter8 : public Device {
	private:
		StandardBus<uint8_t> *A;
		StandardBus<bool> *List[8];
		
	public:
		Splitter8( StandardBus<uint8_t> *a, 
				   StandardBus<bool> *b0, StandardBus<bool> *b1, 
				   StandardBus<bool> *b2, StandardBus<bool> *b3,
				   StandardBus<bool> *b4, StandardBus<bool> *b5,
				   StandardBus<bool> *b6, StandardBus<bool> *b7 ) : Device(0) {
					   
			List[0] = b0; List[1] = b1; List[2] = b2; List[3] = b3; A = a; 
			List[4] = b4; List[5] = b5; List[6] = b6; List[7] = b7;
		}
		
		Splitter8( StandardBus<uint8_t> *a, StandardBus<bool> *lst[]) : Device(0) {
			A = a;
			for(int i = 0; i < 8; i++){
				List[i] = lst[i];
			}
		}
		
		void Evaluate(){
			for(int i = 0; i < 8; i++){
				*(List[i]) = (*A >> i) & 0x01;
			}
		}
};



class Splitter8to1 : public Device {
	private:
		StandardBus<uint8_t> *A;
		StandardBus<bool> *OP;
		int Bit;
		
	public:
		Splitter8to1( StandardBus<uint8_t> *a, StandardBus<bool> *op, int bit) : Device(0) {
					   
			A = a; OP = op; Bit = bit;
		}
		
		void Evaluate(){
			*OP = (*A >> Bit) & 0x01;
		}
};




//======================================== Memory Device Template =========================

//--- Standard Rom/Ram device template ---
template <class AddressCarrier = uint8_t, class DataCarrier = uint8_t>  //(PRESENTATION NOTE)
class MemoryDevice : public Device {
	protected:                                      // This class is meant to be inherited from!!!
		StandardBus<AddressCarrier>* AP;      // address bus pointer
		StandardBus<DataCarrier>* DP;         // data bus pointer
		StandardBus<bool>* EP;                // Chip enable bus pointer (active-LOW)
		StandardBus<bool>* IOP;               // R/W bus pointer (read-HIGH, write-LOW)
		
		int address_width, data_width;              // data and address bit width
		DataCarrier* MemP;                          // memory pointer
		DataCarrier maskD;
		AddressCarrier maskA;
		unsigned int size;
		
	// (PRESENTATION NOTE):  Initialization list of the constructor to set the initial value -------v
		
	public:
		MemoryDevice( int id, StandardBus<bool> *ep,                 // 7 argument constructor with one default argument
				      int aw, StandardBus<AddressCarrier> *ap,       // The "*iop" argument is optional
		              int dw, StandardBus<DataCarrier> *dp, 
				      StandardBus<bool> *iop = NULL                  // if "iop" is specified the device will add RAM functionality
					
					) : Device(id) {    // constant private member data initialization
			       
			EP = ep; AP = ap; DP = dp; IOP = iop; 
			address_width = aw%24; data_width = dw;      // 16MB max width protection
			
			size = 1;
			size = (size << address_width);
			MemP = new DataCarrier [size];               // (PRESENTATION NOTE) memory allocation
			
			maskD = 1;
			maskD = (maskD << data_width) - 1;           // data mask calculation
			
			maskA = 1;
			maskA = (maskA << address_width) - 1;        // address mask calculation
		}
		
		~MemoryDevice(){                                 // (PRESENTATION NOTE) destructor
			delete[] MemP;
		}
		
		DataCarrier& operator [](AddressCarrier n){      // (PRESENTATION NOTE) overloaded [] operator (I/O functionality)
			return *(MemP+n);
		}
		
		unsigned int GetSize() const{
			return size;
		}
		
		void Evaluate(){                                       // overloading the virtual function!
			if(*EP == 0){                                      // if the chip is enabled
				AddressCarrier addr = *AP & maskA;
				if(IOP != NULL && *IOP == 0){                  // writing to memory (RAM functionality)
					*(MemP + addr) = *DP & maskD;
				}
				else{ 
					DataCarrier data = *(MemP + addr);         // reading memory (ROM and RAM functionality)
				    *DP = data & maskD;
				}                        
			}
		}	
};



//======================================== CPU_6510 =======================================

//--- Popular 8 bit processor ---
class CPU_6510 : public Device {
	private:
	
		//--------- private members ----------
	
		StandardBus<uint16_t>* AP;       // address bus pointer
		StandardBus<uint8_t>* DP;        // data bus pointer
		StandardBus<bool>* SYNCP;        // instruction fetch output bus pointer
		StandardBus<bool>* IOP;          // R/W bus pointer (read-HIGH, write-LOW)
		StandardBus<bool>* EP;           // Chip enable bus pointer (active-LOW)
		StandardBus<bool>* CLK;          // clock bus pointer
		CollectorBitBus* IRQ;            // IRQ bus pointer
		StandardBus<bool>* NMI;            // NMI bus pointer
		
		uint8_t Areg, Xreg, Yreg, Sreg, Freg;          // Accomulator, X, Y, Stack, Status(Flag), 
		uint8_t SyncReg, Buff[8];                      // SyncOutput, memory buffer
		uint16_t PC;                                   // program counter, address buffer (written to the AP bus on the every low edge of the clock)
		
		unsigned int cycle, Ireg;                      // cycles per instruction, Instruction (greater than 255 support is needed)
		bool LastClkState, IRQ_Pending, NMI_Pending;   // clock and Interrupts 
		bool LastNmiLevel, RstRqs;                     // NMI is edge triggered (high->low) IRQ is level triggered (low), Reset processor request
													   
		uint16_t AdBuf; uint8_t DtBuf; bool IoBuf;     // these are loaded to the address buss every low edge of the clock. 
		                                               // DtBuf is always writen to on the high edge of the clock.
													   
		//------------ instructions ----------
		
		#include "Opcodes_6502.cpp"
		
		//------- instruction pointers -------
		
		void (CPU_6510::*functionPtr[259])() = {
			//0      1      2      3      4      5      6      7      8      9      A      B      C      D      E      F
			BRK__, ORAix, NOP__, NOP__, NOP__, ORAzp, ASLzp, NOP__, PHP__, ORAim, ASL__, NOP__, NOP__, ORAab, ASLab, NOP__, // 0
			BPLre, ORAiy, NOP__, NOP__, NOP__, ORAzx, ASLzx, NOP__, CLC__, ORAay, NOP__, NOP__, NOP__, ORAax, ASLax, NOP__, // 1
			JSR__, ANDix, NOP__, NOP__, BITzp, ANDzp, ROLzp, NOP__, PLP__, ANDim, ROL__, NOP__, BITab, ANDab, ROLab, NOP__, // 2
			BMIre, ANDiy, NOP__, NOP__, NOP__, ANDzx, ROLzx, NOP__, SEC__, ANDay, NOP__, NOP__, NOP__, ANDax, ROLax, NOP__, // 3
			RTI__, EORix, NOP__, NOP__, NOP__, EORzp, LSRzp, NOP__, PHA__, EORim, LSR__, NOP__, JMPab, EORab, LSRab, NOP__, // 4
			BVCre, EORiy, NOP__, NOP__, NOP__, EORzx, LSRzx, NOP__, CLI__, EORay, NOP__, NOP__, NOP__, EORax, LSRax, NOP__, // 5
			RTS__, ADCix, NOP__, NOP__, NOP__, ADCzp, RORzp, NOP__, PLA__, ADCim, ROR__, NOP__, JMPin, ADCab, RORab, NOP__, // 6
			BVSre, ADCiy, NOP__, NOP__, NOP__, ADCzx, RORzx, NOP__, SEI__, ADCay, NOP__, NOP__, NOP__, ADCax, RORax, NOP__, // 7
			NOP__, STAix, NOP__, NOP__, STYzp, STAzp, STXzp, NOP__, DEY__, NOP__, TXA__, NOP__, STYab, STAab, STXab, NOP__, // 8
			BCCre, STAiy, NOP__, NOP__, STYzx, STAzx, STXzy, NOP__, TYA__, STAay, TXS__, NOP__, NOP__, STAax, NOP__, NOP__, // 9
			LDYim, LDAix, LDXim, NOP__, LDYzp, LDAzp, LDXzp, NOP__, TAY__, LDAim, TAX__, NOP__, LDYab, LDAab, NOP__, NOP__, // A
			BCSre, LDAiy, NOP__, NOP__, LDYzx, LDAzx, LDXzy, NOP__, CLV__, LDAay, TSX__, NOP__, LDYax, LDAax, NOP__, NOP__, // B
			CPYim, CMPix, NOP__, NOP__, CPYzp, CMPzp, DECzp, NOP__, INY__, CMPim, DEX__, NOP__, CPYab, CMPab, NOP__, NOP__, // C
			BNEre, CMPiy, NOP__, NOP__, NOP__, CMPzx, DECzx, NOP__, CLD__, CMPay, NOP__, NOP__, NOP__, CMPax, NOP__, NOP__, // D
			CPXim, SBCix, NOP__, NOP__, CPXzp, SBCzp, INCzp, NOP__, INX__, SBCim, NOP__, NOP__, CPXab, SBCab, NOP__, NOP__, // E
			BEQre, SBCiy, NOP__, NOP__, NOP__, SBCzx, INCzx, NOP__, SED__, SBCay, NOP__, NOP__, NOP__, SBCax, NOP__, NOP__, // F
			NMI__, RST__, IRQ__
		};
		
		//------------ Internal port ---------

		void ProcessPort(){
			/*
			if(AP->Read() == 1){
				if(IOP->Read() == 0){ Preg = DP->Read();}       // I'll leave this -> Notation just for the sake of it :)
				else{ DP->Write(Preg);}
			}
			*/
		}
		
		//--------------- end ----------------
		
	public:
	
		CPU_6510( int id,
				  StandardBus<uint16_t> *ap, StandardBus<uint8_t> *dp,
				  StandardBus<bool> *syncp, StandardBus<bool> *iop,
				  StandardBus<bool> *ep, Clock *clk,
				  CollectorBitBus *irq, StandardBus<bool> *nmi 
				
				) : Device(id){
					  
			AP = ap; DP = dp; SYNCP = syncp; IOP = iop; 
			EP = ep; CLK = clk; IRQ = irq; NMI = nmi;
			
			cycle = 0; SyncReg = 1; LastClkState = *CLK;
			IRQ_Pending = NMI_Pending = false;
			LastNmiLevel = *NMI;
			ResetRequest();
		}
		
		unsigned int GetCpuReg(int idx){
			idx = idx%15;
			unsigned int CpuReg[] = { Areg, Xreg, Yreg, Sreg, Freg, SyncReg, PC, Ireg, cycle, 
							          NMI_Pending, IRQ_Pending, RstRqs, AdBuf, DtBuf, IoBuf };
			return CpuReg[idx];
		}
		
		void ResetRequest(){ RstRqs = true;}       // The next instruction cycle will be a reset one
		
		void Evaluate(){                                                     // overloading the virtual function
		
			if(*NMI != LastNmiLevel && LastNmiLevel == 1){            // NMI trigger occured
				NMI_Pending = true; //cout << "Nmi\n";
			}
			
			if(*IRQ == 0 && (Freg & 0x04) == 0){                      // IRQ trigger occured (IRQ disable flag is checked)
				IRQ_Pending = true;
			}
			
			if(*CLK != LastClkState){                                 // clock edge
				
				if(LastClkState == 1){                                       // (high->low) clock transition. Address bus is updated
					if(cycle == 0){ AdBuf = PC; IoBuf = 1; SyncReg = 0;}
					if(IoBuf == 0){ DP->Write(DtBuf);}
					*AP = AdBuf; *IOP = IoBuf; *SYNCP = SyncReg;
					IoBuf = 1; SyncReg = 1;                                  // IoBuf has a strobe write protection
				}
				else{                                                        // (low->high) clock transition
					ProcessPort();                                           // process the internal port (PP and DP)
					DtBuf = *DP;
					if(cycle == 0){                                          // Interrupt overwrites
					    Ireg = DtBuf;
						if(RstRqs){ Ireg = 257;}                             // 1. Reset 
						else if(NMI_Pending){ Ireg = 256;}                   // 2. NMI execution, NMI instruction (special system instruction)   
						else if(IRQ_Pending){ Ireg = 258;}                   // 3. IRQ execution (BRK sets the 4'th bit of Freg)
					}
					(this->*functionPtr[Ireg])();                            // perform the cycle
					cycle++;
				}
			}
			
			LastNmiLevel = *NMI;                                             // edge trigger update
			LastClkState = *CLK;
		}	
		
		uint16_t getOpcode(){ return Ireg;}
		int getCycle(){ return cycle;}
};


//======================================== Simple LED devices ===========================



class Segment8D : public Device {
	private:
		StandardBus<uint8_t> *A, *B;
		StandardBus<bool> *E;      // enable chnages (active LOW)
		uint8_t *VP, *ui, *uj;
		long Lsize, i, j, k;
		
		int Discharge[8];
		
		void DrawPixel(char Col){ 
			*ui = Col; *(ui+3) = 0xff;
			*(ui+1) = *(ui+2) = 0x00;
		}
		
		void DrawSeg(int pos, char Col){
			
			if(Col == 0x00){ Col = 0x38;} 
			else{ Col = 0xff;}
			
			if(pos == -4 || pos == 4){                              // left (-4) and right (4)
				for(j = 0; j < 8; j+=2){
					for(i = 0; i < (17-j); i++){
						DrawPixel(Col); ui += Lsize;
					}
					ui = ui - Lsize*(17-j-1) + pos;
				}
			}
			
			if(pos == -1 || pos == 1){                             // bottom (-1) and top (1)
				for(j = 0; j < 8; j+=2){
					for(i = 0; i < (17-j); i++){
						DrawPixel(Col); ui += 4;
					}
					ui = ui - 4*(17-j-1) + (Lsize*pos);
				}
			}
			
			if(pos == 0){                                          // bottom (-1) and top (1)
				for( i = 0; i < 15; i++){DrawPixel(Col); ui += 4;} ui = ui + Lsize;
				for( i = 0; i < 17; i++){DrawPixel(Col); ui -= 4;} ui = ui + 4 + Lsize;
				for( i = 0; i < 17; i++){DrawPixel(Col); ui += 4;} ui = ui - 8 + Lsize;
				for( i = 0; i < 15; i++){DrawPixel(Col); ui -= 4;}
			}	
		}
		
		void DrawSeg(uint8_t bin, long x, long y){
			uj = VP + (4*x) + (Lsize*y);
			
			ui = uj; DrawSeg(4, bin & 0x04);                     // F
			ui = uj + 80; DrawSeg(-4, bin & 0x10);               // B
			ui = uj + (Lsize*21); DrawSeg(4, bin & 0x01);        // E
			ui = uj + 80 + (Lsize*21); DrawSeg(-4, bin & 0x20);  // C
			
			ui = uj + 8 - (Lsize*2); DrawSeg(1, bin & 0x08);     // A
			ui = uj + 8 + (Lsize*39); DrawSeg(-1, bin & 0x80);   // D
			
			ui = uj + 12 + (Lsize*17); DrawSeg(0, bin & 0x02);   // G
		}
	
	public:
		Segment8D( StandardBus<uint8_t> *ap, StandardBus<uint8_t> *bp, StandardBus<bool> *ep, 
				   uint8_t *vp, long size, int x, int y) : Device(0) {
					   
			A = ap; B = bp; E = ep; VP = vp + (x*4) + (y*size); Lsize = size;
			for(i = 0; i < 8; i++){ Discharge[i] = 1;} // by setting this to 1, all chars get the blank draw
		}
		
		void Evaluate(){
			uint8_t locA = *A, locB = *B; long xpos = 252;
			locB = locB << 2; locB |= 0x03;                               // there's a shift in the schematics (shouldn't actually be here)

			
			for(k = 0; k < 8; k++, xpos -= 32, locB = locB >> 1){
				
				if(k == 4){ xpos -= 17;}
				
				if(*E == 0){
					if((locB & 0x01) == 0x00){ 
						if(Discharge[k] == 0 || locA != 0x00){ 
							DrawSeg(locA, xpos, 12);                     // strobe 
							Discharge[k] = 100000;
						}
					}
				}
				
				if(Discharge[k] > 0){ 
					Discharge[k]--;
					if(Discharge[k] == 0){ DrawSeg(0x00, xpos, 12);}
				}   // each segment has a discharge time
				
			}
			
		}
};


//---------------- Single LED ----------------

class SquareLed : public Device {
	private:
		StandardBus<bool> *EP;
		uint8_t *VP, *ui; long Lsize;
		
		void DrawPixel(char Col){ 
			*ui = Col; *(ui+3) = 0xff;
			*(ui+1) = *(ui+2) = 0x00;
		}
		
		void DrawLed(char Col){
			int i, j;
			ui = VP;
			
			for(i = 0; i < 9; i++){
				for(j = 0; j < 9; j++){
					DrawPixel(Col); ui += 4;
				}
				ui = ui - 36 + Lsize;
			}
		}
		
	public:
		SquareLed( StandardBus<bool> *ep, uint8_t *vp, long size, int x, int y) : Device(0) {
			EP = ep; Lsize = size; VP = vp + (x*4) + (y*size);
		}
		
		// The LED device is given the enable pin, the main screen pointer, the bytes per line number, and the x/y coordinates
		// on the screen.
		
		void Evaluate(){
			if(*EP == false){ DrawLed(0x38);}
			else{ DrawLed(0xff);}
		}
};


//======================================== VRAM_8_32 ======================================

class VRAM_8_32 : public MemoryDevice<uint16_t, uint8_t> {
	private:
		uint8_t *VbuffP;                                            // pointer to bitmap data
		int Pallete[16][4] = {{  0,  0,  0,255},{255,255,255,255},{136,  0,  0,255},{170,255,238,255},
					   		  {204, 68,204,255},{  0,204, 85,255},{  0,  0,170,255},{238,238,119,255},
							  {221,136, 85,255},{102, 68,  0,255},{255,119,119,255},{ 51, 51, 51,255},
							  {119,119,119,255},{170,255,102,255},{  0,136,255,255},{187,187,187,255}};
	public:
		VRAM_8_32( int id, uint8_t *vp, StandardBus<bool> *ep,     // 6 argument constructor
				   StandardBus<uint16_t> *ap,                      // The "*iop" argument is optional (is data readable)
		           StandardBus<uint8_t> *dp, 
				   StandardBus<bool> *iop ) : MemoryDevice<uint16_t, uint8_t>(id, ep, 10, ap, 8, dp, iop){
			VbuffP = vp; Reset();
		}
		
		void Reset(){
			int i;
			for(i = 0; i < 1024; i++){ *(MemP+i) = 0x00;}
			for(i = 0; i < 4096; i++){ *(VbuffP+i) = 0x00;}
		}
		
		void Evaluate(){                                           // overloading the old evaluate function
			MemoryDevice<uint16_t, uint8_t>::Evaluate();           // (PRESENTATION NOTE) calling the parent's evaluate function
			if(*EP == 0 && *IOP == 0){
				uint16_t addr = *AP & maskA;
				int tmp = addr << 2;
				int tmp2 = *DP; tmp2 %= 16;
				for(int i = 0; i < 4; i++,tmp++){ *(VbuffP+tmp) = char(Pallete[tmp2][3-i]);}
			}
		}			  
};

//======================================== Keyboards ======================================

//-------------- One byte keyboard -------------------

template <class Type>
class KeyBoard : public Device {
	private:
		StandardBus<Type> *D;            // Input data bus
		StandardBus<bool> *E;            // enable 
		unsigned char *KeyP;             // key char pointer
	public:
		KeyBoard( StandardBus<Type> *dp, StandardBus<bool> *ep, unsigned char *kp
		        ) : Device(0) { D = dp; E = ep; KeyP = kp;}
		
		void Evaluate(){ 
			if(*E == 0){ *D = Type(*KeyP);}
		}
};

//------------- Special 6502kit keyboard -------------

class Keyboard_6502kit : public Device {
	private:
		StandardBus<uint8_t> *IB;            // Input data bus
		StandardBus<uint8_t> *OB;            // enable 
		int posX, posY;                      // position on screen
		int *mouseX, *mouseY;
		int *Code;                           // key button code (hosting system can use it special events)
		
		const int KeyMap[6][6] = {{99,99, 4,13,22,31}, {99,99, 3,12,21,30}, {27,25, 2,11,20,29},
								  {18,34, 1,10,19,28}, { 9,16,32,14, 5,15}, { 0, 7,33,23,24, 6}};
		
	public:
		Keyboard_6502kit( StandardBus<uint8_t> *ip, StandardBus<uint8_t> *op, int xpos, int ypos, int *xms, int *yms,
						  int *code ) : Device(0) { 
						  
						  IB = ip; OB = op; posX = xpos; posY = ypos; 
						  mouseX = xms; mouseY = yms; Code = code;
		}
		
		void Evaluate(){ 
			int x, y, i, j, k, xB, yB, idx;
			uint8_t tI, tO;
			
			x = (*mouseX - posX);  xB = -1;
			y = (*mouseY - posY);  yB = -1;
			
			for(i = 19, k = 0; i <= 491; i += 59, k++){
				if(i <= x && x <= (i+43)){ xB = k;}
			}
			
			for(i = 25, k = 0; i <= 178; i += 51, k++){
				if(i <= y && y <= (i+22)){ yB = k;}
			}
			
			if(xB >= 0 && yB >= 0){
				idx = xB + yB*9;
				*Code = idx;
				//cout << idx << endl;
				
				tI = *IB;
				//cout << "xxx:" << int(tI) << endl;
				for(i = 0; i < 6; i++, tI = tI >> 1){    // shift through all
					
					if((tI & 0x01) == 0x00){               // if the bit is zero, construct the result
						tO = 0xff;
						for(j = 0; j < 6; j++){           // construct the lower 6 bits via shifting
							tO = tO << 1;
							if(idx != KeyMap[i][j]){ tO |= 0x01;}
						}
						//cout << "zzz:" << int(tO) << endl;
					}	
				}
				
				*OB = tO;                 // transfer the result
			}
		}
};



//================================= Special Template Functions ============================

template <class Type, class Cast>
bool FileToMemory(Type &Arr, Cast mask, int ArrSize, int ArrOffset, string Path, long FileByteOffset, int NumElem){
	// (PRESENTATION NOTE) This function can handle ANY object (including standard arrays) that supports the [] operator
	int size, count; char buff[1024];
	if(ArrOffset >= ArrSize){ return false;}
	
	ifstream fin;
	fin.open(Path, ios::in | ios::binary);
	if(!fin.is_open()){ return false;}
	fin.seekg(FileByteOffset, ios::beg);
	
	count = 0;
	size = sizeof(Arr[0]); // (PRESENTATION NOTE) use of the overloaded [] operator! ---------v
	
	do{
		fin.read(buff, size);
		Arr[ArrOffset+count] = *(reinterpret_cast<Cast*>(buff));  // (PRESENTATION NOTE) reinterpret
		count++;
		
	}while(((ArrOffset+count) < ArrSize) && (count < NumElem));
	
	
	fin.close();
	return true;
}

template <class Type>
bool MemoryToFile(Type &Arr, int ArrSize, int ArrOffset, string Path, int NumElem){
	// (PRESENTATION NOTE) This function can handle ANY object (including standard arrays) that supports the [] operator
	int size, count;
	
	if(ArrOffset >= ArrSize){ return false;}
	
	ofstream fout;
	fout.open(Path, ios::out | ios::binary);
	if(!fout.is_open()){ return false;}
	
	size = sizeof(Arr[0]);  // (PRESENTATION NOTE) use of the overloaded [] operator! ----v
	
	for(count = 0; ((ArrOffset+count) < ArrSize) && (count < NumElem); count++){
		fout.write(reinterpret_cast<char*>( &(Arr[ArrOffset+count]) ), size);     // (PRESENTATION NOTE) reinterpret
	}
	
	fout.close();
	return true;
}

//=========================================== END =========================================







 








