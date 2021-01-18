/**
*  @file      bq25703_drv.c
*  @brief     bq25703_drv
*  @author    Zack Li and Link Lin
*  @date      11 -2019
*  @copyright
*/

#include <stdio.h>
#include <fcntl.h>
#include <error.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdlib.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <pthread.h>
#include <poll.h>
#include <stdint.h>
#include <time.h>
#include <linux/input.h>
#include <sys/inotify.h>
#include <syslog.h>
#include <systemd/sd-daemon.h>

#include "bq25703_drv.h"
#include "gpio_config.h"

#include "tps65987_interface.h"
#include "bq40z50_interface.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

using namespace std;

#define I2C_FILE_NAME   "/dev/i2c-5"
#define BQ_I2C_ADDR        0x6B

static int fd_i2c;

int fd_chg_ok_pin;
struct pollfd fds_chg_ok_pin[1];


FILE *fp_batt_temp;
int log_batt_temp_flag = 0;


//BQ25703 REGISTER_ADDR
#define CHARGE_OPTION_0_WR                              0x00
#define CHARGE_CURRENT_REGISTER_WR                      0x02
#define MaxChargeVoltage_REGISTER_WR                    0x04
#define OTG_VOLTAGE_REGISTER_WR                         0x06
#define OTG_CURRENT_REGISTER_WR                         0x08
#define INPUT_VOLTAGE_REGISTER_WR                       0x0A
#define MINIMUM_SYSTEM_VOLTAGE_WR                       0x0C
#define INPUT_CURRENT_REGISTER_WR                       0x0E
#define CHARGE_STATUS_REGISTER_R                        0x20
#define PROCHOT_STATUS_REGISTER_R                       0x22
#define INPUT_CURRENT_LIMIT_IN_USE_R                    0x24
#define VBUS_AND_PSYS_VOLTAGE_READ_BACK_R               0x26
#define CHARGE_AND_DISCHARGE_CURRENT_READ_BACK_R        0x28
#define INPUT_CURRENT_AND_CMPIN_VOLTAGE_READ_BACK_R     0x2A
#define SYSTEM_AND_BATTERY_VOLTAGE_READ_BACK_R          0x2C
#define MANUFACTURE_ID_AND_DEVICE_ID_READ_BACK_R        0x2E
#define DEVICE_ID_READ_BACK_R                           0x2F
#define CHARGE_OPTION_1_WR                              0x30
#define CHARGE_OPTION_2_WR                              0x32
#define CHARGE_OPTION_3_WR                              0x34
#define PROCHOT_OPTION_0_WR                             0x36
#define PROCHOT_OPTION_1_WR                             0x38
#define ADC_OPTION_WR                                   0x3A



#define     CHARGE_OPTION_0_SETTING         0x860E //ryder r1 configuration

#define     EN_LEARN                        0x0020

#define     EN_LWPWR                        0x8000


unsigned int battery_relativeStateOfCharge = 0xff;

uint16_t CHARGE_REGISTER_DDR_VALUE_BUF[]= //POGO PIN or USB
{
    CHARGE_OPTION_0_WR,         CHARGE_OPTION_0_SETTING,
    INPUT_VOLTAGE_REGISTER_WR,  INPUT_VOLTAGE_LIMIT_4V1, //here should use the default value:0x0000, means 3200mV
    MINIMUM_SYSTEM_VOLTAGE_WR,  0x1e00, //The charger provides minimum system voltage, means 9216mV
    INPUT_CURRENT_REGISTER_WR,  0x1e00, //Ryder: here only for POGO Pin configuration
    CHARGE_CURRENT_REGISTER_WR, CHARGE_CURRENT_1856mA,
    MaxChargeVoltage_REGISTER_WR, MAX_CHARGE_VOLTAGE,
    OTG_VOLTAGE_REGISTER_WR,    0x0000,
//    OTG_CURRENT_REGISTER_WR,    0x0000,

    CHARGE_OPTION_1_WR,         0x1250,
    CHARGE_OPTION_2_WR,         0x02B7,
    CHARGE_OPTION_3_WR,         0x1000,
    PROCHOT_OPTION_0_WR,        0x4A54,
    PROCHOT_OPTION_1_WR,        0x8120,
    ADC_OPTION_WR,              0xE0FF
};

uint16_t OTG_REGISTER_DDR_VALUE_BUF[]= //USB Detach
{

    OTG_VOLTAGE_REGISTER_WR,    0x0280,//ryder
    OTG_CURRENT_REGISTER_WR,    0x3C00,//ryder
    CHARGE_OPTION_3_WR,         0x1000,//ryder otg enable

};


uint16_t USB_TYPEA_VALUE_BUF[]= //support for  USB A 5V adapto, Hard Reset Event
{
    CHARGE_OPTION_0_WR,         0x860E,
    INPUT_VOLTAGE_REGISTER_WR,  INPUT_VOLTAGE_LIMIT_4V1, //here should use the default value:0x0000, means 3200mV
    
    MINIMUM_SYSTEM_VOLTAGE_WR,  0x1e00, //The charger provides minimum system voltage, means 9216mV
    INPUT_CURRENT_REGISTER_WR,  0x2A00,

    CHARGE_OPTION_3_WR,         0x0000,//ryder otg disable
    CHARGE_CURRENT_REGISTER_WR, CHARGE_CURRENT_1856mA,
    MaxChargeVoltage_REGISTER_WR, MAX_CHARGE_VOLTAGE,

    OTG_VOLTAGE_REGISTER_WR,    0x0280,//ryder
    OTG_CURRENT_REGISTER_WR,    0x3C00,//ryder

	CHARGE_OPTION_3_WR,         0x1000,//ryder otg enable

	CHARGE_OPTION_3_WR,         0x0000,//ryder otg disable
    CHARGE_CURRENT_REGISTER_WR, CHARGE_CURRENT_1856mA,

	CHARGE_OPTION_3_WR,         0x1000,//ryder otg enable
    MaxChargeVoltage_REGISTER_WR, MAX_CHARGE_VOLTAGE,
    
    OTG_VOLTAGE_REGISTER_WR,    0x0280,//ryder
    OTG_CURRENT_REGISTER_WR,    0x3C00,//ryder
};

enum faultStatus
{
	No_Fault = 0,
	Overheat1,
	Overheat2,
	Overcool1,
	Overcool2
};

int OVERHEAT1 = 50;
int OVERHEAT2 = 60;
int OVERCOOL1 = 0;
int OVERCOOL2 = -20;

struct BATTERY_MANAAGE_PARA
{
    unsigned char battery_fully_charged;
    unsigned char need_charge_flag;

    unsigned char temperature_stop_charge;
    unsigned char temperature_allow_charge;

    unsigned char charge_level;

    unsigned char adjust_eq_flag;

    unsigned char low_battery_flag;

    unsigned char battery_is_charging;
	
    unsigned char battery_is_discharging;

    unsigned char charger_is_plug_in;//bit 0 indicates USB, bit 1 indicates POGO_PIN

	unsigned char i2c_silent;
	unsigned char factory_shipment_charge_complete_flag;
	
    LED_BATTERY_DISPLAY_STATE led_battery_display_state;

	int battery_current;
    int battery_temperature;
    int battery_voltage;
	faultStatus fault;

} batteryManagePara;



int i2c_open_bq25703(void)
{
    int ret;
    int val;

    fd_i2c = open(I2C_FILE_NAME, O_RDWR);

    if(fd_i2c < 0)
    {
        perror("Unable to open bq25703 i2c control file");

        return -1;
    }

    printf("open bq25703 i2c file success,fd is %d\n",fd_i2c);

    ret = ioctl(fd_i2c, I2C_SLAVE_FORCE, BQ_I2C_ADDR);
    if (ret < 0)
    {
        perror("i2c: Failed to set i2c device address\n");
        return -1;
    }

    printf("i2c: set i2c device address success\n");

    val = 3;
    ret = ioctl(fd_i2c, I2C_RETRIES, val);
    if(ret < 0)
    {
        printf("i2c: set i2c retry times err\n");
    }

    printf("i2c: set i2c retry times %d\n",val);

    return 0;
}


static int i2c_write(unsigned char dev_addr, unsigned char *val, unsigned char len)
{
    int ret;
    int i;

    struct i2c_rdwr_ioctl_data data;

    struct i2c_msg messages;


    messages.addr = dev_addr;  //device address
    messages.flags = 0;    //write
    messages.len = len;
    messages.buf = val;  //data

    data.msgs = &messages;
    data.nmsgs = 1;

    if(ioctl(fd_i2c, I2C_RDWR, &data) < 0)
    {
        printf("write ioctl err %d\n",fd_i2c);
        return -1;
    }

    printf("i2c write buf = ");
    for(i=0; i< len; i++)
    {
        printf("%02x ",val[i]);
    }
    printf("\n");

    return 0;
}


