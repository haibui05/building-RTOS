# Giải thích chi tiết bản chất và tư duy thiết kế RTOS

## Phần 1: Tại sao cần TCB (Task Control Block)?

Hãy bắt đầu từ vấn đề cơ bản: **CPU chỉ có 1 bộ thanh ghi (registers)**, nhưng ta muốn chạy nhiều task. Mỗi task có trạng thái riêng (giá trị các thanh ghi R0-R15, PC, SP, PSR). Vậy khi chuyển từ Task A sang Task B, làm sao **lưu trạng thái của Task A** để sau này quay lại?

**Câu trả lời:** Cần một cấu trúc dữ liệu để lưu thông tin quan trọng nhất của task - đó chính là **TCB**.

```c
struct TCB {
    uint32_t *stackPointer;        // Con trỏ đến đỉnh stack của task
    struct TCB *nextStackPointer;  // Con trỏ đến TCB của task tiếp theo
};
```

Tại sao chỉ cần lưu `stackPointer` mà không lưu hết R0-R15? Vì tất cả các thanh ghi đã được **push vào stack** rồi! Stack chứa toàn bộ context của task (R0-R15, PC, PSR). Vậy chỉ cần biết **stack ở đâu** (stackPointer) là có thể khôi phục lại toàn bộ task.

Tại sao có `nextStackPointer`? Để liên kết các task thành **vòng tròn** (circular linked list), giúp scheduler biết task tiếp theo là gì. Thiết kế này đơn giản và hiệu quả cho Round-Robin scheduling:

```
TCB[0] → TCB[1] → TCB[2] → TCB[0] → ...
```

---

## Phần 2: Tại sao dùng mảng 2 chiều `TCB_Stack[NUMBER_THREADS][STACK_SIZE]`?

Đây là câu hỏi then chốt! Hãy phân tích từng bước.

**Vấn đề đặt ra:** Mỗi task cần **stack riêng biệt**. Tại sao? Vì stack lưu:
- Biến local của hàm
- Địa chỉ return khi gọi hàm
- Context (các thanh ghi) khi bị interrupt

Nếu 2 task dùng chung stack, khi Task0 push dữ liệu lên stack rồi switch sang Task1, Task1 sẽ ghi đè lên dữ liệu của Task0 → **crash!**

Vậy ta cần **cấp phát stack riêng** cho từng task. Có nhiều cách:

**Cách 1 (không tốt):** Khai báo riêng lẻ
```c
uint32_t stack_task0[100];
uint32_t stack_task1[100];
uint32_t stack_task2[100];
```
Vấn đề: Không linh hoạt, khó maintain khi thêm task.

**Cách 2 (tốt hơn):** Dùng mảng 2 chiều
```c
uint32_t TCB_Stack[NUMBER_THREADS][STACK_SIZE];
// TCB_Stack[0][0..99]   - Stack của task 0
// TCB_Stack[1][0..99]   - Stack của task 1
// TCB_Stack[2][0..99]   - Stack của task 2
```

Tại sao thiết kế này tốt?
1. **Dễ scale:** Muốn thêm task, chỉ cần thay `NUMBER_THREADS = 4`
2. **Dễ truy cập:** `TCB_Stack[thread_number][index]` rất trực quan
3. **Memory layout rõ ràng:** Các stack nằm liên tiếp trong RAM

**Tư duy về memory layout:**
```
RAM:
0x20000000: TCB_Stack[0][0]
0x20000004: TCB_Stack[0][1]
...
0x20000190: TCB_Stack[0][99]
0x20000194: TCB_Stack[1][0]  ← Stack task 1 bắt đầu
...
```

Mỗi stack có kích thước `STACK_SIZE * 4 bytes` = `100 * 4 = 400 bytes`. Tại sao 400 bytes? Đủ cho:
- 16 thanh ghi context (16 * 4 = 64 bytes)
- Function call stack (còn lại ~336 bytes)

---

## Phần 3: Cơ chế Exception/Interrupt của ARM Cortex-M

