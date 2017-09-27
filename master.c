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
        // int this_option_optind = optind ? optind : 1;
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
            // printf("worker_path %s\n", worker_path);
            break;
        case 's':
            num_workers_flag = 1;
            if (optarg != NULL)
            {
                num_workers = atoi(optarg);
            }
            // printf("num_workers %d \n", num_workers);
            break;
        case 'm':
            wait_mech_flag = 1;
            if (optarg != NULL)
            {
                wait_mech = optarg;
            }
            // printf("wait_wait_mech %s\n", wait_mech);
            break;
        case 'x':
            x_flag = 1;
            if (optarg != NULL)
            {
                x = atoi(optarg);
            }
            // printf("x's value is %d\n", x);
            break;
        case 'n':
            n_flag = 1;
            if (optarg != NULL)
            {
                n = atoi(optarg);
            }
            // printf("n's value is %d\n", n);
            break;
        case '?':
            if (x_flag == 0 || n_flag == 0 || worker_path_flag == 0 || wait_mech_flag == 0 || num_workers_flag == 0)
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

    FILE *stdout;
    // int x = 3;
    double exp_val = 1.0;

    int fd[2]; //Read and write file descriptor
    char readbuffer[200];
    char parent_string[50];
    char input_string[50];

    char buf;
    //int num_workers = 4;
    int read_flag = 1;
    char *str_worker_res;
    double worker_res = 0.0;
    char *argsw[5];
    argsw[0] = worker_path;
    argsw[1] = "-x";

    char x_to_char[5];
    char n_to_char[5];
    sprintf(x_to_char, "%d", x);
    argsw[2] = x_to_char;
    argsw[3] = "-n";
    sprintf(n_to_char, "%d", n);
    argsw[4] = n_to_char;
    printf("x is %s and n is %s\n", argsw[2], argsw[4]);
    argsw[5] = NULL;

    double total = 0.0;
    int num = n;
    pid_t childPID;
    int counter = num_workers;
    int big_num = n;

    //if mechanism is "SELECT"
    if (strcmp(wait_mech, "select" /*"epoll"*/) == 0)
    {
        // printf("big num is %d\n", big_num);
        while (big_num > 0)
        {
            num = big_num;
            fd_set rdfs;
            int max_fd = 0;
            int rd_ar[num_workers];
            int wt_ar[num_workers];

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

            for (int i = 0; i < num_workers; i++)
            {
                if (num < 1)
                {
                    break;
                }
                char tmp_num[20];
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
            // printf("finished reading\n");

            struct timeval tv;
            int retval;
            int counter = num_workers;
            num = big_num;
            printf("******** n is %d and num is %d\n", n, num);
            while (counter > 0)
            {
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

                            read(rd_ar[i], readbuffer, sizeof(readbuffer));
                            // printf("read buffer is %s \n", readbuffer);

                            str_worker_res = parse_final_ans(&readbuffer);
                            // printf("and the final val is%s\n", str_worker_res);
                            double tst = atof(str_worker_res);
                            printf("tsts ->%lf\n", tst);
                            total = total + tst;
                            close(rd_ar[i]);
                            rd_ar[i] = 0;
                            counter--;
                            num--;

                            big_num--;
                        }
                    }
                }
            }
        }
    }
    printf("total value is -> %lf\n", total);

    total = 0;
    num = n;
    // printf("---------------------------------------\n");
    //sequential
    /*for (int i = 1; i <= num_workers; i++)
    {
        if (num < 0)
            break;
        char tmp_num[20];
        sprintf(tmp_num, "%d", num--);
        argsw[4] = tmp_num;
        printf("argsw[4] is %s\n", argsw[4]);
        // snprintf(argsw[4], 3, "%d", num);
        // argsw[4] = (char *)num;
        //printf("fd values are %d %d \n", fd[0], fd[1]);
        pipe(fd);
        childPID = fork();
        if (childPID >= 0)
        {
            if (childPID == 0)
            {
                close(fd[0]);
                // write(fd[1], string, (strlen(string) + 1));
                printf("childPID is %d\n", childPID);
                if (dup2(fd[1], STDOUT_FILENO) == -1)
                {
                    printf("error in dup2");
                }
                int err = 0;
                // err = execl("./worker", "./worker", "-x3", "-n4", NULL);
                // args[2] = "-n5";
                printf("reached here\n");
                execv("./worker", argsw);
                perror("execv");
                printf("woop woop\n");
                printf("err is %d\n", err);
                exit(0);
            }
            else
            {

                close(fd[1]);
                printf("parent id ->\n");
                sleep(1);
                // int nbytes = 
                read(fd[0], readbuffer, sizeof(readbuffer));
                // printf("read buffer is %s \n", readbuffer);

                str_worker_res = parse_final_ans(&readbuffer);
                // printf("and the final val is%s\n", str_worker_res);
                double tst = atof(str_worker_res);
                printf("tsts ->%lf\n", tst);
                total = total + tst;
            }
            // gets(input_string);
            // printf("input _string is %s \n", input_string);
        }
    }*/

    // printf("total value is -> %lf\n", total);

    total = 0;
    num = n;
    big_num = n;
    printf("---------------------------------------\n");
    int read_counter = 1;

    if (strcmp(wait_mech, "epoll") == 0)
    {
        while (big_num > 0)
        {
            num = big_num;

            int e_rd;
            int e_wt;
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
                    printf("read called %d\n", ++read_counter);
                    dup2(wt_ar[i], STDOUT_FILENO);
                    execv("./worker", argsw);
                }
                else
                {
                    close(wt_ar[i]);
                }
                // printf("finished reading for epoll-------------------------\n");
            }
            // counter--;
            num = big_num;
            while (counter > 0)
            {
                printf("num is %d\n", num);
                if (num < 1)
                {
                    break;
                }
                printf("counter value is %d\n", counter);
                int eve_i;
                int eve_n = epoll_wait(fd_epoll, events, 100, -1);
                int another_fd_flag = 0;
                printf("eve_n is %d\n", eve_n);
                // eve_n = 1;
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
                        // int fd_index = -1;
                        for (int i = 0; i < num_workers; i++)
                        {
                            if (rd_ar[i] == e_rd)
                            {
                                printf("\n");
                                another_fd_flag = 1;
                                rd_ar[i] = -1;
                                counter--;
                                num--;
                                big_num--;
                                printf("--------probably need to do something-----------\n");
                            }
                        }
                        if (another_fd_flag)
                        {

                            read(e_rd, readbuffer, sizeof(readbuffer));
                            // printf("read buffer is %s \n", readbuffer);

                            str_worker_res = parse_final_ans(&readbuffer);
                            printf("and the final val is%s\n", str_worker_res);
                            double tst = atof(str_worker_res);
                            printf("tsts ->%lf\n", tst);
                            total = total + tst;
                            // close(rd_ar[eve_i]);
                            close(e_rd);
                            printf("total is %lf\n", total);
                            // rd_ar[i] = 0;
                        }
                    }
                }
            }
        }
    }

    printf("total -> %lf\n", total);
    printf("Ending-----\n");

    return 0;
}

char *parse_final_ans(char *readbuffer)
{

    // printf("readbuffer ->%s\n", readbuffer);
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
            // printf("read buffer -> %c", readbuffer[index_buf]);
            read_flag = 0;
            break;
        }
        if (final_flag)
        {
            final_val[final_val_index++] = readbuffer[index_buf];
        }
    }
    final_val[final_val_index] = '\0';
    // printf("final val is%syo\n", final_val);
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