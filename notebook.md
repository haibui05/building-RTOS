Thanh ghi đa năng:
Là các thanh ghi từ R0 đến R12 giúp ích cho việc tính toán và phép toán số học được thực hiện bên trong bộ xử lý.
Thanh ghi đặc biệt:
SP- Stack pointer là thanh ghi dùng để lưu trữ địa chỉ của đỉnh ngăn xếp trong RAM.
LR - Link register là thanh ghi dùng để lưu trữ địa chỉ của chương trình con mà processor return.
PC - Program counter là thanh ghi dùng để lưu trữ địa chỉ của lệnh tiếp theo cần thực thi trong chương trình.

⇒ Vì vậy, khi nào xử lý ngắt, bộ xử lý sẽ thực sự nạp các giá trị r0, r1, r2, r3, r12, LR, PC, thanh ghi trạng thái (xPSR) sẽ được sao chép vào vùng ngăn xếp trong RAM trước khi vào systick_handler và khi hoàn thành thành xử lý ngắt, bộ xử lý sẽ khôi phục dữ liệu đã sao chép từ vị trí ngăn xếp về các thanh ghi tương ứng và tiếp tục quá trình.

Note:
Tại sao thanh ghi PC ở register = 0x8002C0   và ở vùng nhớ Stack = 0x800260 lại khác nhau ?
Khi CPU thực hiện việc sao chép các thanh ghi đa năng thì bị tạm dừng ở systick_handler nên PC ở register sẽ có địa chỉ là 0x8002C0 của systick_handler. Nhưng tại thời điểm sao chép từ main function, lệnh tiếp theo cần được thực thi trước khi ngắt hoặc systick_handler can thiệp vào bộ xử lý sẽ có địa chỉ là 0x800260.
⇒ Vì vậy đó lý do có sự không khớp giữa PC và LR. Như vậy giá trị được lưu trữ trong stack sẽ là địa chỉ thực thi lệnh tiếp theo trong main function.

Operating Systems
Hệ điều hành là gì? Hệ điều hành có thể được gọi là trình quản lý tài nguyên, thực hiện 2 chức năng chính:
Lập lịch các tác vụ (scheduled tasks):
Controlling peripherals:
Vậy sự khác biệt giữa OS và Real Time OS là gì? Hệ điều hành thời gian thực tương tự như hệ điều hành thông thường nhưng nó có sự đáp ứng về mặt thời gian cho việc lập lịch tác vụ và điều khiển thiết bị ngoại vi. Do đó RTOS này có tính dự đoán cao hơn và có ràng buộc về thời gian để hoàn thành mỗi tác vụ. 



Stack là một cấu trúc dữ liệu hoạt động theo nguyên tắc last in first out (LIFO) gồm hai thác tác chính là push và pop. Trong stack có hai con trỏ khác nhau là MSP (Main Stack Pointer) và PSP (Process Stack Pointer)
MSP thường được sử dụng trong các nhân của hệ điều hành (OS - kernel) nện bất cứ khi nào chúng ta cần sử dụng hệ điều hành thời gian thực chúng ta sử dụng MSP để thao tác dữ liệu trong nhân hệ điều hành.
PSP có ích trong các application task.
Mặc định, CPU stack pointer sẽ là main stack pointer xử lý các ISR - interrupt service routine.

Stack trên ARM là Full Descending Stack là stack giảm địa chỉ khi push. Có thể hiểu là stack pointer ban đầu ở đỉnh mảng mỗi lần push stack pointer sẽ giảm xuống.


Thread

Tất cả các thread đều được lập trình trong hệ điều hành thời gian thực và cùng một chương trình được thực thi trên nhiều luồng trong RTOS. Chúng ta có một tập hợp các thanh ghi và một stack riêng cho mỗi thread trong real-time. 

Note: Bộ xử lý của chúng ta chỉ có một tập thanh ghi và một stack trong CPU nhưng tại sao lại có thể thực thi mỗi luồng với các thanh ghi đa năng và chia stack cho mỗi thread khác nhau?  Lúc này sẽ cần bộ lập lịch, bộ lập lịch phát huy tác dụng trong một lõi core CPU, bộ lập lịch sẽ chịu trách nhiệm chuyển đổi giữa các luồng xử lý.

