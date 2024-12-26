Angry about... Java applications
================================

In my mind Java became synonymous with "bad software". Every time Java program is launched, you can start a timer. How
long will it take for `java.<ExceptionName>` to occur? Will it be on startup? Perhaps in 15 minutes out of nowhere? Or
maybe on clicking `save` button?

But let's start with installing such program. First, you need to install Java runtime. Which one? That's right, any one
will do, and if it doesn't work, just try another one! OpenJDK, IcedTea, Amazon Corretto, Eclipse Temurin? Go ahead, all
of them are 100% compatible with your software! Java runs on 6 billion devices!

Then you must tell your application path to newly installed Java. Hopefully you did everything right and it launches.
There are three categories of Java applications:

- one-shot console
- desktop
- long-running server

#### Console

Console ones are usually distributed via GitHub or some forums in zip files or just as plain jars. They may or may not
run. Usually they require some other obscure library and 99% of the time terrorize you with cryptic stack traces if
anything goes wrong, like you provided incorrect file path. Are application developers writing in Java aware that user
can submit wrong data? Or maybe they are not familiar with "handling errors" concept?

```shell
$ java -jar /tmp/nwwmdecrypt.jar  -i /dev/null -o /dev/null -k o0W0mihblfJOPtNQPN8Pc2hLKiTROL5MVERN9OmmkkMNZO3P
Decrypting...
Error during deciphering: com.sony.walkman.systemupdater.util.UpdateDataDecipher$DecihperErrorException: Header process error:java.io.IOException: File read error : ret -1
```

Right, `ret -1`. Do I, as a user, need `com.sony.walkman.systemupdater.util.UpdateDataDecipher$DecihperErrorException`?
Why can't it be just `Error during deciphering: cannot read input file /dev/null`? Hey, at least it's not a full stack
trace. (All right, this is a very specific application targeted at tinkerers, who, most likely, can understand what went
wrong.)

#### Desktop applications

Desktop applications meet you with mismatching GUI theme. That's right, detecting system theme and applying it
automatically is not of developers concern. You can change theme later somewhere in settings if dev was kind enough to
add that option. Usually desktop applications have error handling, but it doesn't stop them from throwing stack traces
at you. Take a look - https://github.com/NationalSecurityAgency/ghidra/issues/5468. Just so you know, application
doesn't crash after that error, leaving you in "Am I still OK? Do I need to restart?" state. At least you can copy
(screenshot?) that stacktrace and contact a developer; maybe it will be fixed. This one was, but there are much more
I've personally encountered during ~3 years of using Ghidra. And this is a nice project, there was software much, much
worse, like Java IPMI widgets for SuperMicro servers, Azureus (torrent client), older versions of OpenOffice, Eclipse,
Jmeter, Gatling and other crap. These days I use Java applications on desktop as a last resort, avoiding them as much as
I can. Even JetBrains IDEs throw some garbage to console from time to time.

#### Server applications

Server applications are terrible. Usually they provide irreplaceable functionality, so you have no other choice but to
use them. Stuff like Apache Kafka, Keycloak, Apache Guacamole, Camunda, Gerrit, Infinispan, JBoss, JUnit, Jenkins,
Apache Zookeper... (what's the deal with Apache and Java?). These days those abominations are usually contained in
Docker with their own Java runtime, but that doesn't mean that you are free from Java issues. Stacktraces are still
there, now with hundreds of lines per trace (remember, this is an important complex software, so everything is 50+
interfaces deep). It doesn't crash, leaving you with same `Am I OK?` question. But is there something else? Of course,
now you also must configure memory limits, otherwise that pile of garbage will crash. No, one memory limit is not
enough, look at this:

```shell
-Xms${HEAP_SIZE_MB}M \
-Xmx${HEAP_SIZE_MB}M \
-Xss1M \
-XX:MaxMetaspaceSize=${METASPACE_SIZE_MB}M \
-XX:CompressedClassSpaceSize=${COMPRESSED_CLASS_SPACE_SIZE_MB}M \
-XX:ReservedCodeCacheSize=${RESERVED_CODE_CACHE_SIZE_MB}M \
-XX:MaxDirectMemorySize=${DIRECT_MEMORY_SIZE_MB}M \
-XX:NativeMemoryTracking=summary \
```

Or maybe `-XX:+UseContainerSupport -XX:MaxRAMPercentage=60.0`? We just don't know.

Then everything works fine for a month or two, stacktraces don't bother you anymore (these were just 150 lines of
*notices*, working as intended), memory consumption is ok... unless it is not. Something is leaking, and you start
profiling that crap, looking for a leak. Maybe you'll find it, maybe you won't.

---

### Summary

This is my typical experience with Java applications. They are horrible in any form. And remember, Java is used by your
bank, hospital, public transport, runs in your phone, car, microwave, refrigerator and other 6 billion devices.