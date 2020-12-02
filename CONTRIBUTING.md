# Development Guidelines

## Contents

- [Git](#git)
- [Style](#style)

## Git



## Style

### Structure Naming



### Function Naming

All of the functions that are to only be used internally inside of a single module should start with a lower case letter

All of the functions that are exposed internally to the other modules of a library, but are not to be exposed outside of a library, should start with an upper case letter

All of the functions that are to be exposed outside of a library should start with an appropriate prefix 

### Function Structure

No Function could affect a state that it is not explicitly exposed to.

### Objects

A method can't implicitly change its object state.

### Indentation

We have one true brace

__Example__:
```C
int power(int x, int y) {
	int result;

	if (y < 0) {
		result = 0;
	} else {
		result = 1;
		while (y-- > 0) {
			result *= x;
		}
	}
	return result;
}
```

