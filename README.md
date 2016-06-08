``rlimit`` is a Unix command line utility for setting resource limits on
processes.  ``rlimit`` can be used by software developers and testers to test
software under resource restrictions.

It is also useful for software which would otherwise consume excessive resources
such as programs that used 100% CPU, set their own priority levels or consume
too much memory.

``rlimit`` is really just a command line interface to the system functions
``getrlimit()`` and ``setrlimit()``.  It is also very similar to the ``ulimit``
command execpt that it only applies the limits to one process and it can
set some limits that ``uname`` cannot.

See ``LICENSE`` for license information.

Run ``rlimit --help`` for command line usage.

# Example: Display detailed default limits

This is similar to running ``ulimit -a`` but with more detail.

    rlimit

This prints something like this:

         Address Space: soft  unlimited  hard  unlimited
             Core Size: soft          0  hard  unlimited
              CPU Time: soft  unlimited  hard  unlimited
     Data Segment Size: soft  unlimited  hard  unlimited
             File Size: soft  unlimited  hard  unlimited
            File Locks: soft  unlimited  hard  unlimited
         Locked Memory: soft      65536  hard      65536
         Message Queue: soft     819200  hard     819200
                  Nice: soft          0  hard          0
      File Descriptors: soft      65536  hard      65536
             Processes: soft      64053  hard      64053
          Resident Set: soft  unlimited  hard  unlimited
    Real-Time Priority: soft          0  hard          0
    Real-Time CPU Time: soft  unlimited  hard  unlimited
       Pending Signals: soft      64053  hard      64053
            Stack Size: soft    8388608  hard  unlimited

# Example: Limit process memory usage

    rlimit -d10M target <options>

The above runs ``target <options>`` with a 10MiB memory allocation limit.
