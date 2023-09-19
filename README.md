# Watchdog Debug Extension

This project is a WinDbg extension that adds several tools to assist in debugging the D3D kernel (dxgkrnl.sys), primarily the portions of it which call into watchdog.sys to perform error reporting and logging.

Watchdog.sys contains functionality to report logging for a number of different categories:
 - 0 Critical
 - 1 Assertion
 - 2 Error
 - 3 Warning
 - 4 Event
 - 5 Trace
 - 6 LowResource
 - 7 DmmEvent
 - 8 PresentTokenEvent
 - 9 Power
 - A Debug

When the dxgkrnl performs various tasks or needs to report the status of things, it calls into watchdog.sys to create a new log entry in one of these categories from a per-category circular buffer. 

Each log entry is 64 bytes (eight UINT64s), and contains the following information:
 - Event Order, a continually increasing value to order the log entries relative to each other
 - Thread, a pointer to the kernel thread object of the thread that created the log entry
 - Address, a pointer to the location where the log entry was created
 - Data[5], five pieces of data that the caller can assign arbitrary meaning to

The log entry is returned to the caller, is filled out, and is then passed back to watchdog.sys to be handled. Depending on the configuration state of the category in question, at this point one of three things happen. If the category is not enabled for live-logging or debug-break, nothing user-facing happens and execution continues uninterrupted. If debug-break is enabled, a message will be logged to the windbg asking the user how to proceed - they can break into the debugger, break into the debugger at the point where the log entry was created, ignore the entry, or ignore all entries coming from this address.

With the extension installed, three new commands are available:

## !watchdog.wdcfg
The wdcfg command takes two hex bitfield arguments, and configures the various watchdog log categories for either debug break or live logging behavior. The first argument controls which categories are live-logged, and the second controls which bring up the debug-break interface. Each argument is a bitfield of the various categories, represented as a single hex value.

This command also displays the current and previous states of all of the categories.

## !watchdog.wdlive
If live-logging is enabled for a given category, placing a breakpoint at `watchdog!WdLogLiveDumpBreakpoint` and running this command when execution stops there will display the details of the logged event.

## !watchdog.wdcurr
If debug-break is enabled for a given category, and the user replies to the prompt with 'b' to break into the debugger, this command will display the details of the logged event.