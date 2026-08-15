/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2023/12/25
 * Description        : Main program body.
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*
 * HyperFan controller note:
 *  - ADC A3(PD2) reads the speed command.
 *  - TIM1_CH1 is remapped to PC6 for the 4-pin fan PWM input.
 *  - PA1 drives the onboard LED. The system clock uses HSI so PA1 is free.
 */

#include "debug.h"
#include <stdlib.h>


/* Global define */
#define ADC_MAX_VALUE   1023U
#define FAN_PWM_ARR     1919U
#define FAN_PWM_PSC     0U
#define FAN_PWM_DUTY_INITIAL ((FAN_PWM_ARR + 1U) / 2U)
#define ADC_CHANGE_THRESHOLD 5

void FanPwm_Init(u16 arr, u16 psc, u16 ccp){
    GPIO_InitTypeDef GPIO_InitStructure={0};
    TIM_OCInitTypeDef TIM_OCInitStructure={0};
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure={0};

    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOC | RCC_APB2Periph_TIM1 | RCC_APB2Periph_AFIO, ENABLE );
    GPIO_PinRemapConfig(GPIO_PartialRemap1_TIM1, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init( GPIOC, &GPIO_InitStructure );

    TIM_TimeBaseInitStructure.TIM_Period = arr;
    TIM_TimeBaseInitStructure.TIM_Prescaler = psc;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit( TIM1, &TIM_TimeBaseInitStructure);

    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
//    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; // debug: inverted PWM polarity

    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = ccp;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC1Init( TIM1, &TIM_OCInitStructure );

    TIM_CtrlPWMOutputs(TIM1, ENABLE );
    TIM_OC1PreloadConfig( TIM1, TIM_OCPreload_Enable );
    TIM_ARRPreloadConfig( TIM1, ENABLE );
    TIM_Cmd( TIM1, ENABLE );
}

static inline void pwm_set_duty(uint16_t duty){
    TIM_SetCompare1(TIM1, duty);
}

static inline uint16_t adc_to_pwm_compare(uint16_t adc){
    if(adc > ADC_MAX_VALUE){
        adc = ADC_MAX_VALUE;
    }

    return (uint16_t)((uint32_t)adc * FAN_PWM_ARR / ADC_MAX_VALUE);
}

void AdcA3_Init(void){
    ADC_InitTypeDef  ADC_InitStructure = {0};
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_ADCCLKConfig(RCC_PCLK2_Div8);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    ADC_DeInit(ADC1);
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);

    ADC_Calibration_Vol(ADC1, ADC_CALVOL_50PERCENT);
    ADC_Cmd(ADC1, ENABLE);
}

u16 Adc_Read(u8 ch){
    u16 val;

    ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_241Cycles);
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);

    while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));
    val = ADC_GetConversionValue(ADC1);

    return val;
}
void LED_Init(void){
    GPIO_InitTypeDef GPIO_InitStructure = {0}; //GPIO Set Structure
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_WriteBit(GPIOA, GPIO_Pin_1, Bit_RESET);
}

int main(void){
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    Delay_Init();
#if (SDI_PRINT == SDI_PR_OPEN)
    SDI_Printf_Enable();
#endif
//    printf("SystemClk:%d\r\n",SystemCoreClock);
//    printf( "ChipID:%08x\r\n", DBGMCU_GetCHIPID() );
//    printf( "test Test 1233\r\n");

    LED_Init();
    AdcA3_Init();

    //SANYO FAN用
    const uint16_t ARR = FAN_PWM_ARR;
    const uint16_t PSC = FAN_PWM_PSC;       // 25kHz
    const uint16_t DUTY0 = FAN_PWM_DUTY_INITIAL; // 初期 50 %
    FanPwm_Init(ARR, PSC, DUTY0);



    while(1){
//        GPIO_WriteBit(GPIOA, GPIO_Pin_1, Bit_RESET);
//        Delay_Ms(1000);
//        GPIO_WriteBit(GPIOA, GPIO_Pin_1, Bit_SET);
//        Delay_Ms(1000);
        static int ad3_prev = -1024;
        int ad3 = Adc_Read(ADC_Channel_3);
        if(abs(ad3_prev - ad3) > ADC_CHANGE_THRESHOLD){
            ad3_prev = ad3;


//SANYO FAN用
//        pwm_set_duty(0);                  // debug: PWM 0%, PC6 Low
//        pwm_set_duty((FAN_PWM_ARR + 1U) / 2U); // debug: PWM 50%
//        pwm_set_duty(FAN_PWM_ARR);        // debug: PWM nearly 100%
        pwm_set_duty(adc_to_pwm_compare((uint16_t)ad3));
        }

//        printf("ADC3:%d\r\n", Adc_Read(ADC_Channel_3));

    }
}
