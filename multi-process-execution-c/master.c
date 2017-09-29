#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <getopt.h>
#include <sys/epoll.h>
#include <getopt.h>

void usage_details();
char *parse_final_ans(char *readbuffer);
int main(int argc, char **argv)
{

    /*Argument Handler Starts*/
    int c;
    int x_flag = 0;
    int n_flag = 0;
    int worker_path_flag = 0;
    int wait_mech_flag = 0;
    char *worker_path;
    char *wait_mech;
    int num_workers_flag = 0;
    int num_workers = 0;
    int x = 0;
    int n = 0;
    while (1)
    {
        int option_index = 0;
        static struct option long_options[] = {
            {"worker_path", required_argument, NULL, 'w'},
            {"num_workers", required_argument, 0, 's'},
            {"wait_mechanism", required_argument, NULL, 'm'},
            {"x_value", required_argument, 0, 'x'},
            {"n_value", required_argument, 0, 'n'},
            {0, 0, 0, 0}};

        c = getopt_long(argc, argv, "w:s:m:x:n:",
                        long_options, &option_index);
        if (c == -1)
            break;

        switch (c)
        {
        case 'w':
            worker_path_flag = 1;
            if (optarg != NULL)
            {
                worker_path = optarg;
            }
            break;
        case 's':
            num_workers_flag = 1;
            if (optarg != NULL)
            {
                num_workers = atoi(optarg);
            }
            break;
        case 'm':
            wait_mech_flag = 1;
            if (optarg != NULL)
            {
                wait_mech = optarg;
            }
            break;
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
            if (x_flag == 0 || n_flag == 0 || worker_path_flag == 0 ||
                wait_mech_flag == 0 || num_workers_flag == 0)
            {
                printf("Not enough arguments\n");
            }
            printf("some other option\n");
            break;
        default:
            printf("default value\n");
            break;
        }
    }
    if (x_flag == 0 || n_flag == 0 || worker_path_flag == 0 ||
        wait_mech_flag == 0 || num_workers_flag == 0)
    {
        printf("Missing options \n");
        usage_details();
    }
    /*Argument Handler Ends*/

    double exp_val = 1.0;

    int fd[2]; //Read and write file descriptor

    //Reads the STDOUT from worker
    char readbuffer[200];

    //Holds the STDOUT value into this string
    char *str_worker_res;

    //The String array which will hold all the worker arguments
    char *argsw[5];

    /*Assigning all the array elements Begins */
    argsw[0] = worker_path;
    argsw[1] = "-x";

    char x_to_char[5];
    char n_to_char[5];
    sprintf(x_to_char, "%d", x);
    argsw[2] = x_to_char;

    argsw[3] = "-n";
    sprintf(n_to_char, "%d", n);
    argsw[4] = n_to_char;

    argsw[5] = NULL;
    /*Assigning all the array elements Ends */

    // Total contains the sum of the expression
    double total = 0.0;

    // num temporary holds the n value, to handle (num_workers>n) condition
    int num = n;

    //big_num holds n value, to handle (num_workers < n) condition
    int big_num = n;

    //Contains the Child Process id once fork() happens
    pid_t childPID;

    //Counter for looping for all the outputs
    int counter = num_workers;

    /* Select logic Begins*/
    if (strcmp(wait_mech, "select" /*"epoll"*/) == 0)
    {
        // While condition to initiate batching, after workers are done
        while (big_num > 0)
        {

            num = big_num;

            //Provides a set for Fds
            fd_set rdfs;

            //Keeps the maximum possible of the fds created by FD_SET
            int max_fd = 0;

            // FD arrays for reading and writing fds for different workers
            int rd_ar[num_workers];
            int wt_ar[num_workers];

            // Resets the fd_set
            FD_ZERO(&rdfs);

            //sets up all file descriptors for workers
            for (int i = 0; i < num_workers; i++)
            {
                pipe(fd);
                FD_SET(fd[0], &rdfs);
                rd_ar[i] = fd[0];
                wt_ar[i] = fd[1];
                if (fd[0] > max_fd)
                {
                    max_fd = fd[0];
                }
            }

            //Createse all the given workers and forks to call all the workers
            for (int i = 0; i < num_workers; i++)
            {
                //Handling the condition where (num < num_workers)
                if (num < 1)
                {
                    break;
                }

                //Adds the latest n value to the argument list of worker.
                char tmp_num[20];
                sprintf(tmp_num, "%d", --num);
                argsw[4] = tmp_num;

                //Forks a new child to call worker
                childPID = fork();
                if (childPID == 0)
                {
                    close(rd_ar[i]);
                    dup2(wt_ar[i], STDOUT_FILENO);
                    execv("./worker", argsw);
                }
                else
                {
                    close(wt_ar[i]);
                }
            }
            // Finished Reading

            struct timeval tv;
            int retval;
            int counter = num_workers;

            //resets num to big num after handling all the workers
            num = big_num;
            while (counter > 0)
            {
                //handles the case where (num<num_worker)
                if (num < 1)
                {
                    break;
                }

                FD_ZERO(&rdfs);
                for (int i = 0; i < num_workers; i++)
                {
                    if (rd_ar[i] != 0)
                    {
                        FD_SET(rd_ar[i], &rdfs);
                    }
                }

                tv.tv_sec = 0;
                tv.tv_usec = 500000;
                retval = select(max_fd + 1, &rdfs, NULL, NULL, &tv);
                if (retval < 0)
                {
                    printf("Error in selecting\n");
                    exit(1);
                }
                else
                {
                    for (int i = 0; i < num_workers; i++)
                    {
                        if (FD_ISSET(rd_ar[i], &rdfs))
                        {

                            //reads the whole buffer into readbuffer
                            read(rd_ar[i], readbuffer, sizeof(readbuffer));

                            //Gets the final value by parsing the last value
                            str_worker_res = parse_final_ans(&readbuffer);
                            double tst = atof(str_worker_res);
                            total = total + tst;
                            close(rd_ar[i]);
                            rd_ar[i] = -1;
                            counter--;
                            num--;
                            big_num--;
                        }
                    }
                }
            }
        }
        printf("Total value for mechanism select is -> %lf\n", total);
    }
    /* Select logic Ends*/

    //Resetting the value for next mechanism
    total = 0;
    num = n;
    big_num = n;

    /* Epoll logic Begins*/
    if (strcmp(wait_mech, "epoll") == 0)
    {
        while (big_num > 0)
        {
            num = big_num;

            // ongoing Reads and Writes FD
            int e_rd;
            int e_wt;

            // Reads and writes FD for all workers
            int rd_ar[num_workers];
            int wt_ar[num_workers];

            int max_fd = 0;

            int counter = num_workers;

            int retval;

            struct epoll_event ev, events[100];
            int fd_epoll;

            fd_epoll = epoll_create1(0);
            if (fd_epoll == -1)
            {
                printf("Error in creating file directory for epoll\n");
                exit(1);
            }

            for (int i = 0; i < num_workers; i++)
            {
                pipe(fd);
                e_rd = fd[0];
                e_wt = fd[1];
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = e_rd;
                retval = epoll_ctl(fd_epoll, EPOLL_CTL_ADD, e_rd, &ev);
                if (retval == -1)
                {
                    printf("Error in epoll_ct \n");
                    exit(1);
                }

                if (fd[0] > max_fd)
                {
                    max_fd = fd[0];
                }
                rd_ar[i] = e_rd;
                wt_ar[i] = e_wt;
            }

            for (int i = 0; i < num_workers; i++)
            {
                char tmp_num[20];
                if (num < 1)
                    break;

                sprintf(tmp_num, "%d", --num);
                argsw[4] = tmp_num;
                childPID = fork();
                if (childPID == 0)
                {
                    close(rd_ar[i]);
                    dup2(wt_ar[i], STDOUT_FILENO);
                    execv("./worker", argsw);
                }
                else
                {
                    close(wt_ar[i]);
                }
            }
            num = big_num;
            while (counter > 0)
            {
                if (num < 1)
                {
                    break;
                }
                int eve_i;
                int eve_n = epoll_wait(fd_epoll, events, 100, -1);
                int another_fd_flag = 0;
                if (eve_n <= 0)
                {
                    printf("Incorrect result on epoll_wait\n");
                    exit(1);
                }
                else
                {
                    for (eve_i = 0; eve_i < eve_n; eve_i++)
                    {
                        another_fd_flag = 0;
                        e_rd = events[eve_i].data.fd;
                        for (int i = 0; i < num_workers; i++)
                        {
                            if (rd_ar[i] == e_rd)
                            {
                                another_fd_flag = 1;
                                rd_ar[i] = -1;
                                counter--;
                                num--;
                                big_num--;
                            }
                        }
                        if (another_fd_flag)
                        {

                            read(e_rd, readbuffer, sizeof(readbuffer));

                            str_worker_res = parse_final_ans(&readbuffer);
                            double tst = atof(str_worker_res);
                            total = total + tst;
                            close(e_rd);
                        }
                    }
                }
            }
        }
        printf("Total value for mechanism epoll is -> %lf\n", total);
    }
    /* Epoll logic Ends*/

    return 0;
}

char *parse_final_ans(char *readbuffer)
{
    char final_val[30];
    int final_val_index = 0;
    int index_buf = 0;
    int read_flag = 1;
    int final_flag = 0;
    while (read_flag)
    {
        if (readbuffer[index_buf++] == '>')
        {
            final_flag = 1;
        }
        if (readbuffer[index_buf] == '\n' && final_flag == 1)
        {
            read_flag = 0;
            break;
        }
        if (final_flag)
        {
            final_val[final_val_index++] = readbuffer[index_buf];
        }
    }
    final_val[final_val_index] = '\0';
    char *test = final_val;

    return test;
}

// Usage Details explaining the correct usage of worker file.
void usage_details()
{
    printf("Usage: ./master --worker_path <String:worker path> ");
    printf("--num_workers <Integer number of workers> ");
    printf("--wait_wait_mech <String type of wait_mech> ");
    printf("-x <Integer> -n <Integer>\n");
    exit(1);
}