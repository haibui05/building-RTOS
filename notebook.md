# Notebook RTOS (ARM Cortex-M)

---

## Thanh ghi và cơ chế ngắt

Bên trong bộ xử lý ARM Cortex-M có hai nhóm thanh ghi chính mà chúng ta cần quan tâm khi xây dựng RTOS.

Nhóm thứ nhất là **thanh ghi đa năng** từ R0 đến R12. Đây là các thanh ghi dùng cho việc tính toán, truyền tham số khi gọi hàm, lưu giá trị tạm trong quá trình chương trình chạy. Ví dụ khi ta gọi một hàm có 4 tham số, CPU sẽ đặt 4 giá trị đó vào R0, R1, R2, R3 trước khi nhảy vào hàm.

Nhóm thứ hai là **thanh ghi đặc biệt**, gồm:
- **SP (Stack Pointer):** giữ địa chỉ đỉnh stack trong RAM. Mỗi khi push dữ liệu vào stack, SP sẽ thay đổi để trỏ tới vị trí mới.
- **LR (Link Register):** giữ địa chỉ quay về sau khi hàm con hoặc ngắt kết thúc. Nhờ LR mà CPU biết phải quay lại đâu để chạy tiếp.
- **PC (Program Counter):** giữ địa chỉ lệnh tiếp theo mà CPU sắp chạy. PC luôn trỏ về phía trước, cho CPU biết bước kế tiếp là gì.
- **xPSR:** thanh ghi trạng thái, chứa các cờ điều kiện (ví dụ cờ zero, cờ carry) và thông tin chế độ hoạt động.

Khi có **ngắt** xảy ra (ví dụ SysTick interrupt), phần cứng sẽ tự động làm một việc rất quan trọng: sao chép 8 thanh ghi R0, R1, R2, R3, R12, LR, PC, xPSR vào vùng stack trong RAM. Việc này gọi là **hardware stacking** và diễn ra hoàn toàn tự động, không cần lập trình viên viết code. Sau khi lưu xong, CPU mới nhảy vào hàm xử lý ngắt (ví dụ SysTick_Handler hoặc PendSV_Handler). Khi hàm xử lý ngắt kết thúc, phần cứng lại tự động lấy 8 thanh ghi đó từ stack ra, nạp lại vào CPU và chương trình tiếp tục chạy đúng chỗ cũ.

**Giải thích trường hợp PC ở register khác PC trong stack:**

Khi debug, bạn có thể thấy PC trong thanh ghi CPU là một giá trị (ví dụ 0x8002C0) và PC lưu trong stack là giá trị khác (ví dụ 0x800260). Đây không phải lỗi. Lý do là:
- PC trong thanh ghi CPU lúc đang ở trong handler chính là địa chỉ của code handler đang chạy (ví dụ SysTick_Handler nằm ở 0x8002C0).
- PC lưu trong stack là địa chỉ lệnh tiếp theo của chương trình chính (main hoặc task) trước khi bị ngắt (ví dụ main đang chạy tới 0x800260 thì bị ngắt).

Hai giá trị khác nhau là hoàn toàn đúng vì chúng đại diện cho hai thời điểm khác nhau. Giá trị PC trong stack mới là điểm mà chương trình sẽ quay lại sau khi xử lý ngắt xong.

Ví dụ dễ hiểu: bạn đang đọc sách trang 100 (chương trình chính). Có cuộc gọi khẩn (ngắt), bạn đánh dấu trang 100 rồi đi nghe máy. Lúc nghe máy, vị trí hiện tại là cuộc gọi (handler). Đánh dấu trang 100 chính là PC được lưu trong stack để quay lại sau.

---

## Hệ điều hành, RTOS và Stack

**Hệ điều hành (OS)** có thể hiểu đơn giản là bộ quản lý tài nguyên của máy. Nó làm hai việc chính: lập lịch cho các tác vụ (quyết định ai được chạy, khi nào chạy) và điều khiển các thiết bị ngoại vi (UART, SPI, GPIO...).

