    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&hello_timer));
  }

  PROCESS_END();
}
