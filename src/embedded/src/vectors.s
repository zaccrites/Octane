
.cpu cortex-m4
.thumb
.syntax unified


.section .text.Default_Handler,"ax"
.weak Default_Handler
.thumb_func
Default_Handler:
    b .


.section .text.Reset_Handler
.weak Reset_Handler
.thumb_func
Reset_Handler:
    ldr sp, =_stack_start
    bl _start
    b .


// https://blog.frankvh.com/2011/12/07/cortex-m3-m4-hard-fault-handler/
.extern Real_HardFault_Handler
.section .text.HardFault_Handler
.global HardFault_Handler
.thumb_func
HardFault_Handler:
    tst lr, #4
    ite eq
    mrseq r0, msp
    mrsne r0, psp
    b Real_HardFault_Handler


.section .isr_vector_table,"a"

    .word _stack_start
    .word  Reset_Handler
    .word  NMI_Handler
    .word  HardFault_Handler
    .word  MemManage_Handler
    .word  BusFault_Handler
    .word  UsageFault_Handler
    .word  0
    .word  0
    .word  0
    .word  0
    .word  SVC_Handler
    .word  DebugMon_Handler
    .word  0
    .word  PendSV_Handler
    .word  SysTick_Handler

    /* External Interrupts */
    .word     WWDG_IRQHandler                   /* Window WatchDog                             */
    .word     PVD_IRQHandler                    /* PVD through EXTI Line detection             */
    .word     TAMP_STAMP_IRQHandler             /* Tamper and TimeStamps through the EXTI line */
    .word     RTC_WKUP_IRQHandler               /* RTC Wakeup through the EXTI line            */
    .word     FLASH_IRQHandler                  /* FLASH                                       */
    .word     RCC_IRQHandler                    /* RCC                                         */
    .word     EXTI0_IRQHandler                  /* EXTI Line0                                  */
    .word     EXTI1_IRQHandler                  /* EXTI Line1                                  */
    .word     EXTI2_IRQHandler                  /* EXTI Line2                                  */
    .word     EXTI3_IRQHandler                  /* EXTI Line3                                  */
    .word     EXTI4_IRQHandler                  /* EXTI Line4                                  */
    .word     DMA1_Stream0_IRQHandler           /* DMA1 Stream 0                               */
    .word     DMA1_Stream1_IRQHandler           /* DMA1 Stream 1                               */
    .word     DMA1_Stream2_IRQHandler           /* DMA1 Stream 2                               */
    .word     DMA1_Stream3_IRQHandler           /* DMA1 Stream 3                               */
    .word     DMA1_Stream4_IRQHandler           /* DMA1 Stream 4                               */
    .word     DMA1_Stream5_IRQHandler           /* DMA1 Stream 5                               */
    .word     DMA1_Stream6_IRQHandler           /* DMA1 Stream 6                               */
    .word     ADC_IRQHandler                    /* ADC1, ADC2 and ADC3s                        */
    .word     0                                 /* Reserved                                    */
    .word     0                                 /* Reserved                                    */
    .word     0                                 /* Reserved                                    */
    .word     0                                 /* Reserved                                    */
    .word     EXTI9_5_IRQHandler                /* External Line[9:5]s                         */
    .word     TIM1_BRK_TIM9_IRQHandler          /* TIM1 Break and TIM9                         */
    .word     TIM1_UP_IRQHandler                /* TIM1 Update                                 */
    .word     TIM1_TRG_COM_TIM11_IRQHandler     /* TIM1 Trigger and Commutation and TIM11      */
    .word     TIM1_CC_IRQHandler                /* TIM1 Capture Compare                        */
    .word     0                                 /* Reserved                                    */
    .word     0                                 /* Reserved                                    */
    .word     0                                 /* Reserved                                    */
    .word     I2C1_EV_IRQHandler                /* I2C1 Event                                  */
    .word     I2C1_ER_IRQHandler                /* I2C1 Error                                  */
    .word     I2C2_EV_IRQHandler                /* I2C2 Event                                  */
    .word     I2C2_ER_IRQHandler                /* I2C2 Error                                  */
    .word     SPI1_IRQHandler                   /* SPI1                                        */
    .word     SPI2_IRQHandler                   /* SPI2                                        */
    .word     USART1_IRQHandler                 /* USART1                                      */
    .word     USART2_IRQHandler                 /* USART2                                      */
    .word     0                                 /* Reserved                                    */
    .word     EXTI15_10_IRQHandler              /* External Line[15:10]s                       */
    .word     RTC_Alarm_IRQHandler              /* RTC Alarm (A and B) through EXTI Line       */
    .word     0                                 /* Reserved                                    */
    .word     0                                 /* Reserved                                    */
    .word     0                                 /* Reserved                                    */
    .word     0                                 /* Reserved                                    */
    .word     0                                 /* Reserved                                    */
    .word     DMA1_Stream7_IRQHandler           /* DMA1 Stream7                                */
    .word     0                                 /* Reserved                                    */
    .word     0                                 /* Reserved                                    */
    .word     TIM5_IRQHandler                   /* TIM5                                        */
    .word     0                                 /* Reserved                                    */
    .word     0                                 /* Reserved                                    */
    .word     0                                 /* Reserved                                    */
    .word     TIM6_DAC_IRQHandler               /* TIM6 and DAC                                */
    .word     0                                 /* Reserved                                    */
    .word     DMA2_Stream0_IRQHandler           /* DMA2 Stream 0                               */
    .word     DMA2_Stream1_IRQHandler           /* DMA2 Stream 1                               */
    .word     DMA2_Stream2_IRQHandler           /* DMA2 Stream 2                               */
    .word     DMA2_Stream3_IRQHandler           /* DMA2 Stream 3                               */
    .word     DMA2_Stream4_IRQHandler           /* DMA2 Stream 4                               */
    .word     0                                 /* Reserved                                    */
    .word     0                                 /* Reserved                                    */
    .word     0                                 /* Reserved                                    */
    .word     0                                 /* Reserved                                    */
    .word     0                                 /* Reserved                                    */
    .word     0                                 /* Reserved                                    */
    .word     0                                 /* Reserved                                    */
    .word     DMA2_Stream5_IRQHandler           /* DMA2 Stream 5                               */
    .word     DMA2_Stream6_IRQHandler           /* DMA2 Stream 6                               */
    .word     DMA2_Stream7_IRQHandler           /* DMA2 Stream 7                               */
    .word     USART6_IRQHandler                 /* USART6                                      */
    .word     0                                 /* Reserved                                    */
    .word     0                                 /* Reserved                                    */
    .word     0                                 /* Reserved                                    */
    .word     0                                 /* Reserved                                    */
    .word     0                                 /* Reserved                                    */
    .word     0                                 /* Reserved                                    */
    .word     0                                 /* Reserved                                    */
    .word     0                                 /* Reserved                                    */
    .word     RNG_IRQHandler                    /* RNG                                         */
    .word     FPU_IRQHandler                    /* FPU                                         */
    .word     0                                 /* Reserved                                    */
    .word     0                                 /* Reserved                                    */
    .word     0                                 /* Reserved                                    */
    .word     SPI5_IRQHandler                   /* SPI5                                        */
    .word     0                                 /* Reserved                                    */
    .word     0                                 /* Reserved                                    */
    .word     0                                 /* Reserved                                    */
    .word     0                                 /* Reserved                                    */
    .word     0                                 /* Reserved                                    */
    .word     0                                 /* Reserved                                    */
    .word     0                                 /* Reserved                                    */
    .word     0                                 /* Reserved                                    */
    .word     0                                 /* Reserved                                    */
    .word     FMPI2C1_EV_IRQHandler             /* FMPI2C1 Event                               */
    .word     FMPI2C1_ER_IRQHandler             /* FMPI2C1 Error                               */
    .word     LPTIM1_IRQHandler                 /* LP TIM1                                     */