**RTOS** cũng là hệ điều hành nhưng thêm yếu tố **thời gian thực**: mỗi tác vụ phải hoàn thành trong khoảng thời gian cho phép. Nếu trễ thì hệ thống có thể hoạt động sai hoặc nguy hiểm. RTOS có tính dự đoán cao hơn OS thường, nghĩa là ta biết trước được thời gian đáp ứng tối đa cho mỗi tác vụ.

**Stack** là cấu trúc dữ liệu kiểu LIFO (vào sau ra trước). Hai thao tác chính là push (đẩy dữ liệu vào) và pop (lấy dữ liệu ra). Trên ARM Cortex-M, stack hoạt động theo kiểu **Full Descending**: mỗi lần push thì SP giảm (đi xuống vùng địa chỉ thấp hơn), mỗi lần pop thì SP tăng (đi lên vùng địa chỉ cao hơn). Có thể hình dung stack pointer ban đầu ở đỉnh mảng, mỗi lần push nó sẽ đi xuống.

ARM Cortex-M có **hai con trỏ stack**:
- **MSP (Main Stack Pointer):** dùng cho kernel và xử lý ngắt (ISR). Đây là stack pointer mặc định khi CPU khởi động.
- **PSP (Process Stack Pointer):** dùng cho các task ứng dụng. Khi RTOS chạy, mỗi task thường dùng PSP riêng.

Việc tách MSP và PSP giúp bảo vệ hệ thống: stack của task ứng dụng bị tràn sẽ không ảnh hưởng đến stack của kernel. Khi debug, luôn kiểm tra CPU đang dùng MSP hay PSP để biết đang nhìn đúng vùng nhớ.

---

## Thread, loại thread và TCB

Trong RTOS, mỗi **thread** (luồng) có stack riêng trong RAM và có ngữ cảnh riêng (tập thanh ghi đã được lưu). Tuy CPU chỉ có một bộ thanh ghi vật lý duy nhất, nhưng nhờ bộ lập lịch (scheduler), CPU có thể luân phiên chạy nhiều thread bằng cách lưu trạng thái thread cũ rồi nạp trạng thái thread mới. Quá trình này gọi là chuyển đổi ngữ cảnh (context switch).

Câu hỏi thường gặp: CPU chỉ có một bộ thanh ghi, sao chạy được nhiều thread? Câu trả lời là scheduler chịu trách nhiệm chuyển đổi. Trước khi rời thread A, scheduler lưu toàn bộ thanh ghi của A vào stack của A. Sau đó nạp thanh ghi từ stack của B lên CPU để B tiếp tục chạy. Cứ luân phiên như vậy, mỗi thread tưởng như nó đang sở hữu CPU riêng.

**Các loại thread hay gặp:**
- **Periodic:** chạy theo chu kỳ cố định, ví dụ cứ 10ms hoặc 100ms lại chạy một lần. Dùng nhiều cho vòng điều khiển PID, đọc cảm biến định kỳ.
- **Sporadic:** chạy khi có sự kiện, nhưng có khoảng cách tối thiểu giữa hai lần chạy. Ví dụ ngắt CAN bus, sự kiện cảm biến.
- **Aperiodic:** chạy theo sự kiện hoàn toàn không dự đoán trước, không có chu kỳ. Ví dụ lệnh từ người dùng gõ vào terminal.
- **Main thread:** luồng chạy đầu tiên khi hệ thống bật nguồn. Thường dùng để khởi tạo phần cứng, tạo các thread khác, rồi khởi động scheduler.

**Thread Control Block (TCB)** là cấu trúc dữ liệu lưu thông tin riêng của mỗi thread. Tối thiểu TCB cần có:
- Con trỏ tới đỉnh stack của thread đó.
- Con trỏ tới thread kế tiếp (dùng cho danh sách liên kết vòng).

