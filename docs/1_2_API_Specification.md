# API Specification

Lettuce has functions, classes, and other components, so is necessary an organization of the interface.

## Naming Convention

- Objects that hold one internal handle SHOULD be named as:
```
[Upper case character][low case characters] 
```

Example:
```
Instance    
```

- When the object handles an Vulkan handle, it SHOULD be a C++ class, however, if the object stores raw data or information, it SHOULd be a C++ struct.

- If the object handles a group of Vulkan handles of the same type, it SHOULD be named as
```
[Upper case character][low case characters]Group
```
Example: 
```
DescriptorGroup    
```

- The names of public methods MUST start with Capital, in contrast, the whole names of private or protected methods MUST be in lower case.

- The names of interfaces MUST have the next format:
```
I[Upper case character][low case characters]
```
Example:
```
IResource 
```



## Object Lifetime

| Setup | Creation | Destruction |
|:----------------------|:--------------------|:------------------------|
| Setup initial data <br> and required objects |Create the object <br> and internal handles | Release memory <br> and/or destroy handles|

The functions that manages these states MUST be called:

| Setup | Creation | Destruction |
|:-----------------|:--------------|:----------|
| Constructor(...) | Assemble(...) | Release() |