Type of threads

Về cơ bản chúng ta sẽ có 3 loại thread: 
Timed Threads:
Periodic Thread: chạy lặp lại theo chu kỳ cố định, biết trước thời gian giữa các lần chạy ⇒ như PID, control loop.
Sporadic Thread: không chạy theo thời gian cố định, bị kích hoạt bởi sự kiện nhưng có khoảng cách tối thiểu giữa 2 lần kích hoạt ⇒ CAN interrupt, sensor event.
Aperiodic Thread: không theo chu kỳ, không có giới hạn thời gian tối thiểu giữa hai lần, xảy ra hoàn toàn không đoán trước ⇒ giao tiếp user, user command.
Event Triggered Threads:
Main Threads: luồng chạy đầu tiên sau khi hệ thống khởi động thường dùng để khởi tạo phần cứng, tạo các thread khác, khởi động scheduler.

Thread Control Block

TCB là một cấu trúc dữ liệu chứa thông tin private của một luồng (thread) và các trường dữ liệu bắt buộc cần được lưu trữ bên trong khối điều khiển luồng (TCB) là con trỏ trỏ đến ngăn xếp của luồng và con trỏ trỏ đến thread tiếp theo. Hai thông tin này là bắt buộc cần được lưu trữ trong thread control block.
Ngoài ra còn có một số information option là các biến lưu trữ trạng thái của luồng, lưu trữ ID của luồng, lưu trữ chu kỳ của luồng, lưu trữ burst time của luồng, lưu trữ độ ưu tiên của luồng, 
Đoạn code example về cấu trúc của TCB như sau:

struct tcb {
    uint32_t *stack_ptr;
    struct tcb *next_thread;
    uint32_t status;
    uint32_t period;
    uint32_t burst_time;
};

Scheduler in RTOS

Là thành phần chính quyết định luồng nào sẽ được chạy trong hệ điều hành thời gian thực dựa trên một thuật toán lập lịch. Cơ bản có 3 trạng thái cho thread.
Đầu tiên là running state, nơi ta có thể thấy luồng đang thực sự hoạt động trong RTOS.
Tiếp theo là ready state, nơi ta có thể thấy luồng đã sẵn sàng để thực thi.
Cuối cùng là blocked state, nơi luồng bị giữ lại đang chờ một đầu vào.
Chỉ có một luồng xử lý được thực thi tại một thời điểm trên CPU. 
Thuật toán lập lịch:
Các luồng từ trạng thái ready có thể chuyển thành trạng thái running bất cứ lúc nào khi luồng ở trạng thái running đã thực thi xong hoặc hết thời gian chờ hoặc luồng ở trạng thái ready có độ ưu tiên cao hơn luồng ở trạng thái đang chạy.


Nếu nhiều luồng cùng ở trạng thái ready và có cùng độ ưu tiên thì bộ lập lịch sẽ chọn luồng theo cơ chế chia thời gian (time slicing) hoặc theo thứ tự vào hàng đợi (FIFO).


Khi luồng đang chạy bị chặn bởi một sự kiện (ví dụ: chờ semaphore, mutex, hoặc I/O) thì luồng đó sẽ chuyển sang trạng thái blocked và bộ lập lịch sẽ chọn một luồng khác trong hàng đợi ready để thực thi.


Khi sự kiện chờ hoàn tất, luồng ở trạng thái blocked sẽ chuyển lại trạng thái ready và có thể được lập lịch chạy nếu có độ ưu tiên phù hợp.


