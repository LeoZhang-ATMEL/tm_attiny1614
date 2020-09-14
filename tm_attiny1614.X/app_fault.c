/*
 * File:   app_fault.c
 * Author: A41547
 *
 * Created on September 14, 2020, 8:39 PM
 */
#include "mcc_generated_files/mcc.h"

typedef enum
{
    APP_FAULT_INIT=0,
    APP_FAULT_SERVICE,
    APP_FAULT_IDLE,
} APP_FAULT_STATES;

typedef struct
{
    /* The application's current state */
    APP_FAULT_STATES state;
} APP_FAULT_DATA;

APP_FAULT_DATA app_faultData;

void APP_FAULT_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    app_faultData.state = APP_FAULT_INIT;
}

/******************************************************************************
  Function:
    void APP_FAULT_Tasks ( void )

  Remarks:
    See prototype in app_info4.h.
 */

void APP_FAULT_Tasks ( void )
{
    /* Check the application's current state. */
    switch ( app_faultData.state )
    {
        /* Application's initial state. */
        case APP_FAULT_INIT:
        {
            app_faultData.state = APP_FAULT_SERVICE;
            break;
        }

        case APP_FAULT_SERVICE:
        {
            /* Set EN Low if FAULT1/FAULT was low */
            if (!FAULT1_GetValue() || !FAULT2_GetValue()) {
                if (EN_GetValue()) {
                    EN_SetLow();
                }
            } else {
                if (EN_GetValue()) {
                    EN_SetHigh();
                }
            }
            break;
        }

        case APP_FAULT_IDLE:
        {
            break;
        }
        /* The default state should never be executed. */
        default:
        {
            /* TODO: Handle error in application's state machine. */
            break;
        }
    }
}