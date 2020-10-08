/******************************************************************************
 * Copyright (C) 2016, Huada Semiconductor Co.,Ltd. All rights reserved.
 *
 * This software is owned and published by:
 * Huada Semiconductor Co.,Ltd ("HDSC").
 *
 * BY DOWNLOADING, INSTALLING OR USING THIS SOFTWARE, YOU AGREE TO BE BOUND
 * BY ALL THE TERMS AND CONDITIONS OF THIS AGREEMENT.
 *
 * This software contains source code for use with HDSC
 * components. This software is licensed by HDSC to be adapted only
 * for use in systems utilizing HDSC components. HDSC shall not be
 * responsible for misuse or illegal use of this software for devices not
 * supported herein. HDSC is providing this software "AS IS" and will
 * not be responsible for issues arising from incorrect user implementation
 * of the software.
 *
 * Disclaimer:
 * HDSC MAKES NO WARRANTY, EXPRESS OR IMPLIED, ARISING BY LAW OR OTHERWISE,
 * REGARDING THE SOFTWARE (INCLUDING ANY ACCOMPANYING WRITTEN MATERIALS),
 * ITS PERFORMANCE OR SUITABILITY FOR YOUR INTENDED USE, INCLUDING,
 * WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, THE IMPLIED
 * WARRANTY OF FITNESS FOR A PARTICULAR PURPOSE OR USE, AND THE IMPLIED
 * WARRANTY OF NONINFRINGEMENT.
 * HDSC SHALL HAVE NO LIABILITY (WHETHER IN CONTRACT, WARRANTY, TORT,
 * NEGLIGENCE OR OTHERWISE) FOR ANY DAMAGES WHATSOEVER (INCLUDING, WITHOUT
 * LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION,
 * LOSS OF BUSINESS INFORMATION, OR OTHER PECUNIARY LOSS) ARISING FROM USE OR
 * INABILITY TO USE THE SOFTWARE, INCLUDING, WITHOUT LIMITATION, ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL OR CONSEQUENTIAL DAMAGES OR LOSS OF DATA,
 * SAVINGS OR PROFITS,
 * EVEN IF Disclaimer HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 * YOU ASSUME ALL RESPONSIBILITIES FOR SELECTION OF THE SOFTWARE TO ACHIEVE YOUR
 * INTENDED RESULTS, AND FOR THE INSTALLATION OF, USE OF, AND RESULTS OBTAINED
 * FROM, THE SOFTWARE.
 *
 * This software may be replicated in part or whole for the licensed use,
 * with the restriction that this Disclaimer and Copyright notice must be
 * included with each copy of this software, whether used in part or whole,
 * at all times.
 */
/******************************************************************************/
/** \file main.c
 **
 ** \brief USB mouse example.
 **
 **   - 2018-12-25  1.0  Wangmin First version for USB mouse demo.
 **
 ******************************************************************************/

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "hc32_ddl.h"
#include "usbd_usr.h"
#include "usbd_desc.h"
#include "usb_bsp.h"
#include "misc.h"

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
USB_OTG_CORE_HANDLE  USB_OTG_dev;

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

/**
 ******************************************************************************
 ** \brief  Initialize the system clock for the sample
 **
 ** \param  None
 **
 ** \return None
 ******************************************************************************/
