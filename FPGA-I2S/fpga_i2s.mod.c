#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0xecae869e, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0xe99548af, __VMLINUX_SYMBOL_STR(platform_driver_unregister) },
	{ 0x369da003, __VMLINUX_SYMBOL_STR(__platform_driver_register) },
	{ 0xeae3dfd6, __VMLINUX_SYMBOL_STR(__const_udelay) },
	{ 0x44fbb05a, __VMLINUX_SYMBOL_STR(__register_chrdev) },
	{ 0x5cf0e98d, __VMLINUX_SYMBOL_STR(devm_request_threaded_irq) },
	{ 0x197f1dd2, __VMLINUX_SYMBOL_STR(platform_get_irq) },
	{ 0xc059854b, __VMLINUX_SYMBOL_STR(devm_ioremap_resource) },
	{ 0x4564d128, __VMLINUX_SYMBOL_STR(platform_get_resource) },
	{ 0x815588a6, __VMLINUX_SYMBOL_STR(clk_enable) },
	{ 0x2e9b56e7, __VMLINUX_SYMBOL_STR(dev_err) },
	{ 0x7c9a7371, __VMLINUX_SYMBOL_STR(clk_prepare) },
	{ 0x86fce953, __VMLINUX_SYMBOL_STR(devm_clk_get) },
	{ 0xbd323548, __VMLINUX_SYMBOL_STR(__init_waitqueue_head) },
	{ 0x4eb32012, __VMLINUX_SYMBOL_STR(devm_kmalloc) },
	{ 0x45a55ec8, __VMLINUX_SYMBOL_STR(__iounmap) },
	{ 0x2724ba66, __VMLINUX_SYMBOL_STR(__ioremap) },
	{ 0x2f5d7a34, __VMLINUX_SYMBOL_STR(__wake_up) },
	{ 0x27bbf221, __VMLINUX_SYMBOL_STR(disable_irq_nosync) },
	{ 0xb35dea8f, __VMLINUX_SYMBOL_STR(__arch_copy_to_user) },
	{ 0xc549cccb, __VMLINUX_SYMBOL_STR(finish_wait) },
	{ 0x26f32e45, __VMLINUX_SYMBOL_STR(prepare_to_wait_event) },
	{ 0x1000e51, __VMLINUX_SYMBOL_STR(schedule) },
	{ 0xa1c76e0a, __VMLINUX_SYMBOL_STR(_cond_resched) },
	{ 0xfcec0987, __VMLINUX_SYMBOL_STR(enable_irq) },
	{ 0xf5899225, __VMLINUX_SYMBOL_STR(devm_ioport_unmap) },
	{ 0xb077e70a, __VMLINUX_SYMBOL_STR(clk_unprepare) },
	{ 0xb6e6d99d, __VMLINUX_SYMBOL_STR(clk_disable) },
	{ 0x6bc3fbc0, __VMLINUX_SYMBOL_STR(__unregister_chrdev) },
	{ 0xdd3f5252, __VMLINUX_SYMBOL_STR(devm_free_irq) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0x1fdc7df2, __VMLINUX_SYMBOL_STR(_mcount) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "C1D4323E1828BC4F677B9C9");
