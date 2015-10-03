/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * 2015.10.03 MCP4922に12bitデータを送信
 * ========================================
*/
#include <project.h>
#include <math.h>
#include "wavetable.h"

#define SAMPLE_CLOCK (48000.0f)

volatile uint32 phaseRegister;
volatile uint32 tuningWord;

void DACSetVoltage(uint16 value)
{
	LDAC_PIN_Write(1u);
	SPIM_SpiUartWriteTxData((value >> 8) | 0x30); // Highバイト(0x30=OUTA/BUFなし/1x/シャットダウンなし)
	SPIM_SpiUartWriteTxData(value & 0xff);
			
	while(0u == (SPIM_GetMasterInterruptSource() & SPIM_INTR_MASTER_SPI_DONE))
	{
		/* Wait while Master completes transfer */
	}
	
    LDAC_PIN_Write(0u);
    
	/* Clear interrupt source after transfer completion */
	SPIM_ClearMasterInterruptSource(SPIM_INTR_MASTER_SPI_DONE);
}

CY_ISR(InterruptHandler)
{
	// Caluclate Wave Value
	phaseRegister += tuningWord;

	// 32bitのphaseRegisterをテーブルの10bit(1024個)に丸める
	uint32 index = phaseRegister >> 22;
    uint16 waveValue = waveTableSine[index];
	
	DACSetVoltage(waveValue);
    
    Sampling_Timer_ClearInterrupt(Sampling_Timer_INTR_MASK_TC);
}

int main()
{
    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    
    // 変数の初期化
	double waveFrequency = 1000.0f;
	tuningWord = waveFrequency * pow(2.0, 32) / SAMPLE_CLOCK;
    phaseRegister = 0;
    
    // コンポーネントの初期化
    Sampling_Timer_Start(); 
    Timer_ISR_StartEx(InterruptHandler);
    SPIM_Start();
    
    CyGlobalIntEnable;
    
    for(;;)
    {
        /* Place your application code here. */
    }
}

/* [] END OF FILE */
