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
	{ 0x707f93dd, "preempt_schedule" },
	{ 0x8f0de1d9, "__preempt_spin_lock" },
	{ 0x1b7d4074, "printk" },
	{ 0x4f90a0f8, "bt_name_gp" },
	{ 0xea001d49, "bt_trace_lvl_g" },
};

static const char __module_depends[]
__attribute_used__
__attribute__((section(".modinfo"))) =
"depends=btp";