static int i2c_read(unsigned char addr, unsigned char reg, unsigned char *val, unsigned char len)
{
    int ret;
    int i;

    struct i2c_rdwr_ioctl_data data;
    struct i2c_msg messages[2];

    messages[0].addr = addr;  //device address
    messages[0].flags = 0;    //write
    messages[0].len = 1;
    messages[0].buf = &reg;  //reg address

    messages[1].addr = addr;       //device address
    messages[1].flags = I2C_M_RD;  //read
    messages[1].len = len;
    messages[1].buf = val;

    data.msgs = messages;
    data.nmsgs = 2;

    if(ioctl(fd_i2c, I2C_RDWR, &data) < 0)
    {
        perror("---");
        printf("read ioctl err %d\n",fd_i2c);

        return -1;
    }

    /*printf("i2c read buf = ");
    for(i = 0; i < len; i++)
    {
        printf("%02x ",val[i]);
    }
    printf("\n");*/

    return 0;
}


static int bq25703a_i2c_write(unsigned char dev_addr, unsigned char reg, unsigned char *val, unsigned char data_len)
{
    unsigned char buf[80] = {0};
    int i;

    if(data_len + 1 >= 80)
    {
        printf("data_len_exceed\n");
        return 1;
    }

    buf[0] = reg;

    for(i = 0; i<data_len; i++)
    {
        buf[1+i] = val[i];
    }

    return i2c_write(dev_addr, buf, data_len+1);
}


static int bq25703a_i2c_read(unsigned char addr, unsigned char reg, unsigned char *val, unsigned char len)
{
    return i2c_read(addr, reg, val, len);
}



int bq25703a_otg_function_init()
{
    int i = 0;

    printf("OTG_REGISTER_DDR_VALUE_BUF:\n");
    for (i = 0; i < sizeof(OTG_REGISTER_DDR_VALUE_BUF)/sizeof(uint16_t); i = i + 2)
    {
        printf("%02x, %04x\n",OTG_REGISTER_DDR_VALUE_BUF[i],OTG_REGISTER_DDR_VALUE_BUF[i+1]);

        if(bq25703a_i2c_write(BQ_I2C_ADDR,OTG_REGISTER_DDR_VALUE_BUF[i],((unsigned char*)(&OTG_REGISTER_DDR_VALUE_BUF[i+1])),2) != 0)
        {
            printf("write reg %02x eer\n",OTG_REGISTER_DDR_VALUE_BUF[i]);
            return -1;
        }
    }

    printf("bq25703a OTG function init success");

    return 0;
}


/*
*   set otg voltage and current
*   voltage：
*   current_mA :
*/
int bq25703a_set_otg_vol_and_current()
{
    if(0 != bq25703a_i2c_write(
           BQ_I2C_ADDR,
           OTG_REGISTER_DDR_VALUE_BUF[2],
           ((unsigned char*)(&OTG_REGISTER_DDR_VALUE_BUF[3])),
           2)
      )
    {
        printf("write %d eer\n",OTG_REGISTER_DDR_VALUE_BUF[2]);
        return -1;
    }

    if(0 != bq25703a_i2c_write(
           BQ_I2C_ADDR,
           OTG_REGISTER_DDR_VALUE_BUF[4],
           ((unsigned char*)(&OTG_REGISTER_DDR_VALUE_BUF[5])),
           2)
      )
    {
        printf("write register addr %d eer\n",OTG_REGISTER_DDR_VALUE_BUF[4]);
        return -1;
    }
    return 0;
}


int bq25703a_charge_function_init()
{
    int i = 0;

    printf("CHARGE_REGISTER_DDR_VALUE_BUF:\n");
    for (i = 0; i < sizeof(CHARGE_REGISTER_DDR_VALUE_BUF)/sizeof(uint16_t); i = i + 2)
    {
        printf("%02x, %04x\n",CHARGE_REGISTER_DDR_VALUE_BUF[i],CHARGE_REGISTER_DDR_VALUE_BUF[i+1]);

        if(bq25703a_i2c_write(BQ_I2C_ADDR,CHARGE_REGISTER_DDR_VALUE_BUF[i],((unsigned char*)(&CHARGE_REGISTER_DDR_VALUE_BUF[i+1])),2) != 0)
        {
            printf("write reg %x eer\n",CHARGE_REGISTER_DDR_VALUE_BUF[i]);
            return -1;
        }
    }

    printf("bq25703a charge_function init success\n");

    return 0;
}


int bq25703_set_ChargeCurrent(unsigned int charge_current_set)
{
    int charge_current = charge_current_set;
    int charge_vol = MAX_CHARGE_VOLTAGE;

    printf("set charge current: %dmA\n",charge_current);

    if(0 != bq25703a_i2c_write(
           BQ_I2C_ADDR,
           CHARGE_CURRENT_REGISTER_WR,
           ((unsigned char*)(&charge_current)),
           2)
      )
    {
        printf("write Current eer\n");
        return -1;
    }


    /*printf("set charge voltage: %dmA\n\n",charge_vol);

    if(0 != bq25703a_i2c_write(
           BQ_I2C_ADDR,
           MaxChargeVoltage_REGISTER_WR,
           ((unsigned char*)(&charge_vol)),
           2)
      )
    {
        printf("write VOLTAGE eer\n");
        return -1;
    }*/

    return 0;
}


int bq25703a_get_ChargeCurrentSetting(void)
{
    unsigned char buf[2] = {0};

    int charge_current = 0;

    if(bq25703a_i2c_read(BQ_I2C_ADDR, CHARGE_CURRENT_REGISTER_WR, buf, 2) != 0)
    {
        return -1;
    }
    else
    {
        printf("read charge_current_reg: 0x%02x 0x%02x\n",buf[0],buf[1]);

        if(buf[0] & 0x40)
        {
            charge_current += 64;
        }

        if(buf[0] & 0x80)
        {
            charge_current += 128;
        }

        if(buf[1] & 0x01)
        {
            charge_current += 256;
        }

        if(buf[1] & 0x02)
        {
            charge_current += 512;
        }

        if(buf[1] & 0x04)
        {
            charge_current += 1024;
        }

        if(buf[1] & 0x08)
        {
            charge_current += 2048;
        }

        if(buf[1] & 0x10)
        {
            charge_current += 4096;
        }

        printf("Charge Current Max: %dmA\n\n",charge_current);

        return charge_current;
    }

}


int bq25703_set_InputVoltageLimit(unsigned int input_voltage_limit_set)
{
    int input_voltage_limit = input_voltage_limit_set;

    printf("set charge input voltage limit: %dmA\n",input_voltage_limit + 3200);

    if(0 != bq25703a_i2c_write(
           BQ_I2C_ADDR,
           CHARGE_CURRENT_REGISTER_WR,
           ((unsigned char*)(&input_voltage_limit)),
           2)
      )
    {
        printf("write Current eer\n");
        return -1;
    }

    return 0;
}

int bq25703_set_InputCurrentLimit(unsigned int input_current_limit_set)
{
    int input_voltage_limit = input_current_limit_set;

    printf("set charge input currnet limit: %dmA\n",input_current_limit_set*50);

    if(0 != bq25703a_i2c_write(
           BQ_I2C_ADDR,
           INPUT_CURRENT_REGISTER_WR,
           ((unsigned char*)(&input_current_limit_set)),
           2)
      )
    {
        printf("write Current eer\n");
        return -1;
    }

    return 0;
}


int bq25703a_get_InputVoltageLimit(void)
{
    unsigned char buf[2] = {0};

    int input_voltage_limit = 3200;

    if(bq25703a_i2c_read(BQ_I2C_ADDR, INPUT_VOLTAGE_REGISTER_WR, buf, 2) != 0)
    {
        return -1;
    }
    else
    {
        printf("read input voltage limit reg: 0x%02x 0x%02x\n",buf[0],buf[1]);

        if(buf[0] & 0x40)
        {
            input_voltage_limit += 64;
        }

        if(buf[0] & 0x80)
        {
            input_voltage_limit += 128;
        }

        if(buf[1] & 0x01)
        {
            input_voltage_limit += 256;
        }

        if(buf[1] & 0x02)
        {
            input_voltage_limit += 512;
        }

        if(buf[1] & 0x04)
        {
            input_voltage_limit += 1024;
        }

        if(buf[1] & 0x08)
        {
            input_voltage_limit += 2048;
        }

        if(buf[1] & 0x10)
        {
            input_voltage_limit += 4096;
        }

        printf("Input Voltage Limit: %dmV\n\n",input_voltage_limit);

        return input_voltage_limit;
    }

}


