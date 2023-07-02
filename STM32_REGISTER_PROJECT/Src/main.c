/*
 ******************************************************************************
 * @file           : main.c
 * @author         : Abdulhakim Calgin
 * @brief          : -----------------
 ******************************************************************************
 */

/************** STEPS TO FOLLOW *****************
1. Enable GPIOD clock
2. Set the PIN PA12-13 as output
3. Configure the output mode i.e state, speed, and pull
************************************************/

#include <stdint.h>

typedef struct
{
  volatile uint32_t MODER;
  volatile uint32_t OTYPER;
  volatile uint32_t OSPEEDR;
  volatile uint32_t PUPDR;
  volatile uint32_t IDR;
  volatile uint32_t ODR;
  volatile uint32_t BSRR;
  volatile uint32_t LCKR;
  volatile uint32_t AFRL;
  volatile uint32_t AFRH;
} GPIO_TypeDef;

#define GPIOD_BASE_ADDR 	0x40020C00
#define RCC_BASE_ADDR 		0x40023800

#define GPIOD               ((GPIO_TypeDef *)GPIOD_BASE_ADDR)

/*
 *  RCC_BASE_ADDR = 0x40023800
 *  AHB1ENR_ADDR = 0x40023830
 *  GPIOD_EN = 1<<3;
 *  GPIOD_BASE_ADDR = 0x40020C00
 *
 */

uint32_t* AHB1ENR 		= 	(uint32_t*)(RCC_BASE_ADDR+0x30);
uint32_t* GPIOD_MODER 	= 	(uint32_t*)(GPIOD_BASE_ADDR);
uint16_t* GPIOD_OTYPER 	= 	(uint16_t*)(GPIOD_BASE_ADDR+0x04);
uint32_t* GPIOD_OSPEEDR = 	(uint32_t*)(GPIOD_BASE_ADDR+0x08);
uint32_t* GPIOD_PUPDR 	= 	(uint32_t*)(GPIOD_BASE_ADDR+0x0C);
uint16_t* GPIOD_ODR 	=	(uint16_t*)(GPIOD_BASE_ADDR+0x14);

void GPIO_INIT(void);
void Delay (uint32_t time);
volatile int bool=0;

int main(void)
{
	GPIO_INIT();
	GPIOD->ODR |= 1<<13;
    /* Loop forever */
	for(;;){
		//*GPIOD_ODR^=1<<12; //worked
		GPIOD->ODR ^= (1<<12| 1<<13) ;   //worked as well
		Delay(1000000);
	}
}

void Delay (uint32_t time)
{
	while (time--);
}

void GPIO_INIT(void){
	*AHB1ENR=1<<3;
	*GPIOD_MODER |=(1<<24 | 1<<26) ;
	*GPIOD_OTYPER &=(0<<12 & 0<<13);
	*GPIOD_OSPEEDR |= (1<<24 | 1<<26);
	*GPIOD_PUPDR &= (0<<24 & 0<<25 & 0<<26 & 0<<27);
}
