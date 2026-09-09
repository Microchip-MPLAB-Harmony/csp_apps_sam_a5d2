/*******************************************************************************
  Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This file contains the "main" function for a project.

  Description:
    This file contains the "main" function for a project.  The
    "main" function calls the "SYS_Initialize" function to initialize the state
    machines of all modules in the system
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include <string.h>
#include "definitions.h"                // SYS function prototypes

#define PAGE_SIZE                  (256U)
#define SECTOR_SIZE                (4096U)

/* Erase, Write and Read 80KBytes of memory */
#define SECTORS_TO_EWR             (20U)

#define BUFFER_SIZE                (SECTOR_SIZE * SECTORS_TO_EWR)

#define MEM_START_ADDRESS          (0x0U)

#define SST26VF032B_JEDEC_ID       (0x004326BFUL)

#define LED_ON                     LED_GREEN_Set
#define LED_OFF                    LED_GREEN_Clear
#define LED_TOGGLE                 LED_GREEN_Toggle

// *****************************************************************************
/* Application states

  Summary:
    Application states enumeration

  Description:
    This enumeration defines the valid application states.  These states
    determine the behavior of the application at various times.
*/

typedef enum
{
    /* The app mounts the disk */
    APP_STATE_INIT = 0,

    /* Reset Flash*/
    APP_STATE_RESET_FLASH,

    /* Enable Quad IO Mode*/
    APP_STATE_ENABLE_QUAD_IO,

    /* Unlock Flash*/
    APP_STATE_UNLOCK_FLASH,

    /* Read JEDEC ID*/
    APP_STATE_READ_JEDEC_ID,

    /* Erase Flash */
    APP_STATE_ERASE_FLASH,

    /* Erase Wait */
    APP_STATE_ERASE_WAIT,

    /* Write to Memory */
    APP_STATE_WRITE_MEMORY,

    /* Erase Wait */
    APP_STATE_WRITE_WAIT,

    /* Read From Memory */
    APP_STATE_READ_MEMORY,

    /* Verify Data Read */
    APP_STATE_VERIFY_DATA,

    /* The app idles */
    APP_STATE_SUCCESS,

    /* An app error has occurred */
    APP_STATE_ERROR

} APP_STATES;

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    Application strings and buffers are be defined outside this structure.
 */

typedef struct
{
    /* Application's current state */
    APP_STATES state;

    /* Application transfer status */
    volatile bool xfer_done;

    /* Jedec-ID*/
    uint32_t jedec_id;

    /* Read Buffer */
    uint8_t readBuffer[BUFFER_SIZE];

    /* Write Buffer*/
    uint8_t writeBuffer[BUFFER_SIZE];
} APP_DATA;

/* SST26 Command set

  Summary:
    Enumeration listing the SST26VF commands.

  Description:
    This enumeration defines the commands used to interact with the SST26VF
    series of devices.

  Remarks:
    None
*/

typedef enum
{
    /* Reset enable command. */
    SST26_CMD_FLASH_RESET_ENABLE = 0x66,

    /* Command to reset the flash. */
    SST26_CMD_FLASH_RESET        = 0x99,

    /* Command to Enable QUAD IO */
    SST26_CMD_ENABLE_QUAD_IO     = 0x38,

    /* Command to Reset QUAD IO */
    SST26_CMD_RESET_QUAD_IO      = 0xFF,

    /* Command to read JEDEC-ID of the flash device. */
    SST26_CMD_JEDEC_ID_READ      = 0x9F,

    /* QUAD Command to read JEDEC-ID of the flash device. */
    SST26_CMD_QUAD_JEDEC_ID_READ = 0xAF,

    /* Command to perfrom High Speed Read */
    SST26_CMD_HIGH_SPEED_READ    = 0x0B,

    /* Write enable command. */
    SST26_CMD_WRITE_ENABLE       = 0x06,

    /* Page Program command. */
    SST26_CMD_PAGE_PROGRAM       = 0x02,

    /* Command to read the Flash status register. */
    SST26_CMD_READ_STATUS_REG    = 0x05,

    /* Command to perform sector erase */
    SST26_CMD_SECTOR_ERASE       = 0x20,

    /* Command to perform Bulk erase */
    SST26_CMD_BULK_ERASE_64K     = 0xD8,

    /* Command to perform Chip erase */
    SST26_CMD_CHIP_ERASE         = 0xC7,

    /* Command to unlock the flash device. */
    SST26_CMD_UNPROTECT_GLOBAL   = 0x98

} SST26_CMD;

