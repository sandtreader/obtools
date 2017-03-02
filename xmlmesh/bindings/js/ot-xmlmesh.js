// Native Javascript implementation of ot-xmlmesh client
// Copyright (c) Paul Clark 2015.  All rights reserved.
// This code comes with NO WARRANTY and is subject to licence agreement
//
// Unusually for a JQuery plugin, $.xmlmesh is a class-like - e.g.
//
//   var xmlmesh = new $.xmlmesh("http://localhost:29180");
//
// All methods take a parameters object with at minimum an optional
// completion function:
// {
//   completion: function(result) { ... }
// }
//
// Additional parameters are documented with each method
//
// The completion function is called with the following result object:
// {
//   success:  true/false
//   error:    Error message if failed
//   response: Response XML JQuery if any
//   subject:  Response subject if any
//   id:       Poll ID for polls
// }
//
// For poll(), the completion can return 'false' if it wants polling to stop
// and will be called with success=false, error=null as the poll times out
// Likewise subscribeAndPoll - all subscribers will be called with null on
// timeout, and can return false to unsubscribe (but the poller keeps running)

(function($)
{
  // -------------------------------------------------------------------------
  // Constructor with server URL prefix (without trailing /)
  // Optional console for debugging (which implements log("..."))
  $.xmlmesh = function(url_prefix, console)
  {
    this.url_prefix = url_prefix;
    this.console = console;
    this.poller_ref = null;
    this.poller_started = false;
    this.subscribers = [];
    this.log("Created new client interface on "+url_prefix);
  }

  // -------------------------------------------------------------------------
  // Internals

  // Generate a random request ID
  function uniqueID()
  {
    var id = "";
    for(var i=0; i<8; i++)
      id += Math.floor(Math.random()*0x10000).toString(16);
    id+="-";
    id+=new Date().getTime().toString(16);
    return id;
  }

  // Construct a full SOAP message from a raw body content, with a unique ID
  function makeSOAP(subject, rsvp, message)
  {
    // Invent ID
    var id = uniqueID();

    var soap;
    soap  = "<env:Envelope xmlns:env='http://www.w3.org/2003/05/soap-envelope'";
    soap += " xmlns:x='http://obtools.com/ns/xmlmesh'>\n";
    soap += "  <env:Header>\n";
    soap += "    <x:routing env:mustUnderstand='true' env:relay='true'\n";
    soap += "     env:role='http://www.w3.org/2003/05/soap-envelope/role/next'\n";
    soap += "     x:id='"+id+"' x:subject='"+subject+"'"+(rsvp?" x:rsvp='true'":"")+"/>\n";
    soap += "  </env:Header>\n";
    soap += "  <env:Body>\n";
    soap += "    "+message+"\n";
    soap += "  </env:Body>\n";
    soap += "</env:Envelope>\n";

    return soap;
  }

  // Parse the response SOAP and return the response body and subject as
  // {body: , subject: }
  function parseSOAP(soap)
  {
    var doc = $.parseXML(soap);
    var body = $(doc).find("env\\:Body, body").children();
    var routing = $(doc).find("x\\:routing, routing");
    var subject = routing.attr("x:subject") || routing.attr("subject");
    return { body: body, subject: subject };
  }

  // Very basic glob match using regular expressions
  function glob(pattern, input)
  {
    // Escape everything interesting in pattern _except_ *
    pattern = pattern.replace(/[.+?^${}()|[\]\\]/g, '\\$&');

    // Replace * with .*
    pattern = pattern.replace(/\*/g, '.*');

    // Use regex to test
    return new RegExp(pattern).test(input);
  }

  // --------------------------------------------------------------------------
  // Public interface
  $.extend($.xmlmesh.prototype,
  {
    // Log a string
    log: function(text)
    {
      if (this.console) this.console.log("XMLMesh: "+text);
    },

    // Send a message, optionally waiting for response
    // {
    //   subject:  XMLMesh subject
    //   message:  XML message (SOAP body content)
    //   rsvp:     Whether to wait for a response
    // }
    send: function(params)
    {
      this.log("Sending "+params.subject+", "
               +(params.rsvp?"with":"no")+" response:");

      var soap = makeSOAP(params.subject, params.rsvp, params.message);
      this.log(soap);

      var url = this.url_prefix+(params.rsvp?"/request":"/send");
      var self = this;
      $.ajax(
        {
          type: "post",
          url: url,
          data: soap,
          dataType: "text",

          error:function(jqXHR, textStatus, errorThrown)
          {
            self.log("POST error: " + textStatus + " " + errorThrown);
            if (params.completion) params.completion(
              {
                success: false,
                error: textStatus+" "+errorThrown
              });
          },

          success: function(response)
          {
            self.log("Response: "+response);

            var result = { success: true }
            if (params.rsvp) result.response = parseSOAP(response).body;
            if (params.completion) params.completion(result);
          }
        });
    },

    // Subscribe for a subject pattern with an optional ID
    // {
    //   pattern:  Subject pattern with glob
    //   ref:      Optional ref (or random one invented)
    // }
    // Also returns result.ref which is unique reference used
    subscribe: function(params)
    {
      this.log("Subscribing for "+params.pattern);
      var message = "<x:join subject='"+params.pattern+"'/>";
      var soap = makeSOAP("xmlmesh.subscription.join", true, message);
      this.log(soap);

      var ref = params.ref || uniqueID();
      this.last_ref = ref;
      var url = this.url_prefix+"/subscribe/"+ref;
      var self = this;
      $.ajax(
        {
          type: "post",
          url: url,
          data: soap,
          dataType: "text",

          error:function(jqXHR, textStatus, errorThrown)
          {
            self.log("Subscribe error: " + textStatus + " " + errorThrown);
            if (params.completion) params.completion(
              {
                success: false,
                error: textStatus+" "+errorThrown
              });
          },

          success: function(response)
          {
            self.log("Response: "+response);

            var result =
            {
              success: true,
              ref: ref
            };
            if (params.rsvp) result.response = parseSOAP(response).body;
            if (params.completion) params.completion(result);
          }
        });
    },

    // Unsubscribe for a subject pattern
    // {
    //   pattern:  Subject pattern with glob
    //   ref:      Mandatory ref used in subscribe()
    // }
    unsubscribe: function(params)
    {
      this.log("Unsubscribing for "+params.pattern);
      var message = "<x:leave subject='"+params.pattern+"'/>";
      var soap = makeSOAP("xmlmesh.subscription.leave", true, message);
      this.log(soap);

      var ref = params.ref;
      var url = this.url_prefix+"/unsubscribe/"+ref;
      var self = this;
      $.ajax(
        {
          type: "post",
          url: url,
          data: soap,
          dataType: "text",

          error:function(jqXHR, textStatus, errorThrown)
          {
            self.log("Unsubscribe error: " + textStatus + " " + errorThrown);
            if (params.completion) params.completion(
              {
                success: false,
                error: textStatus+" "+errorThrown
              });
          },

          success: function(response)
          {
            self.log("Response: "+response);

            var result =
            {
              success: true,
              ref: ref
            };
            if (params.rsvp) result.response = parseSOAP(response).body;
            if (params.completion) params.completion(result);
          }
        });
    },

    // Poll for a message from an earlier subscribe()
    // {
    //   ref:   Optional ref from subscribe, otherwise uses last ref
    //          subscribed with
    //   retry: true to automatically retry polls
    //   id:    poll ID returned to callback
    // }
    //
    // params.completion is mandatory
    // result.success is false and result.error null if it just times out
    poll: function(params)
    {
      this.log("Polling"+(params.id?" ID "+params.id:"")
                        +(params.retry?" with retry":""));
      var ref = params.ref || this.last_ref || "unknown";
      var url = this.url_prefix+"/poll/"+ref;
      var self = this;
      $.ajax(
        {
          type: "get",
          url: url,
          dataType: "text",

          error:function(jqXHR, textStatus, errorThrown)
          {
            self.log("Poll error: " + textStatus + " " + errorThrown);
            params.completion(
            {
              success: false,
              error: textStatus+" "+errorThrown
            });
          },

          success: function(response)
          {
            var retry = params.retry || false;
            if (response.length)
            {
              self.log("Polled response: "+response);
              var soap = parseSOAP(response);
              self.log("Subject: "+soap.subject);
              if (params.completion(
                {
                  success:  true,
                  response: soap.body,
                  subject:  soap.subject,
                  id:       params.id
                }) === false) retry = false;
            }
            else
            {
              self.log("Poll timed out, no response");
              if (params.completion(
                {
                  success: false,
                  error:   null,
                  id:      params.id
                }) === false) retry = false;
            }

            // "Recurse" (not really) to restart if requested
            // Note backwards compatibility for completions that don't return
            // anything
            if (retry !== false) self.poll(params);
          }
        });
    },

    // Multiplexed subscribe - uses a single XMLMesh connection and
    // demultiplexes the results before passing back.  Handles its own
    // polling internally.
    // callback is called with XML message body ($) and id, or null and id
    // for a poll timeout - in either case return false to unsubscribe again
    subscribeAndPoll: function(pattern, callback, id)
    {
      var self=this;

      // Invent poller ref if not already done -
      // will be used for all subscribes as well
      if (!this.poller_ref) this.poller_ref = uniqueID();

      // If creation of poller wanted, agree to do it later to prevent race
      // Note in current server, at least one subscribe request must be sent
      // on a given ref before the poller can be activated - if that ever
      // changed, this could be simpler.
      var start_poller = false;
      if (!this.poller_started)
      {
        start_poller = true;
        this.poller_started = true;
      }

      // Do subscription
      this.subscribe({
        pattern: pattern,
        ref: this.poller_ref,
        completion: function(sub_result)
        {
          if (sub_result.success)
          {
            // Add to subscribers
            self.subscribers.push({ pattern: pattern,
                                    callback: callback,
                                    id: id });

            if (start_poller)
            {
              function unsub(sub_index)
              {
                var sub = self.subscribers[sub_index];
                self.unsubscribe({ pattern: sub.pattern,
                                   ref: self.poller_ref });

                // Remove from our list
                self.subscribers.splice(sub_index, 1);
              }

              // Start permanent poller
              self.poll({
                ref: self.poller_ref,
                retry: true,
                completion: function(poll_result)
                {
                  if (poll_result.success)
                  {
                    // Call any matching subscribers with this message
                    for(var i=0; i<self.subscribers.length; i++)
                    {
                      var sub = self.subscribers[i];

                      if (glob(sub.pattern, poll_result.subject)
                          && !sub.callback(poll_result.response, sub.id))
                        unsub(i);
                    }
                    return true;  // Always keep running
                  }
                  else if (poll_result.error)
                  {
                    self.log("Poll failed: "+poll_result.error);
                    self.poller_started = false;
                    return false;  // Forget this one, try again
                  }
                  else // Poll timed out
                  {
                    // Call all subscribers with null so they get a chance
                    // to unsubscribe
                    for(var i=0; i<self.subscribers.length; i++)
                    {
                      var sub = self.subscribers[i];
                      if (!sub.callback(null, sub.id))
                        unsub(i);
                    }

                    // But continue the poller anyway
                    return true;
                  }

                }
              });
            }
          }
          else
          {
            self.log("Subscription failed: "+sub_result.error)

            // If we said we'd start the poller, own up
            if (start_poller) self.poller_started = false;

            // Try again after a reasonable timeout
            setTimeout(function()
                       { self.subscribeAndPoll(pattern, callback, id); },
                       5000); // ? make optional parameter?
          }
        }
      });

    }
  });
}(jQuery));
