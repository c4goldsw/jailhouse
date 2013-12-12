/*
 * Jailhouse, a Linux-based partitioning hypervisor
 *
 * Copyright (c) Siemens AG, 2013
 *
 * Authors:
 *  Jan Kiszka <jan.kiszka@siemens.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2.  See
 * the COPYING file in the top-level directory.
 */

#include <asm/types.h>
#include <asm/paging.h>
#include <asm/processor.h>

#include <jailhouse/cell-config.h>

/* VMCS Encodings */
enum vmcs_field {
	VIRTUAL_PROCESSOR_ID		= 0x00000000,
	GUEST_ES_SELECTOR		= 0x00000800,
	GUEST_CS_SELECTOR		= 0x00000802,
	GUEST_SS_SELECTOR		= 0x00000804,
	GUEST_DS_SELECTOR		= 0x00000806,
	GUEST_FS_SELECTOR		= 0x00000808,
	GUEST_GS_SELECTOR		= 0x0000080a,
	GUEST_LDTR_SELECTOR		= 0x0000080c,
	GUEST_TR_SELECTOR		= 0x0000080e,
	HOST_ES_SELECTOR		= 0x00000c00,
	HOST_CS_SELECTOR		= 0x00000c02,
	HOST_SS_SELECTOR		= 0x00000c04,
	HOST_DS_SELECTOR		= 0x00000c06,
	HOST_FS_SELECTOR		= 0x00000c08,
	HOST_GS_SELECTOR		= 0x00000c0a,
	HOST_TR_SELECTOR		= 0x00000c0c,
	IO_BITMAP_A			= 0x00002000,
	IO_BITMAP_A_HIGH		= 0x00002001,
	IO_BITMAP_B			= 0x00002002,
	IO_BITMAP_B_HIGH		= 0x00002003,
	MSR_BITMAP			= 0x00002004,
	MSR_BITMAP_HIGH			= 0x00002005,
	VM_EXIT_MSR_STORE_ADDR		= 0x00002006,
	VM_EXIT_MSR_STORE_ADDR_HIGH	= 0x00002007,
	VM_EXIT_MSR_LOAD_ADDR		= 0x00002008,
	VM_EXIT_MSR_LOAD_ADDR_HIGH	= 0x00002009,
	VM_ENTRY_MSR_LOAD_ADDR		= 0x0000200a,
	VM_ENTRY_MSR_LOAD_ADDR_HIGH	= 0x0000200b,
	TSC_OFFSET			= 0x00002010,
	TSC_OFFSET_HIGH			= 0x00002011,
	VIRTUAL_APIC_PAGE_ADDR		= 0x00002012,
	VIRTUAL_APIC_PAGE_ADDR_HIGH	= 0x00002013,
	APIC_ACCESS_ADDR		= 0x00002014,
	APIC_ACCESS_ADDR_HIGH		= 0x00002015,
	EPT_POINTER			= 0x0000201a,
	EPT_POINTER_HIGH		= 0x0000201b,
	GUEST_PHYSICAL_ADDRESS		= 0x00002400,
	GUEST_PHYSICAL_ADDRESS_HIGH	= 0x00002401,
	VMCS_LINK_POINTER		= 0x00002800,
	VMCS_LINK_POINTER_HIGH		= 0x00002801,
	GUEST_IA32_DEBUGCTL		= 0x00002802,
	GUEST_IA32_DEBUGCTL_HIGH	= 0x00002803,
	GUEST_IA32_PAT			= 0x00002804,
	GUEST_IA32_PAT_HIGH		= 0x00002805,
	GUEST_IA32_EFER			= 0x00002806,
	GUEST_IA32_EFER_HIGH		= 0x00002807,
	GUEST_IA32_PERF_GLOBAL_CTRL	= 0x00002808,
	GUEST_IA32_PERF_GLOBAL_CTRL_HIGH= 0x00002809,
	GUEST_PDPTR0			= 0x0000280a,
	GUEST_PDPTR0_HIGH		= 0x0000280b,
	GUEST_PDPTR1			= 0x0000280c,
	GUEST_PDPTR1_HIGH		= 0x0000280d,
	GUEST_PDPTR2			= 0x0000280e,
	GUEST_PDPTR2_HIGH		= 0x0000280f,
	GUEST_PDPTR3			= 0x00002810,
	GUEST_PDPTR3_HIGH		= 0x00002811,
	HOST_IA32_PAT			= 0x00002c00,
	HOST_IA32_PAT_HIGH		= 0x00002c01,
	HOST_IA32_EFER			= 0x00002c02,
	HOST_IA32_EFER_HIGH		= 0x00002c03,
	HOST_IA32_PERF_GLOBAL_CTRL	= 0x00002c04,
	HOST_IA32_PERF_GLOBAL_CTRL_HIGH	= 0x00002c05,
	PIN_BASED_VM_EXEC_CONTROL	= 0x00004000,
	CPU_BASED_VM_EXEC_CONTROL	= 0x00004002,
	EXCEPTION_BITMAP		= 0x00004004,
	PAGE_FAULT_ERROR_CODE_MASK	= 0x00004006,
	PAGE_FAULT_ERROR_CODE_MATCH	= 0x00004008,
	CR3_TARGET_COUNT		= 0x0000400a,
	VM_EXIT_CONTROLS		= 0x0000400c,
	VM_EXIT_MSR_STORE_COUNT		= 0x0000400e,
	VM_EXIT_MSR_LOAD_COUNT		= 0x00004010,
	VM_ENTRY_CONTROLS		= 0x00004012,
	VM_ENTRY_MSR_LOAD_COUNT		= 0x00004014,
	VM_ENTRY_INTR_INFO_FIELD	= 0x00004016,
	VM_ENTRY_EXCEPTION_ERROR_CODE	= 0x00004018,
	VM_ENTRY_INSTRUCTION_LEN	= 0x0000401a,
	TPR_THRESHOLD			= 0x0000401c,
	SECONDARY_VM_EXEC_CONTROL	= 0x0000401e,
	PLE_GAP				= 0x00004020,
	PLE_WINDOW			= 0x00004022,
	VM_INSTRUCTION_ERROR		= 0x00004400,
	VM_EXIT_REASON			= 0x00004402,
	VM_EXIT_INTR_INFO		= 0x00004404,
	VM_EXIT_INTR_ERROR_CODE		= 0x00004406,
	IDT_VECTORING_INFO_FIELD	= 0x00004408,
	IDT_VECTORING_ERROR_CODE	= 0x0000440a,
	VM_EXIT_INSTRUCTION_LEN		= 0x0000440c,
	VMX_INSTRUCTION_INFO		= 0x0000440e,
	GUEST_ES_LIMIT			= 0x00004800,
	GUEST_CS_LIMIT			= 0x00004802,
	GUEST_SS_LIMIT			= 0x00004804,
	GUEST_DS_LIMIT			= 0x00004806,
	GUEST_FS_LIMIT			= 0x00004808,
	GUEST_GS_LIMIT			= 0x0000480a,
	GUEST_LDTR_LIMIT		= 0x0000480c,
	GUEST_TR_LIMIT			= 0x0000480e,
	GUEST_GDTR_LIMIT		= 0x00004810,
	GUEST_IDTR_LIMIT		= 0x00004812,
	GUEST_ES_AR_BYTES		= 0x00004814,
	GUEST_CS_AR_BYTES		= 0x00004816,
	GUEST_SS_AR_BYTES		= 0x00004818,
	GUEST_DS_AR_BYTES		= 0x0000481a,
	GUEST_FS_AR_BYTES		= 0x0000481c,
	GUEST_GS_AR_BYTES		= 0x0000481e,
	GUEST_LDTR_AR_BYTES		= 0x00004820,
	GUEST_TR_AR_BYTES		= 0x00004822,
	GUEST_INTERRUPTIBILITY_INFO	= 0x00004824,
	GUEST_ACTIVITY_STATE		= 0x00004826,
	GUEST_SYSENTER_CS		= 0x0000482A,
	VMX_PREEMPTION_TIMER_VALUE	= 0x0000482E,
	HOST_IA32_SYSENTER_CS		= 0x00004c00,
	CR0_GUEST_HOST_MASK		= 0x00006000,
	CR4_GUEST_HOST_MASK		= 0x00006002,
	CR0_READ_SHADOW			= 0x00006004,
	CR4_READ_SHADOW			= 0x00006006,
	CR3_TARGET_VALUE0		= 0x00006008,
	CR3_TARGET_VALUE1		= 0x0000600a,
	CR3_TARGET_VALUE2		= 0x0000600c,
	CR3_TARGET_VALUE3		= 0x0000600e,
	EXIT_QUALIFICATION		= 0x00006400,
	GUEST_LINEAR_ADDRESS		= 0x0000640a,
	GUEST_CR0			= 0x00006800,
	GUEST_CR3			= 0x00006802,
	GUEST_CR4			= 0x00006804,
	GUEST_ES_BASE			= 0x00006806,
	GUEST_CS_BASE			= 0x00006808,
	GUEST_SS_BASE			= 0x0000680a,
	GUEST_DS_BASE			= 0x0000680c,
	GUEST_FS_BASE			= 0x0000680e,
	GUEST_GS_BASE			= 0x00006810,
	GUEST_LDTR_BASE			= 0x00006812,
	GUEST_TR_BASE			= 0x00006814,
	GUEST_GDTR_BASE			= 0x00006816,
	GUEST_IDTR_BASE			= 0x00006818,
	GUEST_DR7			= 0x0000681a,
	GUEST_RSP			= 0x0000681c,
	GUEST_RIP			= 0x0000681e,
	GUEST_RFLAGS			= 0x00006820,
	GUEST_PENDING_DBG_EXCEPTIONS	= 0x00006822,
	GUEST_SYSENTER_ESP		= 0x00006824,
	GUEST_SYSENTER_EIP		= 0x00006826,
	HOST_CR0			= 0x00006c00,
	HOST_CR3			= 0x00006c02,
	HOST_CR4			= 0x00006c04,
	HOST_FS_BASE			= 0x00006c06,
	HOST_GS_BASE			= 0x00006c08,
	HOST_TR_BASE			= 0x00006c0a,
	HOST_GDTR_BASE			= 0x00006c0c,
	HOST_IDTR_BASE			= 0x00006c0e,
	HOST_IA32_SYSENTER_ESP		= 0x00006c10,
	HOST_IA32_SYSENTER_EIP		= 0x00006c12,
	HOST_RSP			= 0x00006c14,
	HOST_RIP			= 0x00006c16,
};

