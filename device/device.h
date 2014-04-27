#ifndef STORAGE_H_
#define STORAGE_H_

//Read Only Fehlermeldungen / CoRE-Link- und SenML-Antworten
#define RES_B_ERR_05     0x1D000
#define LEN_B_ERR_05     73
#define RES_B_ERR_04     0x1D080
#define LEN_B_ERR_04     51
#define RES_B_ERR_03     0x1D100
#define LEN_B_ERR_03     53
#define RES_B_ERR_02     0x1D180
#define LEN_B_ERR_02     31
#define RES_B_ERR_01     0x1D200
#define LEN_B_ERR_01     61

#define RES_D_CORE       0x1D280
#define LEN_D_CORE       257
#define RES_SENML_LEDB   0x1D400
#define LEN_SENML_LEDB   42
#define RES_SENML_LEDD   0x1D440
#define LEN_SENML_LEDD   43
#define RES_SENML_TMP    0x1D480
#define LEN_SENML_TMP    48
#define RES_SENML_LUX	 0x1D520
#define LEN_SENML_LUX	 43
#define RES_SENML_HUM    0x1D560
#define LEN_SENML_HUM    46
#define RES_SENML_TMP_F  0x1D600
#define LEN_SENML_TMP_F  48
#define RES_SENML_VAL    0x1D640
#define LEN_SENML_VAL    44
#define RES_SENML_BUTTON 0x1D680
#define LEN_SENML_BUTTON 40

//Read Only Vars
#define RES_CONFIG       0x1E000
#define LEN_CONFIG       0x20
#define RES_UUID         0x1E020
#define LEN_UUID         0x10
#define RES_PSK          0x1E030
#define LEN_PSK          0x10
#define RES_ANSCHARS     0x1E040
#define LEN_ANSCHARS     0x40
#define RES_ECC_BASE_X   0x1E080
#define LEN_ECC_BASE_X   0x20
#define RES_ECC_BASE_Y   0x1E0A0
#define LEN_ECC_BASE_Y   0x20
#define RES_ECC_ORDER    0x1E0C0
#define LEN_ECC_ORDER    0x20
#define RES_NAME         0x1E0E0
#define LEN_NAME         0x0F
#define RES_MODEL        0x1E100
#define LEN_MODEL        0x0E
#define RES_FLASHTIME    0x1E120
#define LEN_FLASHTIME    0x04

#endif /* STORAGE_H_ */
