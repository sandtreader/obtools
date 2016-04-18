// Native Javascript implementation of ot-xmlmesh client
// Copyright (c) Paul Clark 2015.  All rights reserved.
// This code comes with NO WARRANTY and is subject to licence agreement
//
// Unusually for a JQuery plugin, $.xmlmesh is a class-like - e.g.
//
//   var xmlmesh = new $.xmlmesh("http://localhost:29180");
//
// All methods take a parameters object with at minimum a completion function:
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
// }
//
// For poll(), the completion can return 'false' if it wants polling to stop

(function($)
{
  // -------------------------------------------------------------------------
  // Constructor with server URL prefix (without trailing /)
  // Optional console for debugging (which implements log("..."))
  $.xmlmesh = function(url_prefix, console)
  {
    this.url_prefix = url_prefix;
    this.console = console;
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

  // Parse the response SOAP and return the response body
  function parseSOAP(soap)
  {
    var doc = $.parseXML(soap);
    var body = $(doc).find("env\\:Body, body");
    return body.children();
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
            params.completion(
              {
                success: false,
                error: textStatus+" "+errorThrown
              });
          },

          success: function(response)
          {
            self.log("Response: "+response);

            var result = { success: true }
            if (params.rsvp) result.response = parseSOAP(response);
            params.completion(result);
          }
        });
    },

    // Subscribe for a subject pattern with an optional ID
    // {
    //   pattern:  Subject pattern with glob
    // }
    // Also returns result.ref which is unique reference used
    subscribe: function(params)
    {
      this.log("Subscribing for "+params.pattern);
      var message = "<x:join subject='"+params.pattern+"'/>";
      var soap = makeSOAP("xmlmesh.subscription.join", true, message);
      this.log(soap);

      var ref = uniqueID();
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
            params.completion(
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
            if (params.rsvp) result.response = parseSOAP(response);
            params.completion(result);
          }
        });
    },

    // Poll for a message from an earlier subscribe()
    // {
    //   ref:   Optional ref from subscribe, otherwise uses last ref
    //          subscribed with
    //   retry: true to automatically retry timed out polls
    // }
    //
    // result.success is false and result.error null if it just times out
    poll: function(params)
    {
      this.log("Polling"+(params.retry?" with retry":""));
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
              retry = params.completion(
              {
                success: true,
                response: parseSOAP(response)
              });
            }
            else
            {
              self.log("Poll timed out, no response");

              // Don't tell them if we're retrying anyway
              if (!retry) retry = params.completion(
              {
                success: false,
                error: null
              });
            }

            // "Recurse" (not really) to restart if requested
            // Note backwards compatibility for completions that don't return
            // anything
            if (retry !== false) self.poll(params);
          }
        });
    },

    // Subscribe-and-poll - simple interface which sets up a subscribe,
    // polls it and calls back to callback function with each received
    // message ($ on XML), logging any errors and retrying automatically
    // If callback returns false (specifically) it will stop polling
    subscribe_and_poll: function(pattern, callback)
    {
      var self=this;
      this.subscribe({
        pattern: pattern,
        completion: function(sub_result)
        {
          if (sub_result.success)
          {
            self.poll({
              ref: sub_result.ref,
              retry: true,
              completion: function(poll_result)
              {
                if (poll_result.success)
                {
                  return callback(poll_result.response);
                }
                else
                {
                  self.log("Poll failed: "+poll_result.error);

                  // Resubscribe and restart
                  self.subscribe_and_poll(pattern, callback);

                  return false;  // Don't continue this poll
                }
              }
            });
          }
          else
          {
            self.log("Subscription failed: "+sub_result.error);

            // Try again after a reasonable timeout
            setTimeout(function()
                       { self.subscribe_and_poll(pattern, callback); },
                       5000); // ? make optional parameter?
          }
        }
      });
    }
  });
}(jQuery));
