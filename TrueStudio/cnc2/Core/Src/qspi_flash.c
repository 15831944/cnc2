#include <stdio.h>

#include "my_types.h"

#include "quadspi.h"
#include "qspi_flash.h"
#include "aux_func.h"

volatile uint8_t CmdCplt, RxCplt, TxCplt, StatusMatch, TimeOut;

/* Buffer used for transmission */
uint8_t tx_buf[] = " ****QSPI communication based on IT****  ****QSPI communication based on IT****  ****QSPI communication based on IT****  ****QSPI communication based on IT****  ****QSPI communication based on IT****  ****QSPI communication based on IT**** ";

/* Buffer used for reception */
uint8_t rx_buf[QSPI_RX_BUFFER_SIZE];

#define MIN(a, b)  (((a) < (b)) ? (a) : (b))

/* Private function prototypes -----------------------------------------------*/
static uint8_t QSPI_WriteEnable(QSPI_HandleTypeDef *hqspi);
static uint8_t QSPI_AutoPollingMemReady(QSPI_HandleTypeDef *hqspi, uint32_t Timeout);
static uint8_t QSPI_DummyCyclesCfg(QSPI_HandleTypeDef *hqspi);
static uint8_t QSPI_ResetMemory(QSPI_HandleTypeDef *hqspi);
static uint8_t QSPI_EnterFourBytesAddress(QSPI_HandleTypeDef *hqspi);
static uint8_t QSPI_EnterMemory_QPI(QSPI_HandleTypeDef *hqspi);
static uint8_t QSPI_ExitMemory_QPI(QSPI_HandleTypeDef *hqspi);
static uint8_t QSPI_OutDrvStrengthCfg(QSPI_HandleTypeDef *hqspi);
/* Private functions ---------------------------------------------------------*/

void QSPI_printResultValue(uint8_t res) {
	switch (res) {
	case QSPI_OK:
		printf("QSPI_OK\n");
		break;
	case QSPI_ERROR:
		printf("QSPI_ERR\n");
		break;
	case QSPI_BUSY:
		printf("QSPI_BUSY\n");
		break;
	case QSPI_NOT_SUPPORTED:
		printf("QSPI_NOT_SUPPORTED\n");
		break;
	case QSPI_SUSPENDED:
		printf("QSPI_SUSPENDED\n");
		break;
	default:
		printf("RES_ERR\n");
		break;
	}
}

void qspi_test_blocking() {
	uint8_t res;
	uint32_t wraddr = 0;

	printf("Init QSPI\n");
	res = BSP_QSPI_Init();
	QSPI_printResultValue(res);

	printf("Erase\n");
	res = BSP_QSPI_Erase_Block(wraddr);
	QSPI_printResultValue(res);

	for (int i = 0; i < sizeof(tx_buf); i++)
		tx_buf[i] = i;

	printf("Write\n");
	res = BSP_QSPI_Write(tx_buf, wraddr, sizeof(tx_buf));
	QSPI_printResultValue(res);

	printf("Read\n");
	memset(rx_buf, 0, sizeof(rx_buf));
	res = BSP_QSPI_Read(rx_buf, wraddr, sizeof(rx_buf));
	QSPI_printResultValue(res);

	printBytes(rx_buf, sizeof(rx_buf), 0, sizeof(rx_buf));
}

