#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

static int ctrl_c = 0;

float getCpuUsage(int delay, int pid)
{
    char path[50];
    sprintf( path, "/proc/stat");

    float a[4], b[4], load;
    FILE *fp;

    fp = fopen(path, "r");
    fscanf(fp, "%*s %f %f %f %f", &a[0], &a[1], &a[2], &a[3]);
    fclose(fp);

    usleep(delay);

    fp = fopen(path, "r");
    fscanf(fp, "%*s %f %f %f %f", &b[0], &b[1], &b[2], &b[3]);
    fclose(fp);

    load = ( (b[0] + b[1] + b[2]) - (a[0] + a[1] + a[2]) ) / ( (b[0] + b[1] + b[2] + b[3]) - (a[0] + a[1] + a[2] + a[3]) );

    //printf("\rCPU Usage: %0.2f%%  ", load * 100);
    //fflush(stdout);

    return (load * 100);
}

int parseLine(char *line){
	int i = strlen(line);
	while( *line < '0' || *line > '9' ) line++;
	line[i-3] = '\0';
	i = atoi(line);
	return i;
}

int getMemValue(int pid){
	char path[50];
	sprintf( path, "/proc/%d/status", pid);
	FILE *file = fopen(path, "r");
	int result = -1;
	char line[128];

	while(fgets(line, 128, file) != NULL){
		if(strncmp(line, "VmRSS:", 6) == 0){
			result = parseLine(line);
			break;
		}
	}
	fclose(file);
	return result; // Note: the value is in KB
}

void catchSig(int dummy)
{
	ctrl_c = 1;
}

int main(int argc, char *argv[])
{
	unsigned long long int Tms=0;
	int mem;
	float usage;
	int pid = atoi(argv[1]);
	FILE *fp = fopen("data.csv", "w");
	fprintf(fp,"T(ms),mem,cpu\n");
	signal(SIGINT, catchSig);
	while(1)
	{
		if( ctrl_c == 0 )
		{
			usage = getCpuUsage(500000, pid);
			mem = getMemValue(pid);
			fprintf(fp,"%lld,%d,%0.2f\n", Tms, mem, usage);
			printf("pid %d  memory usage: %d KB, CPU utilization: %0.2f\n", pid, mem, usage);
			Tms = Tms + 50;
		}

	}
}
