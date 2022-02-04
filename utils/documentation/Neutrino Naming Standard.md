# Neutrino naming standards

### Functions
in *snake_case*. Module-specific functions must start with the module name (for example **cpu** functions are `cpu_get()` and similar)
```c
void function_name(int arg0, char arg1, ...) {
    ...
}
```

### Structure names
in *snake_case* with two leading underscores
```c
struct __cpu {
    ...
}
``` 


### Type names
in _PascalCase_
```c
typedef struct __cpu Cpu;
```

### Define constants
in *SCREAMING_SNAKE_CASE*
```c
#define MAX_CPUS 64
```

### Define macros
in _PascalCase_
```c
#define IsUserTask(flags) (...)
```