Đây là phần **CỰC KỲ QUAN TRỌNG** để hiểu context switching. ARM Cortex-M có cơ chế **hardware tự động** xử lý exception/interrupt, giúp tiết kiệm code và thời gian.

**Khi interrupt xảy ra (ví dụ SysTick), hardware TỰ ĐỘNG làm:**

**Bước 1: Stacking (Hardware tự động push 8 thanh ghi)**
```
Stack trước interrupt:        Stack sau interrupt:
┌────────┐                    ┌────────┐
│  ...   │                    │  PSR   │ ← Mới push
│        │                    │  PC    │ ← Mới push (địa chỉ lệnh bị ngắt)
│        │                    │  LR    │ ← Mới push
│        │                    │  R12   │ ← Mới push
│        │                    │  R3    │ ← Mới push
│        │                    │  R2    │ ← Mới push
│        │                    │  R1    │ ← Mới push
│        │                    │  R0    │ ← Mới push
│        │ ← SP cũ            │  ...   │ ← SP mới
└────────┘                    └────────┘
```

**Bước 2: Cập nhật PC**
- PC = địa chỉ của `SysTick_Handler` (lấy từ Vector Table)

**Bước 3: Cập nhật LR (Link Register)**
- LR = `0xFFFFFFF9` (giá trị magic EXC_RETURN)
- Giá trị này cho CPU biết đang trong exception mode

Điều kỳ diệu ở đây: **Ta không cần viết code để push R0-R3, R12, LR, PC, PSR!** Hardware đã làm hộ rồi! Ta chỉ cần push thêm R4-R11 (8 thanh ghi còn lại).

**Khi return từ interrupt (`BX LR`), hardware TỰ ĐỘNG làm:**

**Bước 1: Unstacking (Hardware tự động pop 8 thanh ghi)**
```
Stack trước return:           Stack sau return:
┌────────┐                    ┌────────┐
│  PSR   │ ← SP               │  ...   │
│  PC    │                    │        │
│  LR    │                    │        │
│  R12   │                    │        │
│  R3    │                    │        │
│  R2    │                    │        │ ← SP mới
│  R1    │                    │        │
│  R0    │                    │        │
│  ...   │                    └────────┘
└────────┘

→ R0-R3, R12, LR, PC, PSR được khôi phục
```

**Bước 2: Cập nhật PC**
- PC = giá trị vừa pop từ stack (địa chỉ lệnh bị ngắt)
- CPU tiếp tục chạy từ điểm bị ngắt!

**Tư duy quan trọng:** Nhờ cơ chế này, ta có thể **"lừa" CPU** rằng task mới vừa bị interrupt! Ta chỉ cần:
1. Setup stack của task mới giống như vừa bị interrupt
2. Khi `BX LR`, hardware sẽ pop stack → jump vào task mới!

---

## Phần 4: Hàm `rtos_kernel_stack_init()` - Tạo "fake exception stack frame"

Bây giờ, hiểu được cơ chế exception, ta có thể thiết kế stack ban đầu cho mỗi task.

**Mục tiêu:** Tạo stack giống như task **vừa mới bị interrupt**, sao cho khi `BX LR` (return from exception), CPU sẽ:
- Pop các thanh ghi từ stack
- Jump vào hàm task (PC)
- Bắt đầu chạy task

**Thiết kế stack từ dưới lên:**

