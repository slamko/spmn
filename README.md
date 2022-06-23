# Suckless Patch Manager
---

### Why

Suckless software starting to gain some popularity nowadays in Linux community. However, it is unlikely to become mainstream because of its suckless nature... People do not like to manually compile and patch (resolve conflicts) suckless software. Another thing that may scareaway some amount of newcomers is suckless.org website... It is so suckless that it consists of markdown files and of course doesn't have any kind of search system. So in order to patch dwm, for example, you should walk through the list of 250 patches and find that you want...  

So initially this project intended to be a command line tool for searching patches on suckless.org. However later on spm got some additional functionality like downloading applying patches. It may sound like Spm now violates suckless principals :) but I don't think so. In fact it just operates on a local clone of suckless.org page, so downloading a patch literally means copying the file, viewing a patch means ```cat```ting the file... So Spm is pretty much suckless :)

---
### Using Spm

There is really not much more to say about the usage. All the commands are pretty intuitive with at most of one command line option:
```
   Usage:
	  spm [command] [args] [options]
	  Commands:
	    search <tool> [keywords] - search a patch for a <tool> with given [keywords] (default command).         
	    load   <tool> <patch>    - download patch for given <tool> with <patch> name.
	    open   <tool> <patch>    - show full description for a <patch> of specified <tool>.           
	    apply  <tool> <patch>    - download and apply the <patch> for a given <tool>.
	    sync                     - synchonize local patches repository.
      
	    help    (--help/-h)    - to see this page.
	    version (--version/-v) - to get version info.
			
	  Options:
	    open: 
	      -b:  show the web page on suckless.org for given patch in browser.
	    load: 
	      -a:  load and apply patch at once (the same as spm apply).
	    search: 
	      -f:  show patch description for each patch found.
	    apply: 
	      -f:  apply the patch directly from given file.
```
