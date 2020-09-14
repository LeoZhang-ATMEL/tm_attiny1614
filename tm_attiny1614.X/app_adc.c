/*
 * File:   main_adc.c
 * Author: A41547
 *
 * Created on September 14, 2020, 8:39 PM
 */
#include "mcc_generated_files/mcc.h"

typedef enum
{
    APP_ADC_INIT=0,
    APP_ADC1_SAMPLE,
    APP_ADC1_SAMPLE_READY,
    APP_ADC2_SAMPLE,
    APP_ADC2_SAMPLE_READY,
    APP_ADC3_SAMPLE,
    APP_ADC3_SAMPLE_READY,
    APP_ADC_FAULT_DETECT,
    APP_ADC_FAULT_RECOVER,
    APP_ADC_IDLE,
} APP_ADC_STATES;

typedef struct
{
    /* The application's current state */
    APP_ADC_STATES state;
    adc_result_t adc1;
    adc_result_t adc2;
    adc_result_t adc3;
} APP_ADC_DATA;

APP_ADC_DATA app_adcData;
#define ADC_THRESHOLD_MAX (uint16_t)((float)1024 * 0.8 / 3.3)
#define ADC_THRESHOLD_DELTA (uint16_t)((float)1024 * 0.2 / 3.3)

void APP_ADC_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    app_adcData.state = APP_ADC_INIT;
}

void APP_ADC_Tasks ( void )
{
    /* Check the application's current state. */
    switch ( app_adcData.state )
    {
        /* Application's initial state. */
        case APP_ADC_INIT:
        {
            app_adcData.state = APP_ADC1_SAMPLE;
            break;
        }

        case APP_ADC1_SAMPLE:
        {
            ADC0_StartConversion(ADC_MUXPOS_AIN1_gc);
            app_adcData.state = APP_ADC1_SAMPLE_READY;
            break;
        }

        case APP_ADC1_SAMPLE_READY:
        {
            if (!ADC0_IsConversionDone()) {
                break;
            }
            app_adcData.adc1 = ADC0_GetConversionResult();
            if (app_adcData.adc1 > ADC_THRESHOLD_MAX) {
                app_adcData.state = APP_ADC_FAULT_DETECT;
            } else {
                app_adcData.state = APP_ADC2_SAMPLE;
            }
            break;
        }
        
        case APP_ADC2_SAMPLE:
        {
            ADC0_StartConversion(ADC_MUXPOS_AIN2_gc);
            app_adcData.state = APP_ADC2_SAMPLE_READY;
            break;
        }

        case APP_ADC2_SAMPLE_READY:
        {
            if (ADC0_IsConversionDone()) {
                break;
            }
            app_adcData.adc2 = ADC0_GetConversionResult();
            if (app_adcData.adc2 > ADC_THRESHOLD_MAX) {
                app_adcData.state = APP_ADC_FAULT_DETECT;
            } else {
                app_adcData.state = APP_ADC2_SAMPLE;
            }

            break;
        }
        
        case APP_ADC3_SAMPLE:
        {
            ADC0_StartConversion(ADC_MUXPOS_AIN3_gc);
            app_adcData.state = APP_ADC3_SAMPLE_READY;
            break;
        }

        case APP_ADC3_SAMPLE_READY:
        {
            if (ADC0_IsConversionDone()) {
                break;
            }
            app_adcData.adc3 = ADC0_GetConversionResult();
            if (app_adcData.adc3 > app_adcData.adc1) {
                if ((app_adcData.adc3 - app_adcData.adc1) > ADC_THRESHOLD_DELTA) {
                    app_adcData.state = APP_ADC_FAULT_DETECT;
                }
            } else if (app_adcData.adc3 < app_adcData.adc1) {
                if ((app_adcData.adc1 - app_adcData.adc3) > ADC_THRESHOLD_DELTA) {
                    app_adcData.state = APP_ADC_FAULT_DETECT;
                }
            } else if (app_adcData.adc3 > app_adcData.adc2) {
                if ((app_adcData.adc3 - app_adcData.adc2) > ADC_THRESHOLD_DELTA) {
                    app_adcData.state = APP_ADC_FAULT_DETECT;
                }
            } else if (app_adcData.adc3 < app_adcData.adc2) {
                if ((app_adcData.adc2 - app_adcData.adc3) > ADC_THRESHOLD_DELTA) {
                    app_adcData.state = APP_ADC_FAULT_DETECT;
                }
            }
            app_adcData.state = APP_ADC_FAULT_RECOVER;
            break;
        }
        case APP_ADC_FAULT_DETECT:
        {
            if (EN_GetValue()) {
                EN_SetHigh();
            }
            app_adcData.state = APP_ADC1_SAMPLE;
            break;
        }
        case APP_ADC_FAULT_RECOVER:
        {
            if (!EN_GetValue()) {
                EN_SetHigh();
            }
            app_adcData.state = APP_ADC1_SAMPLE;
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