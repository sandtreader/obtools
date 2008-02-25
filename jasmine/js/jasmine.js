// Jasmine animation library for Javascript
// (c) xMill Consulting Limited 2008.  All rights reserved.

// Set JASMINE_PATH to relative URL path to the Jasmine library directory
if (!JASMINE_PATH) var JASMINE_PATH="jasmine/";

// Main object, which provides the overall namespace
if (!Jasmine)
{
  var Jasmine = new Object();

  // =============================================================
  // Universe object (singleton class)
  Jasmine.Universe = new Object();

  // List of tickers
  Jasmine.Universe.tickers = [];

  // Start method
  Jasmine.Universe.start = function()
  {
    // Current time
    this.now = new Date().getTime();

    // Start tick at default interval
    setInterval("Jasmine.Universe.tick()", 40);
  }

  // Register a ticker
  // Tickers must provide a tick(now) function
  Jasmine.Universe.add_ticker = function(t)
  {
    this.tickers.push(t);
  }

  // Universe interval tick
  Jasmine.Universe.tick = function()
  {
    // Get new time
    this.now = new Date().getTime();
      
    // Call all tickers
    for (t in this.tickers) this.tickers[t].tick(this.now);
  }
}

