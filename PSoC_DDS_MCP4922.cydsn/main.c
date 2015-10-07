/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * 2015.10.07 2ch同時出力
 * 2015.10.06 2ch DACに対応(2ch同時出力は不可）
 * 2015.10.06 TC_Sampling_TimerのOVをPINに出力
 * 2015.10.03 MCP4922に12bitデータを送信
 * ========================================
*/
#include <project.h>
#include <math.h>
#include "wavetable.h"

#define SAMPLE_CLOCK (48000.0f)
#define WAVE_FREQUENCY_A (1000.0f)
#define WAVE_FREQUENCY_B (2000.0f)

// ERROR CODE
#define ERR_DAC_CHANNEL_OUT_OF_RANGE 0x01

volatile uint32 phaseRegisterA;
volatile uint32 phaseRegisterB;
volatile uint32 tuningWordA;
volatile uint32 tuningWordB;

void DACSetVoltage(uint16 value, int channel)
{
    uint16 txData;
    switch (channel) {
    case 0:
        // Highバイト(0x30=OUTA/BUFなし/1x/シャットダウンなし)
        txData = (value & ~0xF000) | 0x3000;
        break;
    case 1:
        // Highバイト(0xB0=OUTB/BUFなし/1x/シャットダウンなし)
        txData = (value & ~0xF000) | 0xB000;
        break;
    default:
        ;
        // error(ERR_DAC_CHANNEL_OUT_OF_RANGE);
    }
    
	Pin_LDAC_Write(1u);
    SPIM_DAC_SpiUartWriteTxData(txData);
			
	while(0u == (SPIM_DAC_GetMasterInterruptSource() & SPIM_DAC_INTR_MASTER_SPI_DONE))
	{
		/* Wait while Master completes transfer */
	}
	
    Pin_LDAC_Write(0u);
    
	/* Clear interrupt source after transfer completion */
	SPIM_DAC_ClearMasterInterruptSource(SPIM_DAC_INTR_MASTER_SPI_DONE);
}

CY_ISR(ISR_Sampling_Timer_Handler)
{
    uint32 index;
    
	// Caluclate Wave Value A
    //
	phaseRegisterA += tuningWordA;
    
	// 32bitのphaseRegisterをテーブルの10bit(1024個)に丸める
	index = phaseRegisterA >> 22;
    uint16 waveValueA = waveTableSine[index];

    // Caluclate Wave Value B
    //
    phaseRegisterB += tuningWordB;

    index = phaseRegisterB >> 22;
    uint16 waveValueB = waveTableSine[index];
	
    // Output
    //
	DACSetVoltage(waveValueA, 0);
    DACSetVoltage(waveValueB, 1);
    
    TC_Sampling_Timer_ClearInterrupt(TC_Sampling_Timer_INTR_MASK_TC);
}

int main()
{    
    // 変数の初期化
	tuningWordA = WAVE_FREQUENCY_A * pow(2.0, 32) / SAMPLE_CLOCK;
    phaseRegisterA = 0;
    
    tuningWordB = WAVE_FREQUENCY_B * pow(2.0, 32) / SAMPLE_CLOCK;
    phaseRegisterB = 0;
       
    // コンポーネントの初期化
    TC_Sampling_Timer_Start(); 
    ISR_Sampling_Timer_StartEx(ISR_Sampling_Timer_Handler);
    SPIM_DAC_Start();
    
    CyGlobalIntEnable;
    
    for(;;)
    {
        
    }
}

/* [] END OF FILE */
