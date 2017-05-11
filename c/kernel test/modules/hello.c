#include <linux/init.h>
#include <linux/module.h>
MODULE_LICENSE("Dual BSD/GPL");

static int hello_init(void)
{
    printk( "Hello, world&sm\n");
    return 0;
}

static void hello_exit(void)
{
    printk("Goodbye, cruel world\n");
}

module_init(hello_init);
module_exit(hello_exit);

 //声明模块的作者（可选）  
MODULE_AUTHOR("sunming");  
//声明模块的描述（可选）  
MODULE_DESCRIPTION("sm:This is a simple example!/n");  
//声明模块的别名（可选）  
MODULE_ALIAS("A a simplest example");  