void qspi_test() {
	static QSPI_CommandTypeDef sCommand;
	static uint32_t address = 0;
	static uint8_t step = 0;

	BSP_QSPI_Init();
//	BSP_QSPI_EnableMemoryMappedMode();

	sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
	sCommand.AddressSize       = QSPI_ADDRESS_24_BITS;
	sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
	sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
	sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

	while (1) {
		switch (step) {
		case 0:
			CmdCplt = 0;
			memset(rx_buf, 0, QSPI_RX_BUFFER_SIZE);
			QSPI_WriteEnable(&hqspi);

			/* Erasing Sequence -------------------------------------------------- */
			sCommand.Instruction = SECTOR_ERASE_CMD;
			sCommand.AddressMode = QSPI_ADDRESS_1_LINE;
			sCommand.Address     = address;
			sCommand.DataMode    = QSPI_DATA_NONE;
			sCommand.DummyCycles = 0;

			if (HAL_QSPI_Command_IT(&hqspi, &sCommand) != HAL_OK)
				Error_Handler();

			step++;
			break;

		case 1:
			if (CmdCplt != 0) {
				CmdCplt = 0;
				StatusMatch = 0;

				/* Configure automatic polling mode to wait for end of erase ------- */
				QSPI_AutoPollingMemReady(&hqspi, HAL_QPSI_TIMEOUT_DEFAULT_VALUE);

				step++;
			}
			break;

		case 2:
			if (StatusMatch != 0) {
				StatusMatch = 0;
				TxCplt = 0;

				/* Enable write operations ----------------------------------------- */
				QSPI_WriteEnable(&hqspi);

				/* Writing Sequence ------------------------------------------------ */
				sCommand.Instruction = QUAD_IN_FAST_PROG_CMD;
				sCommand.AddressMode = QSPI_ADDRESS_1_LINE;
				sCommand.DataMode    = QSPI_DATA_4_LINES;
				sCommand.NbData      = QSPI_RX_BUFFER_SIZE;

				if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
				  Error_Handler();

				if (HAL_QSPI_Transmit_IT(&hqspi, tx_buf) != HAL_OK)
				  Error_Handler();

				step++;
			}
			break;

		case 3:
    	  if (TxCplt != 0) {
          TxCplt = 0;
          StatusMatch = 0;

          /* Configure automatic polling mode to wait for end of program ----- */
          QSPI_AutoPollingMemReady(&hqspi, HAL_QPSI_TIMEOUT_DEFAULT_VALUE);

          step++;
        }
        break;

      case 4:
        if (StatusMatch != 0) {
			StatusMatch = 0;
			RxCplt = 0;

			/* Configure Volatile Configuration register (with new dummy cycles) */
			QSPI_DummyCyclesCfg(&hqspi);

			/* Reading Sequence ------------------------------------------------ */
			sCommand.Instruction = QUAD_OUT_FAST_READ_CMD;
			sCommand.DummyCycles = MX25L512_DUMMY_CYCLES_READ_QUAD;

			if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
			  Error_Handler();

			if (HAL_QSPI_Receive_IT(&hqspi, rx_buf) != HAL_OK)
			  Error_Handler();

			step++;
        }
        break;

      case 5:
        if (RxCplt != 0) {
          RxCplt = 0;

          /* Result comparison ----------------------------------------------- */
          for (int i = 0; i < MIN(sizeof(rx_buf), sizeof(tx_buf)); i++)
        	  if (rx_buf[i] != tx_buf[i])
        		  printf("Error\n");

          address += MX25L512_PAGE_SIZE;

          step = 0;
        }
        break;

      default :
        Error_Handler();
    }

    if (address >= MX25L512_FLASH_SIZE) {
  	  address = 0;
  	  break;
    }
  }
}

/**
  * @brief  Command completed callbacks.
  * @param  hqspi: QSPI handle
  * @retval None
  */
void HAL_QSPI_CmdCpltCallback(QSPI_HandleTypeDef *hqspi) {
  CmdCplt++;
}

/**
  * @brief  Rx Transfer completed callbacks.
  * @param  hqspi: QSPI handle
  * @retval None
  */
void HAL_QSPI_RxCpltCallback(QSPI_HandleTypeDef *hqspi) {
  RxCplt++;
}

/**
  * @brief  Tx Transfer completed callbacks.
  * @param  hqspi: QSPI handle
  * @retval None
  */
void HAL_QSPI_TxCpltCallback(QSPI_HandleTypeDef *hqspi) {
  TxCplt++;
}

/**
  * @brief  Status Match callbacks
  * @param  hqspi: QSPI handle
  * @retval None
  */
void HAL_QSPI_StatusMatchCallback(QSPI_HandleTypeDef *hqspi) {
  StatusMatch++;
}

/**
  * @brief  Initializes the QSPI interface.
  * @retval QSPI memory status
  */
