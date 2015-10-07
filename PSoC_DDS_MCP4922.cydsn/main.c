/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * 2015.10.06 2ch DACに対応(2ch同時出力は不可）
 * 2015.10.06 TC_Sampling_TimerのOVをPINに出力
 * 2015.10.03 MCP4922に12bitデータを送信
 * ========================================
*/
#include <project.h>
#include <math.h>
#include "wavetable.h"

#define SAMPLE_CLOCK (48000.0f)
#define WAVE_FREQUENCY (1.0f)

// ERROR CODE
#define ERR_DAC_CHANNEL_OUT_OF_RANGE 0x01

volatile uint32 phaseRegister;
volatile uint32 tuningWord;

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
	// Caluclate Wave Value
    //
	phaseRegister += tuningWord;

	// 32bitのphaseRegisterをテーブルの10bit(1024個)に丸める
	uint32 index = phaseRegister >> 22;
    uint16 waveValue = waveTableSine[index];
	
	DACSetVoltage(waveValue, 0);
    DACSetVoltage(waveValue, 1);
    
    TC_Sampling_Timer_ClearInterrupt(TC_Sampling_Timer_INTR_MASK_TC);
}

int main()
{    
    // 変数の初期化
	//double waveFrequency = (BPM * 4 / 60.0f);
    double waveFrequency = WAVE_FREQUENCY;
	tuningWord = waveFrequency * pow(2.0, 32) / SAMPLE_CLOCK;
    phaseRegister = 0;
       
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
