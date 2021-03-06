/**
 * Copyright (C) 2015 Dato, Inc.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license. See the LICENSE file for details.
 */
#include <cppipc/client/console_cancel_handler_unix.hpp>
#include <cppipc/client/comm_client.hpp>
#include <errno.h>
#include <string.h>
#include <util/syserr_reporting.hpp>
#include <export.hpp>

namespace cppipc {

// Set the interrupted flag and take the currently running command as 
// the one we want to cancel
void cancel_handler(int param) {
  console_cancel_handler::get_instance().set_cancel_flag(true);
  auto &c = get_cancelled_command();
  auto &r = get_running_command();
  c.store(r.load());
}

EXPORT console_cancel_handler& console_cancel_handler::get_instance() {
  static console_cancel_handler_unix instance;
  return instance;
}

console_cancel_handler_unix::console_cancel_handler_unix() : super() {
  m_sigint_act.sa_handler = cancel_handler;
  sigemptyset(&m_sigint_act.sa_mask);
  m_sigint_act.sa_flags = 0;
}

bool console_cancel_handler_unix::set_handler() {
  //TODO: Make specific errors available
  if(sigaction(SIGINT, NULL, &m_prev_sigint_act) < 0) {
    logstream(LOG_INFO) << "Error saving signal handler: " <<
      strerror(errno) << std::endl;
    return false;
  }

  if(sigaction(SIGINT, &m_sigint_act, NULL) < 0) {
    logstream(LOG_INFO) << "Error loading signal handler: " <<
      strerror(errno) << std::endl;
    return false;
  }

  m_handler_installed = true;

  return true;
}

bool console_cancel_handler_unix::unset_handler() {
  if(!m_handler_installed) {
    return false;
  }

  if(sigaction(SIGINT, &m_prev_sigint_act, NULL) < 0) {
    logstream(LOG_INFO) << "Error loading previous signal handler: " <<
      strerror(errno) << std::endl;
    return false;
  }

  m_handler_installed = false;

  return true;
}

void console_cancel_handler_unix::raise_cancel() {
  raise(SIGINT);
}

} // namespace cppipc