```c
void rtos_kernel_stack_init(int thread_number)
{
    // Bước 1: SP trỏ vào vị trí để dành cho 16 thanh ghi
    TCBs[thread_number].stackPointer = &TCB_Stack[thread_number][STACK_SIZE - 16];
    
    // Bước 2: Điền PSR (bit 24 = T-bit = 1 cho Thumb mode)
    TCB_Stack[thread_number][STACK_SIZE - 1] = (1U << 24);
    // Tại sao bit 24? ARM Cortex-M CHỈ chạy Thumb instruction.
    // Nếu T-bit = 0 → HardFault!
    
    // Bước 3: PC sẽ được điền sau (địa chỉ hàm task)
    // TCB_Stack[thread_number][STACK_SIZE - 2] = (uint32_t)task_func;
    
    // Bước 4: Điền LR, R12, R0-R3 với dummy (0xAAAAAAAA)
    TCB_Stack[thread_number][STACK_SIZE - 3] = 0xAAAAAAAA; // LR
    TCB_Stack[thread_number][STACK_SIZE - 4] = 0xAAAAAAAA; // R12
    TCB_Stack[thread_number][STACK_SIZE - 5] = 0xAAAAAAAA; // R3
    TCB_Stack[thread_number][STACK_SIZE - 6] = 0xAAAAAAAA; // R2
    TCB_Stack[thread_number][STACK_SIZE - 7] = 0xAAAAAAAA; // R1
    TCB_Stack[thread_number][STACK_SIZE - 8] = 0xAAAAAAAA; // R0
    
    // Bước 5: Điền R4-R11 với dummy
    TCB_Stack[thread_number][STACK_SIZE - 9] = 0xAAAAAAAA;  // R11
    TCB_Stack[thread_number][STACK_SIZE - 10] = 0xAAAAAAAA; // R10
    TCB_Stack[thread_number][STACK_SIZE - 11] = 0xAAAAAAAA; // R9
    TCB_Stack[thread_number][STACK_SIZE - 12] = 0xAAAAAAAA; // R8
    TCB_Stack[thread_number][STACK_SIZE - 13] = 0xAAAAAAAA; // R7
    TCB_Stack[thread_number][STACK_SIZE - 14] = 0xAAAAAAAA; // R6
    TCB_Stack[thread_number][STACK_SIZE - 15] = 0xAAAAAAAA; // R5
    TCB_Stack[thread_number][STACK_SIZE - 16] = 0xAAAAAAAA; // R4
}
```

**Tại sao dùng 0xAAAAAAAA?** Đây là pattern dễ nhận diện khi debug. Khi xem memory dump, thấy `0xAAAAAAAA` là biết đó là giá trị chưa được khởi tạo.

**Tại sao STACK_SIZE - 1, -2, -3...?** Vì stack **grow downward** (tăng từ cao xuống thấp):
```
Địa chỉ cao ↑   [STACK_SIZE - 1] ← Đỉnh stack (PSR)
              │  [STACK_SIZE - 2] ← PC
              │  ...
Địa chỉ thấp ↓   [0]               ← Đáy stack
```

---

## Phần 5: Hàm `rtos_kernel_stack_add_threads()` - Liên kết và khởi tạo

```c
void rtos_kernel_stack_add_threads(void (*task0)(void), void (*task1)(void), void (*task2)(void))
{
    __disable_irq(); // Tắt interrupt để tránh race condition
    
    // Bước 1: Tạo vòng tròn (circular linked list)
    TCBs[0].nextStackPointer = &TCBs[1];
    TCBs[1].nextStackPointer = &TCBs[2];
    TCBs[2].nextStackPointer = &TCBs[0];
    // → Task0 → Task1 → Task2 → Task0 → ...
    
    // Bước 2: Khởi tạo stack cho từng task
    rtos_kernel_stack_init(0);
    TCB_Stack[0][STACK_SIZE - 2] = (uint32_t)(task0); // PC = địa chỉ task0
    
    rtos_kernel_stack_init(1);
    TCB_Stack[1][STACK_SIZE - 2] = (uint32_t)(task1);
    
    rtos_kernel_stack_init(2);
    TCB_Stack[2][STACK_SIZE - 2] = (uint32_t)(task2);
    
    // Bước 3: Bắt đầu từ task 0
    currentPointer = &TCBs[0];
    
    __enable_irq();
}
```

**Tại sao `(uint32_t)(task0)`?** 
- `task0` là function pointer (địa chỉ hàm)
- Cast sang `uint32_t` để lưu vào mảng stack
- Khi CPU pop giá trị này vào PC → jump vào hàm task0!

**Tại sao cần `__disable_irq()`?**
- Đang setup critical data (TCB, stack)
- Nếu SysTick interrupt xảy ra giữa chừng → data bị corrupt → crash!

---