Ngoài ra có thể thêm: trạng thái thread, chu kỳ chạy, thời gian sleep, mức ưu tiên, ID...

```c
struct tcb {
    uint32_t *stack_ptr;
    struct tcb *next_thread;
    uint32_t status;
    uint32_t period;
    uint32_t burst_time;
};
```

---

## Scheduler và các trạng thái thread

Scheduler là bộ não của RTOS, quyết định thread nào được chạy tại mỗi thời điểm. Thread có ba trạng thái cơ bản:
- **Running:** đang thực sự chạy trên CPU. Tại một thời điểm trên một lõi CPU chỉ có đúng một thread ở trạng thái này.
- **Ready:** đã sẵn sàng chạy, đang chờ đến lượt. Khi thread running kết thúc hoặc bị tạm dừng, scheduler sẽ chọn một thread ready để chuyển sang running.
- **Blocked:** đang chờ một sự kiện nào đó (ví dụ chờ semaphore, chờ dữ liệu I/O, chờ hết thời gian sleep). Thread blocked sẽ không được scheduler chọn cho đến khi sự kiện xảy ra.

Luồng chuyển trạng thái diễn ra như sau:
- Ready chuyển sang Running khi scheduler chọn thread đó (ví dụ thread ưu tiên cao nhất, hoặc thread tiếp theo trong round-robin).
- Running chuyển sang Ready khi hết lát thời gian (time slice) hoặc khi thread ưu tiên cao hơn xuất hiện.
- Running chuyển sang Blocked khi thread gọi hàm chờ (ví dụ take semaphore mà semaphore đang bằng 0, hoặc gọi sleep).
- Blocked chuyển sang Ready khi sự kiện chờ xảy ra (ví dụ có thread khác give semaphore, hoặc hết thời gian sleep).

Thread trong RTOS thực chất là các hàm chứa vòng lặp vô hạn, mỗi hàm thực hiện một nhiệm vụ riêng. Nhiều thread phối hợp nhau tạo thành một chương trình hoàn chỉnh.

**Các kiểu scheduler:**
- **Static (tĩnh):** mức ưu tiên được gán trước khi hệ thống chạy, không thay đổi.
- **Dynamic (động):** mức ưu tiên có thể thay đổi trong lúc hệ thống đang chạy tùy theo tình huống.
- **Preemptive (có quyền ngắt):** thread ưu tiên cao có thể ngắt thread ưu tiên thấp đang chạy để chiếm CPU ngay. Ưu điểm là phản hồi nhanh cho sự kiện quan trọng. Nhược điểm là tốn thêm chi phí cho việc lưu và khôi phục ngữ cảnh.
- **Non-preemptive (không có quyền ngắt):** thread đang chạy giữ CPU cho đến khi tự nhường hoặc xong việc, dù có thread ưu tiên cao hơn đang chờ.

Các kiểu có thể kết hợp: Static Preemptive, Dynamic Preemptive, Static Non-Preemptive, Dynamic Non-Preemptive. Trong nhiều RTOS nhúng, kiểu Static Preemptive là phổ biến nhất.

**Các chỉ số đánh giá scheduler:**
- **Throughput:** số task hoàn thành trong một đơn vị thời gian. Càng cao càng tốt.
- **Turnaround Time:** tổng thời gian từ lúc task vào hệ thống đến lúc xong. Càng ngắn càng tốt.
- **Response Time:** thời gian từ lúc task gửi yêu cầu đến lúc nhận được phản hồi đầu tiên. Quan trọng với hệ thống tương tác.
- **CPU Utilization:** tỉ lệ thời gian CPU làm việc có ích so với tổng thời gian. Mục tiêu là dùng CPU hiệu quả nhất.
- **Wait Time:** tổng thời gian task phải chờ trong hàng đợi ready. Càng ít càng công bằng.