typedef enum
{
    /* Buffer is being processed */
    APP_TRANSFER_BUSY,
    /* APP Buffer transfer is successfully completed*/
    APP_TRANSFER_COMPLETED,
    /* APP Buffer transfer had error*/
    APP_TRANSFER_ERROR_UNKNOWN,
    /* APP Transfer has not been asked yet */
    APP_TRANSFER_IDLE
} APP_TRANSFER_STATUS;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Routines
// *****************************************************************************
// *****************************************************************************
/* These routines are called by drivers when certain events occur.
*/

// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_Initialize ( void )

  Summary:
     Application initialization routine.

  Description:
    This function initializes the application.  It places the
    application in its initial state and prepares it to run so that its
    APP_Tasks function can be called.

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    APP_Initialize();
    </code>

  Remarks:
    This routine must be called from the main function.
*/

void APP_Initialize ( void );


/*******************************************************************************
  Function:
    void APP_Tasks ( void )

  Summary:
    Application tasks function

  Description:
    This routine is the Application's tasks function.  It
    defines the application's state machine and core logic.

  Precondition:
    The system and application initialization ("SYS_Initialize") should be
    called before calling this.

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    APP_Tasks();
    </code>

  Remarks:
    This routine must be called from SYS_Tasks() routine.
 */

void APP_Tasks( void );

APP_DATA appData;

static uint32_t write_index = 0;
static uint32_t sector_index = 0;

static qspi_command_xfer_t qspi_command_xfer = { 0 };
static qspi_register_xfer_t qspi_register_xfer = { 0 };
static qspi_memory_xfer_t qspi_memory_xfer = { 0 };

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************

/* This function resets the flash by sending down the reset enable command
 * followed by the reset command. */

static APP_TRANSFER_STATUS APP_ResetFlash(void)
{
    memset((void *)&qspi_command_xfer, 0, sizeof(qspi_command_xfer_t));

    qspi_command_xfer.instruction = SST26_CMD_FLASH_RESET_ENABLE;
    qspi_command_xfer.width = SINGLE_BIT_SPI;

    if (QSPI1_CommandWrite(&qspi_command_xfer, 0) == false)
    {
        return APP_TRANSFER_ERROR_UNKNOWN;
    }

    qspi_command_xfer.instruction = SST26_CMD_FLASH_RESET;
    qspi_command_xfer.width = SINGLE_BIT_SPI;

    if (QSPI1_CommandWrite(&qspi_command_xfer, 0) == false)
    {
        return APP_TRANSFER_ERROR_UNKNOWN;
    }

    return APP_TRANSFER_COMPLETED;
}

/* Enables the QUAD IO on the flash */
static APP_TRANSFER_STATUS APP_EnableQuadIO(void)
{
    memset((void *)&qspi_command_xfer, 0, sizeof(qspi_command_xfer_t));

    qspi_command_xfer.instruction = SST26_CMD_ENABLE_QUAD_IO;
    qspi_command_xfer.width = SINGLE_BIT_SPI;

    if (QSPI1_CommandWrite(&qspi_command_xfer, 0) == false)
    {
        return APP_TRANSFER_ERROR_UNKNOWN;
    }

    return APP_TRANSFER_COMPLETED;
}

/* Sends Write Enable command to flash */
static APP_TRANSFER_STATUS APP_WriteEnable(void)
{
    memset((void *)&qspi_command_xfer, 0, sizeof(qspi_command_xfer_t));

    qspi_command_xfer.instruction = SST26_CMD_WRITE_ENABLE;
    qspi_command_xfer.width = QUAD_CMD;

    if (QSPI1_CommandWrite(&qspi_command_xfer, 0) == false)
    {
        return APP_TRANSFER_ERROR_UNKNOWN;
    }

    return APP_TRANSFER_COMPLETED;
}

/* This function sends down command to perform a global unprotect of the flash. */
static APP_TRANSFER_STATUS APP_UnlockFlash(void)
{
    if (APP_TRANSFER_COMPLETED != APP_WriteEnable())
    {
        return APP_TRANSFER_ERROR_UNKNOWN;
    }

    memset((void *)&qspi_command_xfer, 0, sizeof(qspi_command_xfer_t));

    qspi_command_xfer.instruction = SST26_CMD_UNPROTECT_GLOBAL;
    qspi_command_xfer.width = QUAD_CMD;

    if (QSPI1_CommandWrite(&qspi_command_xfer, 0) == false)
    {
        return APP_TRANSFER_ERROR_UNKNOWN;
    }

    return APP_TRANSFER_COMPLETED;
}

