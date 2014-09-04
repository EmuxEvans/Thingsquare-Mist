#include "mist.h"

/*---------------------------------------------------------------------------*/
PROCESS(router_node_process, "Router node process");
AUTOSTART_PROCESSES(&router_node_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(router_node_process, ev, data)
{
  //  printf("Process begin\r\n");
  PROCESS_BEGIN();
  printf("RNode RPL init\r\n");

  /* Set us up as a RPL root node. */
  simple_rpl_init_dag();
  printf("Finished RPL init\r\n");

  /* Initialize the IP64 module so we'll start translating packets */
  ip64_init();
  printf("Finished IP64 init\r\n");

  /* ... and do nothing more. */
  while(1) {
    PROCESS_WAIT_EVENT();
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