**Quanta (time slice)** là khoảng thời gian mỗi thread được phép chạy trước khi bị scheduler chuyển sang thread khác. Ví dụ quanta = 10ms nghĩa là mỗi thread chạy tối đa 10ms rồi nhường CPU. Trong hệ thống nhúng thường dùng 10-20ms.

**Context switching** nói đơn giản là chuyển từ thread này sang thread khác. Quá trình gồm: lưu trạng thái hiện tại của thread cũ vào bộ nhớ, khôi phục trạng thái của thread mới từ bộ nhớ, rồi cho thread mới chạy tiếp.

---

## Khởi tạo stack cho mỗi thread

Khi tạo thread mới, ta cần chuẩn bị sẵn stack frame giống như thread đã từng chạy và bị ngắt. Làm vậy để khi scheduler nạp ngữ cảnh cho thread lần đầu, thread có thể chạy ngay mà không cần xử lý đặc biệt.

Mỗi thread được cấp một mảng làm stack riêng. Vì Cortex-M cần tổng 16 word để khôi phục đầy đủ ngữ cảnh (8 word do phần cứng quản lý: R0-R3, R12, LR, PC, xPSR và 8 word do kernel quản lý: R4-R11), nên SP ban đầu được đặt tại vị trí `STACK_SIZE - 16`. Với STACK_SIZE = 100 thì mỗi thread có 100 ô nhớ (mỗi ô 4 byte = 1 word).

```c
void rtos_kernel_stack_init(int thread_number)
{
    TCBs[thread_number].stackPointer = &TCB_Stack[thread_number][STACK_SIZE - 16];

    /* Bật bit 24 trong xPSR (Thumb bit) để CPU chạy ở chế độ Thumb */
    TCB_Stack[thread_number][STACK_SIZE - 1] = (1U << 24);

    /* Điền giá trị mẫu 0xAAAAAAAA để dễ nhận biết khi debug */
    TCB_Stack[thread_number][STACK_SIZE - 3]  = 0xAAAAAAAA; // R14 - LR
    TCB_Stack[thread_number][STACK_SIZE - 4]  = 0xAAAAAAAA; // R12
    TCB_Stack[thread_number][STACK_SIZE - 5]  = 0xAAAAAAAA; // R3
    TCB_Stack[thread_number][STACK_SIZE - 6]  = 0xAAAAAAAA; // R2
    TCB_Stack[thread_number][STACK_SIZE - 7]  = 0xAAAAAAAA; // R1
    TCB_Stack[thread_number][STACK_SIZE - 8]  = 0xAAAAAAAA; // R0

    TCB_Stack[thread_number][STACK_SIZE - 9]  = 0xAAAAAAAA; // R11
    TCB_Stack[thread_number][STACK_SIZE - 10] = 0xAAAAAAAA; // R10
    TCB_Stack[thread_number][STACK_SIZE - 11] = 0xAAAAAAAA; // R9
    TCB_Stack[thread_number][STACK_SIZE - 12] = 0xAAAAAAAA; // R8
    TCB_Stack[thread_number][STACK_SIZE - 13] = 0xAAAAAAAA; // R7
    TCB_Stack[thread_number][STACK_SIZE - 14] = 0xAAAAAAAA; // R6
    TCB_Stack[thread_number][STACK_SIZE - 15] = 0xAAAAAAAA; // R5
    TCB_Stack[thread_number][STACK_SIZE - 16] = 0xAAAAAAAA; // R4
}
```

Vì sao `STACK_SIZE - 16`? Vì khi khôi phục ngữ cảnh, CPU sẽ POP 8 thanh ghi do kernel lưu (R4-R11), rồi phần cứng tự POP 8 thanh ghi còn lại (R0-R3, R12, LR, PC, xPSR). Tổng cộng 16 word. Nên SP phải bắt đầu tại vị trí R4 để POP lần lượt đúng thứ tự.