#define GUEST_SEG_LIMIT			(GUEST_ES_LIMIT - GUEST_ES_SELECTOR)
#define GUEST_SEG_AR_BYTES		(GUEST_ES_AR_BYTES - GUEST_ES_SELECTOR)
#define GUEST_SEG_BASE			(GUEST_ES_BASE - GUEST_ES_SELECTOR)

#define GUEST_ACTIVITY_ACTIVE			0
#define GUEST_ACTIVITY_HLT			1

#define VMX_MSR_BITMAP_0000_READ		0
#define VMX_MSR_BITMAP_C000_READ		1
#define VMX_MSR_BITMAP_0000_WRITE		2
#define VMX_MSR_BITMAP_C000_WRITE		3

#define PIN_BASED_NMI_EXITING			0x00000008
#define PIN_BASED_VMX_PREEMPTION_TIMER		0x00000040

#define CPU_BASED_USE_IO_BITMAPS		0x02000000
#define CPU_BASED_USE_MSR_BITMAPS		0x10000000
#define CPU_BASED_ACTIVATE_SECONDARY_CONTROLS	0x80000000

#define SECONDARY_EXEC_VIRTUALIZE_APIC_ACCESSES	0x00000001
#define SECONDARY_EXEC_ENABLE_EPT		0x00000002
#define SECONDARY_EXEC_UNRESTRICTED_GUEST	0x00000080