int bq25703a_get_BatteryVol_and_SystemVol(unsigned int *p_BatteryVol, unsigned int *p_SystemVol)
{
    unsigned char buf[2] = {0};

    if(bq25703a_i2c_read(BQ_I2C_ADDR, SYSTEM_AND_BATTERY_VOLTAGE_READ_BACK_R, buf, 2) != 0)
    {
        return -1;
    }
    else
    {
        printf("read SYSTEM_AND_BATTERY_VOL reg: 0x%02x 0x%02x\n",buf[0],buf[1]);

        //vol = 2880mv + buf[1]*64
        *p_BatteryVol = 2880 + buf[0] * 64;
        *p_SystemVol = 2880 + buf[1] * 64;

        printf("Battery Voltage: %dmV\n",*p_BatteryVol);
        printf("System Voltage: %dmV\n\n",*p_SystemVol);
    }

    return 0;
}


int bq25703a_get_PSYS_and_VBUS(unsigned int *p_PSYS_vol, unsigned int *p_VBUS_vol)
{
    unsigned char buf[2] = {0};

    if(bq25703a_i2c_read(BQ_I2C_ADDR, VBUS_AND_PSYS_VOLTAGE_READ_BACK_R, buf, 2) != 0)
    {
        return -1;
    }
    else
    {
        printf("read PSYS_and_VBUS reg: 0x%02x 0x%02x\n",buf[0],buf[1]);

        //psys = value*12
        *p_PSYS_vol = buf[0] * 12;

        //vbus = 3200mv + value*64
        *p_VBUS_vol = 3200 + buf[1] * 64;

        printf("PSYS: %dmV\n",*p_PSYS_vol);
        printf("VBUS: %dmV\n\n",*p_VBUS_vol);
    }

    return 0;
}


int bq25703a_get_CMPINVol_and_InputCurrent(unsigned int *p_CMPIN_vol, unsigned int *p_input_current)
{
    unsigned char buf[2] = {0};

    if(bq25703a_i2c_read(BQ_I2C_ADDR, INPUT_CURRENT_AND_CMPIN_VOLTAGE_READ_BACK_R, buf, 2) != 0)
    {
        return -1;
    }
    else
    {
        printf("read CMPINVol_and_InputCurrent reg: 0x%02x 0x%02x\n",buf[0],buf[1]);

        //CMPIN: Full range: 3.06 V, LSB: 12 mV
        *p_CMPIN_vol = buf[0] * 12;

        //Iuput Current: Full range: 12.75 A, LSB: 50 mA
        *p_input_current = buf[1] * 50;

        printf("CMPIN Voltage: %dmV\n",*p_CMPIN_vol);
        printf("Input Current: %dmA\n\n",*p_input_current);
    }

    return 0;
}


int bq25703a_get_Battery_Current(unsigned int *p_battery_discharge_current, unsigned int *p_battery_charge_current)
{
    unsigned char buf[2] = {0};

    if(bq25703a_i2c_read(BQ_I2C_ADDR, CHARGE_AND_DISCHARGE_CURRENT_READ_BACK_R, buf, 2) != 0)
    {
        return -1;
    }
    else
    {
        printf("read Battery_Current reg: 0x%02x 0x%02x\n",buf[0],buf[1]);

        //IDCHG: Full range: 32.512 A, LSB: 256 mA
        *p_battery_discharge_current = buf[0] * 256;

        //ICHG: Full range: 8.128 A, LSB: 64 mA
        *p_battery_charge_current = buf[1] * 64;

        printf("Battery discharge current: %dmA\n",*p_battery_discharge_current);
        printf("Battery charge current: %dmA\n\n",*p_battery_charge_current);
    }

    return 0;
}


int bq25703a_get_Charger_Status(void)
{
    s_BQ_Charger_Status bq_charger_status = {0};

    s_BQ_Charger_Status *p_bq_charger_status = NULL;

    p_bq_charger_status = &bq_charger_status;

    if(bq25703a_i2c_read(BQ_I2C_ADDR, CHARGE_STATUS_REGISTER_R, (unsigned char*)p_bq_charger_status, 2) != 0)
    {
        return -1;
    }

    printf("get bq25703 Charger Status: \n");
    printf("Fault_OTG_UCP: %d\n", p_bq_charger_status->Fault_OTG_UCP);
    printf("Fault_OTG_OVP: %d\n", p_bq_charger_status->Fault_OTG_OVP);
    printf("Fault_Latchoff: %d\n", p_bq_charger_status->Fault_Latchoff);
    printf("SYSOVP_STAT: %d\n", p_bq_charger_status->SYSOVP_STAT);
    printf("Fault_ACOC: %d\n", p_bq_charger_status->Fault_ACOC);
    printf("Fault_BATOC: %d\n", p_bq_charger_status->Fault_BATOC);
    printf("Fault_ACOV: %d\n", p_bq_charger_status->Fault_ACOV);
    printf("IN_OTG: %d\n", p_bq_charger_status->IN_OTG);
    printf("IN_PCHRG: %d\n", p_bq_charger_status->IN_PCHRG);
    printf("IN_FCHRG: %d\n", p_bq_charger_status->IN_FCHRG);
    printf("IN_IINDPM: %d\n", p_bq_charger_status->IN_IINDPM);
    printf("IN_VINDPM: %d\n", p_bq_charger_status->IN_VINDPM);
    printf("ICO_DONE: %d\n", p_bq_charger_status->ICO_DONE);
    printf("AC_STAT: %d\n\n", p_bq_charger_status->AC_STAT);

    return 0;
}


int bq25703_init_ChargeOption_0(void)
{
    int charge_option_0_setting = CHARGE_OPTION_0_SETTING;

    printf("charge_option_0_setting: %04x\n",charge_option_0_setting);

    if(0 != bq25703a_i2c_write(
           BQ_I2C_ADDR,
           CHARGE_OPTION_0_WR,
           ((unsigned char*)(&charge_option_0_setting)),
           2)
      )
    {
        printf("write reg eer\n");
        return -1;
    }

    return 0;
}


/*
* LEARN function allows the battery to discharge while the adapter is present. It
* calibrates the battery gas gauge over a complete discharge/charge cycle. When
* the battery voltage is below battery depletion threshold, the system switches
* back to adapter input by the host. When CELL_BATPRESZ pin is LOW, the
* device exits LEARN mode and this bit is set back to 0
*/
int bq25703_enter_LEARN_Mode(void)
{
    int charge_option_0_setting = CHARGE_OPTION_0_SETTING | EN_LEARN;

    printf("charge_option_0_setting: %04x\n",charge_option_0_setting);

    if(0 != bq25703a_i2c_write(
           BQ_I2C_ADDR,
           CHARGE_OPTION_0_WR,
           ((unsigned char*)(&charge_option_0_setting)),
           2)
      )
    {
        printf("write reg eer\n");
        return -1;
    }

    printf("\nbq25703 enter LEARN_Mode\n\n\n");

    return 0;
}


int bq25703_enter_LowPowerMode(void)
{
    int charge_option_0_setting = CHARGE_OPTION_0_SETTING | EN_LWPWR;

    printf("charge_option_0_setting: %04x\n",charge_option_0_setting);

    if(0 != bq25703a_i2c_write(
           BQ_I2C_ADDR,
           CHARGE_OPTION_0_WR,
           ((unsigned char*)(&charge_option_0_setting)),
           2)
      )
    {
        printf("write reg eer\n");
        return -1;
    }

    printf("\nbq25703 enter Low Power Mode\n\n\n");

    return 0;
}


int bq25703a_get_ChargeOption0_Setting(void)
{
    unsigned char buf[2] = {0};

    if(bq25703a_i2c_read(BQ_I2C_ADDR, CHARGE_OPTION_0_WR, buf, 2) != 0)
    {
        return -1;
    }

    printf("get ChargeOption0 setting: 0x%02x%02x\n",buf[1],buf[0]);

    if(buf[1] & 0x80)
    {
        printf("Low Power Mode is enabled\n\n");
    }

    if(buf[0] & 0x20)
    {
        printf("LEARN_Mode is enabled\n\n");
    }

    return 0;
}