Vì mảng trong C nằm liên tiếp trong RAM và địa chỉ tăng dần theo chỉ số, mà ARM stack là kiểu giảm dần (full descending), nên index cao trong mảng tương ứng với địa chỉ cao trong RAM. Khi push, SP đi xuống (index nhỏ hơn); khi pop, SP đi lên (index lớn hơn).

Hình dung bố trí stack trong RAM:

```
Địa chỉ cao
0x2000118C  [STACK_SIZE - 1]   xPSR    ← index cao nhất
0x20001188  [STACK_SIZE - 2]   PC
0x20001184  [STACK_SIZE - 3]   LR
0x20001180  [STACK_SIZE - 4]   R12
0x2000117C  [STACK_SIZE - 5]   R3
0x20001178  [STACK_SIZE - 6]   R2
0x20001174  [STACK_SIZE - 7]   R1
0x20001170  [STACK_SIZE - 8]   R0
0x2000116C  [STACK_SIZE - 9]   R11
0x20001168  [STACK_SIZE - 10]  R10
0x20001164  [STACK_SIZE - 11]  R9
0x20001160  [STACK_SIZE - 12]  R8
0x2000115C  [STACK_SIZE - 13]  R7
0x20001158  [STACK_SIZE - 14]  R6
0x20001154  [STACK_SIZE - 15]  R5
0x20001150  [STACK_SIZE - 16]  R4      ← SP bắt đầu ở đây
─────────────────────────────────────
0x2000114C  (stack trống, mở rộng xuống khi push thêm)
...
0x20001000  [0]                        ← index 0, đáy mảng
Địa chỉ thấp
```

Lưu ý thực tế khi viết hàm init:
- Phải bật bit Thumb (bit 24) trong xPSR, nếu không CPU sẽ lỗi hard fault khi chạy task.
- Vị trí PC trong frame cần gán bằng địa chỉ hàm task thật sự (ví dụ `(uint32_t)task0_function`). Code ở trên để 0 vì chưa gán task cụ thể, nhưng trong code thực tế bắt buộc phải gán.
- LR có thể đặt giá trị đặc biệt để phát hiện task lỡ return (vì task trong RTOS chạy vòng lặp vô hạn, không bao giờ return).
- Giá trị 0xAAAAAAAA chỉ để debug dễ nhìn, không ảnh hưởng logic.

---

## Context switch bằng PendSV và khởi động task đầu tiên

**Vì sao dùng PendSV thay vì SysTick để chuyển đổi ngữ cảnh:**

SysTick interrupt thường dùng để tạo nhịp thời gian (ví dụ mỗi 1ms tăng biến tick). Nếu nhồi cả việc context switch vào SysTick_Handler thì handler sẽ quá dài và có thể trễ các ngắt quan trọng khác. Vì vậy hầu hết RTOS chuyển phần context switch sang PendSV_Handler. PendSV có ưu tiên thấp nhất nên đảm bảo các ngắt quan trọng hơn được xử lý trước, rồi mới đến việc đổi task.

Cách hoạt động: trong SysTick_Handler chỉ cần kích hoạt (trigger) PendSV bằng cách set bit pending. Khi tất cả ngắt ưu tiên cao hơn đã xử lý xong, CPU mới nhảy vào PendSV_Handler để thực hiện context switch.

