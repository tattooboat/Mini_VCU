#include "App_MotorFeedback.h"
#include "can.h"

extern CAN_HandleTypeDef hcan1;

HAL_StatusTypeDef App_MotorFeedback_Send(
    uint8_t state,
    int16_t target,
    uint8_t last_cmd,
    uint8_t seq,
    uint8_t fault_code)
{
    CAN_TxHeaderTypeDef tx_header = {0};
    uint8_t data[8] = {0};
    uint32_t tx_mailbox;
    uint16_t target_u16 = (uint16_t)target;

    tx_header.StdId = MOTOR_FEEDBACK_CAN_ID;
    tx_header.IDE = CAN_ID_STD;
    tx_header.RTR = CAN_RTR_DATA;
    tx_header.DLC = 8U;
    tx_header.TransmitGlobalTime = DISABLE;

    data[0] = MOTOR_FRAME_MARK;
    data[1] = state;
    data[2] = (uint8_t)(target_u16 >> 8);
    data[3] = (uint8_t)(target_u16 & 0xFFU);
    data[4] = last_cmd;
    data[5] = seq;
    data[6] = fault_code;
    data[7] = 0U;

    if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) == 0U)
    {
        return HAL_BUSY;
    }

    return HAL_CAN_AddTxMessage(
        &hcan1,
        &tx_header,
        data,
        &tx_mailbox
    );
}
