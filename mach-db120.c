/*
 * Atheros DB120 reference board support
 *
 * Copyright (c) 2011 Qualcomm Atheros
 * Copyright (c) 2011-2012 Gabor Juhos <juhosg@openwrt.org>
 * Copyright (c) 2015 Sheng Du <dusheng126@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#include <linux/pci.h>
#include <linux/phy.h>
#include <linux/platform_device.h>
#include <linux/ath9k_platform.h>
#include <linux/ar8216_platform.h>

#include <asm/mach-ath79/ar71xx_regs.h>

#include "common.h"
#include "dev-ap9x-pci.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-nfc.h"
#include "dev-spi.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"

#define DB120_GPIO_LED_LAN_R		13
#define DB120_GPIO_LED_LAN_L		14
#define DB120_GPIO_USB_PWR			15
#define DB120_GPIO_LED_WLAN_2G		16
#define DB120_GPIO_LED_WAN_L		18
#define DB120_GPIO_LED_WAN_R		19

#define DB120_GPIO_BTN_RESET		4

/* not use
#define DB120_GPIO_LED_USB			11
#define DB120_GPIO_LED_WLAN_5G		12
#define DB120_GPIO_LED_STATUS		14
#define DB120_GPIO_LED_WPS			15
#define DB120_GPIO_BTN_WPS			16
*/
#define DB120_KEYS_POLL_INTERVAL	20	/* msecs */
#define DB120_KEYS_DEBOUNCE_INTERVAL	(3 * DB120_KEYS_POLL_INTERVAL)

#define DB120_MAC0_OFFSET		0
#define DB120_MAC1_OFFSET		6
#define DB120_WMAC_CALDATA_OFFSET	0x1000
#define DB120_PCIE_CALDATA_OFFSET	0x5000

static struct gpio_led db120_leds_gpio[] __initdata = {
	/* not use
	{
		.name		= "db120:green:status",
		.gpio		= DB120_GPIO_LED_STATUS,
		.active_low	= 1,
	},
	{
		.name		= "db120:green:wps",
		.gpio		= DB120_GPIO_LED_WPS,
		.active_low	= 1,
	},
	{
		.name		= "db120:green:wlan-5g",
		.gpio		= DB120_GPIO_LED_WLAN_5G,
		.active_low	= 1,
	},
	{
		.name		= "db120:green:usb",
		.gpio		= DB120_GPIO_LED_USB,
		.active_low	= 1,
	}
	*/
	{
		.name		= "db120:green:wlan-2g",
		.gpio		= DB120_GPIO_LED_WLAN_2G,
		.active_low	= 1,
	},
	{
		.name		= "db120:green:usb-pwr",
		.gpio		= DB120_GPIO_USB_PWR,
		.active_low	= 1,
	},
	{
		.name		= "db120:green:wan-right",
		.gpio		= DB120_GPIO_LED_WAN_R,
		.active_low	= 1,
	},
	{
		.name		= "db120:green:wan-left",
		.gpio		= DB120_GPIO_LED_WAN_L,
		.active_low	= 1,
	},
	{
		.name		= "db120:green:lan-right",
		.gpio		= DB120_GPIO_LED_LAN_R,
		.active_low	= 1,
	},
	{
		.name		= "db120:green:lan-left",
		.gpio		= DB120_GPIO_LED_LAN_L,
		.active_low	= 1,
	},
};

static struct gpio_keys_button db120_gpio_keys[] __initdata = {
	{
		.desc           = "reset",
		.type           = EV_KEY,
		.code           = KEY_RESTART,
		.debounce_interval = DB120_KEYS_DEBOUNCE_INTERVAL,
		.gpio           = DB120_GPIO_BTN_RESET,
		.active_low     = 1,
	},
};

static void __init db120_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);

	/* not use
	ath79_gpio_output_select(DB120_GPIO_LED_USB, AR934X_GPIO_OUT_GPIO);
	*/
	ath79_register_m25p80(NULL);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(db120_leds_gpio),
				 db120_leds_gpio);
	ath79_register_gpio_keys_polled(-1, DB120_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(db120_gpio_keys),
					db120_gpio_keys);
	ath79_register_usb();
	ath79_register_wmac(art + DB120_WMAC_CALDATA_OFFSET, NULL);
	ap91_pci_init(art + DB120_PCIE_CALDATA_OFFSET, NULL);

	ath79_register_mdio(1, 0x0);
	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_SW_ONLY_MODE);

	/* WAN: GMAC0 is connected to the PHY4 of the internal switch */
	ath79_init_mac(ath79_eth0_data.mac_addr, art + DB120_MAC0_OFFSET, 0);
	ath79_switch_data.phy4_mii_en = 1;
	ath79_switch_data.phy_poll_mask = BIT(4);

	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.phy_mask = BIT(4);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio1_device.dev;
	ath79_register_eth(0);
	
	/* LAN: GMAC1 is connected to the internal switch */
	ath79_init_mac(ath79_eth1_data.mac_addr, art + DB120_MAC1_OFFSET, 0);
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
	ath79_register_eth(1);

	ath79_register_nfc();
}

MIPS_MACHINE(ATH79_MACH_DB120, "DB120", "Atheros DB120 reference board",
	     db120_setup);
