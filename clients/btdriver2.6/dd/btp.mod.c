#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

#undef unix
struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = __stringify(KBUILD_MODNAME),
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
};

static const struct modversion_info ____versions[]
__attribute_used__
__attribute__((section("__versions"))) = {
	{ 0x4cdc490c, "cleanup_module" },
	{ 0xcd58c830, "init_module" },
	{ 0x6fdd1d72, "struct_module" },
	{ 0x899b91ff, "pci_bus_read_config_byte" },
	{ 0x7da8156e, "__kmalloc" },
	{ 0xdcad5d16, "__mod_timer" },
	{ 0x124a3f3d, "mem_map" },
	{ 0xd6ee688f, "vmalloc" },
	{ 0x15413d5, "del_timer" },
	{ 0x3e1555ec, "page_address" },
	{ 0x497e9ef3, "init_mm" },
	{ 0xd286f79a, "pci_find_class" },
	{ 0x2fd1d81c, "vfree" },
	{ 0x449e4b51, "pci_bus_write_config_word" },
	{ 0x1d26aa98, "sprintf" },
	{ 0xda02d67, "jiffies" },
	{ 0x9d46b4f7, "remap_page_range" },
	{ 0xaefc1524, "pci_set_master" },
	{ 0x1b7d4074, "printk" },
	{ 0x8f0de1d9, "__preempt_spin_lock" },
	{ 0x1075bf0, "panic" },
	{ 0x2f287f0d, "copy_to_user" },
	{ 0xcbf92b8b, "__down_failed_trylock" },
	{ 0xe5df0a6, "pci_bus_write_config_dword" },
	{ 0x28c3bbf5, "__down_failed_interruptible" },
	{ 0x707f93dd, "preempt_schedule" },
	{ 0x9eac042a, "__ioremap" },
	{ 0x59293549, "pci_bus_read_config_word" },
	{ 0x65aabd33, "pci_bus_read_config_dword" },
	{ 0x26e96637, "request_irq" },
	{ 0x17d59d01, "schedule_timeout" },
	{ 0xe8e437ec, "register_chrdev" },
	{ 0xa0b04675, "vmalloc_32" },
	{ 0xdbf7a91d, "wake_up_process" },
	{ 0x9f9d91f8, "pci_bus_write_config_byte" },
	{ 0x37a0cba, "kfree" },
	{ 0x5fb196d4, "iounmap" },
	{ 0xc192d491, "unregister_chrdev" },
	{ 0xd22b546, "__up_wakeup" },
	{ 0xd6c963c, "copy_from_user" },
	{ 0x9e7d6bd0, "__udelay" },
	{ 0xf20dabd8, "free_irq" },
};

static const char __module_depends[]
__attribute_used__
__attribute__((section(".modinfo"))) =
"depends=";

