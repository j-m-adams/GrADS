# Installing GrADS with Conda

The recommended way to install GrADS is from conda-forge:

```bash
mamba create -n grads -c conda-forge grads
mamba activate grads
```

This installs the native GrADS tools, supported libraries, map and font data,
and display and print plugins.

The optional Python interface is distributed separately:

```bash
mamba install -n grads -c conda-forge gradspy
```

Verify the native installation with:

```bash
printf "q config\nquit\n" | grads -bl
```

General source-build instructions are in [INSTALL](INSTALL).
