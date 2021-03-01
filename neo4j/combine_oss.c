# include <stdio.h>
# include <stdlib.h>





void main()
{

int i = 0;
srand(100);

FILE * fPtr; //file1.txt : MDS nodes + OSS nodes + LinkEA relationship
FILE * fPtr1; //file2.txt : MDS nodes + OSS nodes + MDS-OSS relationship


fPtr = fopen("file1.txt", "a"); //w: write fresh on every run; a+ is for append
fPtr1 = fopen("file2.txt", "a");

	if(fPtr == NULL)
    {
        /* File not created hence exit */
        printf("Unable to create file.\n");
        exit(EXIT_FAILURE);
    }

	if(fPtr1 == NULL)
    {
        /* File not created hence exit */
        printf("Unable to create file.\n");
        exit(EXIT_FAILURE);
    }




/*-----------------Creating MDS nodes------------------*/
for (i=1; i<= 10; i++) // assuming 10 iterations for 10 consecutive inodes
{


//creating dummy Extended Attributes for each inode (random values).
int inode_number = rand();
int lma = 100/i;
int fid = rand()%50;


fprintf(fPtr, "CREATE (%d:OSS{inode_number:%d,lma:%d,FID:%d})\n", lma, inode_number, lma, fid);
fprintf(fPtr1, "CREATE (%d:OSS{inode_number:%d,lma:%d,FID:%d})\n", lma, inode_number, lma, fid);

}






fclose(fPtr1);
fclose(fPtr);


}
