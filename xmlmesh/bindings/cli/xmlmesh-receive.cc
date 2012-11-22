// =======================================================================
// ObTools::XMLMesh: xmlmesh-receive.cc
//
// Command-line interface to receive XMLMesh messages and spawn script to
// accept it
//
// Subscribes for messages of given subject and spawns sub-process with
// message on input, returning either OK/Error result or full message on
// output
//
// Copyright (c) 2005 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
// ======================================================================= 

#include "ot-xmlmesh-client-otmp.h"
#include "ot-log.h"
#include <fstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

using namespace std;
using namespace ObTools;

#define DEFAULT_TIMESTAMP "%a %d %b %H:%M:%*S [%*L]: "

//--------------------------------------------------------------------------
// Usage
void usage(char *pname)
{
  cout << "ObTools XMLMesh command line message receiver\n\n";
  cout << "Usage:\n";
  cout << "  " << pname << " [options] <subject> <receiver>\n\n";
  cout << "Runs as a daemon and subscribes for given <subject> and spawns <receiver>\n";
  cout << "for each message, with argv[1] as subject and message text on stdin.\n";

  cout << "Options:\n";
  cout << "  -o --observe     Observe only, don't return response even if requested\n";
  cout << "  -c --check       Check return code of receiver and send OK or Error\n";
  cout << "                   If return code is non-zero, any output will go into fault\n";
  cout << "  -r --response    Return response body from output of receiver\n";
  cout << "  -R --response-subject <subject>\n";
  cout << "                   Set subject of response (only when -r)\n";
  cout << "                   Default is received subject with '.response' appended\n";
  cout << "  -s --soap        Pass in full SOAP message wrapper\n";
  cout << "  -v --verbose     More logging\n";
  cout << "  -q --quiet       Less logging\n";
  cout << "  -l --log <file>  Send logging to a file\n";
  cout << "  -f --foreground  Run in foreground rather than as a daemon\n";
  cout << "  -1 --oneshot     Receive only one message and exit (default, loops forever)\n";
  cout << "  -h --host <host> Set XMLMesh host (default 'localhost')\n";
  cout << "  -p --port <port> Set XMLMesh port (default " 
       << XMLMesh::OTMP::DEFAULT_PORT << ")\n";
  cout << "  -? --help       Output this usage\n";
}