Bộ lập lịch luôn đảm bảo rằng tại một thời điểm chỉ có một luồng ở trạng thái running trên mỗi lõi xử lý.
Process and threads 
Thread chỉ là thể hiện trường hợp khác nhau của một task và task là các function chứa vòng lặp vô hạn như hàm main thực hiện một nhiệm vụ cụ thể trong real-time OS.
Các threads phối hợp nhau để đạt mục đích chung còn được gọi là process nhẹ. Nhưng khi các threads kết hợp với nhau chúng sẽ hình thành process. 
Với hệ thống đơn giản thì chỉ cần một process nhưng đối với các hệ thống phức tạp thì việc tăng các process hay các luồng cùng một lúc là điều cần thiết.
Types of scheduler
Các loại của bộ lập lịch là:
Static Schedulers - Bộ lập lịch tĩnh: mức độ ưu tiên của RTOS được xác định trước khi RTOS khởi động.
Dynamic Scheduler - Bộ lập lịch động: đó là mức độ ưu tiên được xác định trong quá trình thực thi các task.
Preemptive Scheduler - Bộ lập lịch ưu tiên: luồng có độ ưu tiên cao hơn có thể truy cập bộ xử lý ngay cả khi một luồng có độ ưu tiên thấp hơn đang được thực thi trên bộ xử lý.
Non Preemptive Scheduler: ngay cả khi luồng có độ ưu tiên thấp hơn nó sẽ không nhường cho luồng có độ ưu tiên cao hơn yêu cầu cho đến khi quá trình thực thi của  luồng có độ ưu tiên thấp hoàn tất ở bộ xử lý.
Các bộ lập lịch có thể được kết hợp với nhau ví dụ như:
Dynamic Preemptive
Static Preemptive
Dynamic Non-Preemptive
Static Non-Preemptive
Schedulers - Preemption
Preemption là cơ chế cho phép bộ lập lịch tạm dừng một luồng đang chạy để chuyển quyền thực thi cho một luồng khác có độ ưu tiên cao hơn. Trong quá trình này, trạng thái của luồng đang chạy sẽ được lưu lại và luồng có độ ưu tiên cao hơn sẽ được đưa vào trạng thái running. Cơ chế này giúp hệ thống đáp ứng nhanh các sự kiện quan trọng trong hệ thống thời gian thực.
Nhìn chung, preemption làm tăng tính linh hoạt và khả năng phản hồi của hệ thống, nhưng đồng thời cũng làm tăng chi phí xử lý do phải thường xuyên lưu và phục hồi ngữ cảnh của các luồng.

Schedulers Metrics - Đánh giá thuật toán lập lịch:
Throughput:
 Là số lượng tiến trình hoặc luồng được xử lý hoàn thành trong một đơn vị thời gian.
Throughput càng cao thì hiệu suất của hệ thống càng lớn.
Turnaround Time:
Là tổng thời gian từ khi một tiến trình hoặc luồng được tạo ra cho đến khi nó hoàn thành hoàn toàn. 
Chỉ số này phản ánh tốc độ xử lý chung của hệ thống.
Response Time:
Là thời gian từ khi tiến trình hoặc luồng gửi yêu cầu cho đến khi nhận được phản hồi đầu tiên từ CPU. 
Chỉ số này thể hiện mức độ phản hồi của hệ thống đối với người dùng.
CPU Utilization:
Là tỉ lệ phần trăm thời gian CPU thực sự được sử dụng để xử lý công việc so với tổng thời gian hoạt động. 
Mục tiêu là tận dụng CPU hiệu quả nhất.
Wait Time:
Là tổng thời gian tiến trình hoặc luồng phải chờ trong hàng đợi sẵn sàng (ready) mà không được thực thi. 
Thời gian chờ càng nhỏ thì hệ thống càng công bằng và hiệu quả.
Cách tối ưu hóa thuật toán lập lịch:
Round Robin Schedulers
Internal processing of round robin scheduler 
Context switching
Nói một cách đơn giản là context switching là chuyển từ luồng này sang luồng khác. Quá trình này bao gồm lưu trạng thái hiện tại của luồng vào bộ nhớ, khôi phục trạng thái của mới của luồng từ bộ nhớ và khởi chạy trên luồng mới.

Quanta 
Là khoảng thời gian (time slice) mà mỗi thread được phép chạy trước khi bị scheduler chuyển sang thread khác.
Ví dụ: quanta = 10 nghĩa là mỗi thread chạy tối đa 10ms rồi chuyển sang thread tiếp theo.
Thông thường dùng 10-20ms cho embedded RTOS.