static void SysClkIni(void)
{
    en_clk_sys_source_t     enSysClkSrc;
    stc_clk_sysclk_cfg_t    stcSysClkCfg;
    stc_clk_xtal_cfg_t      stcXtalCfg;
    stc_clk_mpll_cfg_t      stcMpllCfg;
//    stc_clk_output_cfg_t    stcOutputClkCfg;
//    stc_clk_upll_cfg_t      stcUpllCfg;
    uint16_t timeout = 0u;
    en_flag_status_t status;    

    MEM_ZERO_STRUCT(enSysClkSrc);
    MEM_ZERO_STRUCT(stcSysClkCfg);
    MEM_ZERO_STRUCT(stcXtalCfg);
    MEM_ZERO_STRUCT(stcMpllCfg);
    /* Unlock CLK registers */
    M4_SYSREG->PWR_FPRC |= 0xa501u;
    
    /* Set bus clk div. */
//    stcSysClkCfg.enHclkDiv = ClkSysclkDiv1;
//    stcSysClkCfg.enExclkDiv = ClkSysclkDiv2;
//    stcSysClkCfg.enPclk0Div = ClkSysclkDiv1;
//    stcSysClkCfg.enPclk1Div = ClkSysclkDiv2;
//    stcSysClkCfg.enPclk2Div = ClkSysclkDiv4;
//    stcSysClkCfg.enPclk3Div = ClkSysclkDiv4;
//    stcSysClkCfg.enPclk4Div = ClkSysclkDiv2;
//    CLK_SysClkConfig(&stcSysClkCfg);
    M4_SYSREG->CMU_SCFGR = (0UL << 24U) | (1UL << 20U) | (1UL << 16U) | \
                           (2UL << 12U) | (2UL << 8U)  | (1UL << 4U)  | \
                           (0UL << 0U);

    /* Switch system clock source to MPLL. */
    /* Use Xtal as MPLL source. */
    stcXtalCfg.enMode = ClkXtalModeOsc;
    stcXtalCfg.enDrv = ClkXtalLowDrv;
    stcXtalCfg.enFastStartup = Enable;
    CLK_XtalConfig(&stcXtalCfg);
    CLK_XtalCmd(Enable);

    /* MPLL config. */
    stcMpllCfg.pllmDiv = 2u;
    stcMpllCfg.plln = 48u;
    stcMpllCfg.PllpDiv = 4u;    //MPLLP = 96
    stcMpllCfg.PllqDiv = 8u;
    stcMpllCfg.PllrDiv = 8u;
//    CLK_SetPllSource(ClkPllSrcXTAL);
    M4_SYSREG->CMU_PLLCFGR_f.PLLSRC = ClkPllSrcXTAL;
    CLK_MpllConfig(&stcMpllCfg);

    /* flash read wait cycle setting */
    EFM_Unlock();
    EFM_SetLatency(EFM_LATENCY_2);
    EFM_Lock();

    /* Enable MPLL. */
//    CLK_MpllCmd(Enable);
    M4_SYSREG->CMU_PLLCR_f.MPLLOFF = 0u;
    do
    {
        status = CLK_GetFlagStatus(ClkFlagMPLLRdy);
        timeout++;
    }while((timeout < 0x1000u) && (status != Set));
    /* Wait MPLL ready. */
    while(Set != CLK_GetFlagStatus(ClkFlagMPLLRdy))
    {
        ;
    }
    /* Switch system clock source to MPLL. */
    CLK_SetSysClkSource(CLKSysSrcMPLL);

    /* Set USB clock source */
//    CLK_SetUsbClkSource(ClkUsbSrcMpllq);

    M4_SYSREG->CMU_UFSCKCFGR_f.USBCKS = ClkUsbSrcMpllq;
    /* Lock CLK registers */
    M4_SYSREG->PWR_FPRC = (0xa500u | (M4_SYSREG->PWR_FPRC & (uint16_t)(~1u)));
#if 0
    /* Clk output.*/
    stcOutputClkCfg.enOutputSrc = ClkOutputSrcMpllp;
    stcOutputClkCfg.enOutputDiv = ClkOutputDiv8;
    CLK_OutputClkConfig(ClkOutputCh1,&stcOutputClkCfg);
    CLK_OutputClkCmd(ClkOutputCh1,Enable);

    PORT_SetFunc(PortA, Pin08, Func_Mclkout, Disable);
#endif
}

char rxBuffer[128]; // for test
__IO uint32_t test ;
/**
 *******************************************************************************
 ** \brief  main function for mouse function
 **
 ** \param [in]  None
 **
 ** \retval int32_t Return value, if needed
 **
 ******************************************************************************/
int32_t main (void)
{
    /* clock config */
    SysClkIni();
#ifdef UART_DEBUG_PRINTF
    Ddl_UartInit();
#endif
    MasterSpiInit();
    MasterSpiDmaInit();
    EcIntConfig();

    USBD_Init(&USB_OTG_dev,
#ifdef USE_USB_OTG_FS
              USB_OTG_FS_CORE_ID,
#else
              USB_OTG_HS_CORE_ID,
#endif
              &USR_desc,
              &USBD_CDC_cb,
              &USR_cb);

    while (1)
    {
        //SpiFlash_ReadData(0x00, (uint8_t*)&rxBuffer[0], 4);
        /* read ID cmd */
        SPI_NSS_LOW();
        SpiFlash_WriteReadByte(0xF0);
        SpiFlash_WriteReadByte((uint8_t)((0x00 & 0xFF00u) >> 8u));
        SpiFlash_WriteReadByte((uint8_t)(0x00 & 0xFFu));
        SPI_NSS_HIGH();
        Ddl_Delay1ms(100);
        SPI_NSS_LOW();
        SpiFlash_WriteReadByte(0xF1);
        /* read ID via DMA */
        HAL_SPI_Receive_DMA(M4_SPI3, (uint8_t*)&rxBuffer, 4);
        test = GetECStatus();
    }
}

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
