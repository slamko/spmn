# Suckless Patch Manager
---

### Why

Suckless software is starting to gain some popularity nowadays in Linux community. And, sadly, I think that their website design does not correspond to the popularity of the project. It is so suckless that it consists of markdown files and of course doesn't have any kind of search system. So in order to patch dwm, for example, you should walk through the list of more than 250 patches (this number only grows!) and find the one you want...  

So initially this project intended to be a command line tool for searching patches on suckless.org. However later on ```spmn``` got some additional functionality like downloading applying patches. It may sound like ```spmn``` now violates suckless principals, but in fact it is quite simple: it just operates on a local clone of suckless.org page, so downloading a patch literally means copying the file, viewing a patch means printing the file contents and so on...

---

### Installation 
```spmn``` is currently packaged for Debian and Void Linux.  
You can find binary packages with corresponding SHA sums in the [Releases section](https://github.com/slamko/spmn/releases).  
It is also [available in AUR](https://aur.archlinux.org/packages/spmn) for Arch-based distros.  

Otherwise you can easily compile it from source:

```sh
git clone https://github.com/slamko/spmn.git spmn &&
cd spmn &&
make &&
make install
```


### Using Spmn
Just after installing to start using ```spmn``` you should sync with suckless.org repo:
```shell
spmn sync
```

Example for searching the patch and applying it:

![spmn-aur](https://user-images.githubusercontent.com/72746829/182939782-f62ab3fe-c6a1-464e-9f42-42c0a586d720.png)

There is really not much more to say about the usage. All the commands are pretty intuitive with at most of one command line option:
```
   Usage:
	  spmn [command] [args] [options]
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
	      -a:  load and apply patch at once (the same as spmn apply).
	    search: 
	      -f:  show patch description for each patch found.
	    apply: 
	      -f:  apply the patch directly from given file.
```
