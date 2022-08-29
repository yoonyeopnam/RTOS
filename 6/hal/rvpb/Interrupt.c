/*
 * Interrupt.c
 *
 *  Created on: Sep 21, 2018
 *      Author: maanu
 */

#include "stdint.h"
#include "memio.h"
#include "Interrupt.h"
#include "HalInterrupt.h"
#include "armcpu.h"

extern volatile GicCput_t* GicCpu;
extern volatile GicDist_t* GicDist;

static InterHdlr_fptr sHandlers[INTERRUPT_HANDLER_NUM];

void Hal_interrupt_init(void)
{
    GicCpu->cpucontrol.bits.Enable = 1;
    GicCpu->prioritymask.bits.Prioritymask = GIC_PRIORITY_MASK_NONE;
    GicDist->distributorctrl.bits.Enable = 1;

    for (uint32_t i = 0 ; i < INTERRUPT_HANDLER_NUM ; i++)
    {
        sHandlers[i] = NULL;
    }

    enable_irq();
}

void Hal_interrupt_enable(uint32_t interrupt_num)
{
    if ((interrupt_num < GIC_IRQ_START) || (GIC_IRQ_END < interrupt_num))
    {
        return;
    }

    uint32_t bit_num = interrupt_num - GIC_IRQ_START;

    if (bit_num < GIC_IRQ_START)
    {
        SET_BIT(GicDist->setenable1, bit_num);
    }
    else
    {
        bit_num -= GIC_IRQ_START;
        SET_BIT(GicDist->setenable2, bit_num);
    }
}

void Hal_interrupt_disable(uint32_t interrupt_num)
{
    if ((interrupt_num < GIC_IRQ_START) || (GIC_IRQ_END < interrupt_num))
    {
        return;
    }

    uint32_t bit_num = interrupt_num - GIC_IRQ_START;

    if (bit_num < GIC_IRQ_START)
    {
        CLR_BIT(GicDist->setenable1, bit_num);
    }
    else
    {
        bit_num -= GIC_IRQ_START;
        CLR_BIT(GicDist->setenable2, bit_num);
    }
}

void Hal_interrupt_register_handler(InterHdlr_fptr handler, uint32_t interrupt_num)
{
    sHandlers[interrupt_num] = handler; /* interrupt Handler 에 대한 처리 방법 */
}

void Hal_interrupt_run_handler(void)
{
    uint32_t interrupt_num = GicCpu->interruptack.bits.InterruptID; /*IAR 에서 읽어옴*/

    if (sHandlers[interrupt_num] != NULL)
    {
        sHandlers[interrupt_num](); /*함수 포인터 실행*/
    }

    GicCpu->endofinterrupt.bits.InterruptID = interrupt_num; /*EOIR*/
}
