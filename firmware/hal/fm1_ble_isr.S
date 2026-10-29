/* SPDX-License-Identifier: MPL-2.0 */
.section .text.isr_ble_event,"ax",@progbits
	.globl isr_ble_event
isr_ble_event:
	[--sp] = {psr, rets, reti}
	[--sp] = {r3-r0}
	call ble_wl82_event_irq
	{r3-r0} = [sp++]
	{psr, rets, reti} = [sp++]
	csync
	rti
	.previous
.section .text.isr_ble_rx,"ax",@progbits
	.globl isr_ble_rx
isr_ble_rx:
	[--sp] = {psr, rets, reti}
	[--sp] = {r3-r0}
	call ble_wl82_rx_irq
	{r3-r0} = [sp++]
	{psr, rets, reti} = [sp++]
	csync
	rti
	.previous
