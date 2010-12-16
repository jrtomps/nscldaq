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
	{ 0xd0b96c58, "module_layout" },
	{ 0x6bc3fbc0, "__unregister_chrdev" },
	{ 0x4e7ebfd2, "per_cpu__current_task" },
	{ 0x7a925d98, "pci_bus_read_config_byte" },
	{ 0x12da5bb2, "__kmalloc" },
	{ 0xe23d7acb, "up_read" },
	{ 0x9bbf4ae3, "mem_map" },
	{ 0xd6ee688f, "vmalloc" },
	{ 0xb279da12, "pv_lock_ops" },
	{ 0x6980fe91, "param_get_int" },
	{ 0x6307fc98, "del_timer" },
	{ 0x3457cb68, "param_set_long" },
	{ 0x54eca47a, "set_page_dirty_lock" },
	{ 0x973873ab, "_spin_lock" },
	{ 0x43ab66c3, "param_array_get" },
	{ 0x45d11c43, "down_interruptible" },
	{ 0xec7095ba, "__register_chrdev" },
	{ 0x8fa1fc54, "x86_dma_fallback_dev" },
	{ 0x6a9f26c9, "init_timer_key" },
	{ 0x999e8297, "vfree" },
	{ 0x540147ec, "pci_bus_write_config_word" },
	{ 0xff964b25, "param_set_int" },
	{ 0x712aa29b, "_spin_lock_irqsave" },
	{ 0x45947727, "param_array_set" },
	{ 0x7d11c268, "jiffies" },
	{ 0xb4b0ee4e, "down_read" },
	{ 0x9629486a, "per_cpu__cpu_number" },
	{ 0xe006774a, "pci_set_master" },
	{ 0x7b6902fa, "pci_set_dma_mask" },
	{ 0x9ced38aa, "down_trylock" },
	{ 0xb72397d5, "printk" },
	{ 0xacdeb154, "__tracepoint_module_get" },
	{ 0xe52592a, "panic" },
	{ 0xce5556ff, "kunmap" },
	{ 0xb6ed1e53, "strncpy" },
	{ 0x2f287f0d, "copy_to_user" },
	{ 0xb4390f9a, "mcount" },
	{ 0x7fae5bb, "pci_bus_write_config_dword" },
	{ 0x748caf40, "down" },
	{ 0x4b07e779, "_spin_unlock_irqrestore" },
	{ 0x46085e4f, "add_timer" },
	{ 0x55b3133e, "dma_release_from_coherent" },
	{ 0xfda85a7d, "request_threaded_irq" },
	{ 0xd999c118, "dma_alloc_from_coherent" },
	{ 0x61651be, "strcat" },
	{ 0x2de71793, "module_put" },
	{ 0x3af98f9e, "ioremap_nocache" },
	{ 0xb2007960, "pci_bus_read_config_word" },
	{ 0x160fa2ba, "kmap" },
	{ 0x8bd5b603, "param_get_long" },
	{ 0x1629d02b, "pci_bus_read_config_dword" },
	{ 0xf0fdf6cb, "__stack_chk_fail" },
	{ 0x6cb6fb79, "get_user_pages" },
	{ 0x108e8985, "param_get_uint" },
	{ 0xd62c833f, "schedule_timeout" },
	{ 0xa8eecda3, "pci_unregister_driver" },
	{ 0x7ecb001b, "__per_cpu_offset" },
	{ 0x3ee28e7a, "pci_bus_write_config_byte" },
	{ 0x37a0cba, "kfree" },
	{ 0x5389965d, "remap_pfn_range" },
	{ 0x3285cc48, "param_set_uint" },
	{ 0xedc03953, "iounmap" },
	{ 0x3f1899f1, "up" },
	{ 0x6388a99e, "__pci_register_driver" },
	{ 0x348ec935, "put_page" },
	{ 0x9279ec0e, "vmalloc_to_page" },
	{ 0xccc986bb, "pci_enable_device" },
	{ 0x25a3cdb2, "pci_set_consistent_dma_mask" },
	{ 0xd6c963c, "copy_from_user" },
	{ 0x9e7d6bd0, "__udelay" },
	{ 0x3252bcd, "dma_ops" },
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

MODULE_INFO(srcversion, "8D88A1E15DB27AE29737170");
