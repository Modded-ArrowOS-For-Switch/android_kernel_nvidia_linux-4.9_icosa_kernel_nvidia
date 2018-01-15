/*
 * Copyright (C) 2018, NVIDIA CORPORATION. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <asm/io.h>
#include <linux/printk.h>
#include <linux/io.h>
#include <linux/tegra-mce.h>
#include "tegra19x_la_ptsa.h"

#define FIX_PT(x, y) fixed_point_init(x, y, 32, 32)

/* Non LA/PTSA mmio apertures */
static void __iomem *t19x_mc_base;
static void __iomem *t19x_pipe2uphy_xbar_base;
static void __iomem *t19x_emc_base;
//static void __iomem *t19x_mssnvlink1_base;
//static void __iomem *t19x_mssnvlink2_base;
//static void __iomem *t19x_mssnvlink3_base;
//static void __iomem *t19x_mssnvlink4_base;

/* TODO: Re-use APIs from other drivers */

/* TODO: Enable MSSNVLINK aperture access */

static inline void emc_writel (unsigned int val, unsigned int offset) {
	writel(val, t19x_emc_base + offset);
}

static inline void mssnvl1_writel (unsigned int val, unsigned int offset) {
	//writel(val, t19x_mssnvlink1_base + offset);
}
static inline void mssnvl2_writel (unsigned int val, unsigned int offset) {
	//writel(val, t19x_mssnvlink2_base + offset);
}
static inline void mssnvl3_writel (unsigned int val, unsigned int offset) {
	//writel(val, t19x_mssnvlink3_base + offset);
}
static inline void mssnvl4_writel (unsigned int val, unsigned int offset) {
	//writel(val, t19x_mssnvlink4_base + offset);
}

/* TODO: Use pcie driver interface */
static inline unsigned int pipe2phy_xbar_readl (unsigned int offset) {
	return readl(t19x_pipe2uphy_xbar_base + offset);
}

static inline unsigned int emc_readl (unsigned int offset) {
	return readl(t19x_emc_base + offset);
}
static inline unsigned int mssnvl1_readl (unsigned int offset) {
	//return readl(t19x_mssnvlink1_base + offset);
	return 0;
}
static inline unsigned int mssnvl2_readl (unsigned int offset) {
	//return readl(t19x_mssnvlink2_base + offset);
	return 0;
}
static inline unsigned int mssnvl3_readl (unsigned int offset) {
	//return readl(t19x_mssnvlink3_base + offset);
	return 0;
}
static inline unsigned int mssnvl4_readl (unsigned int offset) {
	//return readl(t19x_mssnvlink4_base + offset);
	return 0;
}


static struct la_ptsa_core lp;
static int tegra_gen_to_t19x_la_id[TEGRA_LA_MAX_ID];
static int tegra_t19x_to_gen_la_id[TEGRA_T19X_LA_MAX_ID];
static int t19x_la_kern_init[TEGRA_T19X_LA_MAX_ID];
static struct la_client_info t19x_la_info_array[TEGRA_T19X_LA_MAX_ID];
static struct dda_info dda_info_array[TEGRA_DDA_MAX_ID];
static struct mc_settings_info mc_settings;
static struct reg_info mc_reg_info_array[TEGRA_KERN_INIT_MC_MAX_ID];
static struct reg_info
mssnvlink1_reg_info_array[TEGRA_KERN_INIT_MSSNVLINK_MAX_ID];
static struct reg_info
mssnvlink2_reg_info_array[TEGRA_KERN_INIT_MSSNVLINK_MAX_ID];
static struct reg_info
mssnvlink3_reg_info_array[TEGRA_KERN_INIT_MSSNVLINK_MAX_ID];
static struct reg_info
mssnvlink4_reg_info_array[TEGRA_KERN_INIT_MSSNVLINK_MAX_ID];
static struct reg_info mcpcie_reg_info_array[TEGRA_KERN_INIT_MCPCIE_MAX_ID];

static void la_init(void)
{
	lp.la_info_array_init(
			t19x_la_info_array,
			tegra_gen_to_t19x_la_id,
			tegra_t19x_to_gen_la_id,
			t19x_la_kern_init,
			&mc_settings);
}

static int t19x_set_init_la(enum tegra_la_id id, unsigned int bw_mbps)
{
	enum tegra_t19x_la_id t19x_id = tegra_gen_to_t19x_la_id[id];
	unsigned int lat_all =
		lp.get_init_la(t19x_la_info_array[t19x_id].client_type,
				&mc_settings);
	if (t19x_la_kern_init[t19x_id])
		program_la(&t19x_la_info_array[t19x_id], lat_all);

	return 0;
}

static void dda_init(void)
{
	lp.dda_info_array_init(dda_info_array, TEGRA_DDA_MAX_ID, &mc_settings);
}

