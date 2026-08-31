/******************************************************************************
* File Name:   main.c
*
* Description: This is the source code for the PSOC Control C1 MCU: SCU Sleep mode
*              for ModusToolbox.
*
* Related Document: See README.md
*
*******************************************************************************
* (c) 2026, Infineon Technologies AG, or an affiliate of Infineon
* Technologies AG. All rights reserved.
* This software, associated documentation and materials ("Software") is
* owned by Infineon Technologies AG or one of its affiliates ("Infineon")
* and is protected by and subject to worldwide patent protection, worldwide
* copyright laws, and international treaty provisions. Therefore, you may use
* this Software only as provided in the license agreement accompanying the
* software package from which you obtained this Software. If no license
* agreement applies, then any use, reproduction, modification, translation, or
* compilation of this Software is prohibited without the express written
* permission of Infineon.
*
* Disclaimer: UNLESS OTHERWISE EXPRESSLY AGREED WITH INFINEON, THIS SOFTWARE
* IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
* INCLUDING, BUT NOT LIMITED TO, ALL WARRANTIES OF NON-INFRINGEMENT OF
* THIRD-PARTY RIGHTS AND IMPLIED WARRANTIES SUCH AS WARRANTIES OF FITNESS FOR A
* SPECIFIC USE/PURPOSE OR MERCHANTABILITY.
* Infineon reserves the right to make changes to the Software without notice.
* You are responsible for properly designing, programming, and testing the
* functionality and safety of your intended application of the Software, as
* well as complying with any legal requirements related to its use. Infineon
* does not guarantee that the Software will be free from intrusion, data theft
* or loss, or other breaches ("Security Breaches"), and Infineon shall have
* no liability arising out of any Security Breaches. Unless otherwise
* explicitly approved by Infineon, the Software may not be used in any
* application where a failure of the Product or any consequences of the use
* thereof can reasonably be expected to result in personal injury.
*******************************************************************************/

#include "cybsp.h"
#include "cy_utils.h"
#include "cy_eru.h"

/*******************************************************************************
* Macros
*******************************************************************************/

#define EXTERNAL_INPUT_SIGNAL_PIN     ERU0_ETL1_INPUTA_P2_5
#define ERU_GROUP_ETL_CHANNEL         ERU0_ETL1
#define ERU_GROUP_OGU_CHANNEL         ERU0_OGU0
#define INTERRUPT_PRIORITY_NODE_ID    IRQ3_IRQn
#define INTERRUPT_EVENT_PRIORITY      (3U)
#define ERU_EXTERNAL_EVENT_HANDLER    IRQ_Hdlr_3
#define CYCLE_DELAY_COUNT             (10000000U)

#define ENABLE_DEEP_SLEEP_MODE        (0U)

/*******************************************************************************
* Data Structure
*******************************************************************************/
/*Structure for initializing ERUx_ETLy module*/
static const Cy_ERU_ETL_CONFIG_t button_event_generator_config =
{
    .input                  = EXTERNAL_INPUT_SIGNAL_PIN,             /*Configures input signal for Event request source unit */
    .source                 = CY_ERU_ETL_SOURCE_A,                  /*Input path combination along with polarity for event generation.*/
    .edge_detection         = CY_ERU_ETL_EDGE_DETECTION_FALLING,    /*Configure the event trigger edge(FE, RE)*/
    .status_flag_mode       = CY_ERU_ETL_STATUS_FLAG_MODE_HWCTRL,   /*Status flag is in non-sticky mode. Automatically cleared by the opposite edge detection*/
    .enable_output_trigger  = true,                                  /*Enables the generation of trigger pulse(PE)*/
    .output_trigger_channel = CY_ERU_ETL_OUTPUT_TRIGGER_CHANNEL0    /*Output channel select(OCS) for ETLx output trigger pulse.*/
};

/*Structure for initializing ERUx_OGUy module.*/
static const Cy_ERU_OGU_CONFIG_t button_event_detection_config =
{
    .service_request        = CY_ERU_OGU_SERVICE_REQUEST_ON_TRIGGER  /*Gating(GP) on service request generation for pattern detection result*/
};

/*******************************************************************************
* Function Name: delay
********************************************************************************
* Summary:
* This is the delay generation function based on the MCU clock cycles
*
* Parameters:
*  uint32_t cycles
*
* Return:
*  void
*
*******************************************************************************/
 void delay(uint32_t cycles)
{
    while(--cycles)
    {
        __NOP();       /* No operation */
    }
}

/*******************************************************************************
* Function Name: ERU_EXTERNAL_EVENT_HANDLER
********************************************************************************
* Summary:
* This is the interrupt handler function for the ERU external interrupt.
* This is use to wake up controller from sleep mode
* Inside Interrupt service routine LED will glow and then goes off after a small delay
*
* Parameters:
*  none
*
* Return:
*  void
*
*******************************************************************************/
void ERU_EXTERNAL_EVENT_HANDLER(void)
{
    /*Toggle the LED*/
    Cy_GPIO_ToggleOutput(CYBSP_USER_LED_PORT, CYBSP_USER_LED_PIN);
    /*DO something, waste cycles here for generating delay*/
    delay(CYCLE_DELAY_COUNT);
    /*Toggle the LED*/
    Cy_GPIO_ToggleOutput(CYBSP_USER_LED_PORT, CYBSP_USER_LED_PIN);
}

/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
* This is the main function.
* This example demonstrate how to program PSOC Control C1 MCU into sleep mode
* Wake up is done by using External Interrupt via ERU
* On Board LED glows with some delay inside external event handler and then go off
* Processor is put back to sleep mode after servicing interrupt service routine
* Wait for Interrupt (WFI) command causes immediate entry into sleep mode
*
* Parameters:
*  none
*
* Return:
*  int
*
*******************************************************************************/
int main(void)
{
    cy_rslt_t result;

    /* Initialize the device and board peripherals */
    result = cybsp_init();
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /*Initializes the selected ERU_ETLx channel with the configuration structure*/
    Cy_ERU_ETL_Init(ERU_GROUP_ETL_CHANNEL, &button_event_generator_config);

    /*Initializes the selected ERU_OGUy channel with the configuration structure*/
    Cy_ERU_OGU_Init(ERU_GROUP_OGU_CHANNEL, &button_event_detection_config);

    /*Set Priority for IRQ*/
    NVIC_SetPriority(INTERRUPT_PRIORITY_NODE_ID,INTERRUPT_EVENT_PRIORITY);

    /*Enable the Interrupt*/
    NVIC_EnableIRQ(INTERRUPT_PRIORITY_NODE_ID);

    /*Enable the sleep or deep sleep mode*/
    #if ENABLE_DEEP_SLEEP_MODE == 1
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
    #else
    SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;
    #endif

    /*Enable sleep-on-exit feature, Immediately enter sleep mode after execution of exception handlers*/
    SCB->SCR |= SCB_SCR_SLEEPONEXIT_Msk;

    /* Infinite loop */
    while(1)
    {
        /*Data Synchronization Barrier,it completes when all explicit memory accesses before this instruction complete*/
        __DSB();

        /* Wait For Interrupt is a hint instruction that suspends execution and enters sleep mode until one of a number of events occurs*/
        __WFI();
    }
}

/* [] END OF FILE */
