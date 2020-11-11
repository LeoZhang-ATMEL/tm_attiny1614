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

static uint8_t LP8863_1_BL_MODE[] = {0x2C, 0x20, 0x02, 0x00};
static uint8_t LP8863_1_LED0_CURRENT[] = {0x2D, 0xC2, 0xCB, 0x09}; /* 90mA */
static uint8_t LP8863_1_LED1_CURRENT[] = {0x2D, 0xC4, 0xCB, 0x09}; /* 90mA */
static uint8_t LP8863_1_LED2_CURRENT[] = {0x2D, 0xC6, 0xCB, 0x09}; /* 90mA */
static uint8_t LP8863_1_LED3_CURRENT[] = {0x2D, 0xC8, 0xCB, 0x09}; /* 90mA */
static uint8_t LP8863_1_LED4_CURRENT[] = {0x2D, 0xCA, 0xCB, 0x09}; /* 90mA */
static uint8_t LP8863_1_LED5_CURRENT[] = {0x2D, 0xCC, 0xCB, 0x09}; /* 90mA */
static uint8_t LP8863_1_DISPLAY_BRT[] = {0x2C, 0x28, 0x00, 0x00};

static uint8_t LP8863_2_BL_MODE[] = {0x3C, 0x20, 0x02, 0x00};
static uint8_t LP8863_2_LED0_CURRENT[] = {0x3D, 0xC2, 0x15, 0x01}; /* 10mA */
static uint8_t LP8863_2_LED2_CURRENT[] = {0x3D, 0xC4, 0xC2, 0x00}; /* 7mA */
static uint8_t LP8863_2_DISPLAY_BRT[] = {0x3C, 0x28, 0x00, 0x00};

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
    {3, LP8863_2_LED2_CURRENT },
    {3, LP8863_2_DISPLAY_BRT }
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
