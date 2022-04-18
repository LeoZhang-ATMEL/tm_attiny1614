/*
 * File:   app_i2c.c
 * Author: A41547
 *
 * Created on November 10, 2020, 9:54 PM
 */

#include "mcc_generated_files/mcc.h"

typedef enum
{
    APP_I2C_INIT=0,
    APP_I2C_WRITE,
    APP_I2C_WRITE_READY,
    APP_I2C_IDLE,
} APP_I2C_STATES;

typedef struct
{
    /* The application's current state */
    APP_I2C_STATES state;
    uint8_t cmdIndex;
} APP_I2C_DATA;

APP_I2C_DATA app_i2cdata;

struct lp8863_cmd_t {
    uint8_t length;
    uint8_t* data;
} ;
/* 1.21V/21K * 2560 (= 0.1475) * (Num / 4095) */
/* 90mA = 2507(0x9CB), 10ma = 277(0x115), 7ma = 194(0xC2) */

#define CURRENT_FORM(MA) ((MA/(float)147.5)*4095)
#define LP8863_1_LED_CUR ((uint16_t)CURRENT_FORM(90))

#define LP8863_2_LED0_CUR ((uint16_t)CURRENT_FORM(10))
#define LP8863_2_LED2_CUR ((uint16_t)CURRENT_FORM(7))
#define LP8863_2_LED0_EN 1
#define LP8863_2_LED2_EN 1

static uint8_t LP8863_1_BL_MODE[] = {0x2C, 0x20, 0x02, 0x00};
static uint8_t LP8863_1_LED0_CURRENT[] = {0x2D, 0xC2, LP8863_1_LED_CUR & 0xFF, LP8863_1_LED_CUR >> 8}; /* 90mA */
static uint8_t LP8863_1_LED1_CURRENT[] = {0x2D, 0xC4, LP8863_1_LED_CUR & 0xFF, LP8863_1_LED_CUR >> 8}; /* 90mA */
static uint8_t LP8863_1_LED2_CURRENT[] = {0x2D, 0xC6, LP8863_1_LED_CUR & 0xFF, LP8863_1_LED_CUR >> 8}; /* 90mA */
static uint8_t LP8863_1_LED3_CURRENT[] = {0x2D, 0xC8, LP8863_1_LED_CUR & 0xFF, LP8863_1_LED_CUR >> 8}; /* 90mA */
static uint8_t LP8863_1_LED4_CURRENT[] = {0x2D, 0xCA, LP8863_1_LED_CUR & 0xFF, LP8863_1_LED_CUR >> 8}; /* 90mA */
static uint8_t LP8863_1_LED5_CURRENT[] = {0x2D, 0xCC, LP8863_1_LED_CUR & 0xFF, LP8863_1_LED_CUR >> 8}; /* 90mA */
static uint8_t LP8863_1_DISPLAY_BRT[] = {0x2C, 0x28, 0xFF, 0xFF};

static uint8_t LP8863_2_BL_MODE[] = {0x3C, 0x20, 0x02, 0x03};
static uint8_t LP8863_2_LED0_CURRENT[] = {0x3D, 0xC2, LP8863_2_LED0_CUR & 0xFF, LP8863_2_LED0_CUR >> 8}; /* 10mA */
static uint8_t LP8863_2_LED2_CURRENT[] = {0x3D, 0xC6, LP8863_2_LED2_CUR & 0xFF, LP8863_2_LED2_CUR >> 8}; /* 7mA */
static uint8_t LP8863_2_GROUPING1[] = {0x3C, 0x30, (LP8863_2_LED0_EN)?0x01:0x00, (LP8863_2_LED2_EN)?0x03:0x00}; /* LED0:Cluster1, LED2:Cluster3 */
static uint8_t LP8863_2_CLUSTER1_BRT[] = {0x3D, 0x3C, 0xFF, 0xFF};
static uint8_t LP8863_2_CLUSTER3_BRT[] = {0x3D, 0x54, 0xFF, 0xFF};
static uint8_t LP8863_2_BRT_DB_CONTROL[] = {0x3D, 0x78, 0x01, 0x00}; /* Must call this after setting LEDx_CURRENT and CLUSTERx_BRT register */

struct lp8863_cmd_t lp8863Cmds[] = {
    //Command, Type, TX/RX Length, Data Address
    {3, LP8863_1_BL_MODE },
    {3, LP8863_1_LED0_CURRENT },
    {3, LP8863_1_LED1_CURRENT },
    {3, LP8863_1_LED2_CURRENT },
    {3, LP8863_1_LED3_CURRENT },
    {3, LP8863_1_LED4_CURRENT },
    {3, LP8863_1_LED5_CURRENT },
    {3, LP8863_1_DISPLAY_BRT },
    {3, LP8863_2_BL_MODE },
    {3, LP8863_2_LED0_CURRENT },
    {3, LP8863_2_LED2_CURRENT }, /* LED2 */
    {3, LP8863_2_GROUPING1},
    {3, LP8863_2_CLUSTER1_BRT},
    {3, LP8863_2_CLUSTER3_BRT}, /* LED2 */
    {3, LP8863_2_BRT_DB_CONTROL}
};
#define TABLE_SIZE  (sizeof(lp8863Cmds)/sizeof(struct lp8863_cmd_t))

void APP_I2C_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    app_i2cdata.state = APP_I2C_INIT;
    app_i2cdata.cmdIndex = 0;
}

/******************************************************************************
  Function:
    void APP_I2C_Tasks ( void )

  Remarks:
    See prototype in app_info4.h.
 */

void APP_I2C_Tasks ( void )
{
    /* Check the application's current state. */
    switch ( app_i2cdata.state )
    {
        /* Application's initial state. */
        case APP_I2C_INIT:
            app_i2cdata.state = APP_I2C_WRITE;
            app_i2cdata.cmdIndex = 0;
            break;
        case APP_I2C_WRITE:
            while(!I2C0_Open(lp8863Cmds[app_i2cdata.cmdIndex].data[0])); // sit here until we get the bus..
            I2C0_SetBuffer(lp8863Cmds[app_i2cdata.cmdIndex].data + 1, lp8863Cmds[app_i2cdata.cmdIndex].length);
            I2C0_MasterWrite();
            app_i2cdata.state = APP_I2C_WRITE_READY;
            break;
        case APP_I2C_WRITE_READY:
        {
            if (I2C0_BUSY == I2C0_Close()) {
                break;
            }
            app_i2cdata.cmdIndex++;
            if (app_i2cdata.cmdIndex >= TABLE_SIZE) {
                app_i2cdata.state = APP_I2C_IDLE;
            } else {
                app_i2cdata.state = APP_I2C_WRITE;
            }
            break;
        }

        case APP_I2C_IDLE:
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
