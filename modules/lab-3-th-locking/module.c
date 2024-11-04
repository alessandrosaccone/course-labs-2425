#include "linux/types.h"
#include <linux/kthread.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");

atomic64_t shared_variable = ATOMIC_INIT(0);
int vl = 10;
uint64_t iter = (1 << 20);

static int add_thread_fn(void *data) {
  uint64_t i;
  for (i = 0; i < iter; i++) {
    
    atomic64_add(vl, &shared_variable);
  }
  pr_info("[ADD] finished: %lld\n", shared_variable.counter);
  return 0;
}


static int subtract_thread_fn(void *data) {
  uint64_t i;
  for (i = 0; i < iter; i++) {
    atomic64_sub(vl, &shared_variable);
  }
  pr_info("[SUB] finished: %lld\n", shared_variable.counter);
  return 0;
}

void broken_share(void) { 
  kthread_run(add_thread_fn, NULL, "add_thread_fn");
  kthread_run(subtract_thread_fn, NULL, "sub_thread");


}

static int __init my_module_init(void) {
  broken_share();
  pr_info("module loaded\n");
  return 0;
}

static void __exit my_module_exit(void) {
  pr_info("Module unloaded\n");
}

module_init(my_module_init);
module_exit(my_module_exit);
