# genwrap

A utility for generating proxy stubs for functions from DLL libraries. This program generates source files in assembly language and C++, which can then be compiled into a static library using the appropriate utilities. 
The text of the generated files will contain a transition table with the names of the corresponding functions inside the namespace specified in the arguments of the generator. Using the `LoadModule` function, you can automatically load all the listed functions.
For example, a wrapper generated for `kernel32.dll` with namespace `kernel` can be called as follows:
```c
kernel::LoadModule();
```