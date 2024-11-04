#include <linux/kthread.h>
#include "linux/gfp.h"
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/slab.h> //kmalloc

MODULE_LICENSE("GPL");

struct list_element {
        int data;
        struct list_head list;
};

static LIST_HEAD(my_list);

static int read_list_thread_rcu(void *data) {
        while(!kthread_should_stop()) {
                struct list_element *entry;
                rcu_read_lock();
                pr_info("[ ");
                list_for_each_entry(entry, &my_list, list) {
                        pr_info("%d ", entry->data);
                        
                }
                pr_info("] \n");
                rcu_read_unlock();
                msleep(100);
        }
        return 0;
}

static int manipulate_list_thread_rcu(void *data) {
        while(!kthread_should_stop()) {
                struct list_element *entry, *temp;
                if(!list_empty(&my_list)) {
                        entry = kmalloc(sizeof(struct list_element), GFP_KERNEL);
                        temp = list_first_entry(&my_list, struct list_element, list);
                        list_del_rcu(&temp->list); //atomically
                        entry->data = temp->data + 1;
                        synchronize_rcu();
                        kfree(temp);               

                } else {
                        entry->data=0;
                }
                list_add_rcu(&entry->list, &my_list);
                msleep(200);
        }
        return 0;
}

static struct task_struct *read_thread;
static struct task_struct *manipulate_thread;

static int __init my_module_init(void) {
        int i;
        for (i=0; i<10; i++) {
                struct list_element *entry = kmalloc(sizeof(struct list_element), GFP_KERNEL);
                entry->data=i;
                INIT_LIST_HEAD(&entry->list);
                list_add_tail(&entry->list, &my_list);
        }
        read_thread = kthread_run(read_list_thread_rcu, NULL, "read_list_thread_rcu");
        manipulate_thread = kthread_run(manipulate_list_thread_rcu, NULL, "manipulate_thread_rcu");
        return 0;
}

static void __exit my_module_exit(void) { kthread_stop(read_thread); kthread_stop(manipulate_thread); }

module_init(my_module_init);
module_exit(my_module_exit);
