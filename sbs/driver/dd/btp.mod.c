#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
 .arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x36ef467a, "struct_module" },
	{ 0xa52c7cac, "per_cpu__current_task" },
	{ 0x869ede3d, "pci_bus_read_config_byte" },
	{ 0x12da5bb2, "__kmalloc" },
	{ 0xba2513f6, "__mod_timer" },
	{ 0xc54df5e7, "mem_map" },
	{ 0xd6ee688f, "vmalloc" },
	{ 0xa5423cc4, "param_get_int" },
	{ 0xfd894486, "del_timer" },
	{ 0xb4e814a7, "page_address" },
	{        0, "init_mm" },
	{ 0xf0b57c68, "param_set_long" },
	{ 0x900e0d6, "_spin_lock" },
	{ 0x7dd8a396, "down_interruptible" },
	{ 0xac8e0310, "pci_get_class" },
	{ 0x999e8297, "vfree" },
	{ 0x1fcb2076, "pci_bus_write_config_word" },
	{ 0xcb32da10, "param_set_int" },
	{ 0xeaa456ed, "_spin_lock_irqsave" },
	{ 0x1d26aa98, "sprintf" },
	{ 0x7d11c268, "jiffies" },
	{ 0x518eb764, "per_cpu__cpu_number" },
	{ 0xa5908852, "pci_set_master" },
	{ 0xb07f7d85, "down_trylock" },
	{ 0x1b7d4074, "printk" },
	{ 0x1075bf0, "panic" },
	{ 0xb6ed1e53, "strncpy" },
	{ 0x2da418b5, "copy_to_user" },
	{ 0xa62d435f, "pci_bus_write_config_dword" },
	{ 0x27147e64, "_spin_unlock_irqrestore" },
	{ 0x61651be, "strcat" },
	{ 0x9634873b, "module_put" },
	{ 0xe914d009, "ioremap_nocache" },
	{ 0xe1c3b4c8, "pci_bus_read_config_word" },
	{ 0x2de9f66f, "param_get_long" },
	{ 0x6a59427d, "pci_bus_read_config_dword" },
	{ 0x1fc91fb2, "request_irq" },
	{ 0xd62c833f, "schedule_timeout" },
	{ 0x7684a370, "register_chrdev" },
	{ 0xa0b04675, "vmalloc_32" },
	{ 0x76c31941, "init_timer" },
	{ 0x29e3c08b, "pci_bus_write_config_byte" },
	{ 0x37a0cba, "kfree" },
	{ 0xbbc366f2, "remap_pfn_range" },
	{ 0x4cce2d0c, "pv_mmu_ops" },
	{ 0xedc03953, "iounmap" },
	{ 0x9ef749e2, "unregister_chrdev" },
	{ 0x381da1, "up" },
	{ 0xf2a644fb, "copy_from_user" },
	{ 0x9e7d6bd0, "__udelay" },
	{ 0xf20dabd8, "free_irq" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