## Phần 6: Hàm `rtos_kernel_scheduler_launch()` - Khởi động task đầu tiên

Đây là hàm **"lừa" CPU** rằng task0 vừa bị interrupt, giờ đang return.

```c
__attribute__((naked)) void rtos_kernel_scheduler_launch()
{
    // Bước 1: Load stack pointer của task0
    __asm("LDR R0, =currentPointer");  // R0 = địa chỉ biến currentPointer
    __asm("LDR R2, [R0]");             // R2 = *currentPointer = &TCBs[0]
    __asm("LDR SP, [R2]");             // SP = TCBs[0].stackPointer
    
    // Bước 2: Pop R4-R11 (ta đã push vào stack ban đầu)
    __asm("POP {R4-R11}");
    
    // Bước 3: Pop R0-R3, R12 (hardware sẽ pop khi return, nhưng ta pop thủ công)
    __asm("POP {R0-R3}");
    __asm("POP {R12}");
    
    // Bước 4: Bỏ qua LR (ADD SP, #4), vì LR dummy không cần
    __asm("ADD SP, SP, #4");
    
    // Bước 5: Pop PC vào LR
    __asm("POP {LR}");
    // Giờ LR = địa chỉ task0 (giá trị PC từ stack)
    
    // Bước 6: Bỏ qua PSR
    __asm("ADD SP, SP, #4");
    
    // Bước 7: Enable interrupt
    __asm("CPSIE I");
    
    // Bước 8: BX LR → Jump vào task0!
    __asm("BX LR");
    // CPU bắt đầu chạy task0(), không bao giờ return về đây!
}
```

**Tại sao `__attribute__((naked))`?**
- Compiler thường tạo **prologue** (push LR, setup stack frame) và **epilogue** (pop LR, return)
- Nhưng ta đang thao tác stack thủ công, không muốn compiler "giúp đỡ"
- `naked` bảo compiler: "Đừng tạo prologue/epilogue, ta tự xử lý!"

**Tại sao pop PC vào LR rồi `BX LR`?**
- Không thể `POP {PC}` trực tiếp vì sẽ không kích hoạt exception return mechanism
- `BX LR` sẽ branch (nhảy) đến địa chỉ trong LR → task0 bắt đầu chạy

---

## Phần 7: Hàm `SysTick_Handler()` - Context switching

Đây là **trái tim của RTOS**! Mỗi `quanta` ms, hàm này được gọi để:
1. Lưu task hiện tại
2. Chọn task tiếp theo
3. Khôi phục task mới

```c
__attribute__((naked)) void SysTick_Handler(void)
{
    // === PHẦN 1: Lưu task hiện tại ===
    __asm("CPSID I");              // Tắt interrupt (critical section)
    __asm("PUSH {R4-R11}");        // Push R4-R11
    // Giờ stack có: R4-R11 (ta push) + R0-R3,R12,LR,PC,PSR (hardware push)
    
    __asm("LDR R0, =currentPointer"); // R0 = địa chỉ currentPointer
    __asm("LDR R1, [R0]");            // R1 = currentPointer (TCB của task hiện tại)
    __asm("STR SP, [R1]");            // Lưu SP vào TCB: currentPointer->stackPointer = SP
    // → Stack của task hiện tại đã được lưu!
    
    // === PHẦN 2: Chọn task tiếp theo ===
    // Code gốc đơn giản:
    // __asm("LDR R1, [R1, #4]");      // R1 = currentPointer->nextStackPointer
    
    // Code của bạn gọi hàm C:
    __asm("PUSH {R0, LR}");           // Lưu R0, LR (chuẩn bị gọi hàm C)
    __asm("BL rtos_scheduler_round_robin"); // Gọi scheduler (cập nhật currentPointer)
    __asm("POP {R0, LR}");            // Khôi phục R0, LR
    
    __asm("LDR R1, [R0]");            // R1 = currentPointer (task mới)
    
    // === PHẦN 3: Khôi phục task mới ===
    __asm("LDR SP, [R1]");            // SP = newTask->stackPointer
    __asm("POP {R4-R11}");            // Pop R4-R11 của task mới
    
    __asm("CPSIE I");                 // Bật interrupt
    __asm("BX LR");                   // Return from exception
    // Hardware tự động pop R0-R3, R12, LR, PC, PSR
    // CPU tiếp tục chạy task mới!
}
```

