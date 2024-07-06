# Run the debugger function if *debug* is on
```
...

if (debug) {
    // run the njvm in debug mode
	csmkdb_run_debugger();
} else {
	// run the njvm in normal mode
    run_njvm();
}
	
...
```

# Add the breaker to the runner function 

```
...
	do {
		...
		if (debug) break;
	}
	while(!HALT)
...

```

# Init debugger variables

```
...

   csmkdb_vars_init(stack, sda, &specialRegister, prog, &stackPointer, &framePointer, &progCounter, instrNumber, sdaNumber, "Dummy", 
   &debug, NJVM_VERSION, run, &breakPoint);
...

```