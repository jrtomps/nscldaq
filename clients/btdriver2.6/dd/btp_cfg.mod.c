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
	{ 0xd286f79a, "pci_find_class" },
	{ 0x2fd1d81c, "vfree" },
	{ 0xaefc1524, "pci_set_master" },
	{ 0x1b7d4074, "printk" },
	{ 0x1075bf0, "panic" },
	{ 0x9eac042a, "__ioremap" },
	{ 0x59293549, "pci_bus_read_config_word" },
	{ 0x26e96637, "request_irq" },
	{ 0xe8e437ec, "register_chrdev" },
	{ 0xa0b04675, "vmalloc_32" },
	{ 0x9f9d91f8, "pci_bus_write_config_byte" },
	{ 0x5fb196d4, "iounmap" },
	{ 0xc192d491, "unregister_chrdev" },
	{ 0xf20dabd8, "free_irq" },
};

static const char __module_depends[]
__attribute_used__
__attribute__((section(".modinfo"))) =
"depends=";

