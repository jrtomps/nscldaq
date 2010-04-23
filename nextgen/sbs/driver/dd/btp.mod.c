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
};

static const struct modversion_info ____versions[]
__attribute_used__
__attribute__((section("__versions"))) = {
	{ 0xe03465db, "struct_module" },
	{ 0xd1042cd2, "pci_bus_read_config_byte" },
	{ 0x12da5bb2, "__kmalloc" },
	{ 0xec7bc0d, "__mod_timer" },
	{ 0x454e1142, "mem_map" },
	{ 0xd6ee688f, "vmalloc" },
	{ 0x89b301d4, "param_get_int" },
	{ 0x4827a016, "del_timer" },
	{ 0xce23e3a1, "page_address" },
	{ 0x321a59ac, "init_mm" },
	{ 0x1992a2ba, "param_set_long" },
	{ 0x1bcd461f, "_spin_lock" },
	{ 0xff08bc8b, "pci_get_class" },
	{ 0x2fd1d81c, "vfree" },
	{ 0xb4ae1083, "pci_bus_write_config_word" },
	{ 0x98bd6f46, "param_set_int" },
	{ 0x87cddf59, "_spin_lock_irqsave" },
	{ 0x1d26aa98, "sprintf" },
	{ 0x7d11c268, "jiffies" },
	{ 0xa61d4920, "pci_set_master" },
	{ 0x1b7d4074, "printk" },
	{ 0x1075bf0, "panic" },
	{ 0x2da418b5, "copy_to_user" },
	{ 0x2cd3086, "__down_failed_trylock" },
	{ 0xadf338a2, "pci_bus_write_config_dword" },
	{ 0x625acc81, "__down_failed_interruptible" },
	{ 0xa20fdde, "_spin_unlock_irqrestore" },
	{ 0x9eac042a, "__ioremap" },
	{ 0xd4d45529, "pci_bus_read_config_word" },
	{ 0x53a21daf, "param_get_long" },
	{ 0x4d374a7c, "pci_bus_read_config_dword" },
	{ 0x26e96637, "request_irq" },
	{ 0x17d59d01, "schedule_timeout" },
	{ 0x496d2664, "register_chrdev" },
	{ 0xa0b04675, "vmalloc_32" },
	{ 0x24b6595b, "wake_up_process" },
	{ 0x19cacd0, "init_waitqueue_head" },
	{ 0xd0b91f9b, "init_timer" },
	{ 0xc83c9d54, "pci_bus_write_config_byte" },
	{ 0x37a0cba, "kfree" },
	{ 0xad54b687, "remap_pfn_range" },
	{ 0xedc03953, "iounmap" },
	{ 0xc192d491, "unregister_chrdev" },
	{ 0x60a4461c, "__up_wakeup" },
	{ 0xf2a644fb, "copy_from_user" },
	{ 0x9e7d6bd0, "__udelay" },
	{ 0xf20dabd8, "free_irq" },
};

static const char __module_depends[]
__attribute_used__
__attribute__((section(".modinfo"))) =
"depends=";

