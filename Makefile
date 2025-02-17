# SPDX-License-Identifier: GPL-2.0
#
# Makefile for the linux ipc.
#

obj-$(CONFIG_SYSVIPC_COMPAT) += compat.o
#/* CMPT 432/832 CHANGE BY dns682, wde364, 2025, 02, 02*/
obj-$(CONFIG_SYSVIPC) += util.o msgutil.o msg.o sem.o shm.o syscall.o \
block_send.o
#/* end CMPT 432/832 CHANGE BY dns682, wde364, 2025, 02, 02*/
obj-$(CONFIG_SYSVIPC_SYSCTL) += ipc_sysctl.o
obj-$(CONFIG_POSIX_MQUEUE) += mqueue.o msgutil.o
obj-$(CONFIG_IPC_NS) += namespace.o
obj-$(CONFIG_POSIX_MQUEUE_SYSCTL) += mq_sysctl.o

