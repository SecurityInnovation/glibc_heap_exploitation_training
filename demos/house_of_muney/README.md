# House of Muney
Code execution via corrupting mmap malloc chunks with ASLR bypass

## Intro
Are you tired of ASLR getting in your way? Annoyed by how unexploitable mmap chunks feel in GLibC malloc?   
Well, today is your lucky day! This is a full explanation on how a **leakless** heap exploitation technique that gives you **code execution**, that is 100% deterministic
What is this, the year 2000!?  
  
This technique IN FULL is described at https://maxwelldulin.com/BlogPost?post=6967456768.

This technique and was first used in a Qualys exploit for a [15 year old QMail vulnerability](https://www.qualys.com/2020/05/19/cve-2005-1513/remote-code-execution-qmail.txt). Another example of this being used is in [cpio](https://github.com/fangqyi/cpiopwn) by fangqyi.

## But How? 
At a high level, this technique rewrites the lazy dynamic symbol resolution process of a library. You are probably thinking: *this is black magic*.  
And that's because this is, but I hope that this repository (and blog post alongside this) can help out with that. 

<ol>
<li>
Overwrite GLibC Malloc mmap chunk <i>size</i> or <i>prev_size</i>
</li>
<li>Free the mmap chunk (with munmap) to small amount of memory mapping of part of LibC (<code>.gnu.hash, .dynsym</code>)</li>
<li>Get mmap chunk over the top of LibC region</li>
<li>Rewrite <code>.gnu.hash</code> and <code>.dynsym</code> sections of LibC ELF</li>
<li>Call previously uncalled function for code execution</li>
</ol>

## When Is this Possible? 
Of course, there are a few drawbacks. But, most of these are just knowledge about how the program *exactly* works, rather than a complete destroyer of the technique (besides the first one).
<ol>
<li>Full RELRO or a static binary would prevent this attack from being successful because all symbols are loaded prior to the program running (no lazy symbol resolution)</li>
<li>Known <b>relative</b> location of the overwritten chunk. In some scenarios, this may need to be brute forced</li>
<li>The LibC (or other library) being attacked must be known or brute forced in order to have the offsets work properly</li>
<li>Fairly good control over the size of allocations in order to get mmap chunks in the proper locations</li>
</ol>

## Using the Repo 
Upon some further expection, it was noticed that the repo did not work AS is in all envs. For whatever reason  
some offsets would change, resulting in a broken exploit. So, here are some steps for running the repo locally and in Docker.

### Docker
So, it only seemed logical to create a quick & easy dockerfile for this. Here are the instructions for using Docker with this: 
- Install docker (pre-req) 
- Build the Image: 
  - `sudo docker build --tag muney .`
- Start & login to the container: 
  - `sudo docker run -it muney /bin/bash`
- Once logged in the, the good stuff is in `house-of-muney`. So, move to this directy. 
- Jump into tmux: 
  - Tmux (terminal multiplexer) is SUPER nice because we can have multiple panes open in the same session. 
  - This allows gdb to open
- Run the code: 
  - `python launch.py`
  - Have fun in GDB and see what is going on!

### Own Machine 
Getting the technique running on your own machine is absolutely possible but may become tedious or annoying to do.   
Regardless, here are some steps for that: 
- Install python, pip, pwntools, tmux, pwndbg, patchelf, gcc and other assorted tools (pre-req)
- Download the repo: `git clone https://github.com/mdulin2/house-of-muney/`
- Fix the loader on the binary: 
  - The exploit code MUST use the proper LibC and loader in the /2.31 file. 
  - In order to do this, the loader was changed using ELFPATCH on the binary itself. 
  - So, either do this yourself with patchelf or run the `./compile.sh` that changes the loader for you. 
- Run the Code: `python launch.py`
  - gdb will come up and STOP right before all of the black magic happens for the symbol resolution
  - Put breakpoints where ever you want to see exactly what is happening
  - If you are having issues with this, your offsets are probably broken... fix these yourself and they are likely in the intervals of page sizes. Good luck!