#define VM_EXIT_HOST_ADDR_SPACE_SIZE		0x00000200
#define VM_EXIT_SAVE_IA32_EFER			0x00100000
#define VM_EXIT_LOAD_IA32_EFER			0x00200000

#define VM_ENTRY_IA32E_MODE			0x00000200
#define VM_ENTRY_LOAD_IA32_EFER			0x00008000

#define VMX_MISC_ACTIVITY_HLT			0x00000040

#define INTR_INFO_UNBLOCK_NMI			0x1000

#define EXIT_REASONS_FAILED_VMENTRY		0x80000000

#define EXIT_REASON_EXCEPTION_NMI		0
#define EXIT_REASON_EXTERNAL_INTERRUPT		1
#define EXIT_REASON_TRIPLE_FAULT		2
#define EXIT_REASON_INIT_SIGNAL			3
#define EXIT_REASON_SIPI			4
#define EXIT_REASON_IO_SMI			5
#define EXIT_REASON_OTHER_SMI			6
#define EXIT_REASON_PENDING_INTERRUPT		7
#define EXIT_REASON_NMI_WINDOW			8
#define EXIT_REASON_TASK_SWITCH			9
#define EXIT_REASON_CPUID			10
#define EXIT_REASON_HLT				12
#define EXIT_REASON_INVD			13
#define EXIT_REASON_INVLPG			14
#define EXIT_REASON_RDPMC			15
#define EXIT_REASON_RDTSC			16
#define EXIT_REASON_VMCALL			18
#define EXIT_REASON_VMCLEAR			19
#define EXIT_REASON_VMLAUNCH			20
#define EXIT_REASON_VMPTRLD			21
#define EXIT_REASON_VMPTRST			22
#define EXIT_REASON_VMREAD			23
#define EXIT_REASON_VMRESUME			24
#define EXIT_REASON_VMWRITE			25
#define EXIT_REASON_VMXOFF			26
#define EXIT_REASON_VMXON			27
#define EXIT_REASON_CR_ACCESS			28
#define EXIT_REASON_DR_ACCESS			29
#define EXIT_REASON_IO_INSTRUCTION		30
#define EXIT_REASON_MSR_READ			31
#define EXIT_REASON_MSR_WRITE			32
#define EXIT_REASON_INVALID_STATE		33
#define EXIT_REASON_MWAIT_INSTRUCTION		36
#define EXIT_REASON_MONITOR_INSTRUCTION		39
#define EXIT_REASON_PAUSE_INSTRUCTION		40
#define EXIT_REASON_MCE_DURING_VMENTRY		41
#define EXIT_REASON_TPR_BELOW_THRESHOLD		43
#define EXIT_REASON_APIC_ACCESS			44
#define EXIT_REASON_EPT_VIOLATION		48
#define EXIT_REASON_EPT_MISCONFIG		49
#define EXIT_REASON_INVEPT			50
#define EXIT_REASON_PREEMPTION_TIMER		52
#define EXIT_REASON_WBINVD			54
#define EXIT_REASON_XSETBV			55
#define EXIT_REASON_INVPCID			58

