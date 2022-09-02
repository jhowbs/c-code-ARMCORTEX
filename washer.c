/*-------------------------------------------------------
PORTS:
 Saídas
 PB0: Solenoide de entrada
 PB1: Solenoide de saída
 PB2: Motor de bater roupa
 PB3: Acionamentro Bomba
 PB4: Alarme

 Entradas:
 PE0: Sensor de nível Máximo
 PE1: Sensor de nível Mínimo
 PE2: Pressostato de água
 PE3: Botão Iniciar/Parar
-------------------------------------------------------*/

#include "PLL.h"
#include "SysTick.h"

#define OUTPUT (*((volatile unsigned long *)0x400050FC))
#define GPIO_PORTB_OUT (*((volatile unsigned long *)0x400050FC))
#define GPIO_PORTB_DIR_R (*((volatile unsigned long *)0x40005400))
#define GPIO_PORTB_AFSEL_R (*((volatile unsigned long *)0x40005420))
#define GPIO_PORTB_DEN_R (*((volatile unsigned long *)0x4000551C))
#define GPIO_PORTB_AMSEL_R (*((volatile unsigned long *)0x40005528))
#define GPIO_PORTB_PCTL_R (*((volatile unsigned long *)0x4000552C))
#define GPIO_PORTE_IN (*((volatile unsigned long *)0x4002400C))
#define INPUTS (*((volatile unsigned long *)0x4002400C))
#define GPIO_PORTE_DIR_R (*((volatile unsigned long *)0x40024400))
#define GPIO_PORTE_AFSEL_R (*((volatile unsigned long *)0x40024420))
#define GPIO_PORTE_DEN_R (*((volatile unsigned long *)0x4002451C))
#define GPIO_PORTE_AMSEL_R (*((volatile unsigned long *)0x40024528))

#define GPIO_PORTE_PCTL_R (*((volatile unsigned long *)0x4002452C))
#define SYSCTL_RCGC2_R (*((volatile unsigned long *)0x400FE108))
#define SYSCTL_RCGC2_GPIOE 0x00000010 
#define SYSCTL_RCGC2_GPIOB 0x00000002

struct State 
{
	unsigned long Out;
	unsigned long Time;
	unsigned long Next[12];
};
typedef const struct State STyp;

#define INICIO 		0
#define ENT_AGUA1 	1
#define LAVA 		2
#define ESCOA1 		3
#define ENT_AGUA2 	4
#define ENXAGUA 	5
#define ESCOA2 		6
#define CENTRIFUGA 	7
#define ESCOA3 		8
#define FIM 		9

STyp FSM[10]=
{
	{0x00, 1, 	{ INICIO,INICIO,INICIO,INICIO,INICIO,INICIO,INICIO,INICIO,INICIO,ENT_AGUA1,ENT_AGUA1,ENT_AGUA1}},
	{0x10, 200, { FIM,FIM,FIM,ENT_AGUA1,ENT_AGUA1,LAVA,FIM,ESCOA3,ESCOA3,FIM,ESCOA3,ESCOA3}},
	{0x04, 600, { ESCOA3,ESCOA3,LAVA,ESCOA3,ESCOA3,LAVA,FIM,ESCOA3,ESCOA3,FIM,ESCOA3,ESCOA3}},
	{0x0A, 200, { FIM,ESCOA1,ESCOA1,ENT_AGUA2,ESCOA1,ESCOA1,FIM,ESCOA3,ESCOA3,FIM,ESCOA3,ESCOA3}},
	{0x10, 200, { FIM,FIM,FIM,ENT_AGUA2,ENT_AGUA2,ENXAGUA,FIM,ESCOA3,ESCOA3,FIM,ESCOA3,ESCOA3}},
	{0x04, 300, { FIM,ESCOA3,ENXAGUA,FIM,ESCOA3,ENXAGUA,FIM,ESCOA3,ESCOA3,FIM,ESCOA3,ESCOA3}},
	{0x0A, 200, { FIM,ESCOA2,ESCOA2,CENTRIFUGA,ESCOA2,ESCOA2,FIM,ESCOA3,ESCOA3,FIM,ESCOA3,ESCOA3}},
	{0x04, 300, { CENTRIFUGA,ESCOA2,ESCOA2,CENTRIFUGA,ESCOA2,ESCOA2,FIM,ESCOA3,ESCOA3,FIM,ESCOA3,ESCOA3}},
	{0x0A, 200, { FIM,ESCOA3,ESCOA3,FIM,ESCOA3,ESCOA3,FIM,ESCOA3,ESCOA3,FIM,ESCOA3,ESCOA3}},
	{0x0B, 80, 	{ FIM,ESCOA3,ESCOA3,FIM,ESCOA3,ESCOA3,FIM,ESCOA3,ESCOA3,FIM,ESCOA3,ESCOA3}},
};

unsigned long S;
unsigned long Input;

int main(void)
{
	volatile unsigned long delay;
	
	PLL_Init(); 			
	SysTick_Init(); 			
	SYSCTL_RCGC2_R |= 0x12; 	
	delay = SYSCTL_RCGC2_R; 			
	GPIO_PORTE_AMSEL_R &= ~0x0F; 		
	GPIO_PORTE_PCTL_R &= ~0x000000FF; 
	GPIO_PORTE_DIR_R &= ~0x0F; 		
	GPIO_PORTE_AFSEL_R &= ~0x0F; 	
	GPIO_PORTE_DEN_R |= 0x0F; 		
	GPIO_PORTB_AMSEL_R &= ~0x3F; 	
	GPIO_PORTB_PCTL_R &= ~0x00FFFFFF; 	
	GPIO_PORTB_DIR_R |= 0x3F; 		
	GPIO_PORTB_AFSEL_R &= ~0x3F; 	
	GPIO_PORTB_DEN_R |= 0x3F; 	
	
	S = INICIO; 
	
	while(1)
	{
		OUTPUT = FSM[S].Out; 
		SysTick_Wait10ms(FSM[S].Time);
		Input = INPUTS; 
		S = FSM[S].Next[Input];
	}
}