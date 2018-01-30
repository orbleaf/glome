# glome
an Orb-Script compiler


Glome (codename Stack) is an Orb-Script compiler for generating Orb-Weaver bytecodes that can be run on any OrbLeaf's powered devices,
it uses LALR parser generated by bison and flex to process sourcecode token for user script
the scripting language influenced by ECMAScript (ECMA262), C and C# with some modification 
to enable collaboration between application package
it support several features such as 
 * context/object oriented programming
 * lambda expression
 * lazy expression
 * string preprocessor
 * dynamic object management
 * APIs scalability (common, specialized)
 * weak, dynamic, duck typing
 
the purpose of Orb-Script to provide easy programming for any embedded device (similar with PHP idea)
accessing direct low-level are only available through native APIs (which is implemented using syscall mechanism)
runtime processing are done by Orb-Weaver virtual machine (OWVM), each variable are treated as dynamic object therefore
it's type cannot be known during compile process, OWVM will trigger an exception and terminate execution
when type operation didn't match with a specified object

the language itself is Generic Purpose Language therefore it can ported for different type of purpose
we're currently working to port the language for web-service development, therefore developing
embedded application and web-service could be done within the same environment, eliminating the needs
to develop in separate platform, one could say "a unified language for different type of platform"

glome are available as
 * CLR library (for integrating with managed DotNET application)
 * console application
 * CGI interpreter (for dynamic website integration)

feel free to provide a new insight of how an Orb-Script should be


FAQ : 
Q : what is the idea behind glome and Orb-Script language?<br>
A : back in 2013, i find that building SIM card application is quite hard, the only SIM card application scripting language is WIB script by SmartTrust (another scripting language is S@T by SIMAlliance), the language is hardly called scripting because it's based on WML (mark-up language similar with XML or HTML) and the scalability is quite low (it only translate generic SIM toolkit proactive command to their API), so i decide to build my own language for building SIM card application, back then the idea is based on early PHP concept for building a dynamic website at late 90s.

Q : what is philosophy? <br>
A : when i designed the language and VM, i have 6S philosophy : stack (for smaller memory footprint), string (capable to process string directly), small (enough to fit within smart card, less than 32KB), seamless (user application could call native API seamlessly), secure (collaboration is possible in secure manner), scalable (APIs can be scaled up to different devices and function without modifying the core engine)

Q : why derived from javascript and not the javascript itself?<br>
A : javascript is quite complex language, if you fully implement the interpreter on embedded device it might require significant resources and not fit for constrainted environment, i use virtual machine to separate the execution time from the parsing time (to reduce resources usage), it need a different approach on the programming side than the interpreter one, therefore a new language is born.

Q : why adding public functionality?<br>
A : i've tried different type of language from assembly, C, C#, Java PHP, javascript, etc, each has their own advantages/dis-advantages over another, i find that javascript lack collaboration features as in object oriented language, all function is javascript is lack function accessor therefore it's shared throughout runtime environment, making it collaboration is an issue when the other party want to hide a specific API from user.

Q : why lazy and lambda expression?<br>
A : first of all because i'm a lazy and un-coordinized person, i find that these expression is quite useful to support my way of programming, also i didn;t like a structured language such as Pascal (everything is well defined) and Phyton (argh those indented structure is killing me off).

Q : why adding pre-processor?<br>
A : pre-processor mostly used on C language, it is very useful when people want to change their program flow without modifying significant source code.

Q : there is no type checking?<br>
A : unfortunately yes, it;s similar with javascript btw, everything is handled by the vm, type checking are done on runtime execution, the vm will trigger exception if the type doesn't matched with the expected.

Q : performance issue?<br>
A : the language itself has no performance issue, but the VM does (only on smart card), my focus is build a VM that is small enough to fit within a smart card, at current release it requires only 24KB of flash memory, but because the core microprocessor still based on 8 bit microprocessor and the instruction set is not optimized for accessing a struct therefore the performance is quite an issue, this issue will be eliminated by changing the microprocessor the a higher one with better instruction set to support struct and pointer access (perhaps ARM secure core series for smart card)

Q : limitation?<br>
A : the language has no limitation, but because smart card is very constrained device, my VM on smart card only capable to process string no longer than 255 bytes, force the VM to process a string longer than this limitation will trigger an exception.

Q : what is glome anyway?<br>
A : glome is a name for fourth dimensional sphere (a type of hypersphere, that can be projected into Orb/Sphere) coined by wolfram alpha (i'm not an expert in mathematics and physics, but i like learning about these fields when i have free-time, mostly through youtube video, the idea to use glome as compiler name came up when i watched a video by Carl Sagan explaining a hypercube, and asked my-self what is the higher dimension of an Orb/Sphere?)
