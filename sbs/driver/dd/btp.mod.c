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
	{ 0xa7672d5a, "module_layout" },
	{ 0x6bc3fbc0, "__unregister_chrdev" },
	{ 0x2e9014ad, "per_cpu__current_task" },
	{ 0x53319a21, "pci_bus_read_config_byte" },
	{ 0x12da5bb2, "__kmalloc" },
	{ 0xe23d7acb, "up_read" },
	{ 0x51150e8d, "mem_map" },
	{ 0xd6ee688f, "vmalloc" },
	{ 0xb279da12, "pv_lock_ops" },
	{ 0x6980fe91, "param_get_int" },
	{ 0x6307fc98, "del_timer" },
	{ 0x3457cb68, "param_set_long" },
	{ 0xd46e5e9e, "set_page_dirty_lock" },
	{ 0x973873ab, "_spin_lock" },
	{ 0x43ab66c3, "param_array_get" },
	{ 0x45d11c43, "down_interruptible" },
	{ 0xe032fea4, "__register_chrdev" },
	{ 0xd1356df6, "x86_dma_fallback_dev" },
	{ 0x6a9f26c9, "init_timer_key" },
	{ 0x999e8297, "vfree" },
	{ 0xec0d0161, "pci_bus_write_config_word" },
	{ 0xff964b25, "param_set_int" },
	{ 0x712aa29b, "_spin_lock_irqsave" },
	{ 0x45947727, "param_array_set" },
	{ 0x7d11c268, "jiffies" },
	{ 0xb4b0ee4e, "down_read" },
	{ 0x9629486a, "per_cpu__cpu_number" },
	{ 0xee72d4cb, "pci_set_master" },
	{ 0x861fa3c3, "pci_set_dma_mask" },
	{ 0x9ced38aa, "down_trylock" },
	{ 0xb72397d5, "printk" },
	{ 0xacdeb154, "__tracepoint_module_get" },
	{ 0xe52592a, "panic" },
	{ 0xca00976d, "kunmap" },
	{ 0xb6ed1e53, "strncpy" },
	{ 0x2f287f0d, "copy_to_user" },
	{ 0xb4390f9a, "mcount" },
	{ 0x9115df12, "pci_bus_write_config_dword" },
	{ 0x748caf40, "down" },
	{ 0x4b07e779, "_spin_unlock_irqrestore" },
	{ 0x46085e4f, "add_timer" },
	{ 0x55b3133e, "dma_release_from_coherent" },
	{ 0xfda85a7d, "request_threaded_irq" },
	{ 0xd999c118, "dma_alloc_from_coherent" },
	{ 0x61651be, "strcat" },
	{ 0x7102133b, "module_put" },
	{ 0x3af98f9e, "ioremap_nocache" },
	{ 0x8a1b1633, "pci_bus_read_config_word" },
	{ 0xb9cbdeac, "kmap" },
	{ 0x8bd5b603, "param_get_long" },
	{ 0x1bc2af34, "pci_bus_read_config_dword" },
	{ 0xf0fdf6cb, "__stack_chk_fail" },
	{ 0xf1b972ca, "get_user_pages" },
	{ 0x108e8985, "param_get_uint" },
	{ 0xd62c833f, "schedule_timeout" },
	{ 0x5a020d9d, "pci_unregister_driver" },
	{ 0x7ecb001b, "__per_cpu_offset" },
	{ 0x23ea5e67, "pci_bus_write_config_byte" },
	{ 0x37a0cba, "kfree" },
	{ 0x5f557f9b, "remap_pfn_range" },
	{ 0x3285cc48, "param_set_uint" },
	{ 0xedc03953, "iounmap" },
	{ 0x3f1899f1, "up" },
	{ 0x29ecd539, "__pci_register_driver" },
	{ 0x77eb0156, "put_page" },
	{ 0x9f614c32, "vmalloc_to_page" },
	{ 0x8a802dbd, "pci_enable_device" },
	{ 0x5dee90ee, "pci_set_consistent_dma_mask" },
	{ 0xd6c963c, "copy_from_user" },
	{ 0x9e7d6bd0, "__udelay" },
	{ 0x78968d6e, "dma_ops" },
	{ 0xf20dabd8, "free_irq" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

MODULE_ALIAS("pci:v0000108Ad00000001sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v0000108Ad00000002sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v0000108Ad00000003sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v0000108Ad00000004sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v0000108Ad00000010sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v0000108Ad00000011sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v0000108Ad00000040sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v0000108Ad00000041sv*sd*bc*sc*i*");

MODULE_INFO(srcversion, "FE9776C6614F13D6DC7456D");
