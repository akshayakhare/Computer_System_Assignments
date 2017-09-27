#include <stdio.h>
#include <math.h>
#include <getopt.h>
#include <stdlib.h>
#include <fcntl.h>

double calculate(int, int);
double fact(int);
void usage_details();

int main(int argc, char **argv)
{
    /* Argument Handler begins */
    int worker_fd = -1;
    int option;
    int x_flag = 0;
    int n_flag = 0;
    int x = 0;
    int n = 0;
    while ((option = getopt(argc, argv, "x:n:")) != -1)
    {
        switch (option)
        {
        case 'x':
            x_flag = 1;
            if (optarg != NULL)
            {
                x = atoi(optarg);
            }
            break;
        case 'n':
            n_flag = 1;
            if (optarg != NULL)
            {
                n = atoi(optarg);
            }
            break;
        case '?':
            if (optopt == 'x' || optopt == 'n')
            {
                printf("The option %c needs a valid integer argument\n ");
                usage_details();
            }
            exit(1);
        default:
            printf("Incorrect option\n");
            usage_details();
            exit(1);
        }
    }
    if (x_flag == 0 || n_flag == 0)
    {
        printf("Not enough or  incorrect arguments\n");
        usage_details();
        exit(1);
    }
    /*Argument Handler Ends*/

    // FILE *stdout;
    char arr[50];
    double result = calculate(x, n);
    printf("Worker Presents: the value as -> %f\n", result);
    // sprintf(arr, "%f", result);
    // printf("stdout is %s\n", arr);
    // fprintf(stdout, arr);
    // printf("test is %f\n", test);
}

// Usage Details explaining the correct usage of worker file.
void usage_details()
{
    printf("Usage: worker -x <integer> -n <integer>\n");
}

// Calculates the final value given as per the expression  (x^n)/n!
// Can be changed if needed.
double calculate(int x, int n)
{
    double pow_val = pow((double)x, (double)n);
    printf("powered value is %f\n", pow_val);
    return (pow_val * 1.0) / fact(n);
}

//Helper function to calculate Factorial.
double fact(int n)
{
    double mul = 1;
    for (double i = 2; i <= n; i++)
    {
        mul = mul * i;
    }
    printf("factorial is finally %f\n", mul);
    return mul;
}