int bq25703_stop_charge(void)
{
    if(bq25703_set_ChargeCurrent(CHARGE_CURRENT_0) == 0)
    {
        //batteryManagePara.battery_is_charging = 0;

        return 0;
    }

    return -1;
}


unsigned char decide_the_ChargeLevel(void)
{
    unsigned char charge_level = 0;

    if(batteryManagePara.battery_voltage <= BATTERY_LOW_VOLTAGE_THRESHOLD)
    {
        if((batteryManagePara.battery_temperature >= BATTERY_CHARGE_ALLOW_TEMPERATURE_LOW_THRESHOLD)
           && (batteryManagePara.battery_temperature < BATTERY_LOW_TEMPERATURE_THRESHOLD))
        {
            charge_level = 1;
        }
        else if((batteryManagePara.battery_temperature >= BATTERY_LOW_TEMPERATURE_THRESHOLD)
                && (batteryManagePara.battery_temperature <= BATTERY_CHARGE_ALLOW_TEMPERATURE_HIGH_THRESHOLD))
        {
            charge_level = 1;
        }
        else
        {
            charge_level = 0;
        }
    }

    if((batteryManagePara.battery_voltage > BATTERY_LOW_VOLTAGE_THRESHOLD) && (batteryManagePara.battery_voltage <= BATTERY_MAX_VOLTAGE_THRESHOLD))
    {
        if((batteryManagePara.battery_temperature >= BATTERY_CHARGE_ALLOW_TEMPERATURE_LOW_THRESHOLD)
           && (batteryManagePara.battery_temperature < BATTERY_LOW_TEMPERATURE_THRESHOLD))
        {
            charge_level = 2;
        }
        else if((batteryManagePara.battery_temperature >= BATTERY_LOW_TEMPERATURE_THRESHOLD)
                && (batteryManagePara.battery_temperature <= BATTERY_CHARGE_ALLOW_TEMPERATURE_HIGH_THRESHOLD))
        {
            charge_level = 3;
        }
        else
        {
            charge_level = 0;
        }
    }

    if(batteryManagePara.battery_voltage > BATTERY_MAX_VOLTAGE_THRESHOLD)
    {
        charge_level = 0;
    }

    return charge_level;
}


int decide_the_ChargeCurrent(void)
{
    unsigned char charge_level = 0;

    int charge_current = CHARGE_CURRENT_0;

    charge_level = decide_the_ChargeLevel();

    printf("\ndecide_the_ChargeLevel: %d by voltage: %dmV, temperature: %dC\n",charge_level, batteryManagePara.battery_voltage, batteryManagePara.battery_temperature);

    switch(charge_level)
    {
        case 1:
            charge_current = CHARGE_CURRENT_LEVEL_1;
            break;

        case 2:
            charge_current = CHARGE_CURRENT_LEVEL_2;
            break;

        case 3:
            charge_current = CHARGE_CURRENT_LEVEL_3;
            break;

        case 0:
        default:
            charge_current = CHARGE_CURRENT_0;
            break;
    }

    printf("decide_the_ChargeCurrent: %dmA\n",charge_current);

    return charge_current;
}


int bq25703_enable_charge(void)
{
    int ret;

    unsigned int VBus_vol = 0;
    unsigned int PSys_vol = 0;

    int tps65987_TypeC_current_type;

    int charge_current = CHARGE_CURRENT_0;

    bq25703a_get_PSYS_and_VBUS(&PSys_vol, &VBus_vol);
    printf("get VBus_vol = %d\n",VBus_vol);


    if(bq25703_init_ChargeOption_0() != 0)
    {
        return -1;
    }


    //check TypeC Current type to decide the charge current
    tps65987_TypeC_current_type = tps65987_get_TypeC_Current();

    if((batteryManagePara.battery_fully_charged))
//       && (tps65987_TypeC_current_type != USB_Default_Current))
    {
        return 0;
    }

    switch(tps65987_TypeC_current_type)
    {
        case USB_Default_Current:
       		batteryManagePara.charger_is_plug_in |= 1;

			//input current set to 2.1A 
			if(0 == (ret = bq25703_set_InputCurrentLimit(0x2a00))){
            	ret = bq25703_set_ChargeCurrent(CHARGE_CURRENT_FOR_USB_Default);
			}
			
            if(ret == 0)
            {
                //batteryManagePara.battery_is_charging = 1;
            }
            break;

        case C_1d5A_Current:
            batteryManagePara.charger_is_plug_in |= 1;
			if(0 == (ret = bq25703_set_InputCurrentLimit(0x2a00))){
	            ret = bq25703_set_ChargeCurrent(CHARGE_CURRENT_FOR_USB_Default);
			}
			
            if(ret == 0)
            {
               // batteryManagePara.battery_is_charging = 1;
            }
            break;

        case C_3A_Current:
            batteryManagePara.charger_is_plug_in |= 1;
			if(0 == (ret = bq25703_set_InputCurrentLimit(0x2a00))){
				
				charge_current = decide_the_ChargeCurrent();
				
				ret = bq25703_set_ChargeCurrent(charge_current);
			}		
			
			if(ret == 0)
			{
				//batteryManagePara.battery_is_charging = 1;
			}

				break;
        case PD_contract_negotiated:
            batteryManagePara.charger_is_plug_in |= 1;
			if(0 == (ret = bq25703_set_InputCurrentLimit(0x1e00))){

	            charge_current = decide_the_ChargeCurrent();

	            ret = bq25703_set_ChargeCurrent(charge_current);
			}
			
            if(ret == 0)
            {
                //batteryManagePara.battery_is_charging = 1;
            }
            break;

        default:
            ret = -1;
            break;

    }

    return ret;

}


int init_Chg_OK_Pin(void)
{
    char file_path[64]= {0};

    int pin_number = CHG_OK_PIN;

    export_gpio(pin_number);
    set_direction(pin_number, "in");
    //set_edge(pin_number, "rising");
    set_edge(pin_number, "both");

    sprintf(file_path, "/sys/class/gpio/gpio%d/value", pin_number);

    fd_chg_ok_pin = open(file_path, O_RDONLY);
    if(fd_chg_ok_pin < 0)
    {
        printf("can't open %s!\n", file_path);
        return -1;
    }

    return 0;
}


int get_Chg_OK_Pin_value(void)
{
    unsigned char value[4];
    int n;

    if(lseek(fd_chg_ok_pin, 0, SEEK_SET) == -1)
    {
        printf("lseek failed!\n");
        return -1;
    }

    n = read(fd_chg_ok_pin, value, sizeof(value));
    printf("read %d bytes %c %c\n", n, value[0],value[1]);

    return value[0];
}


void batteryManagePara_init(void)
{
    batteryManagePara.battery_fully_charged = 0;
    batteryManagePara.need_charge_flag = 0;

    batteryManagePara.temperature_stop_charge = 0;
    batteryManagePara.temperature_allow_charge = 0;

    batteryManagePara.charge_level = 0;

    batteryManagePara.adjust_eq_flag = 0;

    batteryManagePara.low_battery_flag = 0;

    batteryManagePara.battery_is_charging = 0;

    batteryManagePara.charger_is_plug_in = 0;

	batteryManagePara.factory_shipment_charge_complete_flag = 0;

    batteryManagePara.led_battery_display_state = LED_BATTERY_INVALID_VALUE;

    batteryManagePara.battery_temperature = 0;
    batteryManagePara.battery_voltage = 0;

    batteryManagePara.i2c_silent = 0;
	batteryManagePara.fault = No_Fault;
	

}

void batteryManagePara_clear(void)
{
    batteryManagePara.need_charge_flag = 0;
    batteryManagePara.charge_level = 0;
}


void check_BatteryFullyCharged_Task(void)
{
    switch(fuelgauge_check_BatteryFullyCharged())
    {
        case 1:
            if(!batteryManagePara.battery_fully_charged)
            {
                if(bq25703_stop_charge() != 0)
                {
                    break;
                }

                printf("fully charged, stop charging!\n");
            }

            batteryManagePara.battery_fully_charged = 1;
            batteryManagePara.need_charge_flag = 0;
            batteryManagePara.charge_level = 0;
            break;

        case 0:
            if(!batteryManagePara.need_charge_flag)
            {
                if(batteryManagePara.temperature_allow_charge)
                {
                    if(batteryManagePara.charger_is_plug_in & 0x01)
                    {
                        if(bq25703_enable_charge() != 0)
                        {
                            return;
                        }
                    }
                }
            }

            batteryManagePara.battery_fully_charged = 0;
            batteryManagePara.need_charge_flag = 1;
            break;

        default:
            break;
    }
}


