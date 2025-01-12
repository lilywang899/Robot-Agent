/*
 * The Driver Station Library (LibDS)
 * Copyright (c) Lily Wang and other contributors.
 * Open Source Software; you can modify and/or share it under the terms of
 * the MIT license file in the root directory of this project.
 */
#pragma once

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif /* _cplusplus */

#define COM_AGENT       0x1
#define COM_DS          0x2
#define COM_MQTT_CLIENT 0x3
#define COM_CONTROLLER  0x4

#define MAX_PHONE_NUM_LEN        33    
#define MAX_MESSAGE_SIZE         255
#define SMM_OutGoingRequest              0x01

/**************************************************************************
*                                                                         *
*                                  data structure                         *
*                                                                         *
**************************************************************************/

/*************************** SM *******************************/
typedef struct
{
    unsigned char    PhoneNumber[MAX_PHONE_NUM_LEN];
}C_SMM_OUTGOING_REQUEST;

typedef struct
{
    unsigned char    Result; 
}C_SMM_OUTGOING_REQUEST_RESPOND;

typedef struct tagMESSAGE
{
    unsigned char    did;
    unsigned char    type;
    unsigned char    sid;
    unsigned char    length;
union 
    {
        unsigned char    content[MAX_MESSAGE_SIZE];
/**********************SM*************************/
        C_SMM_OUTGOING_REQUEST                smm_OutGoingRequest;
    }Union;
} MESSAGE; 
#ifdef          __cplusplus
}
#endif /* _cplusplus */