/* This function reads and stores the flash id. */
static APP_TRANSFER_STATUS APP_ReadJedecId(uint32_t *jedec_id)
{
    memset((void *)&qspi_register_xfer, 0, sizeof(qspi_register_xfer_t));

    qspi_register_xfer.instruction = SST26_CMD_QUAD_JEDEC_ID_READ;
    qspi_register_xfer.width = QUAD_CMD;
    qspi_register_xfer.dummy_cycles = 2;

    if (QSPI1_RegisterRead(&qspi_register_xfer, jedec_id, 3) == false)
    {
        return APP_TRANSFER_ERROR_UNKNOWN;
    }

    return APP_TRANSFER_COMPLETED;
}

/* Function to read the status register of the flash. */
static APP_TRANSFER_STATUS APP_ReadStatus( uint32_t *rx_data, uint32_t rx_data_length )
{
    memset((void *)&qspi_register_xfer, 0, sizeof(qspi_register_xfer_t));

    qspi_register_xfer.instruction = SST26_CMD_READ_STATUS_REG;
    qspi_register_xfer.width = QUAD_CMD;
    qspi_register_xfer.dummy_cycles = 2;

    if (QSPI1_RegisterRead(&qspi_register_xfer, rx_data, rx_data_length) == false)
    {
        return APP_TRANSFER_ERROR_UNKNOWN;
    }
    return APP_TRANSFER_COMPLETED;
}

/* Checks for any pending transfers Erase/Write */
static APP_TRANSFER_STATUS APP_TransferStatusCheck(void)
{
    uint8_t reg_status = 0;

    if (APP_ReadStatus((uint32_t *)&reg_status, 1) != APP_TRANSFER_COMPLETED)
    {
        return APP_TRANSFER_ERROR_UNKNOWN;
    }

    if(reg_status & (1<<0))
        return APP_TRANSFER_BUSY;
    else
        return APP_TRANSFER_COMPLETED;
}

/* Reads n Bytes of data from the flash memory */
static APP_TRANSFER_STATUS APP_MemoryRead( uint32_t *rx_data, uint32_t rx_data_length, uint32_t address )
{
    memset((void *)&qspi_memory_xfer, 0, sizeof(qspi_memory_xfer_t));

    qspi_memory_xfer.instruction = SST26_CMD_HIGH_SPEED_READ;
    qspi_memory_xfer.width = QUAD_CMD;
    qspi_memory_xfer.dummy_cycles = 6;

    if (QSPI1_MemoryRead(&qspi_memory_xfer, rx_data, rx_data_length, address) == false) {
        return APP_TRANSFER_ERROR_UNKNOWN;
    }
    return APP_TRANSFER_COMPLETED;
}

/* Writes n Bytes of data to the flash memory */
static APP_TRANSFER_STATUS APP_MemoryWrite( uint32_t *tx_data, uint32_t tx_data_length, uint32_t address )
{
    if (APP_TRANSFER_COMPLETED != APP_WriteEnable())
    {
        return APP_TRANSFER_ERROR_UNKNOWN;
    }

    memset((void *)&qspi_memory_xfer, 0, sizeof(qspi_memory_xfer_t));

    qspi_memory_xfer.instruction = SST26_CMD_PAGE_PROGRAM;
    qspi_memory_xfer.width = QUAD_CMD;

    if (QSPI1_MemoryWrite(&qspi_memory_xfer, tx_data, tx_data_length, address) == false)
    {
        return APP_TRANSFER_ERROR_UNKNOWN;
    }

    return APP_TRANSFER_COMPLETED;
}

/* Sends Erase command to flash */
static APP_TRANSFER_STATUS APP_Erase(uint8_t instruction, uint32_t address)
{
    if (APP_WriteEnable() != APP_TRANSFER_COMPLETED)
    {
        return APP_TRANSFER_ERROR_UNKNOWN;
    }

    qspi_command_xfer.instruction = instruction;
    qspi_command_xfer.width = QUAD_CMD;
    qspi_command_xfer.addr_en = 1;

    if (QSPI1_CommandWrite(&qspi_command_xfer, address) == false)
    {
        return APP_TRANSFER_ERROR_UNKNOWN;
    }

    return APP_TRANSFER_COMPLETED;
}

static APP_TRANSFER_STATUS APP_SectorErase(uint32_t address)
{
    return (APP_Erase(SST26_CMD_SECTOR_ERASE, address));
}

// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_Initialize ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Initialize ( void )
{
    uint32_t i = 0;

    /* Place the App state machine in its initial state. */
    appData.state = APP_STATE_INIT;

    for (i = 0; i < BUFFER_SIZE; i++)
        appData.writeBuffer[i] = i;

}