int batteryTemperature_is_overstep_ChargeStopThreshold(int battery_temperature)
{
    if((battery_temperature >= BATTERY_CHARGE_STOP_TEMPERATURE_HIGH_THRESHOLD)
       || (battery_temperature <= BATTERY_CHARGE_STOP_TEMPERATURE_LOW_THRESHOLD))
    {
        return 1;
    }

    return 0;
}

int batteryTemperature_is_in_ChargeAllowThreshold(int battery_temperature)
{
    if(( battery_temperature < BATTERY_CHARGE_ALLOW_TEMPERATURE_HIGH_THRESHOLD)
       && (battery_temperature > BATTERY_CHARGE_ALLOW_TEMPERATURE_LOW_THRESHOLD))
    {
        return 1;
    }

    return 0;
}

int batteryTemperature_is_overstep_AdjustEQThreshold(int battery_temperature)
{
    if((battery_temperature >= BATTERY_DISCHARGE_ADJUST_EQ_TEMPERATURE_HIGH_THRESHOLD)
       && (battery_temperature < BATTERY_DISCHARGE_STOP_TEMPERATURE_HIGH_THRESHOLD))
    {
        return 1;
    }

    return 0;
}

int batteryTemperature_is_in_RecoveryEQThreshold(int battery_temperature)
{
    if(( battery_temperature < BATTERY_DISCHARGE_RECOVERY_EQ_TEMPERATURE_HIGH_THRESHOLD)
       && (battery_temperature > BATTERY_DISCHARGE_STOP_TEMPERATURE_LOW_THRESHOLD))
    {
        return 1;
    }

    return 0;
}

int batteryTemperature_is_overstep_DischargeStopThreshold(int battery_temperature)
{
    if((battery_temperature >= BATTERY_DISCHARGE_STOP_TEMPERATURE_HIGH_THRESHOLD)
       || (battery_temperature <= BATTERY_DISCHARGE_STOP_TEMPERATURE_LOW_THRESHOLD))
    {
        return 1;
    }

    return 0;
}


int batteryVoltage_is_over_MaxThreshold(int battery_voltage)
{
    if(battery_voltage > BATTERY_MAX_VOLTAGE_THRESHOLD)
    {
        return 1;
    }

    return 0;
}


int check_Battery_allow_charge(void)
{
    int battery_temperature;
    int battery_voltage;

    battery_temperature = fuelgauge_get_Battery_Temperature();
    if(battery_temperature == Temperature_UNVALID)
    {
        return -1;
    }

    batteryManagePara.battery_temperature = battery_temperature;


    battery_voltage = fuelgauge_get_Battery_Voltage();
    if(battery_voltage == -1)
    {
        return -1;
    }

    batteryManagePara.battery_voltage = battery_voltage;


    if(batteryTemperature_is_in_ChargeAllowThreshold(batteryManagePara.battery_temperature)
       && (!batteryVoltage_is_over_MaxThreshold(batteryManagePara.battery_voltage)))
    {
        printf("battery allow charging!, temperature %d, voltage %dmV\n",batteryManagePara.battery_temperature, batteryManagePara.battery_voltage);
        return 1;
    }

    printf("battery not allow charging!, temperature %d, voltage %dmV\n",batteryManagePara.battery_temperature, batteryManagePara.battery_voltage);
    return 0;
}


int check_TypeC_current_type(void)
{
    int tps65987_TypeC_current_type;

    //check TypeC Current type
    tps65987_TypeC_current_type = tps65987_get_TypeC_Current();

    if(tps65987_TypeC_current_type == -1)
    {
        return -1;
    }

    switch(tps65987_TypeC_current_type)
    {
        case USB_Default_Current:
            //batteryManagePara.charger_is_plug_in = 0;
            break;

        case C_1d5A_Current:
        case C_3A_Current:
        case PD_contract_negotiated:
            batteryManagePara.charger_is_plug_in |= 1;
            break;
    }

    return 0;
}


int create_batteryTemperture_logFile(void)
{
    fp_batt_temp = fopen("/data/battery_temperature_log","a+");
    if(fp_batt_temp == NULL)
    {
        printf("fail to create battery_temperature_log\n");
        return -1;
    }

    log_batt_temp_flag = 1;

    printf("open battery_temperature_log file success, fd is %d\n", fileno(fp_batt_temp));

    time_t nSeconds;
    struct tm * pTM;

    time(&nSeconds);
    pTM = localtime(&nSeconds);

    if(fprintf(fp_batt_temp, "\n\nstart_new_log: %04d-%02d-%02d %02d:%02d:%02d\n",
               pTM->tm_year + 1900, pTM->tm_mon + 1, pTM->tm_mday,
               pTM->tm_hour, pTM->tm_min, pTM->tm_sec) >= 0)
    {
        fflush(fp_batt_temp);
    }
    else
    {
        printf("write file error");
    }

    return 0;
}

int system_power_off(void)
{
    int pin_number = 113; //system power EN pin

    export_gpio(pin_number);

    set_direction(pin_number, "out");
    
    set_value(pin_number, 1);

    printf("shutdown system\n");
    system("shutdown 0");
    return 0;
}


int update_fuelgauge_BatteryInfo(void)
{
    //get by fuelgauge IC
    int battery_temperature;
    int battery_voltage;
    int battery_current;


    battery_voltage = fuelgauge_get_Battery_Voltage();
    if(battery_voltage == -1)
    {
        return -1;
    }

    batteryManagePara.battery_voltage = battery_voltage;

    battery_current = fuelgauge_get_Battery_Current();
	batteryManagePara.battery_current = battery_current;
	if(battery_current == 0)
	 {
		 batteryManagePara.battery_is_charging = 0;
		 batteryManagePara.battery_is_discharging = 0;
	 }else if(battery_current < 0)
	 {
	 	batteryManagePara.battery_is_discharging = 1;
	 	batteryManagePara.battery_is_charging = 0;
	 }
	 else
	 {
		 batteryManagePara.battery_is_charging = 1;
		 batteryManagePara.battery_is_discharging = 0;
		 printf("Battery is charging, current is %d", battery_current);
	 }

    battery_relativeStateOfCharge = fuelgauge_get_RelativeStateOfCharge();
    if(battery_relativeStateOfCharge != -1)
    {
    	FILE *fp;
		char buff[128];
		char cmd[128];

		snprintf(cmd, 128, "echo %d > /dev/shm/bq-drv-r1-SOC", battery_relativeStateOfCharge);
		system(cmd);
		
		if(battery_relativeStateOfCharge < 5){
            batteryManagePara.low_battery_flag = 1;
			
			if(battery_relativeStateOfCharge > 2)
			{
				batteryManagePara.low_battery_flag |= 1 << 1;
			}else if(battery_relativeStateOfCharge > 1)
			{
				batteryManagePara.low_battery_flag |= 1 << 2;
			}else{
				batteryManagePara.low_battery_flag |= 1 << 3;
			}
        }else if(battery_relativeStateOfCharge > 5){
        
        	if(batteryManagePara.low_battery_flag != 0){
				batteryManagePara.low_battery_flag = 0;
        	}
			
			if(fp = fopen("/dev/shm/shipment_SOC", "r"))
			{
				   fgets(buff, 128, (FILE*)fp);
				   fclose(fp);
				   printf("get shipment SOC %d",atoi(buff));
					if(battery_relativeStateOfCharge >= atoi(buff))
					{
							batteryManagePara.factory_shipment_charge_complete_flag = 1;
							printf("shipment SOC reached\n");
					}else
					{
						 printf("shipment SOC not not reached\n");
					}
			}

			if(battery_relativeStateOfCharge == 100 && batteryManagePara.charger_is_plug_in)
			{
					batteryManagePara.battery_fully_charged |= 0x01;
			}else{
					batteryManagePara.battery_fully_charged &= ~0x01;
			}
		}
    }

    battery_temperature = fuelgauge_get_Battery_Temperature();
    if(battery_temperature == Temperature_UNVALID)
    {
        return -1;
    }

    batteryManagePara.battery_temperature = battery_temperature;

	

    return 0;
}


