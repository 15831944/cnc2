#ifndef INC_QSPI_FLASH_H_
#define INC_QSPI_FLASH_H_

//#include "mt25ql512abb.h"
#include "mx25l512.h"

#define QSPI_RX_BUFFER_SIZE		(MX25L512_PAGE_SIZE)

/* QSPI Error codes */
#define QSPI_OK            ((uint8_t)0x00)
#define QSPI_ERROR         ((uint8_t)0x01)
#define QSPI_BUSY          ((uint8_t)0x02)
#define QSPI_NOT_SUPPORTED ((uint8_t)0x04)
#define QSPI_SUSPENDED     ((uint8_t)0x08)


/* Definition for QSPI clock resources */
//#define QSPI_CLK_ENABLE()          __HAL_RCC_QSPI_CLK_ENABLE()
//#define QSPI_CLK_DISABLE()         __HAL_RCC_QSPI_CLK_DISABLE()
//#define QSPI_CS_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOB_CLK_ENABLE()
//#define QSPI_CLK_GPIO_CLK_ENABLE() __HAL_RCC_GPIOB_CLK_ENABLE()
//#define QSPI_D0_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOC_CLK_ENABLE()
//#define QSPI_D1_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOC_CLK_ENABLE()
//#define QSPI_D2_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOE_CLK_ENABLE()
//#define QSPI_D3_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOD_CLK_ENABLE()

#define QSPI_FORCE_RESET()         __HAL_RCC_QSPI_FORCE_RESET()
#define QSPI_RELEASE_RESET()       __HAL_RCC_QSPI_RELEASE_RESET()

/* QSPI Info */
typedef struct {
  uint32_t FlashSize;          /*!< Size of the flash */
  uint32_t EraseSectorSize;    /*!< Size of sectors for the erase operation */
  uint32_t EraseSectorsNumber; /*!< Number of sectors for the erase operation */
  uint32_t ProgPageSize;       /*!< Size of pages for the program operation */
  uint32_t ProgPagesNumber;    /*!< Number of pages for the program operation */
} QSPI_Info;

/* Exported functions --------------------------------------------------------*/
/** @addtogroup STM32F769I_DISCOVERY_QSPI_Exported_Functions
  * @{
  */
uint8_t BSP_QSPI_Init       ();
uint8_t BSP_QSPI_DeInit     ();
uint8_t BSP_QSPI_Read       (uint8_t* pData, uint32_t ReadAddr, uint32_t Size);
uint8_t BSP_QSPI_Write      (uint8_t* pData, uint32_t WriteAddr, uint32_t Size);
uint8_t BSP_QSPI_Erase_Block(uint32_t BlockAddress);
uint8_t BSP_QSPI_Erase_Chip ();
uint8_t BSP_QSPI_GetStatus  ();
uint8_t BSP_QSPI_GetInfo    (QSPI_Info* pInfo);
uint8_t BSP_QSPI_EnableMemoryMappedMode();

/* These functions can be modified in case the current settings
   need to be changed for specific application needs */
void BSP_QSPI_MspInit(QSPI_HandleTypeDef *hqspi, void *Params);
void BSP_QSPI_MspDeInit(QSPI_HandleTypeDef *hqspi, void *Params);

void QSPI_printResultValue(uint8_t res);

void qspi_test_blocking();
void qspi_test_it();

#endif /* INC_QSPI_FLASH_H_ */