/******************************************************************************
  Function:
    void APP_Tasks ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Tasks ( void )
{
    /* Check the application's current state. */
    switch ( appData.state )
    {
        case APP_STATE_INIT:
        {
            appData.state = APP_STATE_RESET_FLASH;
        }

        case APP_STATE_RESET_FLASH:
        {
            if (APP_ResetFlash() != APP_TRANSFER_COMPLETED)
            {
                appData.state = APP_STATE_ERROR;
            }
            else
            {
                appData.state = APP_STATE_ENABLE_QUAD_IO;
            }
            break;
        }

        case APP_STATE_ENABLE_QUAD_IO:
        {
            if (APP_EnableQuadIO() != APP_TRANSFER_COMPLETED)
            {
                appData.state = APP_STATE_ERROR;
            }
            else
            {
                appData.state = APP_STATE_UNLOCK_FLASH;
            }
            break;
        }

        case APP_STATE_UNLOCK_FLASH:
        {
            if (APP_UnlockFlash() != APP_TRANSFER_COMPLETED)
            {
                appData.state = APP_STATE_ERROR;
            }
            else
            {
                appData.state = APP_STATE_READ_JEDEC_ID;
            }
            break;
        }

        case APP_STATE_READ_JEDEC_ID:
        {
            if (APP_ReadJedecId(&appData.jedec_id) != APP_TRANSFER_COMPLETED)
            {
                appData.state = APP_STATE_ERROR;
                break;
            }

            if (appData.jedec_id != SST26VF032B_JEDEC_ID)
            {
                appData.state = APP_STATE_ERROR;
                break;
            }

            appData.state = APP_STATE_ERASE_FLASH;

            break;
        }

        case APP_STATE_ERASE_FLASH:
        {
            if (APP_SectorErase((MEM_START_ADDRESS + sector_index)) != APP_TRANSFER_COMPLETED)
            {
                appData.state = APP_STATE_ERROR;
                break;
            }

            appData.state = APP_STATE_ERASE_WAIT;

            break;
        }

        case APP_STATE_ERASE_WAIT:
        {
            if (APP_TransferStatusCheck() == APP_TRANSFER_COMPLETED)
            {
                sector_index += SECTOR_SIZE;

                if (sector_index < BUFFER_SIZE)
                {
                    appData.state = APP_STATE_ERASE_FLASH;
                }
                else
                {
                    appData.state = APP_STATE_WRITE_MEMORY;
                }
            }
            break;
        }

        case APP_STATE_WRITE_MEMORY:
        {
            if (APP_MemoryWrite((uint32_t *)&appData.writeBuffer[write_index], PAGE_SIZE, (MEM_START_ADDRESS + write_index)) != APP_TRANSFER_COMPLETED)
            {
                appData.state = APP_STATE_ERROR;
                break;
            }

            appData.state = APP_STATE_WRITE_WAIT;

            break;
        }

        case APP_STATE_WRITE_WAIT:
        {
            if (APP_TransferStatusCheck() == APP_TRANSFER_COMPLETED)
            {
                write_index += PAGE_SIZE;
                if (write_index < BUFFER_SIZE)
                {
                    appData.state = APP_STATE_WRITE_MEMORY;
                }
                else
                {
                    appData.state = APP_STATE_READ_MEMORY;
                }
            }
            break;
        }

        case APP_STATE_READ_MEMORY:
        {
            if (APP_MemoryRead((uint32_t *)&appData.readBuffer[0], BUFFER_SIZE, MEM_START_ADDRESS) != APP_TRANSFER_COMPLETED)
            {
                appData.state = APP_STATE_ERROR;
                break;
            }

            appData.state = APP_STATE_VERIFY_DATA;

            break;
        }

        case APP_STATE_VERIFY_DATA:
        {
            if (!memcmp(appData.writeBuffer, appData.readBuffer, BUFFER_SIZE))
            {
                appData.state = APP_STATE_SUCCESS;
            }
            else
            {
                appData.state = APP_STATE_ERROR;
            }

            break;
        }

        case APP_STATE_SUCCESS:
        {
            PIT_DelayMs(1000);

            LED_TOGGLE();

            break;
        }

        case APP_STATE_ERROR:
        default:
        {
            LED_ON();
            break;
        }
    }
}

// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************

int main ( void )
{
    /* Initialize all modules */
    SYS_Initialize ( NULL );

    APP_Initialize();
    
    while ( true )
    {
        /* Maintain state machines of all polled MPLAB Harmony modules. */
        APP_Tasks();
    }

    /* Execution should not come here during normal operation */

    return ( EXIT_FAILURE );
}


/*******************************************************************************
 End of File
*/