```asm
    AREA |.text|, CODE, READONLY, ALIGN=2
    THUMB
    EXTERN currentPointer
    EXTERN rtos_periodic_scheduler_round_robin
    EXPORT PendSV_Handler
    EXPORT rtos_kernel_scheduler_launch

PendSV_Handler
    CPSID I                              ; Bước 1: tắt ngắt
    PUSH  {R4-R11}                       ; Bước 2: lưu R4-R11 của task cũ
    LDR   R0, =currentPointer            ;         lấy địa chỉ con trỏ task hiện tại
    LDR   R1, [R0]                       ;         R1 = TCB của task hiện tại
    STR   SP, [R1]                       ;         lưu SP vào TCB
    PUSH  {R0, LR}                       ;         giữ R0 và LR trước khi gọi hàm C
    BL    rtos_periodic_scheduler_round_robin  ; Bước 3: gọi scheduler chọn task mới
    POP   {R0, LR}                       ;         khôi phục R0 và LR
    LDR   R1, [R0]                       ;         R1 = TCB của task mới
    LDR   SP, [R1]                       ; Bước 4: nạp SP của task mới
    POP   {R4-R11}                       ;         khôi phục R4-R11 của task mới
    CPSIE I                              ; Bước 5: bật lại ngắt
    BX    LR                             ;         thoát handler, phần cứng tự pop R0-R3,R12,LR,PC,xPSR

rtos_kernel_scheduler_launch
    LDR   R0, =currentPointer
    LDR   R2, [R0]
    LDR   SP, [R2]                       ; nạp SP của task đầu tiên
    POP   {R4-R11}                       ; khôi phục R4-R11
    POP   {R12}
    POP   {R0-R3}
    ADD   SP, SP, #4                     ; bỏ qua LR trong frame
    POP   {LR}
    ADD   SP, SP, #4                     ; bỏ qua xPSR trong frame
    CPSIE I                              ; bật ngắt
    BX    LR                             ; nhảy vào task
    END
```

**Giải thích từng bước PendSV_Handler:**

Bước 1 — Tắt ngắt: dùng CPSID I để tắt ngắt toàn bộ. Tránh trường hợp đang đổi task giữa chừng mà bị ngắt khác chen vào, làm hỏng dữ liệu stack.

Bước 2 — Lưu ngữ cảnh task cũ: phần cứng đã tự push R0-R3, R12, LR, PC, xPSR trước khi vào handler rồi. Kernel chỉ cần tự push thêm R4-R11 (8 thanh ghi còn lại). Sau đó lưu giá trị SP hiện tại vào trường stack_ptr trong TCB của task cũ. Như vậy toàn bộ trạng thái của task cũ đã nằm gọn trong stack của nó.

Bước 3 — Gọi scheduler: hàm C `rtos_periodic_scheduler_round_robin` sẽ cập nhật `currentPointer` để trỏ sang TCB của task kế tiếp.

Bước 4 — Nạp ngữ cảnh task mới: lấy SP từ TCB của task mới, POP R4-R11 để khôi phục thanh ghi. Khi thoát handler, phần cứng sẽ tự pop nốt R0-R3, R12, LR, PC, xPSR. Task mới tiếp tục chạy từ đúng chỗ nó bị dừng lần trước.

Bước 5 — Bật ngắt và thoát: CPSIE I bật lại ngắt. BX LR thoát khỏi handler. Lúc này context switch hoàn tất.

**Hàm `rtos_kernel_scheduler_launch`:**

Hàm này dùng khi kernel mới khởi động, cần chạy task đầu tiên. Lúc này chưa có task nào đang chạy nên không cần lưu ngữ cảnh cũ. Cũng không có exception nào đang diễn ra nên phần cứng không tự pop — kernel phải tự pop toàn bộ 16 thanh ghi từ stack để nạp trạng thái cho task đầu tiên rồi nhảy vào chạy.

**Lỗi hay gặp khi viết context switch:**
- Sai thứ tự push/pop thanh ghi.
- Quên cập nhật currentPointer trong scheduler.
- Bật ngắt quá sớm (trước khi nạp xong SP mới).
- Stack của task bị tràn mà không kiểm tra.
- Nhầm lẫn giữa MSP và PSP.

**Mẹo debug context switch:**
- Điền giá trị đặc trưng (0xAAAAAAAA, 0xBBBBBBBB) cho mỗi task khi init stack, dễ nhìn khi xem memory.
- In ra SP trước và sau khi switch để kiểm tra.
- Đặt counter trong mỗi task, nếu counter không tăng nghĩa là task không được chạy.
- Khi CPU treo, kiểm tra frame stack của task vừa được nạp.

---

