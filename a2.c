#include "a2.h"

int main (int argc, char *argv[])
{
    char *subopts, *value;
    int opt,
        sjf            = 0,
        srtf           = 0,
        verbose        = 0,
        number_of_jobs = 0,
        no_scheduler   = 1;
    char *filename     = NULL;

    if(argc == 1) {
        print_usage(argc, argv);
        return 1;
    }
    while((opt = getopt(argc, argv, "hvn:i:s:")) != -1)
        switch(opt)
    {
        case 'h':
            print_usage(argc, argv);
            break;
        case 'v':
            verbose = 1;
            break;
        case 'n':
            number_of_jobs = strtol(optarg, NULL, 10);
            break;
        case 'i':
            filename = optarg;
            break;
        case 's':
            subopts = optarg;
            while(*subopts != '\0')
                switch(getsubopt(&subopts, scheduler_opts, &value))
            {
                case SJF:
                    sjf = 1;
                    break;
                case SRTF:
                    srtf = 1;
                    break;
                default:
                    printf("nieznany algorytm sortowania -s\n");
                    break;
            }
            no_scheduler = 0;
            break;
        default:
            print_usage(argc, argv);
            abort();
            break;
    }
    if(number_of_jobs > 0 && filename) {
        fprintf(stderr, "nie mozna podac dwoch parametrow -n oraz -i.\n");
        print_usage(argc, argv);
        return 1;
    }
    if(number_of_jobs == 0 && filename == NULL) {
        fprintf(stderr, "musisz podac -n albo -i.\n");
        print_usage(argc, argv);
        return 1;
    }
    if(no_scheduler) {
        fprintf(stderr, "musisz podac algorytm sortowania poprzez parametr -s.\n");
        print_usage(argc, argv);
        return 1;
    }

    if(sjf)
        process_jobs(sjf_comparison, filename, number_of_jobs, verbose);
    if(srtf)
        process_jobs(srtf_comparison, filename, number_of_jobs, verbose);
    return 0;
}