void batteryCharge_handle_Task(int battery_temperature)
{
    if(batteryTemperature_is_overstep_ChargeStopThreshold(battery_temperature))
    {
        if(!batteryManagePara.temperature_stop_charge)
        {
            if(bq25703_stop_charge() != 0)
            {
                return;
            }

            printf("battery temperature %d, is over threshold, stop charging!\n",battery_temperature);
        }

        batteryManagePara.temperature_stop_charge = 1;
        batteryManagePara.temperature_allow_charge = 0;
    }
    else if(batteryTemperature_is_in_ChargeAllowThreshold(battery_temperature))
    {
        if(!batteryManagePara.battery_fully_charged)
        {
            if(batteryManagePara.charger_is_plug_in & 0x01)
            {
                unsigned char charge_level = 0;

                charge_level = decide_the_ChargeLevel();

                switch(charge_level)
                {
                    case 1:
                        if(batteryManagePara.charge_level == 1)
                        {
                            break;
                        }

                        printf("\ndecide_the_ChargeLevel 1 by voltage: %dmV, temperature: %dC\n",batteryManagePara.battery_voltage, batteryManagePara.battery_temperature);

                        if(bq25703_enable_charge() != 0)
                        {
                            break;
                        }

                        batteryManagePara.charge_level = 1;
                        break;

                    case 2:
                        if(batteryManagePara.charge_level == 2)
                        {
                            break;
                        }

                        printf("\ndecide_the_ChargeLevel 2 by voltage: %dmV, temperature: %dC\n",batteryManagePara.battery_voltage, batteryManagePara.battery_temperature);

                        if(bq25703_enable_charge() != 0)
                        {
                            break;
                        }

                        batteryManagePara.charge_level = 2;
                        break;

                    case 3:
                        if(batteryManagePara.charge_level == 3)
                        {
                            break;
                        }

                        printf("\ndecide_the_ChargeLevel 3 by voltage: %dmV, temperature: %dC\n",batteryManagePara.battery_voltage, batteryManagePara.battery_temperature);

                        if(bq25703_enable_charge() != 0)
                        {
                            break;
                        }

                        batteryManagePara.charge_level = 3;
                        break;

                    case 0:
                    default:
                        batteryManagePara.charge_level = 0;
                        break;
                }
            }
        }

        batteryManagePara.temperature_stop_charge = 0;
        batteryManagePara.temperature_allow_charge = 1;
    }
}





void batteryFault_handle_Task(int battery_temperature)
{
	printf("temrature is %d\n", battery_temperature);

	if(battery_temperature > OVERHEAT2 && batteryManagePara.fault != Overheat2)
	{
		batteryManagePara.fault = Overheat2;
		system("adk-message-send 'system_mode_management {name: \"batfault::overheat2\"}'");
	}else if(battery_temperature > OVERHEAT1 && batteryManagePara.fault != Overheat1){
		batteryManagePara.fault = Overheat1;
		system("adk-message-send 'system_mode_management {name: \"batfault::overheat1\"}'");
	}else if(battery_temperature < OVERCOOL2 && batteryManagePara.fault != Overcool2){
		batteryManagePara.fault = Overcool2;
		system("adk-message-send 'system_mode_management {name: \"batfault::overcool2\"}'");
	}else if(battery_temperature < OVERCOOL1 && batteryManagePara.fault != Overcool1){
		batteryManagePara.fault = Overcool1;
		system("adk-message-send 'system_mode_management {name: \"batfault::overcool1\"}'");
	}else if(battery_temperature < OVERHEAT1 && battery_temperature > OVERCOOL1 && batteryManagePara.fault != No_Fault){
		batteryManagePara.fault = No_Fault;
		system("adk-message-send 'system_mode_management {name: \"batfault::nofault\"}'");
	}else{
		printf("fault status not changed.\n");
	}
	
	return;
}

void batteryDisCharge_handle_Task(int battery_temperature)
{
    //when Adapter is pluged, no need to adjust EQ
    if(!batteryManagePara.charger_is_plug_in)
    {
        if(batteryTemperature_is_overstep_AdjustEQThreshold(battery_temperature))
        {
            if(!batteryManagePara.adjust_eq_flag)
            {
                //adjust EQ
                printf("battery temperature %d, overstep AdjustEQThreshold, adjust EQ\n",battery_temperature);
            }

            batteryManagePara.adjust_eq_flag = 1;
        }
        else if(batteryTemperature_is_in_RecoveryEQThreshold(battery_temperature))
        {
            if(batteryManagePara.adjust_eq_flag)
            {
                //recovery EQ
                printf("battery temperature %d, in RecoveryEQThreshold, recovery EQ\n",battery_temperature);
            }

            batteryManagePara.adjust_eq_flag = 0;
        }
    }

    if(batteryTemperature_is_overstep_DischargeStopThreshold(battery_temperature))
    {
        //power off
        if(system_power_off() != 0)
        {
            printf("system power_off fail\n\n");
        }
    }
}


void batteryTemperature_handle_Task(void)
{
    if(update_fuelgauge_BatteryInfo() != 0)
    {
        return;
    }

    if(log_batt_temp_flag)
    {
        //log the battery_temperature for debug
        if(fprintf(fp_batt_temp, "%d\n", batteryManagePara.battery_temperature) >= 0)
        {
            fflush(fp_batt_temp);
        }
        else
        {
            printf("write file error");
        }
    }

    batteryCharge_handle_Task(batteryManagePara.battery_temperature);

    batteryDisCharge_handle_Task(batteryManagePara.battery_temperature);

	batteryFault_handle_Task(batteryManagePara.battery_temperature);
}



void led_battery_display_init(void)
{
    /**********************************************
     It is init by kernel now:
     /sys/class/leds/power_led_r/brightness
     /sys/class/leds/power_led_g/brightness
     /sys/class/leds/power_led_b/brightness
    **********************************************/

}


void led_battery_display(LED_BATTERY_DISPLAY_STATE type)
{
    switch(type)
    {
        case LED_BATTERY_FULLY_CHARGED:
            system("adk-message-send 'led_start_pattern{pattern:32}'");

            //now the 0/1 is reversed
            /*set_battery_led('r', 0);
            set_battery_led('g', 0);
            set_battery_led('b', 0);*/

            printf("display LED_BATTERY_FULLY_CHARGED\n\n");
            break;

        case LED_BATTERY_CHARGEING:
            system("adk-message-send 'led_start_pattern{pattern:30}'");

            /*set_battery_led('r', 1);
            set_battery_led('g', 0);
            set_battery_led('b', 1);*/

            printf("display LED_BATTERY_CHARGEING\n\n");
            break;
			
        case LED_BATTERY_DISCHARGEING:
            system("adk-message-send 'led_start_pattern{pattern:31}'");

            /*set_battery_led('r', 1);
            set_battery_led('g', 0);
            set_battery_led('b', 1);*/

            printf("display LED_BATTERY_DISCHARGEING\n\n");
            break;

        case LED_BATTERY_LOW:
            //system("adk-message-send 'led_start_pattern{pattern:33}'");
			  ;
			  static struct timeval last_time = {.tv_sec = 0, .tv_usec = 0};
			  struct timeval current_time;
  			  gettimeofday(&current_time, NULL);
  			  printf("seconds : %ld\nmicro seconds : %ld",
    				current_time.tv_sec, current_time.tv_usec);
			  if(last_time.tv_sec == 0){
		  			last_time = current_time;
			  }else if(0 < batteryManagePara.low_battery_flag >> 2){
			  //simulate action key power off
					system("adk-message-send 'system_mode_management{name:\"trigger::lowbattery_power_off\"}'");
			  }else if(0 < batteryManagePara.low_battery_flag >> 1){
			  	if(current_time.tv_sec - last_time.tv_sec > 120){
		  			last_time = current_time;						
					system("adk-message-send 'audio_prompt_play{type : \"tone\" name : \"r1-BatteryWarning\" }'");		  
			  	}
			  }else if(0 < batteryManagePara.low_battery_flag){
			  	if(current_time.tv_sec - last_time.tv_sec > 300){
		  			last_time = current_time;						
					system("adk-message-send 'audio_prompt_play{type : \"tone\" name : \"r1-BatteryWarning\" }'");		  
			  	}			  
			  }

            /*set_battery_led('r', 0);
            set_battery_led('g', 1);
            set_battery_led('b', 1);*/

            printf("display LED_BATTERY_LOW\n\n");
            break;

        case LED_BATTERY_OFF:
			//ryder: light off charging indication
            system("adk-message-send 'led_start_pattern{pattern:33}'");

            /*set_battery_led('r', 1);
            set_battery_led('g', 1);
            set_battery_led('b', 1);*/

            printf("display LED_BATTERY_OFF\n\n");
            break;

        default:
            break;

    }
}