**Tại sao phải `PUSH {R0, LR}` trước khi gọi hàm C?**
- Theo ARM calling convention (AAPCS), R0-R3, R12, LR có thể bị hàm C ghi đè
- Ta cần R0 (địa chỉ currentPointer) sau khi return
- LR chứa giá trị magic `0xFFFFFFF9` để return từ exception
- Nếu không lưu → bị mất → crash!

---

## Phần 8: Hàm `rtos_scheduler_round_robin()` - Periodic task

```c
void rtos_scheduler_round_robin(void)
{
    if ((++periodic_tick) == 100) // Mỗi 100 lần SysTick (100 * quanta ms)
    {
        (*task3)();               // Gọi task3 (periodic task)
        periodic_tick = 0;
    }
    currentPointer = currentPointer->nextStackPointer; // Chuyển sang task tiếp theo
}
```

Đây là **extension** của Round-Robin: Thêm 1 task đặc biệt (task3) chạy định kỳ mỗi `100 * quanta` ms. Ví dụ với `quanta = 10ms`:
- Task3 chạy mỗi `100 * 10ms = 1 giây`

**Lưu ý:** `task3()` chạy trong context của SysTick_Handler, không có stack riêng! Vậy task3 **PHẢI rất ngắn** (không được block, không được dùng nhiều stack).

---

## Phần 9: Hàm `rtos_kernel_launch()` - Setup SysTick và khởi động

```c
void rtos_kernel_launch(uint32_t quanta)
{
    volatile uint32_t *pSYST_CSR = (volatile uint32_t*)(SYSTICK_BASE + 0x00); 
    volatile uint32_t *pSYST_RVR = (volatile uint32_t*)(SYSTICK_BASE + 0x04); 
    volatile uint32_t *pSYST_CVR = (volatile uint32_t*)(SYSTICK_BASE + 0x08); 
    
    *pSYST_CSR = 0;                             // Reset SysTick
    *pSYST_CVR = 0;                             // Clear current value
    *pSYST_RVR = (quanta * millis_prescaler) - 1; // Load quanta
    
    NVIC_SetPriority(SysTick_IRQn, 15);         // Set priority thấp nhất
    
    *pSYST_CSR |= (1U << 0);                    // Enable SysTick
    *pSYST_CSR |= (1U << 1);                    // Enable interrupt
    *pSYST_CSR |= (1U << 2);                    // Select CPU clock
    
    rtos_kernel_scheduler_launch();             // Khởi động task0
}
```

**Giải thích các SysTick registers:**

**SYST_CSR (Control and Status Register):**
- Bit 0 (ENABLE): Enable/disable SysTick counter
- Bit 1 (TICKINT): Enable/disable SysTick interrupt
- Bit 2 (CLKSOURCE): 0 = external clock, 1 = CPU clock

**SYST_RVR (Reload Value Register):**
- Giá trị reload khi counter về 0
- `quanta * millis_prescaler - 1` vì counter đếm từ RVR → 0 (inclusive)
- Ví dụ: quanta=10ms, prescaler=72000 → RVR = 719999

**SYST_CVR (Current Value Register):**
- Giá trị hiện tại của counter
- Ghi 0 để reset

**Cách hoạt động:**
```
SysTick counter: 719999 → 719998 → ... → 1 → 0 → INTERRUPT! → 719999 → ...
                 |←────────── 10ms ─────────→|
```

---

## Tóm tắt tư duy thiết kế

**1. Từ vấn đề → Giải pháp:**
- Vấn đề: CPU chỉ có 1 bộ thanh ghi, nhưng nhiều task
- Giải pháp: Lưu thanh ghi vào stack, mỗi task có stack riêng

