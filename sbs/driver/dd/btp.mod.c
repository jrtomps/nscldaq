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
__attribute_used__
__attribute__((section("__versions"))) = {
	{ 0x2db52867, "struct_module" },
	{ 0xb898620f, "per_cpu__current_task" },
	{ 0xc3762f07, "pci_bus_read_config_byte" },
	{ 0x12da5bb2, "__kmalloc" },
	{ 0x83e84bbe, "__mod_timer" },
	{ 0xabd8b649, "mem_map" },
	{ 0xd6ee688f, "vmalloc" },
	{ 0xa5423cc4, "param_get_int" },
	{ 0x98b1f5e8, "del_timer" },
	{ 0x2eeaa550, "page_address" },
	{ 0xf66a23ee, "init_mm" },
	{ 0xf0b57c68, "param_set_long" },
	{ 0xc8be7b15, "_spin_lock" },
	{ 0x707531a0, "pci_get_class" },
	{ 0x2fd1d81c, "vfree" },
	{ 0x74d3838, "pci_bus_write_config_word" },
	{ 0xcb32da10, "param_set_int" },
	{ 0x6e185827, "_spin_lock_irqsave" },
	{ 0x1d26aa98, "sprintf" },
	{ 0x7d11c268, "jiffies" },
	{ 0x518eb764, "per_cpu__cpu_number" },
	{ 0x823991f4, "pci_set_master" },
	{ 0x1b7d4074, "printk" },
	{ 0x1075bf0, "panic" },
	{ 0xb6ed1e53, "strncpy" },
	{ 0x2da418b5, "copy_to_user" },
	{ 0x2cd3086, "__down_failed_trylock" },
	{ 0x7e234193, "pci_bus_write_config_dword" },
	{ 0x625acc81, "__down_failed_interruptible" },
	{ 0xa46dc984, "_spin_unlock_irqrestore" },
	{ 0x9eac042a, "__ioremap" },
	{ 0x61651be, "strcat" },
	{ 0xcce39fd7, "module_put" },
	{ 0x742b9d2a, "pci_bus_read_config_word" },
	{ 0x2de9f66f, "param_get_long" },
	{ 0x823bd1da, "pci_bus_read_config_dword" },
	{ 0x1fc91fb2, "request_irq" },
	{ 0x17d59d01, "schedule_timeout" },
	{ 0x53c6e75c, "register_chrdev" },
	{ 0xa0b04675, "vmalloc_32" },
	{ 0xffd3c7, "init_waitqueue_head" },
	{ 0x679a54f2, "init_timer" },
	{ 0xa1af499f, "pci_bus_write_config_byte" },
	{ 0x37a0cba, "kfree" },
	{ 0x7289bfc1, "remap_pfn_range" },
	{ 0x2e60bace, "memcpy" },
	{ 0x3df0b201, "pv_mmu_ops" },
	{ 0xedc03953, "iounmap" },
	{ 0x9ef749e2, "unregister_chrdev" },
	{ 0x60a4461c, "__up_wakeup" },
	{ 0xf2a644fb, "copy_from_user" },
	{ 0x9e7d6bd0, "__udelay" },
	{ 0xf20dabd8, "free_irq" },
};

static const char __module_depends[]
__attribute_used__
__attribute__((section(".modinfo"))) =
"depends=";