#define T19X_WRITE_PTSA_MIN_MAX_RATE(NAME) \
	do { \
		mc_writel(dda_info_array[TEGRA_DDA_##NAME##_ID].min & \
				MC_##NAME##_PTSA_MIN_0_PTSA_MIN_##NAME##_DEFAULT_MASK, \
				MC_##NAME##_PTSA_MIN_0); \
		mc_writel(dda_info_array[TEGRA_DDA_##NAME##_ID].max & \
				MC_##NAME##_PTSA_MAX_0_PTSA_MAX_##NAME##_DEFAULT_MASK, \
				MC_##NAME##_PTSA_MAX_0); \
		mc_writel(dda_info_array[TEGRA_DDA_##NAME##_ID].rate & \
				MC_##NAME##_PTSA_RATE_0_PTSA_RATE_## \
				NAME##_DEFAULT_MASK, \
				MC_##NAME##_PTSA_RATE_0); \
	} while (0)

#define T19X_WRITE_PTSA_MIN_MAX(NAME) \
	do { \
		mc_writel(dda_info_array[TEGRA_DDA_##NAME##_ID].min & \
				MC_##NAME##_PTSA_MIN_0_PTSA_MIN_##NAME##_DEFAULT_MASK, \
				MC_##NAME##_PTSA_MIN_0); \
		mc_writel(dda_info_array[TEGRA_DDA_##NAME##_ID].max & \
				MC_##NAME##_PTSA_MAX_0_PTSA_MAX_##NAME##_DEFAULT_MASK, \
				MC_##NAME##_PTSA_MAX_0); \
	} while (0)

#define T19X_WRITE_PTSA_RATE(NAME) \
	mc_writel(dda_info_array[TEGRA_DDA_##NAME##_ID].rate & \
			MC_##NAME##_PTSA_RATE_0_PTSA_RATE_## \
			NAME##_DEFAULT_MASK, \
			MC_##NAME##_PTSA_RATE_0)

static void program_kern_init_ptsa(void)
{
	T19X_WRITE_PTSA_MIN_MAX_RATE(AONPC);
	T19X_WRITE_PTSA_MIN_MAX_RATE(APB);
	T19X_WRITE_PTSA_MIN_MAX_RATE(AUD);
	T19X_WRITE_PTSA_MIN_MAX_RATE(BPMPPC);
	T19X_WRITE_PTSA_MIN_MAX(CIFLL_ISO);
	T19X_WRITE_PTSA_MIN_MAX_RATE(CIFLL_SISO);
	T19X_WRITE_PTSA_MIN_MAX_RATE(CIFLL_NISO);
	T19X_WRITE_PTSA_MIN_MAX_RATE(CIFLL_RING0X);
	T19X_WRITE_PTSA_MIN_MAX_RATE(DIS);
	T19X_WRITE_PTSA_MIN_MAX_RATE(DLA0FALPC);
	T19X_WRITE_PTSA_MIN_MAX_RATE(DLA0XA);
	T19X_WRITE_PTSA_MIN_MAX_RATE(DLA0XA2);
	T19X_WRITE_PTSA_MIN_MAX_RATE(DLA0XA3);
	T19X_WRITE_PTSA_MIN_MAX_RATE(DLA1FALPC);
	T19X_WRITE_PTSA_MIN_MAX_RATE(DLA1XA);
	T19X_WRITE_PTSA_MIN_MAX_RATE(DLA1XA2);
	T19X_WRITE_PTSA_MIN_MAX_RATE(DLA1XA3);
	T19X_WRITE_PTSA_MIN_MAX_RATE(EQOSPC);
	T19X_WRITE_PTSA_MIN_MAX_RATE(HDAPC);
	T19X_WRITE_PTSA_MIN_MAX_RATE(HOST);
	T19X_WRITE_PTSA_MIN_MAX_RATE(ISP);
	T19X_WRITE_PTSA_MIN_MAX_RATE(ISP2PC);
	T19X_WRITE_PTSA_MIN_MAX_RATE(ISPPC);
	T19X_WRITE_PTSA_MIN_MAX_RATE(JPG);
	T19X_WRITE_PTSA_MIN_MAX_RATE(MIU0);
	T19X_WRITE_PTSA_MIN_MAX_RATE(MIU1);
	T19X_WRITE_PTSA_MIN_MAX_RATE(MIU2);
	T19X_WRITE_PTSA_MIN_MAX_RATE(MIU3);
	T19X_WRITE_PTSA_MIN_MAX_RATE(MIU4);
	T19X_WRITE_PTSA_MIN_MAX_RATE(MIU5);
	T19X_WRITE_PTSA_MIN_MAX_RATE(MIU6);
	T19X_WRITE_PTSA_MIN_MAX_RATE(MIU7);
	T19X_WRITE_PTSA_MIN_MAX_RATE(MSE);
	T19X_WRITE_PTSA_MIN_MAX_RATE(MSE2);
	T19X_WRITE_PTSA_MIN_MAX_RATE(MSE3);
	T19X_WRITE_PTSA_MIN_MAX_RATE(MSEA);
	T19X_WRITE_PTSA_MIN_MAX_RATE(MSEB);
	T19X_WRITE_PTSA_MIN_MAX_RATE(MSEB1);
	T19X_WRITE_PTSA_MIN_MAX_RATE(NIC);
	T19X_WRITE_PTSA_MIN_MAX_RATE(NVD);
	T19X_WRITE_PTSA_MIN_MAX_RATE(NVD2);
	T19X_WRITE_PTSA_MIN_MAX_RATE(NVD3);
	T19X_WRITE_PTSA_MIN_MAX_RATE(NVD4);
	T19X_WRITE_PTSA_MIN_MAX_RATE(NVD5);
	T19X_WRITE_PTSA_MIN_MAX_RATE(NVD6);
	T19X_WRITE_PTSA_MIN_MAX_RATE(PCIE0X);
	T19X_WRITE_PTSA_MIN_MAX_RATE(PCIE0X2);
	T19X_WRITE_PTSA_MIN_MAX_RATE(PCIE0XA);
	T19X_WRITE_PTSA_MIN_MAX_RATE(PCIE1X);
	T19X_WRITE_PTSA_MIN_MAX_RATE(PCIE1XA);
	T19X_WRITE_PTSA_MIN_MAX_RATE(PCIE4X);
	T19X_WRITE_PTSA_MIN_MAX_RATE(PCIE4XA);
	T19X_WRITE_PTSA_MIN_MAX_RATE(PCIE5X);
	T19X_WRITE_PTSA_MIN_MAX_RATE(PCIE5X2);
	T19X_WRITE_PTSA_MIN_MAX_RATE(PCIE5XA);
	T19X_WRITE_PTSA_MIN_MAX_RATE(PVA0XA);
	T19X_WRITE_PTSA_MIN_MAX_RATE(PVA0XA2);
	T19X_WRITE_PTSA_MIN_MAX_RATE(PVA0XA3);
	T19X_WRITE_PTSA_MIN_MAX_RATE(PVA0XB);
	T19X_WRITE_PTSA_MIN_MAX_RATE(PVA0XB2);
	T19X_WRITE_PTSA_MIN_MAX_RATE(PVA0XB3);
	T19X_WRITE_PTSA_MIN_MAX_RATE(PVA0XC);
	T19X_WRITE_PTSA_MIN_MAX_RATE(PVA1XA);
	T19X_WRITE_PTSA_MIN_MAX_RATE(PVA1XA2);
	T19X_WRITE_PTSA_MIN_MAX_RATE(PVA1XA3);
	T19X_WRITE_PTSA_MIN_MAX_RATE(PVA1XB);
	T19X_WRITE_PTSA_MIN_MAX_RATE(PVA1XB2);
	T19X_WRITE_PTSA_MIN_MAX_RATE(PVA1XB3);
	T19X_WRITE_PTSA_MIN_MAX_RATE(PVA1XC);
	T19X_WRITE_PTSA_MIN_MAX_RATE(RCEPC);
	T19X_WRITE_PTSA_MIN_MAX_RATE(RING2);
	T19X_WRITE_PTSA_MIN_MAX_RATE(SAX);
	T19X_WRITE_PTSA_MIN_MAX_RATE(SCEPC);
	T19X_WRITE_PTSA_MIN_MAX_RATE(SD);
	T19X_WRITE_PTSA_MIN_MAX_RATE(SDM);
	T19X_WRITE_PTSA_MIN_MAX_RATE(SMMU_SMMU);
	T19X_WRITE_PTSA_MIN_MAX_RATE(UFSHCPC);
	T19X_WRITE_PTSA_MIN_MAX_RATE(UFSHCPC2);
	T19X_WRITE_PTSA_MIN_MAX_RATE(USBD);
	T19X_WRITE_PTSA_MIN_MAX_RATE(USBD2);
	T19X_WRITE_PTSA_MIN_MAX_RATE(USBX);
	T19X_WRITE_PTSA_MIN_MAX_RATE(USBX2);
	T19X_WRITE_PTSA_MIN_MAX_RATE(VE);
	T19X_WRITE_PTSA_MIN_MAX_RATE(VICPC);
	T19X_WRITE_PTSA_MIN_MAX_RATE(VICPC2);
	T19X_WRITE_PTSA_MIN_MAX_RATE(VICPC3);
}


static void program_non_kern_init_ptsa(void)
{
	T19X_WRITE_PTSA_RATE(CIFLL_ISO);
	T19X_WRITE_PTSA_MIN_MAX_RATE(MLL_MPCORER);
}

#undef T19X_WRITE_PTSA_MIN_MAX_RATE

static void program_ptsa(void)
{
	program_kern_init_ptsa();
	program_non_kern_init_ptsa();
}

#define T19X_SAVE_PTSA_MIN_MAX_RATE(NAME) \
	do { \
		dda_info_array[TEGRA_DDA_##NAME##_ID].min = \
		mc_readl(MC_##NAME##_PTSA_MIN_0); \
		dda_info_array[TEGRA_DDA_##NAME##_ID].max = \
		mc_readl(MC_##NAME##_PTSA_MAX_0); \
		dda_info_array[TEGRA_DDA_##NAME##_ID].rate = \
		mc_readl(MC_##NAME##_PTSA_RATE_0); \
	} while (0)

static void save_ptsa(void)
{
	T19X_SAVE_PTSA_MIN_MAX_RATE(AONPC);
	T19X_SAVE_PTSA_MIN_MAX_RATE(APB);
	T19X_SAVE_PTSA_MIN_MAX_RATE(AUD);
	T19X_SAVE_PTSA_MIN_MAX_RATE(BPMPPC);
	T19X_SAVE_PTSA_MIN_MAX_RATE(CIFLL_ISO);
	T19X_SAVE_PTSA_MIN_MAX_RATE(CIFLL_SISO);
	T19X_SAVE_PTSA_MIN_MAX_RATE(CIFLL_NISO);
	T19X_SAVE_PTSA_MIN_MAX_RATE(CIFLL_RING0X);
	T19X_SAVE_PTSA_MIN_MAX_RATE(DIS);
	T19X_SAVE_PTSA_MIN_MAX_RATE(DLA0FALPC);
	T19X_SAVE_PTSA_MIN_MAX_RATE(DLA0XA);
	T19X_SAVE_PTSA_MIN_MAX_RATE(DLA0XA2);
	T19X_SAVE_PTSA_MIN_MAX_RATE(DLA0XA3);
	T19X_SAVE_PTSA_MIN_MAX_RATE(DLA1FALPC);
	T19X_SAVE_PTSA_MIN_MAX_RATE(DLA1XA);
	T19X_SAVE_PTSA_MIN_MAX_RATE(DLA1XA2);
	T19X_SAVE_PTSA_MIN_MAX_RATE(DLA1XA3);
	T19X_SAVE_PTSA_MIN_MAX_RATE(EQOSPC);
	T19X_SAVE_PTSA_MIN_MAX_RATE(HDAPC);
	T19X_SAVE_PTSA_MIN_MAX_RATE(HOST);
	T19X_SAVE_PTSA_MIN_MAX_RATE(ISP);
	T19X_SAVE_PTSA_MIN_MAX_RATE(ISP2PC);
	T19X_SAVE_PTSA_MIN_MAX_RATE(ISPPC);
	T19X_SAVE_PTSA_MIN_MAX_RATE(JPG);
	T19X_SAVE_PTSA_MIN_MAX_RATE(MIU0);
	T19X_SAVE_PTSA_MIN_MAX_RATE(MIU1);
	T19X_SAVE_PTSA_MIN_MAX_RATE(MIU2);
	T19X_SAVE_PTSA_MIN_MAX_RATE(MIU3);
	T19X_SAVE_PTSA_MIN_MAX_RATE(MIU4);
	T19X_SAVE_PTSA_MIN_MAX_RATE(MIU5);
	T19X_SAVE_PTSA_MIN_MAX_RATE(MIU6);
	T19X_SAVE_PTSA_MIN_MAX_RATE(MIU7);
	T19X_SAVE_PTSA_MIN_MAX_RATE(MLL_MPCORER);
	T19X_SAVE_PTSA_MIN_MAX_RATE(MSE);
	T19X_SAVE_PTSA_MIN_MAX_RATE(MSE2);
	T19X_SAVE_PTSA_MIN_MAX_RATE(MSE3);
	T19X_SAVE_PTSA_MIN_MAX_RATE(MSEA);
	T19X_SAVE_PTSA_MIN_MAX_RATE(MSEB);
	T19X_SAVE_PTSA_MIN_MAX_RATE(MSEB1);
	T19X_SAVE_PTSA_MIN_MAX_RATE(NIC);
	T19X_SAVE_PTSA_MIN_MAX_RATE(NVD);
	T19X_SAVE_PTSA_MIN_MAX_RATE(NVD2);
	T19X_SAVE_PTSA_MIN_MAX_RATE(NVD3);
	T19X_SAVE_PTSA_MIN_MAX_RATE(NVD4);
	T19X_SAVE_PTSA_MIN_MAX_RATE(NVD5);
	T19X_SAVE_PTSA_MIN_MAX_RATE(NVD6);
	T19X_SAVE_PTSA_MIN_MAX_RATE(PCIE0X);
	T19X_SAVE_PTSA_MIN_MAX_RATE(PCIE0X2);
	T19X_SAVE_PTSA_MIN_MAX_RATE(PCIE0XA);
	T19X_SAVE_PTSA_MIN_MAX_RATE(PCIE1X);
	T19X_SAVE_PTSA_MIN_MAX_RATE(PCIE1XA);
	T19X_SAVE_PTSA_MIN_MAX_RATE(PCIE4X);
	T19X_SAVE_PTSA_MIN_MAX_RATE(PCIE4XA);
	T19X_SAVE_PTSA_MIN_MAX_RATE(PCIE5X);
	T19X_SAVE_PTSA_MIN_MAX_RATE(PCIE5X2);
	T19X_SAVE_PTSA_MIN_MAX_RATE(PCIE5XA);
	T19X_SAVE_PTSA_MIN_MAX_RATE(PVA0XA);
	T19X_SAVE_PTSA_MIN_MAX_RATE(PVA0XA2);
	T19X_SAVE_PTSA_MIN_MAX_RATE(PVA0XA3);
	T19X_SAVE_PTSA_MIN_MAX_RATE(PVA0XB);
	T19X_SAVE_PTSA_MIN_MAX_RATE(PVA0XB2);
	T19X_SAVE_PTSA_MIN_MAX_RATE(PVA0XB3);
	T19X_SAVE_PTSA_MIN_MAX_RATE(PVA0XC);
	T19X_SAVE_PTSA_MIN_MAX_RATE(PVA1XA);
	T19X_SAVE_PTSA_MIN_MAX_RATE(PVA1XA2);
	T19X_SAVE_PTSA_MIN_MAX_RATE(PVA1XA3);
	T19X_SAVE_PTSA_MIN_MAX_RATE(PVA1XB);
	T19X_SAVE_PTSA_MIN_MAX_RATE(PVA1XB2);
	T19X_SAVE_PTSA_MIN_MAX_RATE(PVA1XB3);
	T19X_SAVE_PTSA_MIN_MAX_RATE(PVA1XC);
	T19X_SAVE_PTSA_MIN_MAX_RATE(RCEPC);
	T19X_SAVE_PTSA_MIN_MAX_RATE(RING2);
	T19X_SAVE_PTSA_MIN_MAX_RATE(SAX);
	T19X_SAVE_PTSA_MIN_MAX_RATE(SCEPC);
	T19X_SAVE_PTSA_MIN_MAX_RATE(SD);
	T19X_SAVE_PTSA_MIN_MAX_RATE(SDM);
	T19X_SAVE_PTSA_MIN_MAX_RATE(SMMU_SMMU);
	T19X_SAVE_PTSA_MIN_MAX_RATE(UFSHCPC);
	T19X_SAVE_PTSA_MIN_MAX_RATE(UFSHCPC2);
	T19X_SAVE_PTSA_MIN_MAX_RATE(USBD);
	T19X_SAVE_PTSA_MIN_MAX_RATE(USBD2);
	T19X_SAVE_PTSA_MIN_MAX_RATE(USBX);
	T19X_SAVE_PTSA_MIN_MAX_RATE(USBX2);
	T19X_SAVE_PTSA_MIN_MAX_RATE(VE);
	T19X_SAVE_PTSA_MIN_MAX_RATE(VICPC);
	T19X_SAVE_PTSA_MIN_MAX_RATE(VICPC2);
	T19X_SAVE_PTSA_MIN_MAX_RATE(VICPC3);
}
#undef T19X_SAVE_PTSA_MIN_MAX_RATE

static void t19x_init_ptsa(void)
{
	dda_init();

	lp.update_new_dda_minmax_kern_init(dda_info_array, &mc_settings);
	lp.update_new_dda_rate_frac_kern_init(dda_info_array, &mc_settings);
	dda_info_array[TEGRA_DDA_MLL_MPCORER_ID].min = -6;
	dda_info_array[TEGRA_DDA_MLL_MPCORER_ID].max = 6;

	program_kern_init_ptsa();
}

static int t19x_set_dynamic_ptsa(enum tegra_dda_id id, unsigned int bw_mbps)
{
	struct fixed_point iso_adj_bw;

	if (dda_info_array[id].iso_type == TEGRA_HISO) {
		if (id == TEGRA_DDA_EQOSPC_ID) {
			iso_adj_bw = fixed_point_mult(FIX_PT(250, 0),
					mc_settings.two_stge_ecc_iso_dda_bw_margin);
		} else {
			iso_adj_bw = fixed_point_mult(FIX_PT(bw_mbps, 0),
					mc_settings.dda_bw_margin);
		}
	} else {
		iso_adj_bw = FIX_PT(bw_mbps, 0);
	}

	lp.update_new_dda_rate_frac_use_case(
			dda_info_array,
			&mc_settings,
			id,
			iso_adj_bw);

	mc_writel(dda_info_array[id].rate &
			dda_info_array[id].mask,
			dda_info_array[id].rate_reg_addr);

	return 0;
}

static int t19x_handle_display_la_ptsa(
		enum tegra_la_id id,
		unsigned long emc_freq_hz,
		unsigned int bw_mbps,
		int write_la)
{
	int disp_la = 0;
	struct fixed_point drain_time_usec = FIX_PT(0, 0);
	struct fixed_point la_bw_up_bnd_usec = FIX_PT(0, 0);
	int clientid = lp.convert_la2dda_id_for_dyn_ptsa(id);
	enum tegra_t19x_la_id t19x_id = tegra_gen_to_t19x_la_id[id];

	lp.get_disp_rd_lat_allow_given_disp_bw(
			&mc_settings,
			fixed_point_div(FIX_PT(emc_freq_hz, 0), FIX_PT(1000000, 0)),
			fixed_point_mult(FIX_PT(bw_mbps, 0),
				mc_settings.disp_catchup_factor),
			&disp_la,
			&drain_time_usec,
			&la_bw_up_bnd_usec);

	if (disp_la < 0)
		return -1;

	if (fixed_point_gt(drain_time_usec, la_bw_up_bnd_usec))
		return -1;

	if (write_la) {
		program_la(&t19x_la_info_array[t19x_id], disp_la);

		t19x_set_dynamic_ptsa(clientid, bw_mbps);
	}

	return 0;
}

static int t19x_set_display_la_ptsa(
		enum tegra_la_id id,
		unsigned long emc_freq_hz,
		unsigned int bw_mbps,
		struct dc_to_la_params disp_params)
{
	return t19x_handle_display_la_ptsa(id, emc_freq_hz, bw_mbps, 1);
}

static int t19x_check_display_la_ptsa(
		enum tegra_la_id id,
		unsigned long emc_freq_hz,
		unsigned int bw_mbps,
		struct dc_to_la_params disp_params)
{
	return t19x_handle_display_la_ptsa(id, emc_freq_hz, bw_mbps, 0);
}

static int t19x_set_camera_la_ptsa(
		enum tegra_la_id id,
		unsigned int bw_mbps,
		int is_hiso)
{
	/* Nothing needs to be changed from kernel init values, so do nothing*/
	return 0;
}

static int t19x_set_dynamic_la_ptsa(enum tegra_la_id id, unsigned int bw_mbps)
{
	int clientid = lp.convert_la2dda_id_for_dyn_ptsa(id);

	return t19x_set_dynamic_ptsa(clientid, bw_mbps);
}

static void program_non_la_ptsa(void)
{
	int i;

	for (i = 0; i < TEGRA_KERN_INIT_MC_MAX_ID; i++) {
		mc_writel(mc_reg_info_array[i].val,
				mc_reg_info_array[i].offset);
	}

	for (i = 0; i < TEGRA_KERN_INIT_MSSNVLINK_MAX_ID; i++) {
		mssnvl1_writel(mssnvlink1_reg_info_array[i].val,
				mssnvlink1_reg_info_array[i].offset);
	}

	for (i = 0; i < TEGRA_KERN_INIT_MSSNVLINK_MAX_ID; i++) {
		mssnvl2_writel(mssnvlink2_reg_info_array[i].val,
				mssnvlink2_reg_info_array[i].offset);
	}

	for (i = 0; i < TEGRA_KERN_INIT_MSSNVLINK_MAX_ID; i++) {
		mssnvl3_writel(mssnvlink3_reg_info_array[i].val,
				mssnvlink3_reg_info_array[i].offset);
	}

	for (i = 0; i < TEGRA_KERN_INIT_MSSNVLINK_MAX_ID; i++) {
		mssnvl4_writel(mssnvlink4_reg_info_array[i].val,
				mssnvlink4_reg_info_array[i].offset);
	}
}

static void save_non_la_ptsa(void)
{
	int i;

	for (i = 0; i < TEGRA_KERN_INIT_MC_MAX_ID; i++) {
		mc_reg_info_array[i].val =
			mc_readl(mc_reg_info_array[i].offset);
	}

	for (i = 0; i < TEGRA_KERN_INIT_MSSNVLINK_MAX_ID; i++) {
		mssnvlink1_reg_info_array[i].val =
			mssnvl1_readl(mssnvlink1_reg_info_array[i].offset);
	}

	for (i = 0; i < TEGRA_KERN_INIT_MSSNVLINK_MAX_ID; i++) {
		mssnvlink2_reg_info_array[i].val =
			mssnvl2_readl(mssnvlink2_reg_info_array[i].offset);
	}

	for (i = 0; i < TEGRA_KERN_INIT_MSSNVLINK_MAX_ID; i++) {
		mssnvlink3_reg_info_array[i].val =
			mssnvl3_readl(mssnvlink3_reg_info_array[i].offset);
	}

	for (i = 0; i < TEGRA_KERN_INIT_MSSNVLINK_MAX_ID; i++) {
		mssnvlink4_reg_info_array[i].val =
			mssnvl4_readl(mssnvlink4_reg_info_array[i].offset);
	}
}

#define SET_FIELD_IN_64BIT_REG(var, start, width, field) \
	do { \
		BUG_ON(((field) & ~((1ull<<width)-1)) != 0); \
		var = (var & ~((((1ull<<width)-1)<<(start)))) | \
		(((field) & ((1ull<<width)-1))<<(start)); \
	} while (0)

static void set_nvg_scf_dda(
		unsigned int nvg_ch,
		uint32_t rate,
		uint32_t min,
		uint32_t max)
{
	int ret = 0;
	uint64_t nvg_reg = 0;
	ret = tegra_mce_read_dda_ctrl(nvg_ch, &nvg_reg);
	BUG_ON(ret != 0);

	SET_FIELD_IN_64BIT_REG(nvg_reg, 0, 12, rate);
	SET_FIELD_IN_64BIT_REG(nvg_reg, 12, 11, min);
	SET_FIELD_IN_64BIT_REG(nvg_reg, 23, 11, max);

	ret = tegra_mce_write_dda_ctrl(nvg_ch, nvg_reg);
	BUG_ON(ret != 0);
}

static void set_nvg_scf_gd(
		unsigned int nvg_ch,
		uint32_t int_part,
		uint32_t frac_part)
{
	int ret = 0;
	uint64_t nvg_reg = 0;
	ret = tegra_mce_read_dda_ctrl(nvg_ch, &nvg_reg);
	BUG_ON(ret != 0);

	SET_FIELD_IN_64BIT_REG(nvg_reg, 0, 12, frac_part);
	SET_FIELD_IN_64BIT_REG(nvg_reg, 12, 1, int_part);

	ret = tegra_mce_write_dda_ctrl(nvg_ch, nvg_reg);
	BUG_ON(ret != 0);
}
#undef SET_FIELD_IN_64BIT_REG


static void scf_dda_init(struct mc_settings_info *mc_settings_ptr)
{
	uint32_t gd_int, gd_frac;
	uint32_t eps = 1;
	uint32_t bw100percent;
	unsigned int division_factor = 0;
	struct fixed_point tmp;
	unsigned disable_dvfs_conflicts = 1;

	switch (mc_settings_ptr->num_channels) {
	case 16:
		division_factor = 2; break;
	case 8:
		division_factor = 4; break;
	case 4:
		division_factor = 8; break;
	default:
		BUG_ON(1);
	}
	tmp = fixed_point_div(mc_settings_ptr->max_gd,
			FIX_PT(division_factor, 0));

	/* Rate only has fractional part, so 100% rate < 1 */
	BUG_ON(tmp.int_part != 0);

	/* Lower precision to 12 bits of fraction part. */
	BUG_ON(tmp.frac_prec < 12);
	bw100percent = (tmp.frac_part & tmp.frac_mask) >> (tmp.frac_prec - 12);

	/* Implement 11bit two's compliment for negative min/max. */
#define NEG(val) ((~(val) + 1) & 0x7ff)

	/* Simplified int math with just fraction part.
	 * Multiply first to not lose precision by division. */
#define PERC(val) ((bw100percent * (val)) / 100)

	/* Initialize grant decrements to max value at init.
	 * These will be updated by DVFS later. */
	tmp = mc_settings_ptr->max_gd;
	BUG_ON(tmp.int_part > 1);
	gd_int = tmp.int_part;
	gd_frac = (tmp.frac_part & tmp.frac_mask) >> (tmp.frac_prec - 12);

	if (!disable_dvfs_conflicts) {
		set_nvg_scf_gd(TEGRA_NVG_CHANNEL_DDA_SNOC_GLOBAL_CTRL,
							gd_int, gd_frac);
		set_nvg_scf_gd(TEGRA_NVG_CHANNEL_DDA_L3CTRL_GLOBAL,
							gd_int, gd_frac);
		set_nvg_scf_dda(TEGRA_NVG_CHANNEL_DDA_SNOC_CLIENT_REQ_CTRL,
							PERC(10), NEG(16), 16);
		set_nvg_scf_dda(
			TEGRA_NVG_CHANNEL_DDA_SNOC_CLIENT_REPLENTISH_CTRL,
			PERC(10), NEG(16), 16);
		set_nvg_scf_dda(TEGRA_NVG_CHANNEL_DDA_L3CTRL_LL,
							PERC(10), NEG(16), 16);
		set_nvg_scf_dda(TEGRA_NVG_CHANNEL_DDA_L3CTRL_L3D,
							PERC(10), NEG(4), 4);
	}

	set_nvg_scf_dda(TEGRA_NVG_CHANNEL_DDA_SNOC_MCF,
			eps, NEG(3), 0);

	set_nvg_scf_dda(TEGRA_NVG_CHANNEL_DDA_MCF_SISO,
			0, 1, 1);
	set_nvg_scf_dda(TEGRA_NVG_CHANNEL_DDA_MCF_ORD1,
			PERC(13), NEG(4), 31);
	set_nvg_scf_dda(TEGRA_NVG_CHANNEL_DDA_MCF_ORD2,
			PERC(13), NEG(4), 31);
	set_nvg_scf_dda(TEGRA_NVG_CHANNEL_DDA_MCF_ORD3,
			PERC(7), NEG(4), 31);
	set_nvg_scf_dda(TEGRA_NVG_CHANNEL_DDA_MCF_NISO,
			eps, NEG(3), 0);
	set_nvg_scf_dda(TEGRA_NVG_CHANNEL_DDA_MCF_NISO_REMOTE,
			eps, NEG(3), 0);
	set_nvg_scf_dda(TEGRA_NVG_CHANNEL_DDA_MCF_ISO,
			eps, NEG(3), 0);

	set_nvg_scf_dda(TEGRA_NVG_CHANNEL_DDA_L3CTRL_SISO,
			0, 1, 1);
	set_nvg_scf_dda(TEGRA_NVG_CHANNEL_DDA_L3CTRL_NISO,
			eps, NEG(3), 0);
	set_nvg_scf_dda(TEGRA_NVG_CHANNEL_DDA_L3CTRL_NISO_REMOTE,
			eps, NEG(3), 0);
	set_nvg_scf_dda(TEGRA_NVG_CHANNEL_DDA_L3CTRL_ISO,
			eps, NEG(3), 0);

	set_nvg_scf_dda(TEGRA_NVG_CHANNEL_DDA_L3CTRL_L3FILL,
			0x133, NEG(31), 31);
	set_nvg_scf_dda(TEGRA_NVG_CHANNEL_DDA_L3CTRL_L3WR,
			0, NEG(3), 0);

	set_nvg_scf_dda(TEGRA_NVG_CHANNEL_DDA_L3CTRL_RSP_L3RD_DMA,
			0xE66, NEG(31), 31);
	set_nvg_scf_dda(TEGRA_NVG_CHANNEL_DDA_L3CTRL_RSP_MCFRD_DMA,
			0, NEG(3), 0);

	set_nvg_scf_dda(TEGRA_NVG_CHANNEL_DDA_L3CTRL_FCM_RD,
			0x555, NEG(1024), 72);
	set_nvg_scf_dda(TEGRA_NVG_CHANNEL_DDA_L3CTRL_FCM_WR,
			0x555, NEG(1024), 72);

#undef NEG
#undef PERC
}

static void program_mcpcie(void)
{
	int i;

	for (i = 0; i < TEGRA_KERN_INIT_MCPCIE_MAX_ID; i++) {
		mc_writel(mcpcie_reg_info_array[i].val,
				mcpcie_reg_info_array[i].offset);
	}
}

static void save_mcpcie(void)
{
	int i;

	for (i = 0; i < TEGRA_KERN_INIT_MCPCIE_MAX_ID; i++) {
		mcpcie_reg_info_array[i].val =
			mc_readl(mcpcie_reg_info_array[i].offset);
	}
}

// TODO: Use pcie driver interface
// TODO: Read base address from dtb
#define NV_ADDRESS_MAP_PIPE2UPHY_XBAR_BASE          0x03e00000
static void t19x_mc_pcie_init(void)
{
	unsigned int xbar_cfg, reg_data;

	t19x_pipe2uphy_xbar_base = ioremap(NV_ADDRESS_MAP_PIPE2UPHY_XBAR_BASE, 0x00010000);
	reg_data = pipe2phy_xbar_readl(PCIE_COMMON_APPL_COMMON_CONTROL_0);
	iounmap(t19x_pipe2uphy_xbar_base);

	xbar_cfg = NV_DRF_VAL(PCIE_COMMON, APPL_COMMON_CONTROL, XBAR_CONFIG, reg_data);
	lp.mcpcie_reg_info_array_init(mcpcie_reg_info_array);
	save_mcpcie();
	lp.update_ord_ids(mcpcie_reg_info_array, &mc_settings, xbar_cfg);
	program_mcpcie();
}


/*
 * Get dram type and channels configuration.
 * TODO: Make use of api from emc driver
 */
static enum tegra_dram_t t19x_emc_get_dram_type(void)
{
	unsigned int dram, ch, mem_type;
	enum tegra_dram_t dram_type;

	dram = readl(t19x_emc_base + EMC_FBIO_CFG5_0) & DRAM_TYPE_MASK;
	ch = readl(t19x_mc_base + MC_EMEM_ADR_CFG_CHANNEL_ENABLE_0) & DRAM_CH_MASK;
	mem_type = readl(t19x_emc_base + EMC_PMACRO_PAD_CFG_CTRL_0);
	mem_type = (mem_type >> MEM_MODE_SHIFT) & MEM_MODE_MASK;
	la_debug("mem_type: 0x%x, dram reg: 0x%x, channels reg: 0x%x\n", mem_type, dram, ch);

	BUG_ON(dram != DRAM_LPDDR4);

	switch (ch) {
	case(0xff):
		dram_type = mem_type ? TEGRA_LP4_8CH : TEGRA_LP4X_8CH;
		break;
	case(0xffff):
		dram_type = mem_type ? TEGRA_LP4_16CH : TEGRA_LP4X_16CH;
		break;
	default:
		pr_err("la/ptsa: 0x%x: Unknown memory channel configuration\n", ch);
		BUG_ON(1);
	}
	return dram_type;
}

static void tegra_la_init(void)
{
	enum tegra_dram_t tegra_dram_type = t19x_emc_get_dram_type();
	la_debug("DRAM Type: %d\n", tegra_dram_type);
	init_la_ptsa_core(&lp);
	lp.mc_settings_init(tegra_dram_type, &mc_settings);
	lp.setup_freq_ranges(&mc_settings);
	la_init();

	lp.all_reg_info_array_init(
			mc_reg_info_array,
			mssnvlink1_reg_info_array,
			mssnvlink2_reg_info_array,
			mssnvlink3_reg_info_array,
			mssnvlink4_reg_info_array);
	save_non_la_ptsa();

	lp.write_perf_regs_kern_init(
			&mc_settings,
			mc_reg_info_array,
			mssnvlink1_reg_info_array,
			mssnvlink2_reg_info_array,
			mssnvlink3_reg_info_array,
			mssnvlink4_reg_info_array);
	program_non_la_ptsa();
	scf_dda_init(&mc_settings);

	/* update shadowed registers */
	mc_writel(1, MC_TIMING_CONTROL_0);
}


/* TODO: Get the base address from dtb */
/* Reuse API from other drivers */
#define NV_ADDRESS_MAP_MC_BASE                      0x02c10000
#define NV_ADDRESS_MAP_EMCB_BASE                    0x02C60000
#define NV_ADDRESS_MAP_MSS_NVLINK_0_BASE            0x01F00000
#define NV_ADDRESS_MAP_MSS_NVLINK_1_BASE            0x01F20000
#define NV_ADDRESS_MAP_MSS_NVLINK_2_BASE            0x01F40000
#define NV_ADDRESS_MAP_MSS_NVLINK_3_BASE            0x01F60000
#define NV_ADDRESS_MAP_MSS_NVLINK_4_BASE            0x01F80000

void tegra_la_get_t19x_specific(struct la_chip_specific *cs_la)
{

	cs_la->ns_per_tick = 30;
	cs_la->la_max_value = T19X_MC_LA_MAX_VALUE;

	cs_la->la_info_array = t19x_la_info_array;
	cs_la->la_info_array_size = TEGRA_T19X_LA_MAX_ID;

	cs_la->init_ptsa = t19x_init_ptsa;
	cs_la->update_camera_ptsa_rate = t19x_set_camera_la_ptsa;
	cs_la->set_init_la = t19x_set_init_la;
	cs_la->set_dynamic_la = t19x_set_dynamic_la_ptsa;
	cs_la->set_disp_la = t19x_set_display_la_ptsa;
	cs_la->check_disp_la = t19x_check_display_la_ptsa;
	cs_la->save_ptsa = save_ptsa;
	cs_la->program_ptsa = program_ptsa;
	cs_la->suspend = la_suspend;
	cs_la->resume = la_resume;
	cs_la->mc_pcie_init = t19x_mc_pcie_init;

	/* TODO: Get base address from dtb */
	/* Reuse API from other drivers */

	t19x_mc_base = ioremap(NV_ADDRESS_MAP_MC_BASE, 0x00010000);
	t19x_emc_base = ioremap(NV_ADDRESS_MAP_EMCB_BASE, 0x00010000);
//	t19x_mssnvlink1_base = ioremap(NV_ADDRESS_MAP_MSS_NVLINK_1_BASE, 0x00020000);
//	t19x_mssnvlink2_base = ioremap(NV_ADDRESS_MAP_MSS_NVLINK_2_BASE, 0x00020000);
//	t19x_mssnvlink3_base = ioremap(NV_ADDRESS_MAP_MSS_NVLINK_3_BASE, 0x00020000);
//	t19x_mssnvlink4_base = ioremap(NV_ADDRESS_MAP_MSS_NVLINK_4_BASE, 0x00020000);

	tegra_la_init();

	iounmap(t19x_mc_base);
	iounmap(t19x_emc_base);
//	iounmap(t19x_mssnvlink1_base);
//	iounmap(t19x_mssnvlink2_base);
//	iounmap(t19x_mssnvlink3_base);
//	iounmap(t19x_mssnvlink4_base);

}