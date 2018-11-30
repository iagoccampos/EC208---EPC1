/*
	ADD - 	0000		0000		0000		0000
			Cód. op.	Reg. A		Reg. B		Reg. Sel.
			
	SUB - 	0001		0000		0000		0000
			Cód. op.	Reg. A		Reg. B		Reg. Sel.
			
	LOAD -	1000		0000		00000000
			Cód. op		Reg. sel	Mem. sel

	STORE -	1001		0000		00000000
			Cód. op		Reg. sel	Mem. sel

*/
#include<string>
#include<iostream>
#include<cmath>

using namespace std;

#define addOp 0b0000
#define subOp 0b0001
#define loadOp 0b1000
#define storeOp 0b1001

// 1 load end. 4 (7), guardando no reg 0
// 2 load end. 0 (3), guardando no reg 1
// 3 soma dos regs 0 (10) com 1 guardando no 2
// 4 load end. 3 (4), guardando no reg 3
// 5 sub dos regs 2 (10) com o 3(4) guardando em 4 (6)
// 6 soma dos regs 4 (6) com o 1(3) guardando em 4 (9)
// 7 soma dos regs 4 (9) com o 1(3) guardando em 4 (12)
// 8 soma dos regs 4 (12) com o 1(3) guardando em 4 (15)
// 9 soma dos regs 4 (15) com o 1(3) guardando em 4 (18)
// 10 soma dos regs 4 (18) com o 1(3) guardando em 4 (21)
// 11 soma dos regs 4 (21) com o 1(3) guardando em 4 (24)
// 12 store no end. 1 o reg 4
// resultado esperado: 24
int progMem[] = {0b1000000000000100, 0b1000000100000000, 0b0000000000010010, 0b1000001100000011, 0b0001001000110100, 0b0000010000010100, 0b0000010000010100, 0b0000010000010100,
				0b0000010000010100, 0b0000010000010100, 0b0000010000010100, 0b1001010000000001};

struct block
{
	int valid;
	int tag;
	int *instr;
};

struct memCache
{
	int blockCount;		//Quantidade de blocos
	int wordsPerBlock;	//Palavras por bloco
	int wordEndSize;	//Qtd de bits de end. de palavra
	int tagSize;		//Qtd de bits de tag
	int blockEndSize;	//Qtd de bits de end. de bloco
	
	block *blocks;
		
	void init(){
		blockCount = 8;
		wordsPerBlock = 2;
		tagSize = 11;
		blockEndSize = 3;
		wordEndSize = 1;
		
		blocks = new block[blockCount]; 
		
		clearCache();
	}
	
	void clearCache(){	
		for(int i = 0; i < blockCount; i++){
			blocks[i].valid = 0;						//Reseta Valid
			blocks[i].tag = 0;							//Reseta TAG
			blocks[i].instr = new int[wordsPerBlock]; 	//Ajusta o n. de palavras por bloco
			
			for(int j = 0; j < wordsPerBlock; j++){
				blocks[i].instr[j] = 0;					//Reseta as instruções
			}
		}
	}
	
	void pushMemory(int pc){
		int size = sizeof(progMem)/sizeof(int);
		int count = 0;
		//Aloca na cache o pc e o próximo pc
		for(int i = pc; i < size; i++){
			int line = (i >> wordEndSize)  & (blockCount - 1); 			//MOD 8
			int wordPos = i & (int)(pow(2,wordEndSize) - 1);
			int tag = (i >> blockEndSize + wordEndSize) & (int)(pow(2, tagSize) - 1);
			
			blocks[line].valid = 1;						//Set valid
			blocks[line].tag = tag;						//Atribui a TAG
			blocks[line].instr[wordPos] = progMem[i];	//Preenche a word com o comando
			
			count++;
			if(count == wordsPerBlock)
				break;
		}
	}
	
	int getInstr(int pc){
		int line = (pc >> wordEndSize) & (blockCount - 1); 	//Faz o cálculo da linha, MOD 8
		if(blocks[line].valid == 1){ 		//Se a valid da linha for 1
			int tag = (pc >> blockEndSize + wordEndSize) & (int)(pow(2, tagSize) - 1); //Calcula a tag
			if(blocks[line].tag == tag){	
				int wordPos = pc & (int)(pow(2,wordEndSize) - 1);
				return blocks[line].instr[wordPos];
			}
			else{
				//miss de tag
				return -1;
			}
		}
		else{
			//miss de valid
			return -1;
		}
	}
	
	//Printa os valores da memória cache
	void showCache(){
		cout << "Valid\tTAG\tWord1\tWord2" << endl;  
		for(int i = 0; i < blockCount; i++){
			cout << blocks[i].valid << "\t" << blocks[i].tag << "\t";
			for(int j = 0; j < wordsPerBlock; j++){
				cout << blocks[i].instr[j] << "\t";
			}
			cout << endl;
		}
	}
};

int currentOp = 0;
int currentRegA = 0;
int currentRegB = 0;
int currentRegSel = 0;
int currentMemSel = 0;

int currentInstr = 0;

int memReg[] = {0,0,0,0,0};

int memory[] = {3,2,5,4,7};

void decode();

void execute();

int main(void)
{
	memCache memC;
	
	memC.init();

	int pc = 0;
	
	int programLen = sizeof(progMem)/sizeof(int);
	
	while(pc < programLen)
	{
		cout << "PC: " << pc << endl;
		currentInstr = memC.getInstr(pc);
		
		if(currentInstr == -1){
			cout << "Miss, realocando..." << endl;
			memC.pushMemory(pc);
			currentInstr = memC.getInstr(pc);
		}
		
		decode();
		execute();
		pc++;
	}
	
	//memC.showCache();
	
	cout << "Resultado: " << memory[1] << endl;

	return 0;
}

void decode()
{
	currentOp = currentInstr >> 12;

	if(currentOp == addOp || currentOp == subOp)
	{
		currentRegA = (currentInstr >> 8) & 0b1111;
		currentRegB = (currentInstr >> 4) & 0b1111;
		currentRegSel = currentInstr & 0b1111;
	}
	else if(currentOp == loadOp || currentOp == storeOp)
	{
		currentRegSel = (currentInstr >> 8) & 0b1111;
		currentMemSel = currentInstr & 0b11111111;
	}
	else
	{
		cout << "Erro de codificacao do operador." << endl;
	}
}

void execute()
{
	switch(currentOp)
	{
		case addOp:
			memReg[currentRegSel] = memReg[currentRegA] + memReg[currentRegB];
			break;
		case subOp:
			memReg[currentRegSel] = memReg[currentRegA] - memReg[currentRegB];
			break;
		case loadOp:
			memReg[currentRegSel] = memory[currentMemSel];
			break;
		case storeOp:
			memory[currentMemSel] = memReg[currentRegSel];
			break;
		default:
			cout << "Erro de execucao." << endl;
			break;
	}
}

