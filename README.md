# A Lightweight Transfer Matrix Method 
A small library for calculating reflectance and transmittance data for 1 dimensional multilayer media.

## Example usage
```c
gcc -o main main.c -lm
./main wavelength theta "refractiveindex1" thickness1 "refractiveindex2" thinkness2
```

## Future goals
- Integrate with mpi4py
- More dimensions
- Diagonally anisotropic & fully anisotropic media
- Scattering matrix version