uint8_t BSP_QSPI_Init() {
	hqspi.Instance = QUADSPI;

	/* Call the DeInit function to reset the driver */
	if (HAL_QSPI_DeInit(&hqspi) != HAL_OK)
		return QSPI_ERROR;

	/* System level initialization */
	//  BSP_QSPI_MspInit(&hqspi, NULL);
	HAL_QSPI_MspInit(&hqspi);

	/* QSPI initialization */
	/* QSPI freq = SYSCLK /(1 + ClockPrescaler) = 216 MHz/(1+1) = 108 Mhz */
	hqspi.Init.ClockPrescaler     = 9;   /* QSPI freq = 400 MHz/(1+9) = 40 Mhz */ // 1
	hqspi.Init.FifoThreshold      = 4; // 16
	hqspi.Init.SampleShifting     = QSPI_SAMPLE_SHIFTING_HALFCYCLE;
	hqspi.Init.FlashSize          = POSITION_VAL(MX25L512_FLASH_SIZE) - 1;
	hqspi.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_4_CYCLE; /* Min 30ns for nonRead */
	hqspi.Init.ClockMode          = QSPI_CLOCK_MODE_0;
	hqspi.Init.FlashID            = QSPI_FLASH_ID_1;
	hqspi.Init.DualFlash          = QSPI_DUALFLASH_DISABLE;

	if (HAL_QSPI_Init(&hqspi) != HAL_OK)
		return QSPI_ERROR;

	/* QSPI memory reset */
	if (QSPI_ResetMemory(&hqspi) != QSPI_OK)
		return QSPI_NOT_SUPPORTED;

	/* Put QSPI memory in QPI mode */
	if( QSPI_EnterMemory_QPI( &hqspi )!=QSPI_OK )
		return QSPI_NOT_SUPPORTED;

	/* Set the QSPI memory in 4-bytes address mode */
	if (QSPI_EnterFourBytesAddress(&hqspi) != QSPI_OK)
		return QSPI_NOT_SUPPORTED;

	/* Configuration of the dummy cycles on QSPI memory side */
	if (QSPI_DummyCyclesCfg(&hqspi) != QSPI_OK)
		return QSPI_NOT_SUPPORTED;

	/* Configuration of the Output driver strength on memory side */
	if( QSPI_OutDrvStrengthCfg( &hqspi ) != QSPI_OK )
		return QSPI_NOT_SUPPORTED;

	return QSPI_OK;
}

/**
  * @brief  De-Initializes the QSPI interface.
  * @retval QSPI memory status
  */
