/*
 * This file and its contents are supplied under the terms of the
 * Common Development and Distribution License ("CDDL"), version 1.0.
 * You may only use this file in accordance with the terms of version
 * 1.0 of the CDDL.
 *
 * A full copy of the text of the CDDL should have accompanied this
 * source.  A copy of the CDDL is also available via the Internet at
 * http://www.illumos.org/license/CDDL.
 */

/*
 * Copyright 2019 Joyent, Inc.
 * Copyright 2020 Hayashi Naoyuki
 */

/*
 * MMIO-based virtio
 */

#include <sys/ddi.h>
#include <sys/pci.h>
#include <sys/sunddi.h>
#include <sys/sysmacros.h>

#include "virtio.h"
#include "virtio_impl.h"

/*
 * XXXARM: These figures are different on the PCI side, there's no equivalent
 * to QUEUE_MAX for legacy devices , and alignment is to a 4K page
 */

/* Desired maximum queue size */
#define QUEUE_MAX	1024
/* Required queue alignment  */
#define QUEUE_ALIGN	64

/*
 * Early device initialisation for MMIO virtio devices.
 */
virtio_t *
virtio_init(dev_info_t *dip, uint64_t driver_features, boolean_t allow_indirect)
{
	int r;

	virtio_t *vio = kmem_zalloc(sizeof (*vio), KM_SLEEP);
	vio->vio_dip = dip;

	/*
	 * Map register access.
	 */
	if ((r = ddi_regs_map_setup(dip, 0,
	    (caddr_t *)&vio->vio_bar, 0, 0, &virtio_acc_attr,
	    &vio->vio_barh)) != DDI_SUCCESS) {
		dev_err(dip, CE_WARN, "ddi_regs_map_setup failure (%d)", r);
		kmem_free(vio, sizeof (*vio));
		return (NULL);
	}
	vio->vio_initlevel |= VIRTIO_INITLEVEL_REGS;

	ASSERT(virtio_get32(vio, VIRTIO_MMIO_MAGIC_VALUE) == 0x74726976);
	ASSERT(virtio_get32(vio, VIRTIO_MMIO_VERSION) == 0x1);

	/*
	 * We initialise the mutex without an interrupt priority to ease the
	 * implementation of some of the configuration space access routines.
	 * Drivers using the virtio framework MUST make a call to
	 * "virtio_init_complete()" prior to spawning other threads or enabling
	 * interrupt handlers, at which time we will destroy and reinitialise
	 * the mutex for use in our interrupt handlers.
	 */
	mutex_init(&vio->vio_mutex, NULL, MUTEX_DRIVER, NULL);

	list_create(&vio->vio_queues, sizeof (virtio_queue_t),
	    offsetof(virtio_queue_t, viq_link));

	/*
	 * Legacy virtio devices require a few common steps before we can
	 * negotiate device features.
	 */
	virtio_device_reset(vio);
	virtio_set_status(vio, VIRTIO_STATUS_ACKNOWLEDGE);
	virtio_set_status(vio, VIRTIO_STATUS_DRIVER);

	/*
	 * Negotiate features with the device.  Record the original supported
	 * feature set for debugging purposes.
	 */
	virtio_put32(vio, VIRTIO_MMIO_HOST_FEATURES_SEL, 0);
	vio->vio_features_device = virtio_get32(vio,
	    VIRTIO_MMIO_HOST_FEATURES);
	if (allow_indirect) {
		driver_features |= VIRTIO_F_RING_INDIRECT_DESC;
	}
	vio->vio_features = vio->vio_features_device & driver_features;
	virtio_put32(vio, VIRTIO_MMIO_GUEST_FEATURES_SEL, 0);
	virtio_put32(vio, VIRTIO_MMIO_GUEST_FEATURES, vio->vio_features);

	/*
	 * The device-specific configuration begins at an offset into the BAR
	 * that depends on whether we have enabled MSI-X interrupts or not.
	 * Start out with the offset for pre-MSI-X operation so that we can
	 * read device configuration space prior to configuring interrupts.
	 */
	vio->vio_config_offset = VIRTIO_MMIO_CONFIG;

	return (vio);
}

/*
 * Enable a bit in the device status register.  Each bit signals a level of
 * guest readiness to the host.  Use the VIRTIO_CONFIG_DEVICE_STATUS_*
 * constants for "status".  To zero the status field use virtio_device_reset().
 */
