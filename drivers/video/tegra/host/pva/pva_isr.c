/*
 * PVA ISR code for T194
 *
 * Copyright (c) 2016, NVIDIA Corporation.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/irq.h>
#include <linux/slab.h>
#include <linux/wait.h>

#include "bus_client.h"
#include "pva_regs.h"
#include "dev.h"
#include "pva.h"

static irqreturn_t pva_isr(int irq, void *dev_id)
{
	struct pva *pva = dev_id;
	struct platform_device *pdev = pva->pdev;
	u32 status7 = host1x_readl(pdev, hsp_sm7_r());
	u32 status6 = host1x_readl(pdev, hsp_sm6_r());
	u32 status5 = host1x_readl(pdev, hsp_sm5_r());

	if (status5 & PVA_AISR_INT_PENDING) {
		nvhost_dbg_info("PVA AISR (%x)", status7);

		/* For now, just log the errors */

		if (status5 & PVA_AISR_TASK_ERROR)
			nvhost_warn(&pdev->dev, "PVA AISR: PVA_AISR_TASK_ERROR");
		if (status5 & PVA_AISR_THRESHOLD_EXCEEDED)
			nvhost_warn(&pdev->dev, "PVA AISR: PVA_AISR_THRESHOLD_EXCEEDED");
		if (status5 & PVA_AISR_LOGGING_OVERFLOW)
			nvhost_warn(&pdev->dev, "PVA AISR: PVA_AISR_LOGGING_OVERFLOW");
		if (status5 & PVA_AISR_PRINTF_OVERFLOW)
			nvhost_warn(&pdev->dev, "PVA AISR: PVA_AISR_PRINTF_OVERFLOW");
		if (status5 & PVA_AISR_CRASH_LOG)
			nvhost_warn(&pdev->dev, "PVA AISR: PVA_AISR_CRASH_LOG");

		host1x_writel(pdev, hsp_sm5_r(), 0x0);
	}

	if (status6 & PVA_INT_PENDING) {
		nvhost_warn(&pdev->dev, "Unhandled SWUART ISR (%x)", status6);
		host1x_writel(pdev, hsp_sm6_r(), 0x0);
	}

	if (status7 & PVA_INT_PENDING) {
		nvhost_dbg_info("PVA ISR (%x)", status7);

		pva_mailbox_isr(pva);

		/* Leave PVA_READY bit untouched in purpose as
		 * per ISS recommendation
		 */
		status7 = status7 & PVA_READY;
		host1x_writel(pdev, hsp_sm7_r(), status7);
	}

	/* Copy trace points to ftrace buffer */
	pva_trace_copy_to_ftrace(pva);

	return IRQ_HANDLED;
}

int pva_register_isr(struct platform_device *dev)
{
	struct nvhost_device_data *pdata = platform_get_drvdata(dev);
	struct pva *pva = pdata->private_data;
	int err;

	pva->irq = platform_get_irq(dev, 0);
	if (pva->irq <= 0) {
		dev_err(&dev->dev, "no irq\n");
		err = -ENOENT;
		goto isr_err;
	}

	err = request_threaded_irq(pva->irq, NULL, pva_isr,
				   IRQF_ONESHOT, "pva-isr", pva);
	if (err) {
		pr_err("%s: request_irq(%d) failed(%d)\n", __func__,
		pva->irq, err);
		goto isr_err;
	}

	disable_irq(pva->irq);

isr_err:
	return err;
}