uint8_t BSP_QSPI_DeInit(void)
{
	hqspi.Instance = QUADSPI;

  /* Put QSPI memory in SPI mode */
  if( QSPI_ExitMemory_QPI(&hqspi )!=QSPI_OK )
  {
    return QSPI_NOT_SUPPORTED;
  }

  /* Call the DeInit function to reset the driver */
  if (HAL_QSPI_DeInit(&hqspi) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* System level De-initialization */
  BSP_QSPI_MspDeInit(&hqspi, NULL);

  return QSPI_OK;
}

/**
  * @brief  This function send a Write Enable and wait it is effective.
  * @param  hqspi: QSPI handle
  * @retval None
  */
static uint8_t QSPI_WriteEnable(QSPI_HandleTypeDef *hqspi) {
	QSPI_CommandTypeDef     s_command;
	QSPI_AutoPollingTypeDef s_config;

	/* Enable write operations */
	s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES; // QSPI_INSTRUCTION_1_LINE
	s_command.Instruction       = WRITE_ENABLE_CMD;
	s_command.AddressMode       = QSPI_ADDRESS_NONE;
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DataMode          = QSPI_DATA_NONE;
	s_command.DummyCycles       = 0;
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

	if (HAL_QSPI_Command(hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
		return QSPI_ERROR;

	/* Configure automatic polling mode to wait for write enabling */
	s_config.Match           = MX25L512_SR_WREN;
	s_config.Mask            = MX25L512_SR_WREN;
	s_config.MatchMode       = QSPI_MATCH_MODE_AND;
	s_config.StatusBytesSize = 1;
	s_config.Interval        = 0x10;
	s_config.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;

	s_command.Instruction    = READ_STATUS_REG_CMD;
	s_command.DataMode       = QSPI_DATA_4_LINES; // QSPI_DATA_1_LINE

	if (HAL_QSPI_AutoPolling(hqspi, &s_command, &s_config, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
		return QSPI_ERROR;

	return QSPI_OK;
}

/**
  * @brief  This function read the SR of the memory and wait the EOP.
  * @param  hqspi: QSPI handle
  * @param  Timeout
  * @retval None
  */
static uint8_t QSPI_AutoPollingMemReady(QSPI_HandleTypeDef *hqspi, uint32_t Timeout) {
	QSPI_CommandTypeDef     s_command;
	QSPI_AutoPollingTypeDef s_config;

	/* Configure automatic polling mode to wait for memory ready */
	s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES; // QSPI_INSTRUCTION_1_LINE
	s_command.Instruction       = READ_STATUS_REG_CMD;
	s_command.AddressMode       = QSPI_ADDRESS_NONE;
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DataMode          = QSPI_DATA_4_LINES; // QSPI_DATA_1_LINE
	s_command.DummyCycles       = 0;
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

	s_config.Match           = 0;
	s_config.Mask            = MX25L512_SR_WIP;
	s_config.MatchMode       = QSPI_MATCH_MODE_AND;
	s_config.StatusBytesSize = 1;
	s_config.Interval        = 0x10;
	s_config.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;

	if (HAL_QSPI_AutoPolling(hqspi, &s_command, &s_config, Timeout) != HAL_OK)
		return QSPI_ERROR;

	return QSPI_OK;
}

/**
  * @brief  This function configure the dummy cycles on memory side.
  * @param  hqspi: QSPI handle
  * @retval None
  */
static uint8_t QSPI_DummyCyclesCfg(QSPI_HandleTypeDef *hqspi) {
	QSPI_CommandTypeDef s_command;
	uint8_t reg[2];

	/* Initialize the reading of status register */
	s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES; // QSPI_INSTRUCTION_1_LINE
	s_command.Instruction       = READ_STATUS_REG_CMD;
	s_command.AddressMode       = QSPI_ADDRESS_NONE;
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DataMode          = QSPI_DATA_4_LINES; // QSPI_DATA_1_LINE
	s_command.DummyCycles       = 0;
	s_command.NbData            = 1;
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

	/* Configure the command */
	if (HAL_QSPI_Command(hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
		return QSPI_ERROR;

	/* Reception of the data */
	if (HAL_QSPI_Receive(hqspi, &(reg[0]), HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
		return QSPI_ERROR;

	/* Initialize the reading of configuration register */
	s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
	s_command.Instruction       = READ_CFG_REG_CMD;
	s_command.AddressMode       = QSPI_ADDRESS_NONE;
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DataMode          = QSPI_DATA_4_LINES;
	s_command.DummyCycles       = 0;
	s_command.NbData            = 1;
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

	/* Configure the command */
	if (HAL_QSPI_Command(hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
		return QSPI_ERROR;

	/* Reception of the data */
	if (HAL_QSPI_Receive(hqspi, &(reg[1]), HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
		return QSPI_ERROR;

	/* Enable write operations */
	if (QSPI_WriteEnable(hqspi) != QSPI_OK)
		return QSPI_ERROR;

	/* Update the configuration register with new dummy cycles */
	s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
	s_command.Instruction       = WRITE_STATUS_CFG_REG_CMD;
	s_command.AddressMode       = QSPI_ADDRESS_NONE;
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DataMode          = QSPI_DATA_4_LINES;
	s_command.DummyCycles       = 0;
	s_command.NbData            = 2;
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

	/* MT25QL512_DUMMY_CYCLES_READ_QUAD = 3 for 10 cycles in QPI mode */
	MODIFY_REG( reg[1], MX25L512_CR_NB_DUMMY, (MX25L512_DUMMY_CYCLES_READ_QUAD << POSITION_VAL(MX25L512_CR_NB_DUMMY)));

	/* Configure the write volatile configuration register command */
	if (HAL_QSPI_Command(hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
		return QSPI_ERROR;

	/* Transmission of the data */
	if (HAL_QSPI_Transmit(hqspi, &(reg[0]), HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
		return QSPI_ERROR;

	/* 40ms  Write Status/Configuration Register Cycle Time */
	HAL_Delay( 40 );

	return QSPI_OK;
}

// #################################################################################
/**
  * @brief  Reads an amount of data from the QSPI memory.
  * @param  pData: Pointer to data to be read
  * @param  ReadAddr: Read start address
  * @param  Size: Size of data to read
  * @retval QSPI memory status
  */
uint8_t BSP_QSPI_Read(uint8_t* pData, uint32_t ReadAddr, uint32_t Size) {
	QSPI_CommandTypeDef s_command;

	/* Initialize the read command */
	s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
	s_command.Instruction       = QPI_READ_4_BYTE_ADDR_CMD;
	s_command.AddressMode       = QSPI_ADDRESS_4_LINES;
	s_command.AddressSize       = QSPI_ADDRESS_32_BITS;
	s_command.Address           = ReadAddr;
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DataMode          = QSPI_DATA_4_LINES;
	s_command.DummyCycles       = MX25L512_DUMMY_CYCLES_READ_QUAD_IO;
	s_command.NbData            = Size;
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

	/* Configure the command */
	if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
		return QSPI_ERROR;

	/* Set S# timing for Read command */
	MODIFY_REG(hqspi.Instance->DCR, QUADSPI_DCR_CSHT, QSPI_CS_HIGH_TIME_1_CYCLE);

	/* Reception of the data */
	if (HAL_QSPI_Receive(&hqspi, pData, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
		return QSPI_ERROR;

	/* Restore S# timing for nonRead commands */
	MODIFY_REG(hqspi.Instance->DCR, QUADSPI_DCR_CSHT, QSPI_CS_HIGH_TIME_4_CYCLE);

	return QSPI_OK;
}

/**
  * @brief  Writes an amount of data to the QSPI memory.
  * @param  pData: Pointer to data to be written
  * @param  WriteAddr: Write start address
  * @param  Size: Size of data to write
  * @retval QSPI memory status
  */
uint8_t BSP_QSPI_Write(uint8_t* pData, uint32_t WriteAddr, uint32_t Size) {
	QSPI_CommandTypeDef s_command;
	uint32_t end_addr, current_size, current_addr;

	/* Calculation of the size between the write address and the end of the page */
	current_size = MX25L512_PAGE_SIZE - (WriteAddr % MX25L512_PAGE_SIZE);

	/* Check if the size of the data is less than the remaining place in the page */
	if (current_size > Size)
		current_size = Size;

	/* Initialize the address variables */
	current_addr = WriteAddr;
	end_addr = WriteAddr + Size;

	/* Initialize the program command */
	s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
	s_command.Instruction       = QPI_PAGE_PROG_4_BYTE_ADDR_CMD;
	s_command.AddressMode       = QSPI_ADDRESS_4_LINES;
	s_command.AddressSize       = QSPI_ADDRESS_32_BITS;
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DataMode          = QSPI_DATA_4_LINES;
	s_command.DummyCycles       = 0;
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

	/* Perform the write page by page */
	do {
		s_command.Address = current_addr;
		s_command.NbData  = current_size;

		/* Enable write operations */
		if (QSPI_WriteEnable(&hqspi) != QSPI_OK)
			return QSPI_ERROR;

		/* Configure the command */
		if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
			return QSPI_ERROR;

		/* Transmission of the data */
		if (HAL_QSPI_Transmit(&hqspi, pData, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
			return QSPI_ERROR;

		/* Configure automatic polling mode to wait for end of program */
		if (QSPI_AutoPollingMemReady(&hqspi, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != QSPI_OK)
			return QSPI_ERROR;

		/* Update the address and size variables for next page programming */
		current_addr += current_size;
		pData += current_size;
		current_size = ((current_addr + MX25L512_PAGE_SIZE) > end_addr) ? (end_addr - current_addr) : MX25L512_PAGE_SIZE;
	} while (current_addr < end_addr);

	return QSPI_OK;
}

/**
  * @brief  Erases the specified block of the QSPI memory.
  * @param  BlockAddress: Block address to erase
  * @retval QSPI memory status
  */
uint8_t BSP_QSPI_Erase_Block(uint32_t BlockAddress) {
	QSPI_CommandTypeDef s_command;

	/* Initialize the erase command */
	s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
	s_command.Instruction       = SUBSECTOR_ERASE_4_BYTE_ADDR_CMD;
	s_command.AddressMode       = QSPI_ADDRESS_4_LINES;
	s_command.AddressSize       = QSPI_ADDRESS_32_BITS;
	s_command.Address           = BlockAddress;
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DataMode          = QSPI_DATA_NONE;
	s_command.DummyCycles       = 0;
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

	/* Enable write operations */
	if (QSPI_WriteEnable(&hqspi) != QSPI_OK)
		return QSPI_ERROR;

	/* Send the command */
	if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
		return QSPI_ERROR;

	/* Configure automatic polling mode to wait for end of erase */
	if (QSPI_AutoPollingMemReady(&hqspi, MX25L512_SUBSECTOR_ERASE_MAX_TIME) != QSPI_OK)
		return QSPI_ERROR;

	return QSPI_OK;
}

/**
  * @brief  Erases the entire QSPI memory.
  * @retval QSPI memory status
  */
uint8_t BSP_QSPI_Erase_Chip() {
	QSPI_CommandTypeDef s_command;

	/* Initialize the erase command */
	s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
	s_command.Instruction       = BULK_ERASE_CMD;
	s_command.AddressMode       = QSPI_ADDRESS_NONE;
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DataMode          = QSPI_DATA_NONE;
	s_command.DummyCycles       = 0;
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

	/* Enable write operations */
	if (QSPI_WriteEnable(&hqspi) != QSPI_OK)
		return QSPI_ERROR;

	/* Send the command */
	if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
		return QSPI_ERROR;

	/* Configure automatic polling mode to wait for end of erase */
	if (QSPI_AutoPollingMemReady(&hqspi, MX25L512_BULK_ERASE_MAX_TIME) != QSPI_OK)
		return QSPI_ERROR;

	return QSPI_OK;
}

/**
  * @brief  Reads current status of the QSPI memory.
  * @retval QSPI memory status
  */
uint8_t BSP_QSPI_GetStatus() {
	QSPI_CommandTypeDef s_command;
	uint8_t reg;

	/* Initialize the read flag status register command */
	s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
	s_command.Instruction       = READ_STATUS_REG_CMD;
	s_command.AddressMode       = QSPI_ADDRESS_NONE;
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DataMode          = QSPI_DATA_4_LINES;
	s_command.DummyCycles       = 0;
	s_command.NbData            = 1;
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

	/* Configure the command */
	if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
		return QSPI_ERROR;

	/* Reception of the data */
	if (HAL_QSPI_Receive(&hqspi, &reg, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
		return QSPI_ERROR;

	/* Check the value of the register*/
	if ((reg & MX25L512_SR_WIP) == 0)
		return QSPI_OK;
	else
		return QSPI_BUSY;
}

/**
  * @brief  Return the configuration of the QSPI memory.
  * @param  pInfo: pointer on the configuration structure
  * @retval QSPI memory status
  */
uint8_t BSP_QSPI_GetInfo(QSPI_Info* pInfo) {
	/* Configure the structure with the memory configuration */
	pInfo->FlashSize          = MX25L512_FLASH_SIZE;
	pInfo->EraseSectorSize    = MX25L512_SUBSECTOR_SIZE;
	pInfo->EraseSectorsNumber = (MX25L512_FLASH_SIZE/MX25L512_SUBSECTOR_SIZE);
	pInfo->ProgPageSize       = MX25L512_PAGE_SIZE;
	pInfo->ProgPagesNumber    = (MX25L512_FLASH_SIZE/MX25L512_PAGE_SIZE);

	return QSPI_OK;
}

/**
  * @brief  Configure the QSPI in memory-mapped mode
  * @retval QSPI memory status
  */
uint8_t BSP_QSPI_EnableMemoryMappedMode() {
	QSPI_CommandTypeDef      s_command;
	QSPI_MemoryMappedTypeDef s_mem_mapped_cfg;

	/* Configure the command for the read instruction */
	s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
	s_command.Instruction       = QPI_READ_4_BYTE_ADDR_CMD;
	s_command.AddressMode       = QSPI_ADDRESS_4_LINES;
	s_command.AddressSize       = QSPI_ADDRESS_32_BITS;
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DataMode          = QSPI_DATA_4_LINES;
	s_command.DummyCycles       = MX25L512_DUMMY_CYCLES_READ_QUAD_IO;
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

	/* Configure the memory mapped mode */
	s_mem_mapped_cfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE;
	s_mem_mapped_cfg.TimeOutPeriod     = 0;

	if (HAL_QSPI_MemoryMapped(&hqspi, &s_command, &s_mem_mapped_cfg) != HAL_OK)
		return QSPI_ERROR;

	return QSPI_OK;
}

/**
  * @brief  This function reset the QSPI memory.
  * @param  hqspi: QSPI handle
  * @retval None
  */
static uint8_t QSPI_ResetMemory(QSPI_HandleTypeDef *hqspi) {
	QSPI_CommandTypeDef      s_command;
	QSPI_AutoPollingTypeDef  s_config;
	uint8_t                  reg;

	/* Send command RESET command in QPI mode (QUAD I/Os) */
	/* Initialize the reset enable command */
	s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
	s_command.Instruction       = RESET_ENABLE_CMD;
	s_command.AddressMode       = QSPI_ADDRESS_NONE;
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DataMode          = QSPI_DATA_NONE;
	s_command.DummyCycles       = 0;
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
	/* Send the command */
	if (HAL_QSPI_Command(hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
		return QSPI_ERROR;

	/* Send the reset memory command */
	s_command.Instruction = RESET_MEMORY_CMD;
	if (HAL_QSPI_Command(hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
		return QSPI_ERROR;

	/* Send command RESET command in SPI mode */
	/* Initialize the reset enable command */
	s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
	s_command.Instruction       = RESET_ENABLE_CMD;

	/* Send the command */
	if (HAL_QSPI_Command(hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
		return QSPI_ERROR;

	/* Send the reset memory command */
	s_command.Instruction = RESET_MEMORY_CMD;
	if (HAL_QSPI_Command(hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
		return QSPI_ERROR;

	/* After reset CMD, 1000ms requested if QSPI memory SWReset occured during full chip erase operation */
	HAL_Delay(1000);

	/* Configure automatic polling mode to wait the WIP bit=0 */
	s_config.Match           = 0;
	s_config.Mask            = MX25L512_SR_WIP;
	s_config.MatchMode       = QSPI_MATCH_MODE_AND;
	s_config.StatusBytesSize = 1;
	s_config.Interval        = 0x10;
	s_config.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;

	s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	s_command.Instruction     = READ_STATUS_REG_CMD;
	s_command.DataMode        = QSPI_DATA_1_LINE;

	if (HAL_QSPI_AutoPolling(hqspi, &s_command, &s_config, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
		return QSPI_ERROR;

	/* Initialize the reading of status register */
	s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
	s_command.Instruction       = READ_STATUS_REG_CMD;
	s_command.AddressMode       = QSPI_ADDRESS_NONE;
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DataMode          = QSPI_DATA_1_LINE;
	s_command.DummyCycles       = 0;
	s_command.NbData            = 1;
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

	/* Configure the command */
	if (HAL_QSPI_Command(hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
		return QSPI_ERROR;

	/* Reception of the data */
	if (HAL_QSPI_Receive(hqspi, &reg, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
		return QSPI_ERROR;

	/* Enable write operations, command in 1 bit */
	/* Enable write operations */
	s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
	s_command.Instruction       = WRITE_ENABLE_CMD;
	s_command.AddressMode       = QSPI_ADDRESS_NONE;
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DataMode          = QSPI_DATA_NONE;
	s_command.DummyCycles       = 0;
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

	if (HAL_QSPI_Command(hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
		return QSPI_ERROR;

	/* Configure automatic polling mode to wait for write enabling */
	s_config.Match           = MX25L512_SR_WREN;
	s_config.Mask            = MX25L512_SR_WREN;
	s_config.MatchMode       = QSPI_MATCH_MODE_AND;
	s_config.StatusBytesSize = 1;
	s_config.Interval        = 0x10;
	s_config.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;

	s_command.Instruction    = READ_STATUS_REG_CMD;
	s_command.DataMode       = QSPI_DATA_1_LINE;

	if (HAL_QSPI_AutoPolling(hqspi, &s_command, &s_config, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
		return QSPI_ERROR;

	/* Update the configuration register with new dummy cycles */
	s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
	s_command.Instruction       = WRITE_STATUS_CFG_REG_CMD;
	s_command.AddressMode       = QSPI_ADDRESS_NONE;
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DataMode          = QSPI_DATA_1_LINE;
	s_command.DummyCycles       = 0;
	s_command.NbData            = 1;
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

	/* Enable the Quad IO on the QSPI memory (Non-volatile bit) */
	reg |= MX25L512_SR_QUADEN;

	/* Configure the command */
	if (HAL_QSPI_Command(hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
		return QSPI_ERROR;

	/* Transmission of the data */
	if (HAL_QSPI_Transmit(hqspi, &reg, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
		return QSPI_ERROR;

	/* 40ms  Write Status/Configuration Register Cycle Time */
	HAL_Delay(40);

	return QSPI_OK;
}

/**
  * @brief  This function set the QSPI memory in 4-byte address mode
  * @param  hqspi: QSPI handle
  * @retval None
  */
static uint8_t QSPI_EnterFourBytesAddress(QSPI_HandleTypeDef *hqspi) {
	QSPI_CommandTypeDef s_command;

	/* Initialize the command */
	s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
	s_command.Instruction       = ENTER_4_BYTE_ADDR_MODE_CMD;
	s_command.AddressMode       = QSPI_ADDRESS_NONE;
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DataMode          = QSPI_DATA_NONE;
	s_command.DummyCycles       = 0;
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

	/* Enable write operations */
	if (QSPI_WriteEnable(hqspi) != QSPI_OK)
		return QSPI_ERROR;

	/* Send the command */
	if (HAL_QSPI_Command(hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
		return QSPI_ERROR;

	/* Configure automatic polling mode to wait the memory is ready */
	if (QSPI_AutoPollingMemReady(hqspi, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != QSPI_OK)
		return QSPI_ERROR;

	return QSPI_OK;
}

/**
  * @brief  This function put QSPI memory in QPI mode (quad I/O).
  * @param  hqspi: QSPI handle
  * @retval None
  */
static uint8_t QSPI_EnterMemory_QPI(QSPI_HandleTypeDef *hqspi) {
	QSPI_CommandTypeDef      s_command;
	QSPI_AutoPollingTypeDef  s_config;

	/* Initialize the QPI enable command */
	/* QSPI memory is supported to be in SPI mode, so CMD on 1 LINE */
	s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
	s_command.Instruction       = ENTER_QUAD_CMD;
	s_command.AddressMode       = QSPI_ADDRESS_NONE;
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DataMode          = QSPI_DATA_NONE;
	s_command.DummyCycles       = 0;
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

	/* Send the command */
	if (HAL_QSPI_Command(hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
		return QSPI_ERROR;

	/* Configure automatic polling mode to wait the QUADEN bit=1 and WIP bit=0 */
	s_config.Match           = MX25L512_SR_QUADEN;
	s_config.Mask            = MX25L512_SR_QUADEN|MX25L512_SR_WIP;
	s_config.MatchMode       = QSPI_MATCH_MODE_AND;
	s_config.StatusBytesSize = 1;
	s_config.Interval        = 0x10;
	s_config.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;

	s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
	s_command.Instruction       = READ_STATUS_REG_CMD;
	s_command.DataMode          = QSPI_DATA_4_LINES;

	if (HAL_QSPI_AutoPolling(hqspi, &s_command, &s_config, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
		return QSPI_ERROR;

	return QSPI_OK;
}

/**
  * @brief  This function put QSPI memory in SPI mode.
  * @param  hqspi: QSPI handle
  * @retval None
  */
static uint8_t QSPI_ExitMemory_QPI(QSPI_HandleTypeDef *hqspi) {
	QSPI_CommandTypeDef      s_command;

	/* Initialize the QPI enable command */
	/* QSPI memory is supported to be in QPI mode, so CMD on 4 LINES */
	s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
	s_command.Instruction       = EXIT_QUAD_CMD;
	s_command.AddressMode       = QSPI_ADDRESS_NONE;
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DataMode          = QSPI_DATA_NONE;
	s_command.DummyCycles       = 0;
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

	/* Send the command */
	if (HAL_QSPI_Command(hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
		return QSPI_ERROR;

	return QSPI_OK;
}

/**
  * @brief  This function configure the Output driver strength on memory side.
  * @param  hqspi: QSPI handle
  * @retval None
  */
static uint8_t QSPI_OutDrvStrengthCfg(QSPI_HandleTypeDef *hqspi) {
	QSPI_CommandTypeDef s_command;
	uint8_t reg[2];

	/* Initialize the reading of status register */
	s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
	s_command.Instruction       = READ_STATUS_REG_CMD;
	s_command.AddressMode       = QSPI_ADDRESS_NONE;
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DataMode          = QSPI_DATA_4_LINES;
	s_command.DummyCycles       = 0;
	s_command.NbData            = 1;
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

	/* Configure the command */
	if (HAL_QSPI_Command(hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
		return QSPI_ERROR;

	/* Reception of the data */
	if (HAL_QSPI_Receive(hqspi, &(reg[0]), HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
		return QSPI_ERROR;

	/* Initialize the reading of configuration register */
	s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
	s_command.Instruction       = READ_CFG_REG_CMD;
	s_command.AddressMode       = QSPI_ADDRESS_NONE;
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DataMode          = QSPI_DATA_4_LINES;
	s_command.DummyCycles       = 0;
	s_command.NbData            = 1;
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

	/* Configure the command */
	if (HAL_QSPI_Command(hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
		return QSPI_ERROR;

	/* Reception of the data */
	if (HAL_QSPI_Receive(hqspi, &(reg[1]), HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
		return QSPI_ERROR;

	/* Enable write operations */
	if (QSPI_WriteEnable(hqspi) != QSPI_OK)
		return QSPI_ERROR;

	/* Update the configuration register with new output driver strength */
	s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
	s_command.Instruction       = WRITE_STATUS_CFG_REG_CMD;
	s_command.AddressMode       = QSPI_ADDRESS_NONE;
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DataMode          = QSPI_DATA_4_LINES;
	s_command.DummyCycles       = 0;
	s_command.NbData            = 2;
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

	/* Set Output Strength of the QSPI memory 15 ohms */
	MODIFY_REG( reg[1], MX25L512_CR_ODS, (MX25L512_CR_ODS_15 << POSITION_VAL(MX25L512_CR_ODS)));

	/* Configure the write volatile configuration register command */
	if (HAL_QSPI_Command(hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
		return QSPI_ERROR;

	/* Transmission of the data */
	if (HAL_QSPI_Transmit(hqspi, &(reg[0]), HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
		return QSPI_ERROR;

	return QSPI_OK;
}