void
virtio_set_status(virtio_t *vio, uint8_t status)
{
	VERIFY3U(status, !=, 0);

	mutex_enter(&vio->vio_mutex);

	uint8_t old = virtio_get32(vio, VIRTIO_MMIO_STATUS);
	virtio_put32(vio, VIRTIO_MMIO_STATUS, status | old);

	mutex_exit(&vio->vio_mutex);
}

void
virtio_device_reset_locked(virtio_t *vio)
{
	virtio_put32(vio, VIRTIO_MMIO_STATUS, VIRTIO_STATUS_RESET);
}

virtio_queue_t *
virtio_queue_alloc(virtio_t *vio, uint16_t qidx, const char *name,
    ddi_intr_handler_t *func, void *funcarg, boolean_t force_direct,
    uint_t max_segs)
{
	uint16_t qsz;
	char space_name[256];

	if (max_segs < 1) {
		/*
		 * Every descriptor, direct or indirect, needs to refer to at
		 * least one buffer.
		 */
		dev_err(vio->vio_dip, CE_WARN, "queue \"%s\" (%u) "
		    "segment count must be at least 1", name, (uint_t)qidx);
		return (NULL);
	}

	mutex_enter(&vio->vio_mutex);

	if (vio->vio_initlevel & VIRTIO_INITLEVEL_PROVIDER) {
		/*
		 * Cannot configure any more queues once initial setup is
		 * complete and interrupts have been allocated.
		 */
		dev_err(vio->vio_dip, CE_WARN, "queue \"%s\" (%u) "
		    "alloc after init complete", name, (uint_t)qidx);
		mutex_exit(&vio->vio_mutex);
		return (NULL);
	}

	/*
	 * There is no way to negotiate a different queue size for legacy
	 * devices.  We must read and use the native queue size of the device.
	 */
	virtio_put32(vio, VIRTIO_MMIO_QUEUE_SEL, qidx);
	virtio_put32(vio, VIRTIO_MMIO_QUEUE_ALIGN, QUEUE_ALIGN);
	qsz = virtio_get32(vio, VIRTIO_MMIO_QUEUE_NUM_MAX);
	qsz = MIN(qsz, QUEUE_MAX);
	if (qsz == 0) {
		/*
		 * A size of zero means the device does not have a queue with
		 * this index.
		 */
		dev_err(vio->vio_dip, CE_WARN, "queue \"%s\" (%u) "
		    "does not exist on device", name, (uint_t)qidx);
		mutex_exit(&vio->vio_mutex);
		return (NULL);
	}
	virtio_put32(vio, VIRTIO_MMIO_QUEUE_NUM, qsz);

	mutex_exit(&vio->vio_mutex);

	virtio_queue_t *viq = kmem_zalloc(sizeof (*viq), KM_SLEEP);
	viq->viq_virtio = vio;
	viq->viq_name = name;
	viq->viq_index = qidx;
	viq->viq_size = qsz;
	viq->viq_func = func;
	viq->viq_funcarg = funcarg;
	viq->viq_max_segs = max_segs;
	avl_create(&viq->viq_inflight, virtio_inflight_compar,
	    sizeof (virtio_chain_t), offsetof(virtio_chain_t, vic_node));

	/*
	 * Allocate the mutex without an interrupt priority for now, as we do
	 * with "vio_mutex".  We'll reinitialise it in
	 * "virtio_init_complete()".
	 */
	mutex_init(&viq->viq_mutex, NULL, MUTEX_DRIVER, NULL);

	if (virtio_feature_present(vio, VIRTIO_F_RING_INDIRECT_DESC) &&
	    !force_direct) {
		/*
		 * If we were able to negotiate the indirect descriptor
		 * feature, and the caller has not explicitly forced the use of
		 * direct descriptors, we'll allocate indirect descriptor lists
		 * for each chain.
		 */
		viq->viq_indirect = B_TRUE;
	}

	/*
	 * Track descriptor usage in an identifier space.
	 */
	(void) snprintf(space_name, sizeof (space_name), "%s%d_vq_%s",
	    ddi_get_name(vio->vio_dip), ddi_get_instance(vio->vio_dip), name);
	if ((viq->viq_descmap = id_space_create(space_name, 0, qsz)) == NULL) {
		dev_err(vio->vio_dip, CE_WARN, "could not allocate descriptor "
		    "ID space");
		virtio_queue_free(viq);
		return (NULL);
	}

	/*
	 * For legacy devices, memory for the queue has a strict layout
	 * determined by the queue size.
	 */
	size_t sz_descs = sizeof (virtio_vq_desc_t) * qsz;
	size_t sz_driver = P2ROUNDUP_TYPED(sz_descs +
	    sizeof (virtio_vq_driver_t) +
	    sizeof (uint16_t) * qsz,
	    QUEUE_ALIGN, size_t);
	size_t sz_device = P2ROUNDUP_TYPED(sizeof (virtio_vq_device_t) +
	    sizeof (virtio_vq_elem_t) * qsz,
	    QUEUE_ALIGN, size_t);

	if (virtio_dma_init(vio, &viq->viq_dma, sz_driver + sz_device,
	    &virtio_dma_attr_queue, DDI_DMA_RDWR | DDI_DMA_CONSISTENT,
	    KM_SLEEP) != DDI_SUCCESS) {
		dev_err(vio->vio_dip, CE_WARN, "could not allocate queue "
		    "DMA memory");
		virtio_queue_free(viq);
		return (NULL);
	}

	ASSERT(((uintptr_t)virtio_dma_va(&viq->viq_dma, 0) &
	    (QUEUE_ALIGN - 1)) == 0);

	/*
	 * NOTE: The viq_dma_* members below are used by
	 * VIRTQ_DMA_SYNC_FORDEV() and VIRTQ_DMA_SYNC_FORKERNEL() to calculate
	 * offsets into the DMA allocation for partial synchronisation.  If the
	 * ordering of, or relationship between, these pointers changes, the
	 * macros must be kept in sync.
	 */
	viq->viq_dma_descs = virtio_dma_va(&viq->viq_dma, 0);
	viq->viq_dma_driver = virtio_dma_va(&viq->viq_dma, sz_descs);
	viq->viq_dma_device = virtio_dma_va(&viq->viq_dma, sz_driver);

	/*
	 * Install in the per-device list of queues.
	 */
	mutex_enter(&vio->vio_mutex);
	for (virtio_queue_t *chkvq = list_head(&vio->vio_queues); chkvq != NULL;
	    chkvq = list_next(&vio->vio_queues, chkvq)) {
		if (chkvq->viq_index == qidx) {
			dev_err(vio->vio_dip, CE_WARN, "attempt to register "
			    "queue \"%s\" with same index (%d) as queue \"%s\"",
			    name, qidx, chkvq->viq_name);
			mutex_exit(&vio->vio_mutex);
			virtio_queue_free(viq);
			return (NULL);
		}
	}
	list_insert_tail(&vio->vio_queues, viq);

	/*
	 * Ensure the zeroing of the queue memory is visible to the host before
	 * we inform the device of the queue address.
	 */
	membar_producer();
	VIRTQ_DMA_SYNC_FORDEV(viq);

	virtio_put32(vio, VIRTIO_MMIO_QUEUE_SEL, qidx);
	virtio_put32(vio, VIRTIO_MMIO_GUEST_PAGE_SIZE, 1 << VIRTIO_PAGE_SHIFT);
	virtio_put32(vio, VIRTIO_MMIO_QUEUE_PFN,
	    virtio_dma_cookie_pa(&viq->viq_dma, 0) >> VIRTIO_PAGE_SHIFT);

	mutex_exit(&vio->vio_mutex);
	return (viq);
}

