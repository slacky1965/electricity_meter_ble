#ifndef SRC_INCLUDE_APP_ATT_H_
#define SRC_INCLUDE_APP_ATT_H_

///////////////////////////////////// ATT  HANDLER define ///////////////////////////////////////
typedef enum
{
    ATT_H_START = 0,


    //// Gap ////
    /**********************************************************************************************/
    GenericAccess_PS_H,                     //UUID: 2800,   VALUE: uuid 1800
    GenericAccess_DeviceName_CD_H,          //UUID: 2803,   VALUE:              Prop: Read | Notify
    GenericAccess_DeviceName_DP_H,          //UUID: 2A00,   VALUE: device name
    GenericAccess_Appearance_CD_H,          //UUID: 2803,   VALUE:              Prop: Read
    GenericAccess_Appearance_DP_H,          //UUID: 2A01,   VALUE: appearance
    CONN_PARAM_CD_H,                        //UUID: 2803,   VALUE:              Prop: Read
    CONN_PARAM_DP_H,                        //UUID: 2A04,   VALUE: connParameter


    //// gatt ////
    /**********************************************************************************************/
    GenericAttribute_PS_H,                  //UUID: 2800,   VALUE: uuid 1801
    GenericAttribute_ServiceChanged_CD_H,   //UUID: 2803,   VALUE:              Prop: Indicate
    GenericAttribute_ServiceChanged_DP_H,   //UUID: 2A05,   VALUE: service change
    GenericAttribute_ServiceChanged_CCB_H,  //UUID: 2902,   VALUE: serviceChangeCCC


    //// device information ////
    /**********************************************************************************************/
    DeviceInformation_PS_H,                 //UUID: 2800,   VALUE: uuid 180A
    DeviceInformation_ModName_CD_H,         //UUID: 2803,   VALUE:              Prop: Read
    DeviceInformation_ModName_DP_H,         //UUID: 2A24,   VALUE: Model Number String
    DeviceInformation_SerialN_CD_H,         //UUID: 2803,   VALUE:              Prop: Read
    DeviceInformation_SerialN_DP_H,         //UUID: 2A25,   VALUE: Serial Number String
    DeviceInformation_FirmRev_CD_H,         //UUID: 2803,   VALUE:              Prop: Read
    DeviceInformation_FirmRev_DP_H,         //UUID: 2A26,   VALUE: Firmware Revision String
    DeviceInformation_HardRev_CD_H,         //UUID: 2803,   VALUE:              Prop: Read
    DeviceInformation_HardRev_DP_H,         //UUID: 2A27,   VALUE: Hardware Revision String
    DeviceInformation_SoftRev_CD_H,         //UUID: 2803,   VALUE:              Prop: Read
    DeviceInformation_SoftRev_DP_H,         //UUID: 2A28,   VALUE: Software Revision String
    DeviceInformation_ManName_CD_H,         //UUID: 2803,   VALUE:              Prop: Read
    DeviceInformation_ManName_DP_H,         //UUID: 2A29,   VALUE: Manufacturer Name String


    //// Electricity meter service ////
    /**********************************************************************************************/
    EMETER_PS_H,                             //UUID: 2800,    VALUE: uuid 181C
    TARIFF1_LEVEL_INPUT_CD_H,                //UUID: 2803,    VALUE:              Prop: Read | Notify
    TARIFF1_LEVEL_INPUT_DP_H,                //UUID: 2AF2,    VALUE: Energy
    TARIFF1_LEVEL_INPUT_CCB_H,               //UUID: 2902,    VALUE: tariff1ValCCC

    TARIFF2_LEVEL_INPUT_CD_H,                //UUID: 2803,    VALUE:              Prop: Read | Notify
    TARIFF2_LEVEL_INPUT_DP_H,                //UUID: 2AF3,    VALUE: Energy
    TARIFF2_LEVEL_INPUT_CCB_H,               //UUID: 2902,    VALUE: tariff2ValCCC

    TARIFF3_LEVEL_INPUT_CD_H,                //UUID: 2803,    VALUE:              Prop: Read | Notify
    TARIFF3_LEVEL_INPUT_DP_H,                //UUID: 2B89,    VALUE: Energy
    TARIFF3_LEVEL_INPUT_CCB_H,               //UUID: 2902,    VALUE: tariff3ValCCC

    POWER_LEVEL_INPUT_CD_H,                  //UUID: 2803,    VALUE:              Prop: Read | Notify
    POWER_LEVEL_INPUT_DP_H,                  //UUID: 2B05,    VALUE: Power
    POWER_LEVEL_INPUT_CCB_H,                 //UUID: 2902,    VALUE: powerValCCC

    VOLTAGE_LEVEL_INPUT_CD_H,                //UUID: 2803,    VALUE:              Prop: Read | Notify
    VOLTAGE_LEVEL_INPUT_DP_H,                //UUID: 2B18,    VALUE: Voltage
    VOLTAGE_LEVEL_INPUT_CCB_H,               //UUID: 2902,    VALUE: voltageValCCC

    //// Ota ////
    /**********************************************************************************************/
    OTA_PS_H,                               //UUID: 2800,   VALUE: telink ota service uuid
    OTA_CMD_OUT_CD_H,                       //UUID: 2803,   VALUE:              Prop: read | write_without_rsp | Notify
    OTA_CMD_OUT_DP_H,                       //UUID: telink ota uuid,  VALUE: otaData
    OTA_CMD_OUT_DESC_H,                     //UUID: 2901,   VALUE: otaName

    //// Custom RxTx ////
    /**********************************************************************************************/
    RxTx_PS_H,                              //UUID: 2800,   VALUE: 1F10 RxTx service uuid
    RxTx_CMD_OUT_CD_H,                      //UUID: 2803,   VALUE:              Prop: read | write_without_rsp
    RxTx_CMD_OUT_DP_H,                      //UUID: 1F1F,   VALUE: RxTxData
    RxTx_CMD_OUT_DESC_H,                    //UUID: 2902,   VALUE: RxTxValueInCCC

    ATT_END_H,

}ATT_HANDLE;

extern int otaWrite(void * p);
extern int RxTxWrite(void * p);
extern uint16_t tariff1ValueInCCC;
extern uint16_t tariff2ValueInCCC;
extern uint16_t tariff3ValueInCCC;
extern uint16_t powerValueInCCC;
extern uint16_t voltageValueInCCC;
extern uint16_t RxTxValueInCCC;
void my_att_init(void);
uint32_t size_my_devName();



#endif /* SRC_INCLUDE_APP_ATT_H_ */
