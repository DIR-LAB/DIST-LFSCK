//reads file2.txt and builds mds-oss relationship

# include <stdio.h>
# include <stdlib.h>





void main()
{


FILE * fPtr1;



//fPtr1 = fopen("sample_read.txt", "r"); //sample_read file
fPtr1 = fopen("file2.txt", "r");


	if(fPtr1 == NULL)
    {
        /* File not created hence exit */
        printf("Unable to create file.\n");
        exit(EXIT_FAILURE);
    }



/*-----------------------------sample_read file-----------------------------*/	

//int b;


char a[20] = "";
char b[75] = "";
/*char c[20] = "";
char d[20] = "";
char e[20] = "";
char f[20] = "";
char g[20] = "";
char h[20] = "";
char i[20] = "";
char j[20] = "";
char k[20] = "";
//char line[15];*/
char line[68];



while (fgets(line, sizeof line, fPtr1) != NULL) {
//puts(line);
//sscanf(line, "%s %s %s %s %s %s %s %s %s %s %s", a,b,c,d,e,f,g,h,i,j,k);
sscanf(line, "%s %s",a,b);
}

printf("1: %s\n",a);
printf("2: %d\n",b);
/*printf("3: %s\n",c);

printf("1: %s\n", d);
printf("2: %d\n",e);
printf("3: %s\n",f);

printf("1: %s\n", g);
printf("2: %d\n",h);
printf("3: %s\n",i);

printf("1: %s\n", j);
printf("2: %d\n",k);
*/
/*---------------------------------------------------------------------------------*/













  fclose(fPtr1);

}