## Cooperative scheduler, periodic scheduler, semaphore, PendSV, thread sleep và sporadic thread

**Cooperative scheduler (lập lịch hợp tác):**

Trong round-robin cơ bản, mỗi task được chạy hết lát thời gian (quanta) mới nhường CPU. Nhưng nếu task xong việc sớm thì phần thời gian còn lại bị lãng phí. Cooperative scheduler cho phép task tự nguyện nhường CPU bằng cách gọi hàm `osYield()`. Hàm này sẽ kích hoạt SysTick handler hoặc PendSV để chuyển sang task tiếp theo ngay lập tức, không đợi hết quanta.

**Periodic scheduler (lập lịch định kỳ):**

Ý tưởng: một số task cần chạy theo chu kỳ cố định, ví dụ cứ 100ms lại chạy một lần. Scheduler sẽ kiểm tra chu kỳ và chỉ cho task chạy khi đến đúng thời điểm. Các task còn lại phải chờ. Bộ lập lịch định kỳ rất phù hợp cho các bài toán điều khiển, đọc cảm biến, cập nhật hiển thị.

**Semaphore (cơ chế đồng bộ):**

Semaphore dùng biến đếm để kiểm soát quyền truy cập tài nguyên hoặc đồng bộ giữa các task và interrupt. Chỉ cho một task dùng tài nguyên tại một thời điểm, các task khác phải chờ. Gồm 2 thao tác chính:

- **take (wait):** task xin quyền dùng tài nguyên. Nếu semaphore > 0 thì giảm counter đi 1 và chạy tiếp. Nếu semaphore = 0 thì task bị blocked, nhường CPU cho task khác, chờ đến khi có task khác give semaphore.
- **give (signal):** task trả quyền, tăng giá trị semaphore lên 1. Task nào đang chờ (blocked) sẽ được đánh thức và chuyển sang trạng thái ready.

Ví dụ hai task luân phiên nhau bằng semaphore:

```c
__attribute__((noreturn)) void task1(void)
{
    for (;;)
    {
        rtos_semaphore_take(&semaphore_1);
        task1_runner++;
        light_off();
        rtos_semaphore_give(&semaphore_2);
    }
}

__attribute__((noreturn)) void task2(void)
{
    for (;;)
    {
        rtos_semaphore_take(&semaphore_2);
        task2_runner++;
        light_on();
        rtos_semaphore_give(&semaphore_1);
    }
}
```

Giả sử ban đầu semaphore_1 = 1 và semaphore_2 = 0. Task 1 gọi take semaphore_1, giá trị giảm từ 1 xuống 0, task 1 được chạy. Sau khi xong việc, task 1 gọi give semaphore_2, giá trị tăng từ 0 lên 1. Lúc này task 2 đang chờ semaphore_2 sẽ được đánh thức, gọi take semaphore_2 thành công (giảm từ 1 xuống 0), chạy xong rồi give semaphore_1 (tăng từ 0 lên 1). Cứ như vậy hai task luân phiên chạy, mỗi thời điểm chỉ một task được thực thi.

**PendSV Handler — vai trò chuyên biệt:**

Như đã giải thích ở trên, SysTick không nên làm quá nhiều việc. SysTick chỉ cập nhật tick rồi set pending cho PendSV. PendSV có ưu tiên thấp nên chạy sau tất cả ngắt quan trọng hơn, an toàn cho việc đổi task.

**Thread sleep (cho task tạm nghỉ):**

Đây là cơ chế để task tự nguyện tạm dừng trong một khoảng thời gian nhất định. Task không bị xóa mà chuyển sang trạng thái blocked/sleeping. Sau khi đủ số tick, scheduler sẽ chuyển task về trạng thái ready để chạy lại.

Ví dụ `rtos_thread_sleep(100)` nghĩa là task hiện tại muốn nghỉ 100 tick. Scheduler sẽ bỏ qua task này trong 100 tick tiếp theo và gọi task khác chạy thay.

