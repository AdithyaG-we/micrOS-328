#include <avr/io.h>

.global scheduler_context_switch
.type scheduler_context_switch, @function

scheduler_context_switch:
    ; saving cpu context of current task.
    push r0
    in r0, _SFR_IO_ADDR(SREG)
    push r0
    push r1
    clr r1 ; restore r1 to 0x00

    push r2
    push r3
    push r4
    push r5
    push r6
    push r7
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
    push r16
    push r17
    push r18
    push r19
    push r20
    push r21
    push r22
    push r23
    push r24
    push r25
    push r26
    push r27
    push r28
    push r29
    push r30
    push r31 

    lds r26, current_running_task
    lds r27, current_running_task + 1
    sbiw r26, 0
    breq .Lskip_sp_save

    in r0, _SFR_IO_ADDR(SPL)
    st X+, r0
    in r0, _SFR_IO_ADDR(SPH)
    st X, r0

.Lskip_sp_save:
    ; switch pointer from current task to next task.
    lds r26, next_task_to_run
    lds r27, next_task_to_run + 1
    sts current_running_task, r26
    sts current_running_task + 1, r27

    ;loading the SP for this new task.
    ld r0, X+
    out _SFR_IO_ADDR(SPL), r0
    ld r0, X
    out _SFR_IO_ADDR(SPH), r0

    ;restoring the cpu context for this task.
    pop r31
    pop r30
    pop r29
    pop r28
    pop r27
    pop r26
    pop r25
    pop r24
    pop r23
    pop r22
    pop r21
    pop r20
    pop r19
    pop r18
    pop r17
    pop r16
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop r7
    pop r6
    pop r5
    pop r4
    pop r3
    pop r2
    pop r1

    pop r0
    out _SFR_IO_ADDR(SREG), r0

    pop r0
    reti 