**2. Tận dụng hardware:**
- ARM Cortex-M tự động push/pop 8 thanh ghi khi exception
- Ta chỉ cần xử lý 8 thanh ghi còn lại → giảm code, tăng tốc độ

**3. Thiết kế data structure hợp lý:**
- TCB: Lưu thông tin tối thiểu (stack pointer + next pointer)
- Mảng 2 chiều: Stack riêng, dễ quản lý
- Circular linked list: Round-robin đơn giản

**4. "Lừa" CPU bằng cách giả lập exception:**
- Setup stack ban đầu giống như vừa bị interrupt
- `BX LR` → CPU nghĩ đang return from exception → task bắt đầu chạy

**5. Assembly khi cần thiết:**
- Thao tác trực tiếp với SP, PC, LR
- Không muốn compiler can thiệp → dùng `naked` function

---

## Timeline hoạt động của RTOS

```
t=0ms:
  main() gọi osKernelInit()
  → Tính millis_prescaler = 72000

  main() gọi osKernelAddThreads(task0, task1, task2)
  → Setup stack cho 3 tasks
  → Tạo vòng tròn: TCB[0]→TCB[1]→TCB[2]→TCB[0]
  → currentPointer = &TCB[0]

  main() gọi osKernelLaunch(10)
  → Setup SysTick: 10ms per tick
  → Gọi rtos_kernel_scheduler_launch()
  → Jump vào task0()

t=0-10ms:
  task0() đang chạy...

t=10ms:
  SysTick interrupt!
  → SysTick_Handler() được gọi
  → Lưu context của task0 vào stack
  → rtos_scheduler_round_robin() chọn task1
  → Load context của task1 từ stack
  → Return → task1() chạy

t=10-20ms:
  task1() đang chạy...

t=20ms:
  SysTick interrupt!
  → Lưu task1 → Chọn task2 → Load task2
  → task2() chạy

t=20-30ms:
  task2() đang chạy...

t=30ms:
  SysTick interrupt!
  → Lưu task2 → Chọn task0 (vòng lặp) → Load task0
  → task0() tiếp tục từ chỗ bị ngắt lúc t=10ms

... Lặp lại mãi mãi ...
```

---

## Câu hỏi thường gặp

**Q: Tại sao stack grow downward (từ cao xuống thấp)?**
A: Đây là convention của ARM. SP giảm khi PUSH, tăng khi POP. Giúp tránh overflow vào vùng code/data ở địa chỉ thấp.

**Q: Tại sao dùng 0xAAAAAAAA mà không phải 0x00000000?**
A: Pattern `0xAAAAAAAA` dễ nhận diện hơn khi debug. `0x00000000` có thể bị nhầm với NULL hoặc giá trị đã clear.

**Q: Task có thể gọi hàm được không?**
A: Có! Stack riêng của mỗi task cho phép gọi hàm bình thường. Nhưng phải đảm bảo stack đủ lớn.

**Q: Nếu task0 set biến global, task1 có thấy không?**
A: Có! Global variables được chia sẻ giữa các task. Cần mutex/semaphore để tránh race condition.

**Q: Tại sao không dùng main stack cho task?**
A: Main stack có thể bị overflow khi task chạy. Mỗi task cần stack riêng với kích thước cố định, dễ quản lý.

**Q: RTOS này có preemptive hay cooperative?**
A: Preemptive! SysTick interrupt tự động switch task, không cần task tự yield().

**Q: Làm sao biết STACK_SIZE = 100 có đủ không?**
A: Phụ thuộc vào task:
- Đơn giản (toggle GPIO): 100 words đủ
- Phức tạp (nhiều biến local, call stack sâu): cần 200-500 words
- Dùng debug watchpoint để kiểm tra stack usage

---

## Tài liệu tham khảo

1. **ARM Cortex-M3 Technical Reference Manual**
2. **ARM Architecture Reference Manual (ARMv7-M)**
3. **"The Definitive Guide to ARM Cortex-M3" - Joseph Yiu**
4. **FreeRTOS source code** (học cách professional RTOS thiết kế)
5. **STM32F1 Reference Manual** (SysTick, NVIC)