Implement
Khởi tạo stack:
{
    "cells": [
        {
            "cell_type": "markdown",
            "metadata": {
                "language": "markdown"
            },
            "source": [
                "# RTOS Notebook (ARM Cortex-M)",
                "Ghi chú rút gọn theo dạng notebook để ôn nhanh và map vào code triển khai."
            ]
        },
        {
            "cell_type": "markdown",
            "metadata": {
                "language": "markdown"
            },
            "source": [
                "## Cell 1: CPU & Stack model",
                "- Register chính: R0-R12 (general purpose), SP, LR, PC, xPSR.",
                "- Stack Cortex-M là full descending: push thì SP giảm, pop thì SP tăng.",
                "- Có 2 stack pointer:",
                "  - MSP: cho kernel và ISR",
                "  - PSP: cho application thread"
            ]
        },
        {
            "cell_type": "markdown",
            "metadata": {
                "language": "markdown"
            },
            "source": [
                "## Cell 2: Interrupt & Context Save",
                "- Khi có interrupt, hardware tự push: R0, R1, R2, R3, R12, LR, PC, xPSR.",
                "- Kết thúc handler, hardware tự pop và resume chương trình cũ.",
                "- PC trong handler là địa chỉ handler; PC trên stack là địa chỉ resume thật.",
                "- Context switch trên Cortex-M:",
                "  - Hardware save: R0-R3, R12, LR, PC, xPSR",
                "  - Kernel save thêm: R4-R11",
                "  - Tổng cộng: 16 words"
            ]
        },
        {
            "cell_type": "markdown",
            "metadata": {
                "language": "markdown"
            },
            "source": [
                "## Cell 3: Thread, TCB, Scheduler",
                "- Mỗi thread có stack riêng và context riêng.",
                "- Scheduler luân phiên save/load context giữa các thread.",
                "- Loại thread: periodic, sporadic, aperiodic.",
                "- Trạng thái: Running, Ready, Blocked.",
                "- Chính sách: static/dynamic priority, preemptive/non-preemptive, RR/FIFO khi cùng ưu tiên."
            ]
        },
        {
            "cell_type": "code",
            "metadata": {
                "language": "c"
            },
            "source": [
                "struct tcb {",
                "    uint32_t *stack_ptr;",
                "    struct tcb *next_thread;",
                "    uint32_t status;",
                "    uint32_t period;",
                "    uint32_t burst_time;",
                "};"
            ]
        },
        {
            "cell_type": "markdown",
            "metadata": {
                "language": "markdown"
            },
            "source": [
                "## Cell 4: Kernel flow & Synchronization",
                "- Init stack thread: SP = &stack[STACK_SIZE - 16] (chừa 16 ô context).",
                "- PendSV flow: disable IRQ -> push R4-R11 -> save SP -> scheduler -> load SP mới -> pop R4-R11 -> enable IRQ -> return.",
                "- Launch kernel lần đầu: không có context cũ để save.",
                "- Cooperative: osYield().",
                "- Periodic ví dụ: tick 1ms, period 100ms -> chạy lại mỗi 100 tick.",
                "- Semaphore: take/give để block-wakeup.",
                "- Sleep: rtos_thread_sleep(100) -> Blocked -> Ready sau 100 tick.",
                "- Sporadic chuẩn: ISR chỉ signal (give), task xử lý ở context thread (take)."
            ]
        },
        {
            "cell_type": "markdown",
            "metadata": {
                "language": "markdown"
            },
            "source": [
                "## Cell 5: RTOS Core Checklist",
                "- Core: Scheduler + Context switch + Stack per thread + IPC + Timer.",
                "- Metrics: Throughput, Turnaround time, Response time, CPU utilization, Wait time.",
                "- Mục tiêu: đúng thời gian, dự đoán được, tối ưu tài nguyên, không trễ task quan trọng."
            ]
        }
    ]
}
		CPSIE I