//--------------------------------------------------------------------------
// Main
int main(int argc, char **argv)
{
  if (argc < 3)
  {
    usage(argv[0]);
    return 0;
  }

  string subject;
  string process;
  bool check = false;
  bool get_response = false;
  bool observe = false;
  string response_subject;
  bool soap = false;
  bool foreground = false;
  bool oneshot = false;
  string host("localhost");
  int port = XMLMesh::OTMP::DEFAULT_PORT;
  int log_level = Log::LEVEL_SUMMARY;
  string logfile;

  // Parse options
  for(int i=1; i<argc; i++)
  {
    string opt(argv[i]);

    if (opt[0] == '-')
    {
      if (opt == "-o" || opt == "--observe")
	observe = true;
      else if (opt == "-c" || opt == "--check")
	check = true;
      else if (opt == "-r" || opt == "--response")
	get_response = true;
      else if ((opt == "-R" || opt == "--response-subject") && i<argc-2)
	response_subject = argv[++i];
      else if (opt == "-s" || opt == "--soap")
	soap = true;
      else if (opt == "-v" || opt == "--verbose")
	log_level++;
      else if (opt == "-q" || opt == "--quiet")
	log_level--;
      else if ((opt == "-l" || opt == "--log") && i<argc-2)
	logfile = argv[++i];
      else if (opt == "-f" || opt == "--foreground")
	foreground = true;
      else if (opt == "-1" || opt == "--oneshot")
	oneshot = true;
      else if ((opt == "-h" || opt == "--host") && i<argc-2)
	host = argv[++i];
      else if ((opt == "-p" || opt == "--port") && i<argc-2)
	port = atoi(argv[++i]);
      else if (opt == "-?" || opt == "--help")
      {
	usage(argv[0]);
	return 0;
      }
      else
      {
	cerr << "Unknown option: " << opt << endl;
	usage(argv[0]);
	return 2;
      }
    }
    else if (subject.empty())
      subject = opt;
    else if (process.empty())
      process = opt;
    else
    {
      cerr << "Extra arguments ignored\n";
      break;
    }
  }

  // Go daemon
  if (!foreground && daemon(0,0))
  {
    cerr << "Can't become daemon: " << strerror(errno) << endl;
    return 2;
  }

  // Set up logging
  Log::StreamChannel *chan_out;
  if (logfile.empty())
  {
    chan_out = new Log::StreamChannel(cout);
  }
  else
  {
    ofstream *logstream = new ofstream(logfile.c_str(),ios::app);
    if (!*logstream)
    {
      cerr << "Unable to open logfile " << logfile << endl;
      return 2;
    }
    chan_out = new Log::StreamChannel(*logstream);
  }

  Log::TimestampFilter tsfilter(DEFAULT_TIMESTAMP, *chan_out);
  Log::LevelFilter     level_out((Log::Level)log_level, tsfilter);
  Log::logger.connect(level_out);
  Log::Streams log;

  // Resolve name
  Net::IPAddress addr(host);
  if (!addr)
  {
    log.error << "Can't resolve host: " << host << endl;
    return 1;
  }

  log.summary << "Host: " << addr 
	      << " (" << addr.get_hostname() << ")" << endl;

  // Start client
  Net::EndPoint server(addr, port);
  XMLMesh::OTMPClient client(server);

  // Subscribe for subject
  log.summary << "Subscribing for subject: " << subject << endl;
  if (!client.subscribe(subject))
  {
    log.error << "Can't subscribe to XMLMesh\n";
    return 2;
  }

  // Loop forever unless oneshot
  do
  {
    XMLMesh::Message msg;

    // Get message
    if (!client.wait(msg))
    {
      log.error << "Message transport restarted: Messages might have been missed\n";
      continue;
    }

    string subject = msg.get_subject();
    string text;
    bool rsvp = msg.get_rsvp();

    if (soap)
      text = msg.get_text();  // The whole thing
    else
      text = msg.get_body().to_string();  // Just the first body

    log.summary << "Received message, subject " << subject 
		<< (rsvp?", RSVP":"") << endl;
    log.detail << text;

    // Create pipes - named relative to child process
    int stdin_pipe[2];
    int stdout_pipe[2];

    if (pipe(stdin_pipe) || pipe(stdout_pipe))
    {
      log.error << "Can't create pipes: " << strerror(errno) << endl;
      return 2;
    }

    // Fork the child
    pid_t child = fork();

    if (child < 0)
    {
      log.error << "Can't fork: " << strerror(errno) << endl;
      return 2;
    }

    if (child)
    {
      // PARENT PROCESS
      log.summary << "Child '" << process << "' pid " << child << " started\n";

      // Close input of child's stdin and output of child's stdout
      close(stdin_pipe[0]);
      close(stdout_pipe[1]);

      // Send it some stuff!
      ssize_t length = text.size();
      if (write(stdin_pipe[1], text.data(), length) != length)
	log.error << "Problem writing text to pipe\n";

      // Close it to indicate end
      close(stdin_pipe[1]);

      // Read back error or output
      string response;
      if (check || get_response)
      {
	char buf[1024];
	while ((length = read(stdout_pipe[0], buf, 1024)) > 0)
	  response.append(buf, length);

	log.detail << "Child response:\n" << response;
      }

      // Close its output
      close(stdout_pipe[0]);

      // Wait for it to exit
      int status;
      int died = waitpid(child, &status, 0);

      // Check for fatal failure
      if (died && !WIFEXITED(status))
      {
	log.error << "Child process died\n";

	// Respond with fatal error if requested
	if (rsvp) 
	  client.respond(SOAP::Fault::CODE_RECEIVER, 
			 "Receiving process failed", msg);
      }
      else
      {
	int rc = WEXITSTATUS(status);
	if (rc)
	{
	  log.error << "Child process returned code " << rc 
		    << ", response " << response;

	  // Respond with error, capturing any output as fault
	  if (rsvp)
	    client.respond(SOAP::Fault::CODE_RECEIVER, response, msg);
	}
	else
	{
	  log.summary << "Child process returned OK\n";

	  // Respond OK or with response
	  if (rsvp)
	  {
	    if (check)
	    {
	      client.respond(msg); // Simple OK
	    }
	    else if (get_response)
	    {
	      // Create response from output

	      // Create default response subject from incoming if not set
	      if (response_subject.empty())
		response_subject = subject+".response";
	      log.summary << "Sending response, subject " 
			  << response_subject << endl;

	      XMLMesh::Message rmsg(response_subject, 
				    response, false, msg.get_id());
	      client.send(rmsg);
	    }
	    else if (!observe)
	    {
	      log.error << "RSVP requested but no --observe, "
			<< "--check nor --get_response specified\n";
	      client.respond(SOAP::Fault::CODE_RECEIVER, 
			     "Receiver not configured to return result", msg);
	    }
	  }
	}
      }
    }
    else
    {
      // CHILD PROCESS

      // Close and replace my current stdin with pipe
      dup2(stdin_pipe[0], 0);// Replace stdin with the pipe
      close(stdin_pipe[0]);  // close the pipe ends
      close(stdin_pipe[1]);

      // Same for stdout
      dup2(stdout_pipe[1], 1);
      close(stdout_pipe[0]);
      close(stdout_pipe[1]);

      // stderr remains the same
      // Exec the receiver
      if (execl(process.c_str(), process.c_str(), subject.c_str(),
                (char *)NULL))
      {
	log.error << "Can't exec " << process << ": " << strerror(errno) << endl;
	log.summary << "Can't start receiving process\n";
	return 2;
      }
    }
  } while (!oneshot);

  // Sleep to ensure messages sent
  MT::Thread::sleep(1);

  return 0;
}