// If there is no explcit handler function defined for an interrupt,
// fall back to the default implementation.

    .weak      NMI_Handler
    .thumb_set NMI_Handler,Default_Handler

    // HardFault_Handler omitted intentionally

    .weak      MemManage_Handler
    .thumb_set MemManage_Handler,Default_Handler

    .weak      BusFault_Handler
    .thumb_set BusFault_Handler,Default_Handler

    .weak      UsageFault_Handler
    .thumb_set UsageFault_Handler,Default_Handler

    .weak      SVC_Handler
    .thumb_set SVC_Handler,Default_Handler

    .weak      DebugMon_Handler
    .thumb_set DebugMon_Handler,Default_Handler

    .weak      PendSV_Handler
    .thumb_set PendSV_Handler,Default_Handler

    .weak      SysTick_Handler
    .thumb_set SysTick_Handler,Default_Handler

    .weak      WWDG_IRQHandler
    .thumb_set WWDG_IRQHandler,Default_Handler

    .weak      PVD_IRQHandler
    .thumb_set PVD_IRQHandler,Default_Handler

    .weak      TAMP_STAMP_IRQHandler
    .thumb_set TAMP_STAMP_IRQHandler,Default_Handler

    .weak      RTC_WKUP_IRQHandler
    .thumb_set RTC_WKUP_IRQHandler,Default_Handler

    .weak      FLASH_IRQHandler
    .thumb_set FLASH_IRQHandler,Default_Handler

    .weak      RCC_IRQHandler
    .thumb_set RCC_IRQHandler,Default_Handler

    .weak      EXTI0_IRQHandler
    .thumb_set EXTI0_IRQHandler,Default_Handler

    .weak      EXTI1_IRQHandler
    .thumb_set EXTI1_IRQHandler,Default_Handler

    .weak      EXTI2_IRQHandler
    .thumb_set EXTI2_IRQHandler,Default_Handler

    .weak      EXTI3_IRQHandler
    .thumb_set EXTI3_IRQHandler,Default_Handler

    .weak      EXTI4_IRQHandler
    .thumb_set EXTI4_IRQHandler,Default_Handler

    .weak      DMA1_Stream0_IRQHandler
    .thumb_set DMA1_Stream0_IRQHandler,Default_Handler

    .weak      DMA1_Stream1_IRQHandler
    .thumb_set DMA1_Stream1_IRQHandler,Default_Handler

    .weak      DMA1_Stream2_IRQHandler
    .thumb_set DMA1_Stream2_IRQHandler,Default_Handler

    .weak      DMA1_Stream3_IRQHandler
    .thumb_set DMA1_Stream3_IRQHandler,Default_Handler

    .weak      DMA1_Stream4_IRQHandler
    .thumb_set DMA1_Stream4_IRQHandler,Default_Handler

    .weak      DMA1_Stream5_IRQHandler
    .thumb_set DMA1_Stream5_IRQHandler,Default_Handler

    .weak      DMA1_Stream6_IRQHandler
    .thumb_set DMA1_Stream6_IRQHandler,Default_Handler

    .weak      ADC_IRQHandler
    .thumb_set ADC_IRQHandler,Default_Handler

    .weak      EXTI9_5_IRQHandler
    .thumb_set EXTI9_5_IRQHandler,Default_Handler

    .weak      TIM1_BRK_TIM9_IRQHandler
    .thumb_set TIM1_BRK_TIM9_IRQHandler,Default_Handler

    .weak      TIM1_UP_IRQHandler
    .thumb_set TIM1_UP_IRQHandler,Default_Handler

    .weak      TIM1_TRG_COM_TIM11_IRQHandler
    .thumb_set TIM1_TRG_COM_TIM11_IRQHandler,Default_Handler

    .weak      TIM1_CC_IRQHandler
    .thumb_set TIM1_CC_IRQHandler,Default_Handler

    .weak      I2C1_EV_IRQHandler
    .thumb_set I2C1_EV_IRQHandler,Default_Handler

    .weak      I2C1_ER_IRQHandler
    .thumb_set I2C1_ER_IRQHandler,Default_Handler

    .weak      I2C2_EV_IRQHandler
    .thumb_set I2C2_EV_IRQHandler,Default_Handler

    .weak      I2C2_ER_IRQHandler
    .thumb_set I2C2_ER_IRQHandler,Default_Handler

    .weak      SPI1_IRQHandler
    .thumb_set SPI1_IRQHandler,Default_Handler

    .weak      SPI2_IRQHandler
    .thumb_set SPI2_IRQHandler,Default_Handler

    .weak      USART1_IRQHandler
    .thumb_set USART1_IRQHandler,Default_Handler

    .weak      USART2_IRQHandler
    .thumb_set USART2_IRQHandler,Default_Handler

    .weak      EXTI15_10_IRQHandler
    .thumb_set EXTI15_10_IRQHandler,Default_Handler

    .weak      RTC_Alarm_IRQHandler
    .thumb_set RTC_Alarm_IRQHandler,Default_Handler

    .weak      DMA1_Stream7_IRQHandler
    .thumb_set DMA1_Stream7_IRQHandler,Default_Handler

    .weak      TIM5_IRQHandler
    .thumb_set TIM5_IRQHandler,Default_Handler

    .weak      TIM6_DAC_IRQHandler
    .thumb_set TIM6_DAC_IRQHandler,Default_Handler

    .weak      DMA2_Stream0_IRQHandler
    .thumb_set DMA2_Stream0_IRQHandler,Default_Handler

    .weak      DMA2_Stream1_IRQHandler
    .thumb_set DMA2_Stream1_IRQHandler,Default_Handler

    .weak      DMA2_Stream2_IRQHandler
    .thumb_set DMA2_Stream2_IRQHandler,Default_Handler

    .weak      DMA2_Stream3_IRQHandler
    .thumb_set DMA2_Stream3_IRQHandler,Default_Handler

    .weak      DMA2_Stream4_IRQHandler
    .thumb_set DMA2_Stream4_IRQHandler,Default_Handler

    .weak      DMA2_Stream5_IRQHandler
    .thumb_set DMA2_Stream5_IRQHandler,Default_Handler

    .weak      DMA2_Stream6_IRQHandler
    .thumb_set DMA2_Stream6_IRQHandler,Default_Handler

    .weak      DMA2_Stream7_IRQHandler
    .thumb_set DMA2_Stream7_IRQHandler,Default_Handler

    .weak      USART6_IRQHandler
    .thumb_set USART6_IRQHandler,Default_Handler

    .weak      RNG_IRQHandler
    .thumb_set RNG_IRQHandler,Default_Handler

    .weak      FPU_IRQHandler
    .thumb_set FPU_IRQHandler,Default_Handler

    .weak      SPI5_IRQHandler
    .thumb_set SPI5_IRQHandler,Default_Handler

    .weak     FMPI2C1_EV_IRQHandler
    .thumb_set FMPI2C1_EV_IRQHandler,Default_Handler

    .weak      FMPI2C1_ER_IRQHandler
    .thumb_set FMPI2C1_ER_IRQHandler,Default_Handler

    .weak      LPTIM1_IRQHandler
    .thumb_set LPTIM1_IRQHandler,Default_Handler
