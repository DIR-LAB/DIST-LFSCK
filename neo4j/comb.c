# include <stdio.h>
# include <stdlib.h>





void main()
{

int i = 0;
srand(100);

FILE * fPtr; //file1.txt : MDS nodes + OSS nodes + LinkEA relationship
FILE * fPtr1; //file2.txt : MDS nodes + OSS nodes + MDS-OSS relationship


fPtr = fopen("file1.txt", "w"); //w: write fresh on every run; a+ is for append
fPtr1 = fopen("file2.txt", "w");

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
int linkEA = 200/i;
int lovEA = rand()%50;


fprintf(fPtr, "CREATE (%d:MDS_{inode_number:%d,lma:%d,linkEA:%d,lovEA:%d}) \n", lma, inode_number, lma, linkEA, lovEA);
fprintf(fPtr1, "CREATE (%d:MDS{inode_number:%d,lma:%d,linkEA:%d,lovEA:%d}) \n", lma, inode_number, lma, linkEA, lovEA);

}





/*-----------------Creating linkEA-Relationship between the created mds nodes------------------*/

for (i=1; i<= 10; i++) // assuming 10 iterations for 10 consecutive inodes
{

//creating dummy Extended Attributes for each inode (random values).
int inode_number = rand();
int lma = 100/i;
int linkEA = 200/i;
int lovEA = rand()%50;


if(linkEA != 0)
{ fprintf(fPtr, "MATCH (%d:MDS {inode_number:%d,lma:%d,linkEA:%d, lovEA:%d}) \n", lma, inode_number, lma, linkEA, lovEA);
  fprintf(fPtr, "CREATE (%d)-[Child_DIR_of]->(%d)\n", lma, linkEA);
}
	 
}







fclose(fPtr1);
fclose(fPtr);


}
