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
	{ 0x15413d5, "del_timer" },
	{ 0x28c3bbf5, "__down_failed_interruptible" },
	{ 0xdcad5d16, "__mod_timer" },
	{ 0xda02d67, "jiffies" },
	{ 0xcbf92b8b, "__down_failed_trylock" },
	{ 0x1b7d4074, "printk" },
	{ 0x4f90a0f8, "bt_name_gp" },
	{ 0xd22b546, "__up_wakeup" },
	{ 0xea001d49, "bt_trace_lvl_g" },
};

static const char __module_depends[]
__attribute_used__
__attribute__((section(".modinfo"))) =
"depends=btp";

