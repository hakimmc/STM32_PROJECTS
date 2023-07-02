 /*______________________________INCLUDES_____________________________*/
#include "main.h"
#include "cmsis_os.h"
#include "string.h"
#include "stdio.h"

 /*______________________________TYPEDEF______________________________*/
CAN_HandleTypeDef hcan;
UART_HandleTypeDef huart2;

/*______________________________TASK_HANDLES______________________________*/
/* Definitions for CAN_TASK */
osThreadId_t CAN_TASKHandle;
const osThreadAttr_t CAN_TASK_attributes = {
  .name = "CAN_TASK",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for UART_TASK */
osThreadId_t UART_TASKHandle;
const osThreadAttr_t UART_TASK_attributes = {
  .name = "UART_TASK",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/*__________________________DECLARE_FUNCTIONS_________________________*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_CAN_Init(void);
static void MX_USART2_UART_Init(void);
void StartCanTask(void *argument);
void StartUartTask(void *argument);
int CheckSum(int summofbyte);
void CAN_PARAMS_INIT(void);
int Calculate_SummOfByte(uint8_t dlc_number);
void PREPARE_UARTS_DATA();

/*__________________________DECLARE_PARAMETERS________________________*/
volatile uint8_t receive_flag=0;
uint32_t __DLC = 1;
uint32_t __STD_ID = 0x12;
uint32_t __FILTER_ID_HIGH = 0x0000<<5;
uint32_t __FILTER_ID_LOW = 0x0000<<5;
uint32_t __FILTER_MASK_ID_HIGH = 0x0000<<5;
uint32_t __FILTER_MASK_ID_LOW = 0x0000<<5;
CAN_TxHeaderTypeDef pTxHeader;
CAN_RxHeaderTypeDef pRxHeader;
CAN_FilterTypeDef sFilterConfig;
uint32_t pTxMailbox;
uint8_t TxData;
uint8_t RxData[8];
uint8_t uart_data[25];

struct{
	//start frame 0xDD;
	union{
		int CAN_ID;
		struct{
			uint8_t BYTE4 :8;
			uint8_t BYTE3 :8;
			uint8_t BYTE2 :8;
			uint8_t BYTE1 :8;
		}_CAN_ID_BYTES;
	}_CAN_ID_UNION;
}CAN_FRAMES;


int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_CAN_Init();
  MX_USART2_UART_Init();
  HAL_CAN_Start(&hcan);
  CAN_PARAMS_INIT();
  osKernelInitialize();
  /*MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM*/
  CAN_TASKHandle = osThreadNew(StartCanTask, NULL, &CAN_TASK_attributes);
  /*WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW*/
  UART_TASKHandle = osThreadNew(StartUartTask, NULL, &UART_TASK_attributes);
  /*MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM*/
  osKernelStart();
  while (1)
  {
    /*RTOS?: 134:: CAN_TASK
     * 			::
     * */
  }
}

void StartCanTask(void *argument)
{
  for(;;)
  {
	  if(!(HAL_CAN_GetRxMessage(&hcan, CAN_RX_FIFO0, &pRxHeader, RxData))){
		  CAN_FRAMES._CAN_ID_UNION.CAN_ID=pRxHeader.StdId;
	  	  receive_flag=1;
	  	  uint8_t feedback[1];feedback[0]=pRxHeader.StdId;
	  	  HAL_CAN_AddTxMessage(&hcan, &pTxHeader, feedback, &pTxMailbox);
	  	  HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
	  }
  }
}

void StartUartTask(void *argument)
{
  for(;;)
  {
	if(receive_flag){
		//int Check_Sum = CheckSum(Calculate_SummOfByte(pRxHeader.DLC));
		PREPARE_UARTS_DATA();
		HAL_UART_Transmit(&huart2, uart_data,(uint16_t)(1+1+4+1+(int)(pRxHeader.DLC)+1) , 1000);
		receive_flag=0;
		  HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
	}
	else{
		//HAL_UART_Transmit(&huart2, (uint8_t*)"ff", sizeof("ff"), 100);
	}
	HAL_Delay(1000);
  }
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_CAN_Init(void)
{
  hcan.Instance = CAN1;
  hcan.Init.Prescaler = 4;
  hcan.Init.Mode = CAN_MODE_NORMAL;
  hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_15TQ;
  hcan.Init.TimeSeg2 = CAN_BS2_2TQ;
  hcan.Init.TimeTriggeredMode = DISABLE;
  hcan.Init.AutoBusOff = DISABLE;
  hcan.Init.AutoWakeUp = DISABLE;
  hcan.Init.AutoRetransmission = DISABLE;
  hcan.Init.ReceiveFifoLocked = DISABLE;
  hcan.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_USART2_UART_Init(void)
{
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 9600;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

void PREPARE_UARTS_DATA(){
	/*sprintf(uart_data,"0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x",0xDD,0x0C,CAN_FRAMES._CAN_ID_UNION._CAN_ID_BYTES.BYTE1,CAN_FRAMES._CAN_ID_UNION._CAN_ID_BYTES.BYTE2,CAN_FRAMES._CAN_ID_UNION._CAN_ID_BYTES.BYTE3,CAN_FRAMES._CAN_ID_UNION._CAN_ID_BYTES.BYTE4,__DLC);
	for(int i=0;i<pRxHeader.DLC;i++){
		strcat(uart_data,"0x%x",RxData[i]);
	}
	strcat(uart_data,"0x%x",CheckSum(Calculate_SummOfByte(pRxHeader.DLC)));*/
	uart_data[0]=0xDD;
	uart_data[1]=(uint8_t)(1+1+4+1+(uint8_t)(pRxHeader.DLC)+1);
	uart_data[2]=(uint8_t)(CAN_FRAMES._CAN_ID_UNION._CAN_ID_BYTES.BYTE1);
	uart_data[3]=(uint8_t)(CAN_FRAMES._CAN_ID_UNION._CAN_ID_BYTES.BYTE2);
	uart_data[4]=(uint8_t)(CAN_FRAMES._CAN_ID_UNION._CAN_ID_BYTES.BYTE3);
	uart_data[5]=(uint8_t)(CAN_FRAMES._CAN_ID_UNION._CAN_ID_BYTES.BYTE4);
	uart_data[6]=(uint8_t)(pRxHeader.DLC);
	for(int i=0;i<pRxHeader.DLC;i++){
		uart_data[7+i]=RxData[i];
	}
	uart_data[(uint8_t)(pRxHeader.DLC)+7]=CheckSum(Calculate_SummOfByte((uint8_t)(pRxHeader.DLC)));
}
void CAN_PARAMS_INIT(){
	pTxHeader.DLC = __DLC;
	pTxHeader.IDE = CAN_ID_STD;
	pTxHeader.RTR = CAN_RTR_DATA;
	pTxHeader.StdId = __STD_ID;

	//set filter parameters
	sFilterConfig.FilterActivation = ENABLE;
	sFilterConfig.FilterBank = 0;
	sFilterConfig.FilterFIFOAssignment = CAN_FILTER_FIFO0;
	sFilterConfig.FilterIdHigh = __FILTER_ID_HIGH ;
	sFilterConfig.FilterIdLow = __FILTER_ID_LOW ;
	sFilterConfig.FilterMaskIdHigh = __FILTER_MASK_ID_HIGH ;
	sFilterConfig.FilterMaskIdLow = __FILTER_MASK_ID_LOW ;
	sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
	sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;

	HAL_CAN_ConfigFilter(&hcan, &sFilterConfig);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }
}
int Calculate_SummOfByte(uint8_t dlc_number){
	int summ;
	for(int i=0;i<__DLC;i++){
		summ+=RxData[i];
	}
	return summ;
}
int CheckSum(int summofbyte){
	return 0xFF-(summofbyte%0xFF);
}

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}

#ifdef  USE_FULL_ASSERT

void assert_failed(uint8_t *file, uint32_t line)
{
}
#endif
