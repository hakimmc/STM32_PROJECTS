 /*______________________________INCLUDES_____________________________*/
#include "main.h"

 /*______________________________TYPEDEF______________________________*/
CAN_HandleTypeDef hcan1;

/*__________________________DECLARE_FUNCTIONS_________________________*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_CAN1_Init(void);
void Error_Handler(void);
int WaitForMission(uint8_t dest_id,uint8_t id,int timeout,uint8_t data[]);
void CAN_PARAMS_INIT(void);

/*__________________________DECLARE_PARAMETERS________________________*/
uint32_t __DLC = 4;
uint32_t __STD_ID = 0x103;
uint32_t __FILTER_ID_HIGH = 0x0000<<5;
uint32_t __FILTER_ID_LOW = 0x0000<<5;
uint32_t __FILTER_MASK_ID_HIGH = 0x0000<<5;
uint32_t __FILTER_MASK_ID_LOW = 0x0000<<5;
CAN_TxHeaderTypeDef pTxHeader;
CAN_RxHeaderTypeDef pRxHeader;
CAN_FilterTypeDef sFilterConfig;
uint32_t pTxMailbox;
uint8_t TxData[4]={0x3,0x4,0x5,0x6};

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_CAN1_Init();

  HAL_CAN_Start(&hcan1);
  CAN_PARAMS_INIT();
  while (1)
  {
	  WaitForMission(0, __STD_ID, 250, TxData);
	  //HAL_CAN_AddTxMessage(&hcan1, &pTxHeader, TxData, &pTxMailbox);
	  HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_12);

	  //HAL_Delay(100);
  }
}


/*______________________________FUNCTIONS_____________________________*/

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    //Error_Handler();
	NVIC_SystemReset();
  }
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    //Error_Handler();
	NVIC_SystemReset();
  }
}
static void MX_CAN1_Init(void)
{
  hcan1.Instance = CAN1;
  hcan1.Init.Prescaler = 6;
  hcan1.Init.Mode = CAN_MODE_NORMAL;
  hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan1.Init.TimeSeg1 = CAN_BS1_11TQ;
  hcan1.Init.TimeSeg2 = CAN_BS2_2TQ;
  hcan1.Init.TimeTriggeredMode = DISABLE;
  hcan1.Init.AutoBusOff = DISABLE;
  hcan1.Init.AutoWakeUp = DISABLE;
  hcan1.Init.AutoRetransmission = DISABLE;
  hcan1.Init.ReceiveFifoLocked = DISABLE;
  hcan1.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan1) != HAL_OK)
  {
    //Error_Handler();
	NVIC_SystemReset();
  }
}

static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_RESET);

  /*Configure GPIO pins : PD12 PD13 PD14 PD15 */
  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

}

int WaitForMission(uint8_t dest_id,uint8_t id,int timeout,uint8_t data[]){
	int t_out=0;
	int r_val;
	//int durum=0;
	while(1){
		HAL_CAN_AddTxMessage(&hcan1, &pTxHeader, data, &pTxMailbox);
		uint8_t rxData[1];
		HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &pRxHeader, rxData);
		if(rxData[0]==id && (dest_id==pRxHeader.StdId || !dest_id)){ // dest_id == 0: check every id
			r_val = 1;
			break;
		}
		t_out++;
		HAL_Delay(1);
		if(t_out==timeout){
			r_val = 0;
			break;
		}
	}
	return r_val;

	//return 1: OK
	//return 0: TIME PROBLEM
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

	HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig);
}

void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */


