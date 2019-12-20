This is the code and report for my third year undergraduate project for the 'Object Oriented Programming in C++' course.
I am mostly putting it on github to show off - I was pretty proud of this at the time.
I would strongly recommend against using this interpreter for anything practical, 
I didn't have much time for this project and I prioritised simplicity and legibility over basically everything else.
As a result there are several design choices that are pretty braindead, including using explicit recursion for the parser, using a direct tree-walking interpretation with no intermediate representation, and using immutable data structures everywhere leading to heap allocations galore.
It's also missing all but the most primitive functionality.
Some of the dumb choices (like hand-rolling a parser) were enforced by a course requirement to not use any external libraries.

On the other hand, it is a real interpreter, powerful enough to execute pretty arbitrary programs (very slowly) and it doesn't leak memory, so I think it was pretty good for a 3rd year physics undergrad.
