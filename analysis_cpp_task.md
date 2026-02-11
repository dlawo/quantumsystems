# Analysis of cpp_task.cpp

The code starts two worker threads that both wait for some time and then increment a counter.
Once the loop_counter2 is incremented often enough to reach if (loop_counter2 < 5) = false, the thread aborts the loop.
There is a data race condition with loop_counter1 and loop_counter2. 
This causes undefined behavior.
The worker threads are accessing the variables and the main can read them.
That's why when the programm is executed, loop_counter2 is printed as 6 to the terminal.
The compiler doesn't find that but if the program is executed with thread sanatizer, it is possible to see that there is a problem.

The problem can be solved by converting the counters to an atomic integer, so std::atomic<int>