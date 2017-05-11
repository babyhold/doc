/*copyright:  (C) 2013 fulinux<fulinux@sina.com> 
 *                  All rights reserved.
 *
 *       Filename:  mod_b.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(07/12/2013~)
 *         Author:  fulinux <fulinux@sina.com>
 *      ChangeLog:  1, Release initial version on "07/12/2013 10:29:55 AM"
 *                 
 ********************************************************************************/

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

static int func2(void)
{
    extern int func1(void);
    func1();
    printk("In Func: %s...\n",__func__);
    return 0;
}
static int func3(void)
{
    printk("In Func: %s...\n",__func__);
    return 0;
}

//EXPORT_SYMBOL(func3);
static int __init hello_init(void)
{
    printk("Module 2, Init!\n");
    func2();
    return 0;
}

static void __exit hello_exit(void)
{
    printk("Module 2, Exit!\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