void
virtio_queue_free(virtio_queue_t *viq)
{
	virtio_t *vio = viq->viq_virtio;

	/*
	 * We are going to destroy the queue mutex.  Make sure we've already
	 * removed the interrupt handlers.
	 */
	VERIFY(!(vio->vio_initlevel & VIRTIO_INITLEVEL_INT_ADDED));

	mutex_enter(&viq->viq_mutex);

	/*
	 * If the device has not already been reset as part of a shutdown,
	 * detach the queue from the device now.
	 */
	if (!viq->viq_shutdown) {
		virtio_put32(vio, VIRTIO_MMIO_QUEUE_SEL, viq->viq_index);
		virtio_put32(vio, VIRTIO_MMIO_QUEUE_PFN, 0);
	}

	virtio_dma_fini(&viq->viq_dma);

	VERIFY(avl_is_empty(&viq->viq_inflight));
	avl_destroy(&viq->viq_inflight);
	if (viq->viq_descmap != NULL) {
		id_space_destroy(viq->viq_descmap);
	}

	mutex_exit(&viq->viq_mutex);
	mutex_destroy(&viq->viq_mutex);

	kmem_free(viq, sizeof (*viq));
}

void
virtio_queue_flush_locked(virtio_queue_t *viq)
{
	VERIFY(MUTEX_HELD(&viq->viq_mutex));

	/*
	 * Make sure any writes we have just made to the descriptors
	 * (vqdr_ring[]) are visible to the device before we update the ring
	 * pointer (vqdr_index).
	 */
	membar_producer();
	viq->viq_dma_driver->vqdr_index = viq->viq_driver_index;
	VIRTQ_DMA_SYNC_FORDEV(viq);

	/*
	 * Determine whether the device expects us to notify it of new
	 * descriptors.
	 */
	VIRTQ_DMA_SYNC_FORKERNEL(viq);
	if (!(viq->viq_dma_device->vqde_flags & VIRTQ_USED_F_NO_NOTIFY)) {
		virtio_put32(viq->viq_virtio, VIRTIO_MMIO_QUEUE_NOTIFY,
		    viq->viq_index);
	}
}

