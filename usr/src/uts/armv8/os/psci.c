/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */
/*
 * Copyright 2017 Hayashi Naoyuki
 * Copyright 2009 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */
#include <sys/types.h>
#include <sys/psci.h>
#include <sys/promif.h>
#include <sys/ddi.h>
#include <sys/sunddi.h>

static uint32_t pcsi_version_id = 0x84000000;
static uint32_t psci_cpu_suspend_id = 0xc4000001;
static uint32_t psci_cpu_off_id = 0x84000002;
static uint32_t psci_cpu_on_id = 0xc4000003;
static uint32_t psci_affinity_info_id = 0xc4000004;
static uint32_t psci_migrate_id = 0xc4000005;
static uint32_t psci_migrate_info_type_id = 0x84000006;
static uint32_t psci_migrate_info_up_cpu_id = 0xc4000007;
static uint32_t psci_system_off_id = 0x84000008;
static uint32_t psci_system_reset_id = 0x84000009;
static uint32_t psci_features_id = 0x8400000a;
static uint32_t psci_cpu_freeze_id = 0x8400000b;
static uint32_t psci_cpu_default_suspend_id = 0xc400000c;
static uint32_t psci_node_hw_state_id = 0xc400000d;
static uint32_t psci_system_suspend_id = 0xc400000e;
static uint32_t psci_set_suspend_mode_id = 0x8400000f;
static uint32_t psci_stat_residency_id = 0xc4000010;
static uint32_t psci_stat_count_id = 0xc4000011;
static boolean_t pcsi_method_is_hvc = B_FALSE;

static inline uint64_t
psci_smc64(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3)
{
	register uint64_t x0 __asm__ ("x0") = a0;
	register uint64_t x1 __asm__ ("x1") = a1;
	register uint64_t x2 __asm__ ("x2") = a2;
	register uint64_t x3 __asm__ ("x3") = a3;

	__asm__ volatile ("smc #0"
	    : "+r"(x0), "+r"(x1), "+r"(x2), "+r"(x3)
	    :
	    :
	    "x4", "x5", "x6", "x7", "x8", "x9", "x10", "x11",
	    "x12", "x13", "x14", "x15", "x16", "x17", "x18", "memory", "cc");

	return x0;
}

static inline uint64_t
psci_hvc64(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3)
{
	register uint64_t x0 __asm__ ("x0") = a0;
	register uint64_t x1 __asm__ ("x1") = a1;
	register uint64_t x2 __asm__ ("x2") = a2;
	register uint64_t x3 __asm__ ("x3") = a3;

	__asm__ volatile ("hvc #0"
	    : "+r"(x0), "+r"(x1), "+r"(x2), "+r"(x3)
	    :
	    :
	    "x4", "x5", "x6", "x7", "x8", "x9", "x10", "x11",
	    "x12", "x13", "x14", "x15", "x16", "x17", "x18", "memory", "cc");

	return x0;
}

static inline uint64_t
psci_call(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3)
{
	if (pcsi_method_is_hvc)
		return psci_hvc64(a0, a1, a2, a3);
	else
		return psci_smc64(a0, a1, a2, a3);
}

static void
find_psci(pnode_t node, void *arg)
{
	if (!prom_is_compatible(node, "arm,psci"))
		return;
	*(pnode_t *)arg = node;
}

void
psci_init(void)
{
	pnode_t node = 0;
	prom_walk(find_psci, &node);
	if (node > 0) {
		int len;
		len = prom_getproplen(node, "method");
		if (len > 0) {
			char *method = __builtin_alloca(len + 1);
			memset(method, 0, len + 1);
			prom_getprop(node, "method", method);
			if (strcmp(method, "hvc") == 0)
				pcsi_method_is_hvc = B_TRUE;
		}
		psci_cpu_suspend_id = (uint32_t)prom_get_prop_int(node, "cpu_suspend", (int)psci_cpu_suspend_id);
		psci_cpu_off_id = (uint32_t)prom_get_prop_int(node, "cpu_off", (int)psci_cpu_off_id);
		psci_cpu_on_id = (uint32_t)prom_get_prop_int(node, "cpu_on", (int)psci_cpu_on_id);
		psci_migrate_id = (uint32_t)prom_get_prop_int(node, "migrate", (int)psci_migrate_id);
	}
}

uint32_t
psci_version(void)
{
	return psci_call(pcsi_version_id, 0, 0, 0);
}

int32_t
psci_cpu_suspend(uint32_t power_state, uint64_t entry_point_address, uint64_t context_id)
{
	return psci_call(psci_cpu_suspend_id, power_state, entry_point_address, context_id);
}

int32_t
psci_cpu_off(void)
{
	return psci_call(psci_cpu_off_id, 0, 0, 0);
}

int32_t
psci_cpu_on(uint64_t target_cpu, uint64_t entry_point_address, uint64_t context_id)
{
	return psci_call(psci_cpu_on_id, target_cpu, entry_point_address, context_id);
}

int32_t
psci_affinity_info(uint64_t target_affinity, uint32_t lowest_affinity_level)
{
	return psci_call(psci_affinity_info_id, target_affinity, lowest_affinity_level, 0);
}

int32_t
psci_migrate(uint64_t target_cpu)
{
	return psci_call(psci_migrate_id, target_cpu, 0, 0);
}

int32_t
psci_migrate_info_type(void)
{
	return psci_call(psci_migrate_info_type_id, 0, 0, 0);
}

uint64_t
psci_migrate_info_up_cpu(void)
{
	return psci_call(psci_migrate_info_up_cpu_id, 0, 0, 0);
}

void
psci_system_off(void)
{
	psci_call(psci_system_off_id, 0, 0, 0);
}

void
psci_system_reset(void)
{
	psci_call(psci_system_reset_id, 0, 0, 0);
}

int32_t
psci_features(uint32_t psci_func_id)
{
	return psci_call(psci_features_id, psci_func_id, 0, 0);
}

int32_t
psci_cpu_freeze(void)
{
	return psci_call(psci_cpu_freeze_id, 0, 0, 0);
}

int32_t
psci_cpu_default_suspend(uint64_t entry_point_address, uint64_t context_id)
{
	return psci_call(psci_cpu_default_suspend_id, entry_point_address, context_id, 0);
}

int32_t
psci_node_hw_state(uint64_t target_cpu, uint32_t power_level)
{
	return psci_call(psci_node_hw_state_id, target_cpu, power_level, 0);
}

int32_t
psci_system_suspend(uint64_t entry_point_address, uint64_t context_id)
{
	return psci_call(psci_system_suspend_id, entry_point_address, context_id, 0);
}

int32_t
psci_set_suspend_mode(uint32_t mode)
{
	return psci_call(psci_set_suspend_mode_id, mode, 0, 0);
}

uint64_t
psci_stat_residency(uint64_t target_cpu, uint32_t power_state)
{
	return psci_call(psci_stat_residency_id, target_cpu, power_state, 0);
}

uint64_t
psci_stat_count(uint64_t target_cpu, uint32_t power_state)
{
	return psci_call(psci_stat_count_id, target_cpu, power_state, 0);
}
