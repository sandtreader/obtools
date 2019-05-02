//==========================================================================
// ObTools::Exec: command.cc
//
// Command executor
//
// Copyright (c) 2016 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-exec.h"
#include "ot-text.h"
#include <unistd.h>
#include <sys/types.h>
#if !defined(PLATFORM_WINDOWS)
#include <sys/wait.h>
#endif
#include <errno.h>
#include <string.h>
#include <sstream>

namespace ObTools { namespace Exec {

//==========================================================================
// Capture thread class - captures output from a pipe and reflects it to
// an ostream
class CaptureThread: public MT::Thread
{
  ostream& stream;
  int stderr_pipe;
  int pid;

  virtual void run()
  {
    ostringstream oss;
    oss << "<" << pid << "> ";
    string prefix = oss.str();

    // Read from error pipe until it closes, and output to log
    char buf[1024];
    int length;
    bool first = true;
    while ((length = read(stderr_pipe, buf, 1024)) > 0)
    {
      string msg(buf, length);

      // Add prefix
      if (pid)
      {
        if (first) { stream << prefix; first = false; }
        msg = Text::subst(msg, "\n", "\n"+prefix);
      }

      stream << msg;
    }
  }

public:
  CaptureThread(ostream &_stream, int _pipe, int _pid):
    stream(_stream), stderr_pipe(_pipe), pid(_pid) { start(); }
};

//--------------------------------------------------------------------------
// Constructor from a single command - splits arguments
Command::Command(const string& command_with_args)
{
  args = Text::split_words(command_with_args);
}

//--------------------------------------------------------------------------
// Execute a command, passing the given input as stdin and capturing stdout
// as output_p.  stderr is captured to Log::Error
// Returns whether the command ran OK
bool Command::execute(const string& input, string& output_p)
{
  Log::Streams log;

#if defined(PLATFORM_WINDOWS)
  log.error << "Command execution unsupported on this platform" <<endl;
  (void)input;
  (void)output_p;
  return false;
#else
  // Create pipes - named relative to child process
  int stdin_pipe[2];
  int stdout_pipe[2];
  int stderr_pipe[2];

  if (pipe(stdin_pipe) || pipe(stdout_pipe) || pipe(stderr_pipe))
  {
    log.error << "Can't create pipes: " << strerror(errno) << endl;
    return false;
  }

  // Fork the child
  pid_t child = fork();

  if (child < 0)
  {
    log.error << "Can't fork: " << strerror(errno) << endl;
    return false;
  }

  if (child)
  {
    // PARENT PROCESS
    log.detail << "Child process pid " << child << " started\n";

    // Close input of child's stdin and output of child's stdout and stderr
    close(stdin_pipe[0]);
    close(stdout_pipe[1]);
    close(stderr_pipe[1]);

    // Start a thread to read and capture stdout
    ostringstream oss;
    CaptureThread stdout_thread(oss, stdout_pipe[0], 0);  // No prefixing

    // Start a thread to read and log stderr
    // Be doubly sure of safety by giving it a separate log stream
    Log::Error log2;
    CaptureThread stderr_thread(log2, stderr_pipe[0], child);

    // Send it the input
    ssize_t length = input.size();
    if (write(stdin_pipe[1], input.data(), length) != length)
      log.error << "Problem writing text to pipe\n";

    // Close it to indicate end
    close(stdin_pipe[1]);

    // Wait for it to exit
    int status;
    int died = waitpid(child, &status, 0);

    // Sleep for a while to allow threads to capture data
    this_thread::sleep_for(chrono::milliseconds{100});

    // Close up
    close(stdout_pipe[0]);
    close(stderr_pipe[0]);

    // Check for fatal failure
    if (died && !WIFEXITED(status))
    {
      log.error << "Child process " << child << " died\n";
      return false;
    }
    else
    {
      int rc = WEXITSTATUS(status);
      if (rc)
      {
        log.error << "Child process " << child << " returned code "
                  << rc << endl;
        return false;
      }
      else
      {
        log.detail << "Child process " << child << " returned OK\n";

        // Capture output
        output_p = oss.str();
        return true;
      }
    }
  }
  else
  {
    // CHILD PROCESS

    // Close and replace my current stdin with pipe
    dup2(stdin_pipe[0], 0);// Replace stdin with pipe input
    close(stdin_pipe[0]);  // close the pipe ends
    close(stdin_pipe[1]);

    // Same for stdout
    dup2(stdout_pipe[1], 1);
    close(stdout_pipe[0]);
    close(stdout_pipe[1]);

    // ... and stderr
    dup2(stderr_pipe[1], 2);
    close(stderr_pipe[0]);
    close(stderr_pipe[1]);

    // Create argv
    char **argv = new char *[args.size()+1];
    for(auto i=0u; i<args.size(); i++)
      argv[i] = const_cast<char *>(args[i].c_str());
    argv[args.size()] = 0;

    // Create empty envp
    char *envp[1];
    envp[0] = 0;

    // Exec the command
    execve(argv[0], argv, envp);

    // Shouldn't return, but if it does...
    cerr << "Can't exec " << argv[0] << ": " << strerror(errno) << endl;
    _exit(2);  // Hard exit, don't try to clean up
  }
#endif
}

}} // namespaces