void led_battery_display_handle(void)
{
//ryder: consider POGO Pin charge
/*	FILE* fp;
	char buff[128];
	if(fp = fopen("/dev/shm/r1sysState", "r"))
	{
		   fgets(buff, 128, (FILE*)fp);
		   fclose(fp);
		   printf("system state %s",buff);
			if(strcmp(buff, "Docked") == 0)
			{
				batteryManagePara.battery_is_charging = 1;
			}
	}
*/
//ryder:if charge for shipment, send message to set shipment mode.

	printf("ryder: battery display state is %d",batteryManagePara.led_battery_display_state);
	printf("ryder: battery charging state is %d",batteryManagePara.battery_is_charging);
	printf("ryder: battery plugged state is %d",batteryManagePara.charger_is_plug_in);


	if(batteryManagePara.factory_shipment_charge_complete_flag)
	{
		system("adk-message-send 'system_mode_management{name:\"trigger::factory_charge_complete\"}'");
	}

    if(batteryManagePara.battery_is_charging)
    {

    	if(batteryManagePara.led_battery_display_state != LED_BATTERY_CHARGEING)
        {
            led_battery_display(LED_BATTERY_CHARGEING);
        }

		
		batteryManagePara.led_battery_display_state = LED_BATTERY_CHARGEING;
    }
    else if(batteryManagePara.battery_is_discharging && batteryManagePara.charger_is_plug_in)
	{
	    if(batteryManagePara.led_battery_display_state != LED_BATTERY_DISCHARGEING)
        {
            led_battery_display(LED_BATTERY_DISCHARGEING);
        }
		
		batteryManagePara.led_battery_display_state = LED_BATTERY_DISCHARGEING;
	}else if(batteryManagePara.battery_fully_charged){
	        if(batteryManagePara.charger_is_plug_in)
	        {

	            if(batteryManagePara.led_battery_display_state != LED_BATTERY_FULLY_CHARGED)
	            {
	                led_battery_display(LED_BATTERY_FULLY_CHARGED);
	            }

	            batteryManagePara.led_battery_display_state = LED_BATTERY_FULLY_CHARGED;
	        }else
	  		{
	  			 if(batteryManagePara.led_battery_display_state != LED_BATTERY_OFF)
	  			 {
	  				 led_battery_display(LED_BATTERY_OFF);
	  			 }
	  		
	  			 batteryManagePara.led_battery_display_state = LED_BATTERY_OFF;
	  		}

	}else if(batteryManagePara.low_battery_flag){
            /*if(batteryManagePara.led_battery_display_state != LED_BATTERY_LOW)*/
            {
                led_battery_display(LED_BATTERY_LOW);
            }

            //batteryManagePara.led_battery_display_state = LED_BATTERY_LOW;
			
	}else/* if(batteryManagePara.led_battery_display_state == LED_BATTERY_CHARGEING)*/{
            if(batteryManagePara.led_battery_display_state != LED_BATTERY_OFF)
            {
                led_battery_display(LED_BATTERY_OFF);
            }

            batteryManagePara.led_battery_display_state = LED_BATTERY_OFF;
    }
}




void *check_batteryShutdownMode_thread(void *arg)
{
    int ret;

    int i, length;
    unsigned char buffer[256];

    int inotifyFd;
    struct inotify_event *event;

    char *p_name = "batt_shut_down";

    inotifyFd = inotify_init();
    if(inotifyFd < 0)
    {
        perror("Unable to create inotifyFd");
        return -1;
    }

    ret = inotify_add_watch(inotifyFd, "/tmp", IN_CREATE | IN_DELETE);

    if(ret < 0)
    {
        close(inotifyFd);
        perror("Unable to add inotify watch");
        return -1;
    }

    while(1)
    {
        length = read(inotifyFd, buffer, 256);

        if(length < 0)
        {
            perror("Failed to read.\n");
            continue;
        }

        event = (struct inotify_event*)(buffer);

        if(memcmp(event->name, p_name, strlen(p_name)) == 0)
        {
            if(event->mask & IN_CREATE)
            {
                printf("file %s create\n", event->name);

                printf("make battery enter shutdown mode\n");

                if(bq25703_stop_charge() != 0)
                {
                    continue;
                }

                usleep(100*1000);

                if(fuelgauge_battery_enter_shutdown_mode() != 0)
                {
                    printf("battery enter_shutdown_mode err\n");
                    continue;
                }

                fuelgauge_disable_communication();
            }

            if(event->mask & IN_DELETE)
            {
                printf("file %s delete\n", event->name);

                fuelgauge_enable_communication();
            }
        }
    }

}



void *check_gpiokey_thread(void *arg)
{
    int fd = 0;

    struct input_event event;

    int ret = 0;

    int err_cnt = 0;

    int key_power_pressed = 0;

    struct timeval tBeginTime, tEndTime;
    float fCostTime = 0;

    fd = open("/dev/input/event1", O_RDONLY);

    if(fd < 0)
    {
        perror("Unable to open INPUT_DEV");
        return;
    }

    printf("open INPUT_DEV file success, fd in thread is %d\n",fd);

    while(1)
    {
        ret = read(fd, &event, sizeof(event));

        if(ret < 0)
        {
            perror("Failed to read.\n");
            continue;
        }

        if(event.type != EV_SYN)
        {
            printf("type:%d, code:%d, value:%d\n", event.type, event.code, event.value);

            if(event.code == KEY_POWER)
            {
                switch(event.value)
                {
                    case 1:
                        key_power_pressed = 1;

                        gettimeofday(&tBeginTime, NULL);
                        break;

                    case 0:
                        if(key_power_pressed != 1)
                        {
                            break;
                        }

                        key_power_pressed = 0;

                        gettimeofday(&tEndTime, NULL);

                        fCostTime = 1000000*(tEndTime.tv_sec-tBeginTime.tv_sec) + (tEndTime.tv_usec-tBeginTime.tv_usec);
                        fCostTime /= 1000000;

                        printf("[gettimeofday]Cost Time = %fSec\n", fCostTime);

                        if(fCostTime > 5 && fCostTime < 10)
                        {
                            printf("system power_off\n\n");

                            system("adk-message-send 'led_start_pattern{pattern:35}'");

                            for(err_cnt = 0; bq25703_enter_LowPowerMode()!= 0; err_cnt++)
                            {
                                if(err_cnt > 3)
                                    break;
                            }

                            sleep(2);
                            system_power_off();
                        }

                        break;
                }
            }
        }
    }
}

