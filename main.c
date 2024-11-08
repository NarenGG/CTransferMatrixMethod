#include <stdio.h>
#include <math.h>
#include <complex.h>
#include <time.h>
#include <stdlib.h>

complex double complex_modulo(complex double z1, complex double z2) {
    // Calculate the absolute values of the complex numbers
    double abs_z1 = cabs(z1);
    double abs_z2 = cabs(z2);
    
    // Calculate the modulus
    double remainder = fmod(abs_z1, abs_z2);
    
    // Return the remainder as a complex number with the same argument as z1
    double angle_z1 = carg(z1);
    complex double result = remainder * cexp(I * angle_z1);
    
    return result;
}

complex double sin_complex(complex double v) {
    // Take advantage of periodicity of trigonometric functions to evaluate at large values
    double a = fmod(creal(v), 2 * M_PI);
    complex double b = complex_modulo(cimag(v) * I, 2 * M_PI * I);

    return (sin(a) * cosh(cimag(b))) + I * (cos(a) * sinh(cimag(b)));
}

complex double cos_complex(complex double v) {
    // Take advantage of periodicity of trigonometric functions to evaluate at large values
    double a = fmod(creal(v), 2 * M_PI);
    complex double b = complex_modulo(cimag(v) * I, 2 * M_PI * I);

    return cos(a) * cosh(cimag(b)) - I * sin(a) * sinh(cimag(b));
}

void transfer_matrix(complex double result[2][2], double k_0, complex double n, double d, double theta) {
    complex double k_z = k_0 * n * cos(theta); // Calculate longitudinal K

    // Reduce redundant calculations in construction T_i
    complex double q_1 = cos_complex(k_z * d);
    complex double q_2 = I * sin_complex(k_z * d);
    complex double n_cos_th = n * cos(theta);

    // Transfer matrix calculation derived from Maxwell's equations
    result[0][0] = q_1;
    result[0][1] = q_2 / n_cos_th;
    result[1][0] = n_cos_th * q_2;
    result[1][1] = q_1;
}

void solve_tmm(double* R, double* T, complex double layers[][2], int num_layers, double wavelength, double theta) {
    // Scales wave propagation
    double k_0 = (2 * M_PI) / wavelength;

    // Global transfer matrix initialization
    complex double M[2][2] = { {1, 0}, {0, 1} };

    complex double n_0 = layers[0][0];
    complex double n_l = layers[num_layers - 1][0];
    
    for (int i = 1; i < num_layers - 1; ++i) {
        // Get layer parameters
        complex double n = layers[i][0];
        double d = creal(layers[i][1]);

        // Calculate transfer matrix
        double theta_i = asin(creal(n_0) * sin(theta) / creal(n));
        complex double T_i[2][2];
        transfer_matrix(T_i, k_0, n, d, theta_i);

        // Update global transfer matrix with dot product
        complex double temp[2][2];
        for (int j = 0; j < 2; ++j) {
            for (int k = 0; k < 2; ++k) {
                temp[j][k] = 0;
                for (int l = 0; l < 2; ++l) {
                    temp[j][k] += M[j][l] * T_i[l][k];
                }
            }
        }
        for (int j = 0; j < 2; ++j) {
            for (int k = 0; k < 2; ++k) {
                M[j][k] = temp[j][k];
            }
        }
    }

    // Calculate M_in and M_out
    complex double q_1 = n_0 * cos(theta);
    double theta_l = asin(creal(n_0) * sin(theta) / creal(n_l));
    complex double q_2 = n_l * cos(theta_l);
                   
    complex double M_in[2][2] = {
        {1, 1},
        {q_1, -q_1}
    };
    
    complex double M_out[2][2] = {
        {1, 1},
        {q_2, -q_2}
    };

    // Calculate M_global final
    complex double M_total[2][2];
    complex double M_temp[2][2];
    complex double M_inv[2][2];

    // Calculate inverse of M_in
    complex double det = M_in[0][0] * M_in[1][1] - M_in[0][1] * M_in[1][0];
    M_inv[0][0] = M_in[1][1] / det;
    M_inv[0][1] = -M_in[0][1] / det;
    M_inv[1][0] = -M_in[1][0] / det;
    M_inv[1][1] = M_in[0][0] / det;

    // M_temp = M * M_out
    for (int j = 0; j < 2; ++j) {
        for (int k = 0; k < 2; ++k) {
            M_temp[j][k] = 0;
            for (int l = 0; l < 2; ++l) {
                M_temp[j][k] += M[j][l] * M_out[l][k];
            }
        }
    }

    // M_total = M_inv * M_temp
    for (int j = 0; j < 2; ++j) {
        for (int k = 0; k < 2; ++k) {
            M_total[j][k] = 0;
            for (int l = 0; l < 2; ++l) {
                M_total[j][k] += M_inv[j][l] * M_temp[l][k];
            }
        }
    }

    // Extract reflection and transmission
    complex double r = M_total[1][0] / M_total[0][0];
    complex double t = 1 / M_total[0][0];
    *R = pow(cabs(r), 2);
    *T = pow(cabs(t), 2) * creal(n_l * cos(theta_l)) / creal(n_0 * cos(theta));
}

int main(int argc, char** argv) {
    if (argc % 2 != 1) {
        fprintf(stderr, "Usage: %s <complex double> <integer> ...\n", argv[0]);
        return 1;
    }

    clock_t begin = clock();
    double R, T;
    int num_layers = (argc - 3) / 2;
    complex double layers[num_layers][2];

    for (int i = 0; i < num_layers; ++i) {
        double real_part = 0.0, imag_part = 0.0;
        sscanf(argv[2 * i + 3], "%lf%lf", &real_part, &imag_part);
        layers[i][0] = real_part + imag_part * I;
        layers[i][1] = atof(argv[2 * i + 4]);
    }

    double theta = atof(argv[2]);
    double wl = atof(argv[1]);
    solve_tmm(&R, &T, layers, num_layers, wl, theta);
    printf("Reflectance: %f\n", R);
    printf("Transmittance: %f\n", T);
    
    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Execution Time: %f ms\n", time_spent*pow(10,6));
}