```c
void rtos_thread_sleep(uint32_t sleep_time)
{
    __disable_irq();                       // tắt ngắt tránh tranh chấp dữ liệu
    currentPointer->sleepTime = sleep_time; // gán thời gian nghỉ
    __enable_irq();                        // bật lại ngắt
    osYield();                             // nhường CPU ngay
}
```

Scheduler có phiên bản hỗ trợ sleep: bỏ qua tất cả thread có sleepTime > 0.

```c
void rtos_periodic_scheduler_round_robin_with_sleep(void)
{
    currentPointer = currentPointer->nextStackPointer;
    while (currentPointer->sleepTime > 0) {
        currentPointer = currentPointer->nextStackPointer;
    }
}
```

Lợi ích của thread sleep so với busy wait (vòng lặp chờ):
- Không tốn CPU vô ích.
- Tiết kiệm năng lượng.
- Các task khác có thêm thời gian chạy.
- Code periodic task dễ viết hơn.

**Sporadic thread (task chạy theo sự kiện):**

Sporadic thread không chạy theo chu kỳ, chỉ chạy khi có sự kiện xảy ra như nhấn nút, ngắt ngoài, hoặc dữ liệu đến từ ngoại vi.

Mô hình phổ biến dùng semaphore:
- Trong hàm xử lý ngắt (ISR), chỉ thực hiện một việc duy nhất: gọi `give_semaphore()` để báo hiệu có sự kiện.
- Task chính gọi `take_semaphore()` để chờ. Khi ISR give, task sẽ được đánh thức và chạy hàm xử lý trong ngữ cảnh task (không phải trong ISR).

Cách làm này có lợi ích lớn:
- ISR luôn ngắn gọn, thoát nhanh.
- Việc xử lý nặng nằm ở task, an toàn hơn và có thể bị scheduler quản lý.
- Tránh gọi hàm phức tạp hoặc dùng tài nguyên chung trong ISR.

---

## Tóm tắt và checklist

**Tóm tắt ngắn:**
- RTOS gồm scheduler + context switch + timer + cơ chế đồng bộ (semaphore, mailbox).
- Mỗi task có stack riêng, kernel đổi qua lại bằng lưu/nạp ngữ cảnh.
- PendSV dùng để đổi task, SysTick tạo nhịp thời gian.
- Semaphore giúp task không tranh chấp tài nguyên.
- Sleep giúp tiết kiệm CPU, tránh vòng lặp chờ.

**Checklist khi tự viết kernel nhỏ:**
- [ ] Tạo TCB đủ trường cần dùng (stack_ptr, next_thread, status, period, sleepTime...).
- [ ] Khởi tạo stack frame đúng thứ tự thanh ghi.
- [ ] Bật Thumb bit trong xPSR (bit 24).
- [ ] Gán địa chỉ hàm task vào vị trí PC trong stack frame.
- [ ] Viết PendSV_Handler lưu/khôi phục đúng thứ tự.
- [ ] Viết scheduler chọn task kế tiếp an toàn.
- [ ] Có cơ chế block/wakeup bằng semaphore hoặc sleep.
- [ ] Có cách kiểm tra stack tràn (ví dụ đặt giá trị canh gác ở đáy stack).
- [ ] ISR luôn ngắn gọn, không làm việc nặng.

**4 ý cốt lõi để nhớ:**
1. Phần cứng tự lưu 8 thanh ghi (R0-R3, R12, LR, PC, xPSR), kernel phải tự lưu thêm 8 thanh ghi (R4-R11), tổng 16 word.
2. SP bắt đầu tại STACK_SIZE - 16 vì cần chừa chỗ cho toàn bộ frame.
3. PC register khác PC stack khi đang ở handler là chuyện bình thường.
4. PendSV ưu tiên thấp nhất, chuyên dùng để đổi task.

Nếu nắm chắc 4 ý trên, bạn đã hiểu phần lõi của context switch trong ARM Cortex-M RTOS.