void *bq25703a_stdin_thread(void *arg)
{
    std::istream &mystream = std::cin;
    std::string event;

	/* Enable so that POGO pin can shart charging.
	bq25703a_charge_function_init();
	*/

	
    sd_notifyf(0, "READY=1\n"
    "STATUS=Processing requests...\n"
    "MAINPID=%lu",
    (unsigned long) getpid());
    
    while (mystream.good())
    {
        getline(mystream, event);
		printf("got event: %s\n", event.c_str());
        if(event.compare("button_LANTERN_DP") == 0)
        {
            if(battery_relativeStateOfCharge <= 100)
            {
                if(battery_relativeStateOfCharge >= 75)
                {
                 	system("adk-message-send 'led_start_pattern{pattern:34}'");
                }else if(battery_relativeStateOfCharge >= 35){
					system("adk-message-send 'led_start_pattern{pattern:35}'");

                }else{
					system("adk-message-send 'led_start_pattern{pattern:36}'");

                }

            }
        }else if(event.compare("trigger::GPIO115falling") == 0){
					batteryManagePara.charger_is_plug_in |= 0x02;
					//to add charger configration for POGO PIN
					int ret_val = check_Battery_allow_charge();
					if(ret_val == 1){
						if(!bq25703a_charge_function_init()){
							syslog(LOG_ERR, "POGO PIN CHARGE configuration Error.");
						}
					}
		}else if(event.compare("trigger::GPIO115rising") == 0){
		
					batteryManagePara.charger_is_plug_in &= ~0x02;
					if(batteryManagePara.charger_is_plug_in & 0x01)
					{
						int ret_val = check_Battery_allow_charge();
						
						if(ret_val == 1)
						{

							if(bq25703_enable_charge() == 0)
							{

							}else{
								syslog(LOG_ERR, "USB　charge configuration Error.");
							}
						}
						else if(ret_val == 0)
						{
							syslog(LOG_DEBUG, "charge not allowed.");
						}
					}
												
		}else if(event.compare("trigger::GPIO33falling") == 0){
				//clear all interrupts
		/*		char buf[11];
				if(0 == tps65987_get_Intevents(buf))
				{
					s_TPS_status tpStatus;

					if((buf[4] & 0x04) && !tps65987_get_Status(&tpStatus)){
						if(0 == tpStatus.VbusStatus)
							//USB disconnected
							 if(batteryManagePara.charger_is_plug_in & 0x01){
									batteryManagePara.charger_is_plug_in &= ~0x01;
							 }else{
							 }
						}else{
				    		syslog(LOG_DEBUG, "Error get pdstatus");
						}
					usleep(100000);	//give PD time to send commands.
					if(0 == tps65987_clear_Intevents())
					{
					}else{
						syslog(LOG_DEBUG, "Error clear intstatus");
					}
				}else
				{
					syslog(LOG_DEBUG, "Error get intstatus");
				}*/
				
				//batteryManagePara.charger_is_plug_in &= ~0x01;
				//otg configuration
			
				/*if(bq25703a_otg_function_init()){
					syslog(LOG_ERR, "OTG configuration Error.");
				}*/

			batteryManagePara.charger_is_plug_in &= ~0x01;	

		}else if(
			(event.compare("trigger::USB_CONNECTED") == 0 && (batteryManagePara.charger_is_plug_in & 0x02))||
			(event.compare("trigger::GPIO33rising")== 0 && ((batteryManagePara.charger_is_plug_in & 0x02) == 0))
			)
		{

				std::string line;
				std::ifstream infile("/sys/class/gpio/gpio115/value");
			
				std::getline( infile, line );
				size_t pg_value = 0;
				std::stoi(line, &pg_value);
				syslog(LOG_DEBUG, "pg value is %d", pg_value);

				if(pg_value == 1)
				{
					batteryManagePara.charger_is_plug_in &= ~0x02; 
				}

				batteryManagePara.charger_is_plug_in |= 0x01; 
					
				if(batteryManagePara.charger_is_plug_in == 0x01){
					int ret_val = check_Battery_allow_charge();

		            if(ret_val == 1)
		            {
		                if(bq25703_enable_charge())
		                {
							syslog(LOG_DEBUG, "USB CHARGE configuration error");	

		                }
		            }
		            else if(ret_val == 0)
		            {

		            }					
			    }

			//to add charger configration for USB
		}else if(event.compare("Test::bqdrv_ic_silent_toggle") == 0){
			batteryManagePara.i2c_silent ^= 1;
			syslog(LOG_DEBUG, "i2c silent value is %d", batteryManagePara.i2c_silent);
		}else if(event.find("setval::FaultVal:") != string::npos){
			OVERHEAT2 = std::stoi(event.substr(17,3));
			OVERHEAT1 = std::stoi(event.substr(21,3));
			OVERCOOL2 = std::stoi(event.substr(25,3));
			OVERCOOL1 = std::stoi(event.substr(29,3));
			printf("fault thresh changed to %d, %d, %d, %d\n", OVERHEAT2, OVERHEAT1,OVERCOOL2, OVERCOOL1);

		}else if(event.find("getval::battery") != string::npos){
        std::string payload;
        std::stringstream ps;

        ps << "adk-message-send 'system_power_data {battery:\"" <<  batteryManagePara.battery_current << ":" <<  batteryManagePara.battery_voltage
        << ":" << batteryManagePara.battery_temperature << ":" <<  battery_relativeStateOfCharge << "\"}'";

        std::string s = ps.str();
        char payloadstr[strlen(s.c_str()) + 1];
        strcpy(payloadstr,s.c_str());
        
        system(payloadstr);

    	}else{
			printf("event not identified.\n");
		}

		led_battery_display_handle();
    }
}


//Not used
void *bq25703a_chgok_irq_thread(void *arg)
{
    int ret;
    unsigned char j = 0;

    fds_chg_ok_pin[0].fd = fd_chg_ok_pin;
    fds_chg_ok_pin[0].events = POLLPRI;

    while(1)
    {
        /*
        * When VBUS rises above 3.5V or
        * falls below 24.5V, CHRG_OK is HIGH after 50ms deglitch time. When VBUS is falls below
        * 3.2 V or rises above 26 V, CHRG_OK is LOW. When fault occurs, CHRG_OK is asserted
        * LOW.
        */

        //wait for CHRG_OK Pin to be RISING HIGH
        ret = poll(fds_chg_ok_pin, 1, -1);
        printf("poll rising return = %d\n",ret);

        if(ret > 0)
        {
            if(fds_chg_ok_pin[0].revents & POLLPRI)
            {
                printf("CHRG_OK Edge detect, count = %d\n", j++);

                usleep(50*1000); //wait for a while

                unsigned char pin_value = get_Chg_OK_Pin_value();

                //AC unplug
                if(pin_value == '0')
                {
                    batteryManagePara.charger_is_plug_in = 0;
                    batteryManagePara.battery_is_charging = 0;
                    batteryManagePara.charge_level = 0;
                }
                else if(pin_value == '1')
                {
                    sleep(1); //wait for status to be stable and tps check, no more shorter

                    //reset the params when AC plug in
                    batteryManagePara_clear();

                    int ret_val;

                    int tps_err_cnt = 0;
                    int err_cnt = 0;

                    while(get_Chg_OK_Pin_value() == '1')
                    {
                        if(check_TypeC_current_type() == -1)
                        {
                            if(tps_err_cnt++ > 3)
                            {
                                break;
                            }

                            usleep(10*1000);
                            continue;
                        }

                        tps65987_get_ActiveContractPDO();

                        ret_val = check_Battery_allow_charge();

                        if(ret_val == 1)
                        {
                            if(bq25703_enable_charge() == 0)
                            {
                                break;
                            }
                        }
                        else if(ret_val == 0)
                        {
                            break;
                        }

                        if(err_cnt++ > 3)
                        {
                            break;
                        }

                        usleep(10*1000);
                    }

                }

                led_battery_display_handle();
            }
        }
    }
}



int main(int argc, char* argv[])
{
    int i;
    int err_cnt = 0;

    unsigned int VBUS_vol;
    unsigned int PSYS_vol;

    unsigned int charge_current_set;

    int tps65987_port_role;
    int tps65987_TypeC_current_type;

    pthread_t thread_check_chgok_ntid;
    pthread_t thread_check_stdin_ntid;
    pthread_t thread_check_gpiokey_ntid;

    pthread_t thread_check_batteryShutdownMode_ntid;
	printf("Version:developing 002\n");
    if(argc > 1)
    {
        for(i = 0; i < argc; i++)
        {
            printf("Argument %d is %s\n", i, argv[i]);
        }

        if(strcmp(argv[1],"log_batt_temp") == 0)
        {
            if(create_batteryTemperture_logFile() != 0)
            {
                return -1;
            }
        }
    }

	 openlog("slog", LOG_PID|LOG_CONS, LOG_USER);


    batteryManagePara_init();

    if(i2c_open_bq25703() != 0)
    {
        printf("i2c can't open bq25703!\n");
        return -1;
    }

/*
    while(bq25703a_charge_function_init() != 0)
    {
        if(err_cnt++ > 3)
        {
            return -1;
        }

        usleep(100*1000);
    }
*/

    if(i2c_open_tps65987() != 0)
    {
        printf("i2c can't open tps65987!\n");
        return -1;
    }

    if(i2c_open_fuelgauge() != 0)
    {
        printf("i2c can't open fuelgauge!\n");
        return -1;
    }

/*
    if(init_Chg_OK_Pin() != 0)
    {
        printf("init Chg_OK_Pin fail!\n");
        return -1;
    }
*/
    led_battery_display_init();

   // ryder: r1 is otg, tbd
   // pthread_create(&thread_check_chgok_ntid, NULL, bq25703a_chgok_irq_thread, NULL);

    pthread_create(&thread_check_stdin_ntid, NULL, bq25703a_stdin_thread, NULL);
    pthread_create(&thread_check_gpiokey_ntid, NULL, check_gpiokey_thread, NULL);

    pthread_create(&thread_check_batteryShutdownMode_ntid, NULL, check_batteryShutdownMode_thread, NULL);

    while(1)
    {
    	if(!batteryManagePara.i2c_silent){
	        bq25703a_get_ChargeOption0_Setting();
	        bq25703a_get_PSYS_and_VBUS(&PSYS_vol, &VBUS_vol);
	        charge_current_set = bq25703a_get_ChargeCurrentSetting();
			
	        check_BatteryFullyCharged_Task();

	        batteryTemperature_handle_Task();

	        led_battery_display_handle();
    	}
        printf("\n\n\n");

        sleep(5);
    }
	closelog();

    return 0;
}


