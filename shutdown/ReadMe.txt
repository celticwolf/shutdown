What
====

Shutdown is an open source command-line utility for Windows that allows you to shutdown, reboot or log off of a PC running Microsoft Windows XP, Vista, 7, Server 2000, Server 2003 and Server 2008. You can shutdown or reboot your PC or a remote PC. You can log off your session on the local PC. It combines features of the Windows native shutdown utility with features of the GNU shutdown utility for POSIX systems, providing a more flexible interface than the native Windows version.

Why
===

Windows XP and newer versions all come with a native shutdown utiltity. Why create a new one that does the same thing? We had a couple of reasons.

Windows 2000

There are still Windows 2000 workstations and servers out there, and we have to deal with them until they're retired. There is no shutdown utility included with a stock installation of Windows 2000, though you can get one from the Windows 2000 Resource Kit. Of course, that only helps if the resource kit is or can be installed. When considering how to handle this issue, we looked around for alternatives, but all of them were closed source. We aren't comfortable installing closed source utilities from random web-sites on our clients' systems. Most of them are probably safe, but it only takes one to infect a client's entire network with malware. We didn't find an open source utility, so we decided to write one.

The Native Utility Can Be Unfriendly

While the native utility provided by Microsoft has lots of features, it can be a bit unfriendly. For example, the only way to specify a delay between when the command is issued and when the shutdown occurs is to specify the number of seconds. If you want anything more than a trivial delay, grab the calculator. The GNU utility, in contrast, allows you to specify the time using more friendly formats:

    * +20 will provide a 20 minute delay
    * +90 will provide an hour and a half delay
    * 02:00 will cause the system to shut down at 2:00 am
    * "now" will cause it to shut down immediately

There is room for improvement in other areas. For example, when using the native utility, it always tries to act on remote systems using the credentials of the account that is used to run the utility. Why not allow the user to specify a different set of credentials? We're planning to add features like this to shutdown in the future.

License
=======
This software is licensed under the terms of the GNU GPL version 2.  See the enclosed license.txt or <http://www.gnu.org/licenses/gpl-2.0.html>.

How
===

Installation
------------
Just place shutdown in a directory that's on your path.  There is no need to install anything.

Features
--------

Shutdown has many of the same features as the native utility.  It can shutdown or reboot the local system or a remote system (provided that you have shutdown privileges on the target system).  It can log off the current user session on the local system.  It can schedule any of these actions at a future time or abort a shutdown that has already been scheduled.  It also supports logging a reason and message in the system log.

Usage
-----

shutdown [options] [time] [message]

"Options" is one of the command-line options listed below.

"Time" can be one of the following:

   1. "now" - the action will be performed immediately.
   2. "+mm" - "mm" is the number of minutes to delay before taking action.
   3. "HH:MM" - "HH" is an hour (using a 24 hour clock) and "MM" is a minute. The specified action will be taken at the next time the clock reaches that time. With this option, you cannot schedule an action more than 23 hours and 59 minutes in the future.

"message" is an optional comment to be written to the system log. 


Quick Examples
--------------

   1. To shut down now: shutdown now
   2. To reboot in five minutes: shutdown -r +5
   3. To log off at 3:00 pm: shutdown -l 15:00
   4. To reboot the system named "Daffy" for minor hardware maintenance with a message: shutdown -r -m \\Daffy -d p:1:1 now "Shutting down to replace a case fan"


Command-line options
--------------------

Most of the command-line options supported by the native utility are also supported by shutdown. Notably "-s" is excluded. Shutting down is the default action and will be taken if neither -r nor -l is specified.

This application has no GUI, so the "-i" option is not supported.

The "-c" comment option from the native utility has been omitted.  Instead, simply supply the comment (a.k.a. message) as the last argument.


Logoff

-l (lower case "L") causes the current session to log off, instead of shutting down. This option is only available on the local PC; you can't use it to log someone off of a remote system. If necessary, it can be combined with the -f switch to force stubborn programs to close.

Reboot

-r causes the system to reboot. If necessary, it can be combined with the -f switch to force stubborn programs to close.

Abort

-a aborts a shutdown that is currently in process. For example, if you scheduled the system to reboot in an hour and then an emergency arrises, you can use this option to stop it from shutting down.

Remote System

-m \\computername will cause the command to be executed on the specified system, rather than the local system. For example, shutdown -m \\Harvey would cause the system named "Harvey" to be shutdown immediately. The account that runs the command must have shutdown privileges on the specified system. Check the domain security policy or local system policy if you run into errors.

Force Programs to Close

-f will attempt to force all applications to close. Note that unsaved data may be lost.

Reason Code

-d allows you to specify a reason code that will be saved in the system log. The format is [u|p]:MM:mm where:

    * "u" stands for user defined reason. Cannot be combined with "p".
    * "p" stands for planned shutdown. Cannot be combined with "u".
    * "MM" is the major reason code. See Microsoft's list of codes: <http://msdn.microsoft.com/en-us/library/aa376885%28VS.85%29.aspx>
    * "mm" is the minor reason code

Example: shutdown -d p:1:1 would indicate that the shutdown was a planned event in order to do minor hardware maintenance.

Verbose Output

Outputs some debugging info along the way, depending on options selected.  This can be useful for troubleshooting.

Download It
===========

The program and source code can be obtained here: <http://www.celticwolf.com/shutdown-an-open-source-shutdown-utility-for-windows>.