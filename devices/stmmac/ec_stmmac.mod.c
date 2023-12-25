#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

MODULE_INFO(depends, "phylink,ec_master,pcs-xpcs");

MODULE_ALIAS("of:N*T*Crockchip,px30-gmac");
MODULE_ALIAS("of:N*T*Crockchip,px30-gmacC*");
MODULE_ALIAS("of:N*T*Crockchip,rk1808-gmac");
MODULE_ALIAS("of:N*T*Crockchip,rk1808-gmacC*");
MODULE_ALIAS("of:N*T*Crockchip,rk3328-gmac");
MODULE_ALIAS("of:N*T*Crockchip,rk3328-gmacC*");
MODULE_ALIAS("of:N*T*Crockchip,rk3399-gmac");
MODULE_ALIAS("of:N*T*Crockchip,rk3399-gmacC*");
MODULE_ALIAS("of:N*T*Crockchip,rk3528-gmac");
MODULE_ALIAS("of:N*T*Crockchip,rk3528-gmacC*");
MODULE_ALIAS("of:N*T*Crockchip,rk3562-gmac");
MODULE_ALIAS("of:N*T*Crockchip,rk3562-gmacC*");
MODULE_ALIAS("of:N*T*Crockchip,rk3568-gmac");
MODULE_ALIAS("of:N*T*Crockchip,rk3568-gmacC*");
MODULE_ALIAS("of:N*T*Crockchip,rk3588-gmac");
MODULE_ALIAS("of:N*T*Crockchip,rk3588-gmacC*");