#define EPT_FLAG_READ				0x001
#define EPT_FLAG_WRITE				0x002
#define EPT_FLAG_EXECUTE			0x004
#define EPT_FLAG_WB_TYPE			0x030

#define EPT_TYPE_UNCACHEABLE			0
#define EPT_TYPE_WRITEBACK			6
#define EPT_PAGE_WALK_LEN			((4-1) << 3)

#define EPT_PAGE_WALK_4				(1UL << 6)
#define EPTP_WB					(1UL << 14)
#define EPT_INVEPT				(1UL << 20)
#define EPT_INVEPT_SINGLE			(1UL << 25)
#define EPT_INVEPT_GLOBAL			(1UL << 26)
#define EPT_MANDATORY_FEATURES			(EPT_PAGE_WALK_4 | EPTP_WB | \
						 EPT_INVEPT)

#define VMX_INVEPT_SINGLE			1
#define VMX_INVEPT_GLOBAL			2

#define APIC_ACCESS_OFFET_MASK			0x00000fff
#define APIC_ACCESS_TYPE_MASK			0x0000f000
#define APIC_ACCESS_TYPE_LINEAR_READ		0x00000000
#define APIC_ACCESS_TYPE_LINEAR_WRITE		0x00001000

void vmx_init(void);

int vmx_cell_init(struct cell *cell);
void vmx_cell_shrink(struct cell *cell, struct jailhouse_cell_desc *config);
void vmx_cell_exit(struct cell *cell);

int vmx_cpu_init(struct per_cpu *cpu_data);
void vmx_cpu_exit(struct per_cpu *cpu_data);

void __attribute__((noreturn)) vmx_cpu_activate_vmm(struct per_cpu *cpu_data);
void vmx_handle_exit(struct registers *guest_regs, struct per_cpu *cpu_data);
void vmx_entry_failure(struct per_cpu *cpu_data);

void vmx_invept(void);

void vmx_schedule_vmexit(struct per_cpu *cpu_data);
void vmx_cpu_park(void);
