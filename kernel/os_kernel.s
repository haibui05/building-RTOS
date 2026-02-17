				AREA |.text|,CODE,READONLY,ALIGN=2
				THUMB
				EXTERN currentPointer			
                EXTERN rtos_periodic_scheduler_round_robin_with_sleep				
                EXPORT PendSV_Handler
                EXPORT rtos_kernel_scheduler_launch

PendSV_Handler
			    CPSID I
			    PUSH {R4-R11}
			    LDR R0, =currentPointer
			    LDR R1, [R0]
			    STR SP, [R1]
			    PUSH {R0, LR}
			    BL rtos_periodic_scheduler_round_robin_with_sleep
			    POP {R0, LR}
			    LDR R1, [R0]
			    LDR SP, [R1]
			    STR R1, [R0]
			    LDR SP, [R1]
			    POP {R4-R11}
			    CPSIE I
			    BX LR

rtos_kernel_scheduler_launch
                LDR R0, =currentPointer
                LDR R2, [R0]
                LDR SP, [R2]
                POP {R4-R11}
                POP {R12}
                POP {R0-R3}
                ADD SP,SP,#4
                POP {LR}
                ADD SP,SP,#4
                CPSIE I
                BX LR
				
				END