uint_t
virtio_shared_isr(caddr_t arg0, caddr_t arg1)
{
	virtio_t *vio = (virtio_t *)arg0;
	uint_t r = DDI_INTR_UNCLAIMED;
	uint32_t isr;

	mutex_enter(&vio->vio_mutex);

	/*
	 * Check the ISR status to see if the interrupt applies to us.  Reading
	 * this field resets it to zero.
	 */
	isr = virtio_get32(vio, VIRTIO_MMIO_INTERRUPT_STATUS);
	virtio_put32(vio, VIRTIO_MMIO_INTERRUPT_ACK, isr);
	if ((isr & VIRTIO_ISR_CHECK_QUEUES) == 0) {
		goto done;
	}

	for (virtio_queue_t *viq = list_head(&vio->vio_queues); viq != NULL;
	    viq = list_next(&vio->vio_queues, viq)) {
		if (viq->viq_func != NULL) {
			mutex_exit(&vio->vio_mutex);
			if (viq->viq_func(viq->viq_funcarg, arg0) ==
			    DDI_INTR_CLAIMED) {
				r = DDI_INTR_CLAIMED;
			}
			mutex_enter(&vio->vio_mutex);

			if (vio->vio_initlevel & VIRTIO_INITLEVEL_SHUTDOWN) {
				/*
				 * The device was shut down while in a queue
				 * handler routine.
				 */
				goto done;
			}
		}
	}

done:
	mutex_exit(&vio->vio_mutex);
	return (r);
}


void
virtio_interrupts_unwind(virtio_t *vio)
{
	VERIFY(MUTEX_HELD(&vio->vio_mutex));

	if (vio->vio_interrupt_type == DDI_INTR_TYPE_MSIX) {
		for (virtio_queue_t *viq = list_head(&vio->vio_queues);
		    viq != NULL; viq = list_next(&vio->vio_queues, viq)) {
			if (!viq->viq_handler_added) {
				continue;
			}

			virtio_put16(vio, VIRTIO_LEGACY_QUEUE_SELECT,
			    viq->viq_index);
			virtio_put16(vio, VIRTIO_LEGACY_MSIX_QUEUE,
			    VIRTIO_LEGACY_MSI_NO_VECTOR);
		}

		if (vio->vio_cfgchange_handler_added) {
			virtio_put16(vio, VIRTIO_LEGACY_MSIX_CONFIG,
			    VIRTIO_LEGACY_MSI_NO_VECTOR);
		}
	}

	if (vio->vio_interrupt_cap & DDI_INTR_FLAG_BLOCK) {
		(void) ddi_intr_block_disable(vio->vio_interrupts,
		    vio->vio_ninterrupts);
	} else {
		for (int i = 0; i < vio->vio_ninterrupts; i++) {
			(void) ddi_intr_disable(vio->vio_interrupts[i]);
		}
	}

	/*
	 * Disabling the interrupts makes the MSI-X fields disappear from the
	 * BAR once more.
	 */
	vio->vio_config_offset = VIRTIO_MMIO_CONFIG;
}
