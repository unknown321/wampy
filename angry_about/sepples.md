Angry about C/C++
=================

Why did I use C/C++ in my project instead of Go? There were no proper GUI frameworks written in Go at the time, so I
settled on Dear ImGui, which is a C++ library. Go bindings were adding unwanted complexity on top of cross-compilation.
There also was a feeling of "this will require GUI hacks", meaning working with source as close as possible. So, C++.

---

#### Code standards

Coming from Go, I expected some standards. You know, like passing errors in some nice way, perhaps, `errno`? Ehh no:

> The value in errno is significant only when the return value of the call indicated an error (i.e., -1 from most system
> calls; -1 or NULL from most library functions); a function that succeeds is allowed to change errno. The value of
> errno is never set to zero by any system call or library function.

So `errno != 0` is not an error indicator, need also to check for return value. Like this:

```shell
NOTES
       A common mistake is to do

           if (somecall() == -1) {
               printf("somecall() failed\n");
               if (errno == ...) { ... }
           }

       where errno no longer needs to have the value it had upon return
       from somecall() (i.e., it may have been changed by the
       printf(3)).  If the value of errno should be preserved across a
       library call, it must be saved:

           if (somecall() == -1) {
               int errsv = errno;
               printf("somecall() failed\n");
               if (errsv == ...) { ... }
           }
           
       Note that the POSIX threads APIs do not set errno on error.
       Instead, on failure they return an error number as the function
       result.  These error numbers have the same meanings as the error
       numbers returned in errno by other APIs.
```

Why even bother with errno at this point, if you can just put error code in return value?

And then there are functions like this:

```C
void doSomething(int *value, int *error);
```

What about C++ functions that throw god knows what type of exceptions? Just wrap everything in `try-catch` blocks?

Consistent style? Nah. People complaining about Go err checking (`if v, err := do(); err != nil {return err}`) should
write some C code first with error handling; that will make them appreciate Go more.

#### On C/C++ ecosystem

It doesn't exist (in a way I expect it to be). There are libraries living on personal websites,
self-hosted git repos, distributed as zip archives, built with Make, CMake, Ninja, Bazel or good
old `gcc something.c -o something`. While looking for INI reading library, I had to plow through those looking for
CMake-compatible one. Guess what, most popular one was unable to
write `ini`, [only read it](https://github.com/ndevilla/iniparser/issues/91), because it's a `parser` (and I figured it
out two days after). If that library doesn't support CMake, you'll have to integrate it into your project somehow. Go
wins again with its superior tooling.

There is no definitive standard library to use with functions needed to work with files, use strings and threading.
GitHub keeps suggesting me different libs with same functionality.

#### On project file structure

I just don't know anymore. Here is [mindforger](https://github.com/dvorka/mindforger),
Qt C++ project. Source is in `app`, libs are in `lib`. Is this structure enforced by Qt? Don't know. And here
is [cryptopp](https://github.com/weidai11/cryptopp). Source is mostly in root directory, just like most C libs. Is there
a nice standard on source file organization? Go enforces user to put packages into separate directories, thus avoiding
cryptopp-like clutter. Do I keep code in header file, like Dear ImGui does? Or header + cpp? There is no definitive
answer. I assume this is a complex problems that depends on tooling.

#### On C++11 standard library

It's crap. Unintuitive ways of doing things, lack of expected functions, no UTF8 support. No, I can't switch to a newer
standard (or can I?).

```c++
std::string input = "text";
std::ofstream out("file");
out << input;
out.close();
```

Do I need to deal with `ofstream` and `<<` operator just to write some text to file? This looks uncomfortable to read
and not intuitive to write. There is `.` already, why not use it? Like this: `out.ingest(input)`.

Reading all text from file into `std::string`?

```c++
std::ifstream f;
f.open(licensePath);
std::string contents((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
```

What is `std::istreambuf_iterator<char>(f)`? Can't I just do `f.readAll(contents)`, like in literally any other
modern language? Why does it have to be this way?

Formatting strings? Not in C++11. Take a look - https://stackoverflow.com/a/26221725/1938027. Template usage, unique
pointers, static cast, zero byte stripping - is this real? Look at other answers, some of them use c-strings, some
mention C++20 (not possible in our case), others look as complicated as accepted answer. After a while I just started to
use plain C functions to avoid that nonsense.

Stack traces? Of course not out of the box. You must build your application with `libunwind`. And that's only for your
application. It doesn't help if segfault occurs in standard library, like it did after 4 hours of waiting.

And people say that it was even *worse* before C++11 - https://news.ycombinator.com/item?id=42495135.

#### Documentation

Documentation online is scattered between a couple of sites dedicated to C++, obscure C forums
with `duplicate question, lurk more` posts, Stack Overflow with conflicting answers and C++11 standard, which is
currently withdrawn from ISO website (not even sure what's in it, are there any best practices on some simple stuff,
like reading files?).

Keywords are confusing; take a look
at `static` - [storage duration](https://en.cppreference.com/w/cpp/language/storage_duration). Okay, function can be
made static, meaning it is not tied to object instance. But if variable is static it becomes global-like and keeps its
state and is also placed in `.data` or `.bss` section and if it is a member... How many conditions are there already?

What about r/g/lvalues? https://blog.knatten.org/2018/03/09/lvalues-rvalues-glvalues-prvalues-xvalues-help/. Love this
quote: `Instead, I only hope to give a basic intuition`. We can only hope to understand language.

---

### Summary

I don't think that C++ is a user-friendly modern language. It doesn't make sense to write new user applications using
C++ unless performance is critical. Hopefully Carbon will take its place: code looks cleaner and tooling is expected to
be on Go level. Perhaps it was my mistake of having expectations before using C++11 without 10+ years of commercial
experience.

On the other side, modern languages are nice, because they were made with intention to be friendlier than C++.