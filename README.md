# LinuxKernel-blockingIPC

## Overview
This project implements blocking inter-process communication (IPC) system calls in the Linux kernel. The new syscalls allow a sender to send a message and block until it receives a reply, while the receiver blocks until a message arrives. All the send requests are queued for the receiver to recevie send requests in order of their arrival.

## Features
- **Blocking send (`pSend`)** – Sends a message and blocks until a reply is received.
- **Blocking receive (`pReceive`)** – Waits for a message and blocks until one arrives.
- **Reply (`pReply`)** – Unblocks a sender once the receiver has processed the message.
- **Senders count (`pMsgWaits`)** – returns number of waiting send requests for current process without blocking.

## Implementation Details
- **New Syscall Files:**
  - `ipc/block_send.c` – Implements the system calls.
  - `include/linux/block_send.h` – Header file for struct used in syscalls.
- **Kernel Modifications:**
  - `unistd.h` – Registers new syscalls.
  - syscall.tbl files for corresponding architecture.
  - `task_struct` modifications – Stores messages in process structures.
- **Synchronization:**
  - Spinlocks are used to prevent race conditions.
  - Processes are put to sleep and woken up as necessary.
- **Error Handling:**
  - Same error code with basic error message is used for now which will be updated to specific error codes providing a more precise error message.

## Installation & Compilation
1. Copy the required files to the Linux kernel source tree as described in the design document.
2. Modify the kernel’s Makefile to include `block_send.c`.
3. Compile the kernel:
   ```bash
   make menuconfig  # Configure the kernel (if needed)
   make -j$(nproc)  # Compile the kernel using all available CPU threads
   make modules
   make modules_install
   make install
   sudo reboot  # Reboot into the new kernel (make sure to update boot sequence if needed)
   ```

## Testing
- Run test programs that invoke `pSend`, `pReceive`, and `pReply` to validate the functionality (testing files block_tester_server.c and blocked_tester_client.c can be used for testing).
- Use `dmesg` to check kernel logs for syscall debugging messages.
- Test on both x86 (Ubuntu VM) and ARM (BeagleBone Black) architectures.

## Dependencies
- Linux kernel source (v5.10.168 or v5.10.168-ti-r71 for ARM)
- Cross-compilation toolchain for ARM builds (`arm-linux-gnueabihf-gcc`)
- QEMU (optional for ARM emulation)

## Contributing
- Fork the repository and create a feature branch.
- Follow kernel coding guidelines.
- Submit a pull request with detailed commit messages.
